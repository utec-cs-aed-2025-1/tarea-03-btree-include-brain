#ifndef BTree_H
#define BTree_H
#include <iostream>
#include <vector>
#include <climits>
#include "node.h"
using namespace std;

template <typename TK>
class BTree {
  //La implementación de este BTree no soporta valores repetidos
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

  int size() const { return n; }

  static BTree* build_from_ordered_vector(const vector<TK>& elements, int M) {
  if (M < 3) throw std::invalid_argument("M debe ser al menos 3");
  BTree* tree = new BTree(M);
  if (elements.empty()) return tree;
    for (size_t i = 1; i < elements.size(); ++i)
    if (!(elements[i - 1] < elements[i]))
      throw std::invalid_argument("Los elementos deben estar estrictamente ordenados y sin duplicados");

  long long N = static_cast<long long>(elements.size());
  int max_h_est = 64;
  vector<long long> minK, maxK;
  compute_minmax_per_height(M, max_h_est, minK, maxK);

  int h = choose_height_for_root(N, M, minK, maxK);
  if (h == -1) { delete tree; throw runtime_error("No se pudo determinar altura adecuada"); }

  size_t pos = 0;
  Node<TK>* root = build_subtree_from_sorted(elements, M, h, N, pos, minK, maxK, true);
  if (!root) { delete tree; throw runtime_error("Construcción fallida"); }
  tree->root = root;
  tree->n = static_cast<int>(N);
  return tree;
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

  //metodos de apoyo para la construccion de un arbol B desde un vector ordenado

static void compute_minmax_per_height(int M, int max_h, vector<long long>& minK, vector<long long>& maxK) {
  int maxKeys = M - 1;
  int minKeys = (M + 1) / 2 - 1;
  int minChildren = minKeys + 1;
  int maxChildren = M;

  minK.assign(max_h + 1, 0);
  maxK.assign(max_h + 1, 0);

  // altura 0 = hoja
  minK[0] = (minKeys > 0) ? minKeys : 1;
  maxK[0] = maxKeys;

  for (int h = 1; h <= max_h; ++h) {
    long long min_sub = (long long)minChildren * minK[h - 1] + (minChildren - 1);
    long long max_sub = (long long)maxChildren * maxK[h - 1] + (maxChildren - 1);
    minK[h] = (min_sub < 0) ? LLONG_MAX : min_sub;
    maxK[h] = (max_sub < 0) ? (1LL<<60) : std::min(max_sub, (1LL<<60));
  }
}

//  altura minima posible para la raíz que admite N claves
static int choose_height_for_root(long long N, int M, const vector<long long>& minK, const vector<long long>& maxK) {
  int max_h = (int)minK.size() - 1;
  for (int h = 0; h <= max_h; ++h) {
    long long min_root, max_root;
    if (h == 0) {
      min_root = 1;
      max_root = maxK[0];
    } else {
      min_root = 2 * minK[h - 1] + 1; // raíz con al menos 2 hijos
      max_root = maxK[h];
    }
    if (N >= min_root && N <= max_root) return h;
  }
  return -1;
}

// Numero de hijos k válido para un nodo interno
static int choose_k_for_internal(long long target_n, long long minPer, long long maxPer, int k_min, int k_max) {
  for (int k = k_min; k <= k_max; ++k) {
    long long min_allowed = (long long)k * minPer + (k - 1);
    long long max_allowed = (long long)k * maxPer + (k - 1);
    if (target_n >= min_allowed && target_n <= max_allowed) return k;
  }
  // fallback razonable (calcula kb1 y ajusta al rango)
  long long numero = target_n + 1;
  long long kb1 = (numero + (maxPer + 1) - 1) / (maxPer + 1); // techo((target_n+1)/(maxPer+1))
  long long kb = std::max((long long)k_min, std::min((long long)k_max, kb1));
  if (kb < k_min) kb = k_min;
  if (kb > k_max) kb = k_max;
  return (int)kb;
}

// Repartir el child_total entre k hijos respetando limites
static vector<long long> distribute_children_sizes(long long child_total, int k, long long minPer, long long maxPer) {
  vector<long long> sizes(k, 0);
  // asignar mínimos
  for (int i = 0; i < k; ++i) sizes[i] = minPer;
  long long remaining = child_total - (long long)k * minPer;
  if (remaining < 0) remaining = 0;

  // distribuir en hasta dos pasadas equilibradas (no sobrepasar maxPer)
  for (int pass = 0; pass < 2 && remaining > 0; ++pass) {
    for (int i = 0; i < k && remaining > 0; ++i) {
      long long cap = maxPer - sizes[i];
      if (cap <= 0) continue;
      long long give = std::min(cap, remaining);
      sizes[i] += give;
      remaining -= give;
    }
  }

  // Si sobre, repartir unitariamente
  for (int i = 0; remaining > 0 && i < k; ++i) {
    long long cap = maxPer - sizes[i];
    if (cap <= 0) continue;
    sizes[i]++; remaining--;
  }

  // Por si aun sobra
  if (remaining > 0) {
    long long base = child_total / k;
    long long rem2 = child_total % k;
    for (int i = 0; i < k; ++i) sizes[i] = base + (i < rem2 ? 1 : 0);
  }

  return sizes;
}

// ajuste local: si algún hijo quedó por debajo de minPer, intentar tomar de vecinos
static void adjust_child_sizes(vector<long long>& sizes, long long minPer) {
  int k = (int)sizes.size();
  for (int i = 0; i < k; ++i) {
    if (sizes[i] < minPer) {
      long long need = minPer - sizes[i];
      // tomar de la derecha
      for (int j = i + 1; j < k && need > 0; ++j) {
        long long can = sizes[j] - minPer;
        long long take = std::min(can, need);
        if (take > 0) { sizes[j] -= take; sizes[i] += take; need -= take; }
      }
      // tomar de la izquierda
      for (int j = i - 1; j >= 0 && need > 0; --j) {
        long long can = sizes[j] - minPer;
        long long take = std::min(can, need);
        if (take > 0) { sizes[j] -= take; sizes[i] += take; need -= take; }
      }
    }
  }
}

// Construye un subárbol con target_n llaves, consumiendo desde una posicion global pos
static Node<TK>* build_subtree_from_sorted(const vector<TK>& elements, int M, int height, long long target_n,
size_t& pos, const vector<long long>& minK, const vector<long long>& maxK, bool is_root) {
  if (target_n <= 0) return nullptr;

  int minKeys = (M + 1) / 2 - 1;
  int minChildren = minKeys + 1;
  int maxChildren = M;

  // caso hoja
  if (height == 0) {
    Node<TK>* leaf = new Node<TK>(M);
    leaf->leaf = true;
    leaf->count = static_cast<int>(target_n);
    for (int i = 0; i < leaf->count; ++i) leaf->keys[i] = elements[pos++];
    return leaf;
  }

  long long minPer = minK[height - 1];
  long long maxPer = maxK[height - 1];

  int k_min = is_root ? 2 : minChildren;
  int k_max = maxChildren;

  int k = choose_k_for_internal(target_n, minPer, maxPer, k_min, k_max);
  long long child_total = target_n - (k - 1);

  // repartir child_total entre k hijos
  vector<long long> child_sizes = distribute_children_sizes(child_total, k, minPer, maxPer);
  adjust_child_sizes(child_sizes, minPer);

  // construir padre y sus hijos
  Node<TK>* parent = new Node<TK>(M);
  parent->leaf = false;
  int key_idx = 0;
  for (int i = 0; i < k; ++i) {
    Node<TK>* child = build_subtree_from_sorted(elements, M, height - 1, child_sizes[i], pos, minK, maxK, false);
    parent->children[i] = child;
    if (i < k - 1) {
      parent->keys[key_idx++] = elements[pos++]; // "separador" tomado de la secuencia
    }
  }
  parent->count = key_idx;
  return parent;
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
