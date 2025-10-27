#ifndef BTree_H
#define BTree_H
#include <iostream>
#include <vector>
#include "node.h"

using namespace std;

template <typename TK>
class BTree {
 private:
  Node<TK>* root;
  int M;  // grado u orden del arbol
  int n;  // total de elementos en el arbol

 public:
  BTree(int _M) : root(nullptr), M(_M) {}

  //indica si se encuentra o no un elemento
  bool search(TK key) {
    return search(this->root, key);
  }

  void insert(TK key) {
    if (!root) {
      root = new Node<TK>(M);
      root->leaf = true;
      root->keys[0] = key;
      root->count = 1;
      for (int i = 0; i <= M; ++i) root->children[i] = nullptr;
      n = 1;
      return;
    }

    bool promoted = false;
    TK up{};
    Node<TK>* right = nullptr;

    bool inserted = insert_rec(root, key, promoted, up, right);
    if (!inserted) return;

    if (promoted) {
      Node<TK>* r = new Node<TK>(M);
      r->leaf = false;
      r->count = 1;
      r->keys[0] = up;
      r->children[0] = root;
      r->children[1] = right;
      for (int i = 2; i <= M; ++i) r->children[i] = nullptr;
      root = r;
    }
    ++n;
  }

  void remove(TK key) {
    if (!root) return;
    bool removed = remove_rec(root, key);
    if (!removed) return;
    --n;
    if (!root->leaf && root->count == 0) {
      Node<TK>* old = root;
      root = root->children[0];
      old->children[0] = nullptr;
      old->killSelf();
      delete old;
    }
    if (root && root->count == 0 && root->leaf) {
      delete root;
      root = nullptr;
    }
  }

  //altura del arbol. Considerar altura 0 para arbol vacio
  int height() {
    if (!root) return 0;
    int cont = 0;
    Node<TK>* temp = root;

    while (!temp->leaf) {
      temp = temp->children[0];
      cont++;
    }
    return cont;
  }

  // recorrido inorder
  string toString(const string& sep) {
    if (!root) return "";
    string out;
    bool first = true;
    toString(root, sep, out, first);
    return out;
  }

  vector<TK> rangeSearch(TK begin, TK end) {
    vector<TK> out;
    if (!root) return out;
    if (end < begin) std::swap(begin, end);
    range_search(root, begin, end, out);
    return out;
  }

  // mínimo valor del árbol
  TK minKey() {
    if (!root) throw runtime_error("The tree is empty");
    Node<TK>* temp = root;

    while (!temp->leaf) {
      temp = temp->children[0];
    }
    return temp->keys[0];
  }

  // máximo valor del árbol
  TK maxKey() {
    if (!root) throw runtime_error("The tree is empty");
    Node<TK>* temp = root;

    while (!temp->leaf) {
      temp = temp->children[temp->count];
    }
    return temp->keys[temp->count - 1];
  }

  // eliminar todos lo elementos del arbol
  void clear() {
    if (root) {
      root->killSelf();
      delete root;
      root = nullptr;
    }
    n = 0;
  }

  // retorna el total de elementos insertados
  int size() const {
    return n;
  }

  // Construya un árbol B a partir de un vector de elementos ordenados
  static BTree<TK>* build_from_ordered_vector(const vector<TK>& elements, int M) {
    BTree<TK>* T = new BTree<TK>(M);
    T->n = static_cast<int>(elements.size());
    if (elements.empty()) return T;

    const int maxK = M - 1;
    vector<Node<TK>*> level;

    // hojas
    for (size_t i = 0; i < elements.size();) {
      Node<TK>* leaf = new Node<TK>(M);
      leaf->leaf = true;
      int take = std::min<int>(maxK, static_cast<int>(elements.size() - i));
      for (int j = 0; j < take; ++j) leaf->keys[j] = elements[i + j];
      leaf->count = take;
      for (int c = 0; c <= leaf->count; ++c) leaf->children[c] = nullptr;
      level.push_back(leaf);
      i += take;
    }

    if (level.size() == 1) {
      T->root = level[0];
      return T;
    }

    // niveles internos
    while (level.size() > 1) {
      vector<Node<TK>*> next;

      int nchildren = static_cast<int>(level.size());
      int p = (nchildren + M - 1) / M;
      int minC = (M + 1) / 2;
      while (p > 1 && p * minC > nchildren) --p;
      int base = nchildren / p;
      int rem = nchildren % p;

      int idx = 0;
      for (int pi = 0; pi < p; ++pi) {
        int cnum = base + (pi < rem ? 1 : 0);
        if (cnum < minC && p > 1) cnum = minC;
        if (cnum > M) cnum = M;

        Node<TK>* parent = new Node<TK>(M);
        parent->leaf = false;
        parent->count = cnum - 1;

        for (int c = 0; c < cnum; ++c) {
          parent->children[c] = level[idx + c];
        }
        for (int c = cnum; c <= M; ++c) parent->children[c] = nullptr;

        for (int k = 0; k < parent->count; ++k) {
          parent->keys[k] = parent->children[k + 1]->keys[0];
        }
        next.push_back(parent);
        idx += cnum;
      }

      if (next.size() == 1 && next[0]->count == 0 && next[0]->children[0]) {
        Node<TK>* onlyChild = next[0]->children[0];
        delete next[0];
        next[0] = onlyChild;
      }

      level.swap(next);
    }

    T->root = level[0];
    return T;
  }

  // Verifique las propiedades de un árbol B
  bool check_properties() {
    if (!root) return true;
    int leaf_level = -1;
    bool has_prev = false;
    TK prev{};
    return check(root, true, 1, leaf_level, has_prev, prev);
  }

  ~BTree() {
    clear();
  }

 private:
  // remove
  int find_index(Node<TK>* x, const TK& k) {
    int i = 0;
    while (i < x->count && x->keys[i] < k) ++i;
    return i;
  }

  TK get_pred(Node<TK>* x, int idx) {
    Node<TK>* cur = x->children[idx];
    while (!cur->leaf) cur = cur->children[cur->count];
    return cur->keys[cur->count - 1];
  }

  TK get_succ(Node<TK>* x, int idx) {
    Node<TK>* cur = x->children[idx + 1];
    while (!cur->leaf) cur = cur->children[0];
    return cur->keys[0];
  }

  void borrow_from_prev(Node<TK>* x, int idx) {
    Node<TK>* child = x->children[idx];
    Node<TK>* sib = x->children[idx - 1];

    for (int i = child->count; i > 0; --i) child->keys[i] = child->keys[i - 1];
    if (!child->leaf) {
      for (int i = child->count + 1; i > 0; --i) child->children[i] = child->children[i - 1];
    }
    child->keys[0] = x->keys[idx - 1];
    if (!child->leaf) child->children[0] = sib->children[sib->count];
    child->count += 1;

    x->keys[idx - 1] = sib->keys[sib->count - 1];
    if (!sib->leaf) sib->children[sib->count] = nullptr;
    sib->count -= 1;
  }

  void borrow_from_next(Node<TK>* x, int idx) {
    Node<TK>* child = x->children[idx];
    Node<TK>* sib = x->children[idx + 1];

    child->keys[child->count] = x->keys[idx];
    if (!child->leaf) child->children[child->count + 1] = sib->children[0];
    child->count += 1;

    x->keys[idx] = sib->keys[0];

    for (int i = 1; i < sib->count; ++i) sib->keys[i - 1] = sib->keys[i];
    if (!sib->leaf) {
      for (int i = 1; i <= sib->count; ++i) sib->children[i - 1] = sib->children[i];
      sib->children[sib->count] = nullptr;
    }
    sib->count -= 1;
  }

  void merge_children(Node<TK>* x, int idx) {
    Node<TK>* left = x->children[idx];
    Node<TK>* right = x->children[idx + 1];

    left->keys[left->count] = x->keys[idx];
    for (int i = 0; i < right->count; ++i) left->keys[left->count + 1 + i] = right->keys[i];
    if (!left->leaf) {
      for (int i = 0; i <= right->count; ++i) left->children[left->count + 1 + i] = right->children[i];
    }
    left->count += 1 + right->count;

    for (int i = idx + 1; i < x->count; ++i) x->keys[i - 1] = x->keys[i];
    for (int i = idx + 2; i <= x->count; ++i) x->children[i - 1] = x->children[i];
    x->children[x->count] = nullptr;
    x->count -= 1;

    right->killSelf();
    delete right;
  }

  void fill_child(Node<TK>* x, int idx) {
    int t = (M + 1) / 2;
    if (idx > 0 && x->children[idx - 1]->count >= t) {
      borrow_from_prev(x, idx);
    } else if (idx < x->count && x->children[idx + 1]->count >= t) {
      borrow_from_next(x, idx);
    } else {
      if (idx < x->count)
        merge_children(x, idx);
      else
        merge_children(x, idx - 1);
    }
  }

  bool remove_rec(Node<TK>* x, const TK& key) {
    int idx = find_index(x, key);

    if (idx < x->count && eq(x->keys[idx], key)) {
      if (x->leaf) {
        for (int i = idx + 1; i < x->count; ++i) x->keys[i - 1] = x->keys[i];
        x->count -= 1;
        return true;
      } else {
        int t = (M + 1) / 2;
        Node<TK>* left = x->children[idx];
        Node<TK>* right = x->children[idx + 1];

        if (left->count >= t) {
          TK pred = get_pred(x, idx);
          x->keys[idx] = pred;
          return remove_rec(left, pred);
        } else if (right->count >= t) {
          TK succ = get_succ(x, idx);
          x->keys[idx] = succ;
          return remove_rec(right, succ);
        } else {
          merge_children(x, idx);
          return remove_rec(x->children[idx], key);
        }
      }
    } else {
      if (x->leaf) return false;

      bool lastChild = (idx == x->count);
      int t = (M + 1) / 2;

      if (x->children[idx]->count < t) fill_child(x, idx);

      if (lastChild && idx > x->count) idx = x->count;
      return remove_rec(x->children[idx], key);
    }
  }

  // insert
  static bool eq(const TK& a, const TK& b) { return !(a < b) && !(b < a); }

  void insert_key_at(Node<TK>* x, int pos, const TK& k, Node<TK>* rightChild) {
    for (int i = x->count; i > pos; --i) x->keys[i] = x->keys[i - 1];
    x->keys[pos] = k;
    if (!x->leaf) {
      for (int i = x->count + 1; i > pos + 1; --i) x->children[i] = x->children[i - 1];
      x->children[pos + 1] = rightChild;
    }
    x->count += 1;
  }

  void split_node(Node<TK>* x, bool& promoted, TK& up, Node<TK>*& right) {
    int mid = M / 2;  // clave de subida
    up = x->keys[mid];

    right = new Node<TK>(M);
    right->leaf = x->leaf;

    int rcount = M - mid - 1;  // claves que van al nodo derecho
    for (int i = 0; i < rcount; ++i) right->keys[i] = x->keys[mid + 1 + i];
    right->count = rcount;

    if (!x->leaf) {
      for (int i = 0; i <= rcount; ++i) right->children[i] = x->children[mid + 1 + i];
      for (int i = rcount + 1; i <= M; ++i) right->children[i] = nullptr;
    } else {
      for (int i = 0; i <= M; ++i) right->children[i] = nullptr;
    }

    x->count = mid;
    promoted = true;
  }

  bool insert_rec(Node<TK>* x, const TK& key, bool& promoted, TK& up, Node<TK>*& right) {
    int i = 0;
    while (i < x->count && x->keys[i] < key) ++i;
    if (i < x->count && eq(x->keys[i], key)) return false;

    if (x->leaf) {
      for (int j = x->count; j > i; --j) x->keys[j] = x->keys[j - 1];
      x->keys[i] = key;
      x->count += 1;
    } else {
      bool c_prom = false;
      TK c_up{};
      Node<TK>* c_right = nullptr;

      if (!insert_rec(x->children[i], key, c_prom, c_up, c_right)) return false;
      if (c_prom) insert_key_at(x, i, c_up, c_right);
    }

    if (x->count == M) {
      split_node(x, promoted, up, right);
    } else {
      promoted = false;
      right = nullptr;
    }
    return true;
  }

  // helpers
  void range_search(Node<TK>* x, const TK& a, const TK& b, vector<TK>& out) {
    if (!x) return;

    if (x->leaf) {
      for (int i = 0; i < x->count; ++i) {
        if (x->keys[i] < a) continue;
        if (b < x->keys[i]) break;
        out.push_back(x->keys[i]);
      }
      return;
    }

    for (int i = 0; i < x->count; ++i) {
      range_search(x->children[i], a, b, out);

      if (!(x->keys[i] < a) && !(b < x->keys[i]))
        out.push_back(x->keys[i]);
      else if (b < x->keys[i])
        return;
    }
    range_search(x->children[x->count], a, b, out);
  }

  bool check(Node<TK>* x,
             bool is_root,
             int depth,
             int& leaf_level,
             bool& has_prev,
             TK& prev) {
    const int max_keys = M - 1;
    const int min_keys_non_root = (M + 1) / 2 - 1;

    if (x->count < 0 || x->count > max_keys) return false;
    if (is_root) {
      if (!x->leaf && x->count < 1) return false;
    } else {
      if (x->count < min_keys_non_root) return false;
    }

    for (int i = 1; i < x->count; ++i)
      if (!(x->keys[i - 1] < x->keys[i])) return false;

    if (x->leaf) {
      for (int i = 0; i <= x->count; ++i)
        if (x->children[i] != nullptr) return false;
      if (leaf_level == -1)
        leaf_level = depth;
      else if (leaf_level != depth)
        return false;
      for (int i = 0; i < x->count; ++i) {
        if (has_prev && !(prev < x->keys[i])) return false;
        prev = x->keys[i];
        has_prev = true;
      }
      return true;
    }

    for (int i = 0; i <= x->count; ++i)
      if (x->children[i] == nullptr) return false;

    if (!check(x->children[0], false, depth + 1, leaf_level, has_prev, prev))
      return false;

    for (int i = 0; i < x->count; ++i) {
      if (has_prev && !(prev < x->keys[i])) return false;
      prev = x->keys[i];
      has_prev = true;
      if (!check(x->children[i + 1], false, depth + 1, leaf_level, has_prev, prev))
        return false;
    }
    return true;
  }

  bool search(Node<TK>* nodo, TK key) {
    if (!nodo) return false;  // árbol vacío

    int i = 0;
    while (i < nodo->count && key > nodo->keys[i]) {
      i++;
    }
    if (i < nodo->count && key == nodo->keys[i]) {
      return true;
    }
    if (nodo->leaf) {
      return false;
    }
    return search(nodo->children[i], key);
  }

  void toString(Node<TK>* nodo, const string& sep, string& out, bool& first) {
    if (!nodo) return;

    for (int i = 0; i < nodo->count; ++i) {
      if (!nodo->leaf) toString(nodo->children[i], sep, out, first);

      if (!first) out += sep;
      out += std::to_string(nodo->keys[i]);
      first = false;
    }
    if (!nodo->leaf) toString(nodo->children[nodo->count], sep, out, first);
  }

  void range_search(Node<TK>* nodo, TK begin, TK end, vector<int>& ans) {
    for (int i = 0; i < nodo->count; ++i) {
      if (!nodo->leaf) range_search(nodo->children[i], begin, end, ans);
      if (nodo->keys[i] >= begin && nodo->keys[i] <= end)
        ans.push_back(nodo->keys[i]);
    }
    if (!nodo->leaf) range_search(nodo->children[nodo->count], begin, end, ans);
  }
};

#endif
