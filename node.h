#ifndef NODE_H
#define NODE_H

using namespace std;

template <typename TK>
struct Node {
  // array de keys
  TK* keys;
  // array de punteros a hijos
  Node** children;
  // cantidad de keys
  int count;
  // indicador de nodo hoja
  bool leaf;

  Node() : keys(nullptr), children(nullptr), count(0), leaf(true) {}

  Node(int M) {
    keys = new TK[M];
    children = new Node<TK>*[M + 1];
    for (int i = 0; i < M + 1; ++i)
      children[i] = nullptr;
    count = 0;
    leaf = true;
  }

  void killSelf() {
    if (children) {
      int limit = leaf ? 0 : (count + 1);
      for (int i = 0; i < limit; ++i) {
        delete children[i];
        children[i] = nullptr;
      }
      delete[] children;
      children = nullptr;
    }
    delete[] keys;
    keys = nullptr;
  }

  ~Node(){
    killSelf();
  }
};

#endif
