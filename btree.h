#ifndef BTree_H
#define BTree_H
#include <iostream>
#include <vector>
#include <stdexcept>
#include "node.h"

using namespace std;

template <typename TK>
class BTree {
 private:
  Node<TK>* root;
  int M;  // grado u orden del arbol
  int n;  // total de elementos en el arbol

 public:
  BTree(int _M) : root(nullptr), M(_M), n(0){}

  //indica si se encuentra o no un elemento
  bool search(TK key) {
    return search_rec(this->root, key);
  }

  Node<TK>* insert(TK key){
    //caso1: arbol sin raiz
    if(!root){
      root = new Node<TK>(M);
      root->keys[0] = key;
      root->count = 1;
      root->leaf = true;
      n = 1;
      return root;
    }

    //caso2: insertar normalmente
    n++;
    TK promoted_key;
    Node<TK>* new_child = insert_rec(root, key, promoted_key);
    if(!new_child) return root;

    //caso3: split en la raiz
    Node<TK>* new_root = new Node<TK>(M);
    new_root->leaf = false;
    new_root->keys[0] = promoted_key;
    new_root->count = 1;
    new_root->children[0] = root;
    new_root->children[1] = new_child;
    root = new_root;
    return root;
  }

  void remove(TK key) {
    if (!root) return;
    bool found = remove_rec(root, key);
    if (found) {
      n--;
      // Si la raíz quedó vacía pero tiene un hijo, promoverlo
      if (root->count == 0 && !root->leaf) {
        Node<TK>* old_root = root;
        root = root->children[0];
        old_root->children[0] = nullptr;
        delete old_root;
      }
      // Si el árbol quedó completamente vacío
      if (root && root->count == 0 && root->leaf) {
        delete root;
        root = nullptr;
      }
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
    range_search_rec(root, begin, end, out);
    return out;
  }

  // mínimo valor del árbol
  TK minKey() {
    if (!root) throw runtime_error("El árbol está vacío");
    Node<TK>* temp = root;

    while (!temp->leaf) {
      temp = temp->children[0];
    }
    return temp->keys[0];
  }

  // máximo valor del árbol
  TK maxKey() {
    if (!root) throw runtime_error("El árbol está vacío");
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
  int size() const { return n; }

  // Construya un árbol B a partir de un vector de elementos ordenados
  static BTree* build_from_ordered_vector(const std::vector<TK>& elements, int M) {
    if (M < 3) throw std::invalid_argument("M debe ser como minimo 3");
    BTree* tree = new BTree(M);
    if (elements.empty()) return tree;

    for (size_t i = 1; i < elements.size(); ++i)
      if (!(elements[i-1] < elements[i]))
        throw std::invalid_argument("Los elementos deben estar ordenados");

        tree->root = build_recursive(elements, 0, elements.size(), M, /*isRoot=*/true);
    return tree;
  }
    // construye nodo con rango elements[start,end)
    static Node<TK>* build_recursive(const std::vector<TK>& elements, size_t start, size_t end, int M, bool isRoot) {
        size_t n = end - start; //calcular el tamaño
        if (n == 0) return nullptr;

        Node<TK>* node = new Node<TK>(M);
    //calcular propiedades de llaves
        int maxKeys = M - 1;
        int minKeys = (M + 1) / 2 - 1;

        // Caso base: es una hoja
        if (n <= static_cast<size_t>(maxKeys)) {
            node->leaf = true;
            node->count = static_cast<int>(n);
            for (size_t i = 0; i < n; ++i) node->keys[i] = elements[start + i];
            return node;
        }
        //Nodos internos

        node->leaf = false;

        // child_total = n - (k-1) (k:numero de hijos)
        int minChildSize = isRoot ? 1 : minKeys;
        int chosen_k = 0;

        int max_k_possible = std::min(M,(int)(n));

        for (int k = max_k_possible; k >= 2; --k) {
            int parent_keys = k - 1;
            int child_total = static_cast<int>(n) - parent_keys;
            if (child_total < k * minChildSize) continue;  // no alcanza mínimo por hijo
            if (child_total > k * maxKeys) continue;       // excede máximo por hijo
            chosen_k = k;
            break;
        }

        // Por defecto se toma k como binario
        if (chosen_k == 0) chosen_k = 2;

        int k = chosen_k;
        int parent_keys = k - 1;
        int child_total = (int)(n) - parent_keys;

        vector<size_t> child_sizes(k, child_total / k);
        //sobrantes
        int rem = child_total % k;
        for (int i = 0; i < rem; ++i) child_sizes[i]++;

        size_t idx = start;
        int key_idx = 0;
        for (int i = 0; i < k; ++i) {
            size_t child_start = idx;
            size_t child_end = idx + child_sizes[i];

            // subárbol del hijo
            Node<TK>* child = build_recursive(elements, child_start, child_end, M, false);
            node->children[i] = child;

            //asignar llaves del padre
            if (i < k - 1) {
                node->keys[key_idx++] = elements[child_end];
                // omitimos esa llave usada
                idx = child_end + 1;
            } else {
                idx = child_end;
            }
        }

        node->count = key_idx; //n
        return node;
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
  // metodos para la insercion
  Node<TK>* split(Node<TK>* node, TK& promoted_key, bool is_leaf) {
    int mid_idx = M / 2;
    promoted_key = node->keys[mid_idx];

    // partir a la mitad el nodo actual
    Node<TK>* right = new Node<TK>(M);
    right->leaf = is_leaf;
    int j = 0;
    for(int i = mid_idx + 1; i < node->count; i++){
      right->keys[j++] = node->keys[i];
    }
    right->count = j;

    // organizar los hijos si no es hoja
    if(!is_leaf) {
      for(int i = mid_idx + 1, k = 0; i <= node->count; i++, k++){
        right->children[k] = node->children[i];
        node->children[i] = nullptr;
      }
    }
    node->count = mid_idx;
    return right; // se retorna la key que sube y el nodo partido
  }

  Node<TK>* insert_rec(Node<TK>* node, TK key, TK& promoted_key){
    //busqueda binaria del indice > key
    int left = 0, right = node->count - 1;
    int child_idx = 0;
    while(left <= right){
      int mid = left + (right - left) / 2;
      if(node->keys[mid] == key) {
          n--; // llave duplicada, sin insercion
          return nullptr;
      }
      if(node->keys[mid] < key) {
          left = mid + 1;
          child_idx = left;
      } else {
          right = mid - 1;
      }
    }

    if(node->leaf){
      // insertar en hoja desplazando las keys
      int insert_pos = node->count;
      while(insert_pos > 0 && node->keys[insert_pos - 1] > key){
          node->keys[insert_pos] = node->keys[insert_pos - 1];
          insert_pos--;
      }
      node->keys[insert_pos] = key;
      node->count++;

      //split en hoja
      if(node->count == M){
          return split(node, promoted_key, true);
      }
      return nullptr;
    }
    else {
      // nodo interno = descender recursivamente
      TK child_promoted_key;
      Node<TK>* new_child = insert_rec(node->children[child_idx], key, child_promoted_key);
      if(!new_child) return nullptr; // sin split

      // hubo split, insertar la clave promovida en padre
      int insert_pos = node->count;
      while(insert_pos > child_idx && node->keys[insert_pos - 1] > child_promoted_key){
          node->keys[insert_pos] = node->keys[insert_pos - 1];
          node->children[insert_pos + 1] = node->children[insert_pos];
          insert_pos--;
      }
      node->keys[insert_pos] = child_promoted_key;
      node->children[insert_pos + 1] = new_child;
      node->count++;

      // split en padre
      if(node->count == M){
          return split(node, promoted_key, false);
      }
      return nullptr;
    }
  }

  // metodos para la eliminacion
  bool remove_rec(Node<TK>* node, TK key) {
    if (!node) return false;
    
    int min_keys = (M + 1) / 2 - 1;

    // busqueda binaria para encontrar la posición
    int left = 0, right = node->count - 1;
    int pos = -1;
    int child_idx = 0;
    
    while (left <= right) {
      int mid = left + (right - left) / 2;
      if (node->keys[mid] == key) {
        pos = mid;
        break;
      }
      if (node->keys[mid] < key) {
        left = mid + 1;
        child_idx = left;
      } else {
        right = mid - 1;
        child_idx = mid;
      }
    }
    
    //caso3: key en nodo interno
    if (pos != -1 && !node->leaf) {
      // reemplazar con sucesor
      TK successor = get_successor(node->children[pos + 1]);
      node->keys[pos] = successor;
      return remove_rec(node->children[pos + 1], successor); // eliminar sucesor
    }
    
    // CASO 0, 1, 2: key en nodo hoja o descender
    if (node->leaf) {
      if (pos == -1) return false; // Key no encontrada
      
      // eliminar la key desplazando elementos
      for (int i = pos; i < node->count - 1; i++) {
        node->keys[i] = node->keys[i + 1];
      }
      node->count--;
      return true;
    }
    
    //nodo interno: descender al hijo apropiado
    Node<TK>* child = node->children[child_idx];
    bool found = remove_rec(child, key);
    
    if (!found) return false;
    
    // si el hijo quedó con menos del mínimo
    if (child->count < min_keys) {
      fix_child(node, child_idx);
    }
    
    return true;
  }

  TK get_successor(Node<TK>* node) {
    while (!node->leaf) {
      node = node->children[0];
    }
    return node->keys[0];
  }
  
  // arreglar un hijo que quedó con menos del mínimo
  void fix_child(Node<TK>* parent, int child_idx) {
    int min_keys = (M + 1) / 2 - 1;
    
    //caso1: intentar borrow de hermano izquierdo
    if (child_idx > 0 && parent->children[child_idx - 1]->count > min_keys) {
      borrow_from_left(parent, child_idx);
      return;
    }
    
    //  caso2: intentar borrow de hermano derecho
    if (child_idx < parent->count && parent->children[child_idx + 1]->count > min_keys) {
      borrow_from_right(parent, child_idx);
      return;
    }
    
    //caso3 : merge con hermano
    if (child_idx > 0) {
      merge_with_left(parent, child_idx);
    } else {
      merge_with_right(parent, child_idx);
    }
  }
  
  // Rotar: tomar una key del hermano izquierdo
  void borrow_from_left(Node<TK>* parent, int child_idx) {
    Node<TK>* child = parent->children[child_idx];
    Node<TK>* left_sibling = parent->children[child_idx - 1];
    for (int i = child->count; i > 0; i--) {
      child->keys[i] = child->keys[i - 1];
    }
    
    // Desplazar children si no es hoja
    if (!child->leaf) {
      for (int i = child->count + 1; i > 0; i--) {
        child->children[i] = child->children[i - 1];
      }
    }

    child->keys[0] = parent->keys[child_idx - 1];
    child->count++;
    parent->keys[child_idx - 1] = left_sibling->keys[left_sibling->count - 1];
    
    //moover el último hijo del hermano al child (si no es hoja)
    if (!child->leaf) {
      child->children[0] = left_sibling->children[left_sibling->count];
      left_sibling->children[left_sibling->count] = nullptr;
    }
    left_sibling->count--;
  }

  void borrow_from_right(Node<TK>* parent, int child_idx) {
    Node<TK>* child = parent->children[child_idx];
    Node<TK>* right_sibling = parent->children[child_idx + 1];
    child->keys[child->count] = parent->keys[child_idx];
    child->count++;
    parent->keys[child_idx] = right_sibling->keys[0];
    if (!child->leaf) {
      child->children[child->count] = right_sibling->children[0];
    }
    for (int i = 0; i < right_sibling->count - 1; i++) {
      right_sibling->keys[i] = right_sibling->keys[i + 1];
    }
    if (!right_sibling->leaf) {
      for (int i = 0; i < right_sibling->count; i++) {
        right_sibling->children[i] = right_sibling->children[i + 1];
      }
      right_sibling->children[right_sibling->count] = nullptr;
    }
    right_sibling->count--;
  }
  
  // fusinar child con su hermano izquierdo
  void merge_with_left(Node<TK>* parent, int child_idx) {
    Node<TK>* child = parent->children[child_idx];
    Node<TK>* left_sibling = parent->children[child_idx - 1];
    
    // bajar la key del padre al hermano izquierdo
    left_sibling->keys[left_sibling->count] = parent->keys[child_idx - 1];
    left_sibling->count++;
    
    // copiar todas las keys del child al hermano izquierdo
    for (int i = 0; i < child->count; i++) {
      left_sibling->keys[left_sibling->count] = child->keys[i];
      left_sibling->count++;
    }
    
    //copiar los children si no es hoja
    if (!child->leaf) {
      for (int i = 0; i <= child->count; i++) {
        left_sibling->children[left_sibling->count - child->count + i] = child->children[i];
        child->children[i] = nullptr;
      }
    }
    
    //eliminar key del padre y ajustar children
    for (int i = child_idx - 1; i < parent->count - 1; i++) {
      parent->keys[i] = parent->keys[i + 1];
    }
    for (int i = child_idx; i < parent->count; i++) {
      parent->children[i] = parent->children[i + 1];
    }
    parent->children[parent->count] = nullptr;
    parent->count--;
    delete child;
  }

  void merge_with_right(Node<TK>* parent, int child_idx) {
    Node<TK>* child = parent->children[child_idx];
    Node<TK>* right_sibling = parent->children[child_idx + 1];

    child->keys[child->count] = parent->keys[child_idx];
    child->count++;

    for (int i = 0; i < right_sibling->count; i++) {
      child->keys[child->count] = right_sibling->keys[i];
      child->count++;
    }

    if (!child->leaf) {
      for (int i = 0; i <= right_sibling->count; i++) {
        child->children[child->count - right_sibling->count + i] = right_sibling->children[i];
        right_sibling->children[i] = nullptr;
      }
    }

    for (int i = child_idx; i < parent->count - 1; i++) {
      parent->keys[i] = parent->keys[i + 1];
    }
    for (int i = child_idx + 1; i < parent->count; i++) {
      parent->children[i] = parent->children[i + 1];
    }
    parent->children[parent->count] = nullptr;
    parent->count--;
    delete right_sibling;
  }

  // helpers
  void range_search_rec(Node<TK>* x, const TK& a, const TK& b, vector<TK>& out) {
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
      range_search_rec(x->children[i], a, b, out);

      if (!(x->keys[i] < a) && !(b < x->keys[i]))
        out.push_back(x->keys[i]);
      else if (b < x->keys[i])
        return;
    }
    range_search_rec(x->children[x->count], a, b, out);
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

  bool search_rec(Node<TK>* nodo, TK key) {
    if (!nodo) return false;
    int left = 0, right = nodo->count - 1;
    int pos = nodo->count;
    while (left <= right) {
      int mid = left + (right - left) / 2;
      if (nodo->keys[mid] == key) {
        return true;
      }
      if (key < nodo->keys[mid]) {
        pos = mid;
        right = mid - 1;
      } else {
        left = mid + 1;
        pos = left;
      }
    }
    if (nodo->leaf) {
      return false;
    }
    return search_rec(nodo->children[pos], key);
  }

  void toString(Node<TK>* nodo, const string& sep, string& out, bool& first) {
    if (!nodo) return;

    // Si es hoja, simplemente imprime todas las claves en orden
    if (nodo->leaf) {
      for (int i = 0; i < nodo->count; ++i) {
        if (!first) out += sep;
        out += std::to_string(nodo->keys[i]);
        first = false;
      }
      return;
    }

    // Nodo interno (inorder)
    for (int i = 0; i < nodo->count; ++i) {
      toString(nodo->children[i], sep, out, first);

      if (!first) out += sep;
      out += std::to_string(nodo->keys[i]);
      first = false;
    }

    toString(nodo->children[nodo->count], sep, out, first);
  }
};

#endif
