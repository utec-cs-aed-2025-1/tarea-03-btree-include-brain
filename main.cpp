#include <iostream>
#include "btree.h"
#include "tester.h"

using namespace std;

int main() {
  //La implementación de este BTree no soporta valores repetidos
  BTree<int>* btree = new BTree<int>(3);
  int numbers[10] = {45, 75, 100, 36, 120, 70, 11, 111, 47, 114};
  for (int i = 0; i < 10; i++) {
    btree->insert(numbers[i]);
  }

  if(! btree->check_properties()) return 0;

  ASSERT(btree->toString(" ") == "11 36 45 47 70 75 100 111 114 120",
         "The function insert is not working");
  ASSERT(btree->height() == 2, "The function height is not working");
  for (int i = 0; i < 10; i=i+2) {
    ASSERT(btree->search(numbers[i]) == true,
           "The function search is not working");
  }
  ASSERT(btree->minKey() == 11, "The function minKey is not working");
  ASSERT(btree->maxKey() == 120, "The function maxKey is not working");

  btree->remove(100);
  btree->remove(111);
  btree->remove(45);

  if(! btree->check_properties()) return 0;

  ASSERT(btree->toString(" ") == "11 36 47 70 75 114 120",
         "The function remove is not working");
  ASSERT(btree->height() == 1, "The function height is not working");

  btree->remove(114);
  btree->insert(40);

  ASSERT(btree->toString(" - ") == "11 - 36 - 40 - 47 - 70 - 75 - 120",
         "The function toString is not working");
  ASSERT(btree->height() == 2, "The function height is not working");

  btree->insert(125);
  btree->insert(115);

  ASSERT(btree->maxKey() == 125, "The function maxKey is not working");
  ASSERT(btree->search(47) == true,  "The function search is not working");

  btree->remove(11);

  ASSERT(btree->search(11) == false,  "The function search is not working");
  ASSERT(btree->minKey() == 36, "The function minKey is not working");
  ASSERT(btree->size() == 8, "The function size is not working");

  btree->clear();
  ASSERT(btree->size() == 0, "The function size is not working");
  ASSERT(btree->height() == 0, "The function height is not working");

  // TODO: agregar pruebas
  std::vector<int> elements = {1,2,3,4,5,6,7,8,9,10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
  BTree<int>* btree2 = BTree<int>::build_from_ordered_vector(elements, 4);
  ASSERT(btree2->toString(",") == "1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20", "The function build_from_ordered_vector is not working");
  if(btree2->check_properties()){
      cout<<"El árbol 2 cumple con las propiedades de un árbol B."<<endl;
  }else{
      cout<<"El árbol 2 no cumple con las propiedades de un árbol B."<<endl;
  }

  delete btree;
  delete btree2;

  // Pruebas adicionales

  // Insertar 100 elementos con M=5
  BTree<int>* btree3 = new BTree<int>(5);
  for (int i = 1; i <= 100; i++) btree3->insert(i);
  ASSERT(btree3->size() == 100, "Error en size");
  ASSERT(btree3->minKey() == 1, "Error en minKey");
  ASSERT(btree3->maxKey() == 100, "Error en maxKey");
  ASSERT(btree3->check_properties(), "No cumple propiedades");

  // rangeSearch
  vector<int> r1 = btree3->rangeSearch(10, 20);
  ASSERT(r1.size() == 11, "Error en rangeSearch");
  vector<int> r2 = btree3->rangeSearch(1, 5);
  ASSERT(r2.size() == 5, "Error en rangeSearch");

  // Eliminar la mitad
  for (int i = 1; i <= 50; i++) btree3->remove(i);
  ASSERT(btree3->size() == 50, "Error despues de eliminar");
  ASSERT(btree3->check_properties(), "No cumple propiedades");
  delete btree3;

  // Insertar descendente
  BTree<int>* btree4 = new BTree<int>(3);
  for (int i = 10; i >= 1; i--) btree4->insert(i);
  ASSERT(btree4->toString(" ") == "1 2 3 4 5 6 7 8 9 10", "Error al insertar descendente");
  delete btree4;

  // Probar M=7
  BTree<int>* btree5 = new BTree<int>(7);
  for (int i = 1; i <= 50; i++) btree5->insert(i);
  ASSERT(btree5->check_properties(), "No cumple propiedades con M=7");
  delete btree5;

  // Clear y reutilizar arbol
  BTree<int>* btree7 = new BTree<int>(4);
  for (int i = 1; i <= 20; i++) btree7->insert(i);
  btree7->clear();
  ASSERT(btree7->size() == 0, "Error en clear");
  for (int i = 100; i <= 105; i++) btree7->insert(i);
  ASSERT(btree7->size() == 6, "Error al reutilizar");
  delete btree7;

  // build_from_ordered_vector
  vector<int> vec;
  for (int i = 1; i <= 50; i++) vec.push_back(i);
  BTree<int>* btree8 = BTree<int>::build_from_ordered_vector(vec, 5);
  ASSERT(btree8->size() == 50, "Error en build_from_ordered_vector");
  ASSERT(btree8->check_properties(), "No cumple propiedades");
  delete btree8;

  // Insertar en arbol vacio
  BTree<int>* btree9 = new BTree<int>(3);
  btree9->insert(1);
  ASSERT(btree9->size() == 1, "Error al insertar en arbol vacio");
  ASSERT(btree9->minKey() == 1, "Error minKey con un elemento");
  ASSERT(btree9->maxKey() == 1, "Error maxKey con un elemento");
  delete btree9;

  // Eliminar unico elemento
  BTree<int>* btree10 = new BTree<int>(3);
  btree10->insert(42);
  btree10->remove(42);
  ASSERT(btree10->size() == 0, "Error al eliminar unico elemento");
  ASSERT(btree10->height() == 0, "Error height despues de eliminar todo");
  delete btree10;

  // Eliminar de arbol vacio y elemento inexistente
  BTree<int>* btree11 = new BTree<int>(3);
  btree11->remove(999);
  ASSERT(btree11->size() == 0, "Error al eliminar de arbol vacio");
  for (int i = 1; i <= 5; i++) btree11->insert(i);
  int sizeAntes = btree11->size();
  btree11->remove(999);
  ASSERT(btree11->size() == sizeAntes, "Error al eliminar elemento inexistente");
  delete btree11;

  // Eliminar minimo y maximo repetidamente
  BTree<int>* btree12 = new BTree<int>(3);
  for (int i = 1; i <= 20; i++) btree12->insert(i);
  for (int i = 1; i <= 10; i++) {
    ASSERT(btree12->minKey() == i, "Error minKey durante eliminacion secuencial");
    btree12->remove(i);
  }
  for (int i = 20; i > 10; i--) {
    ASSERT(btree12->maxKey() == i, "Error maxKey durante eliminacion secuencial");
    btree12->remove(i);
  }
  ASSERT(btree12->size() == 0, "Error despues de eliminar todos");
  delete btree12;

  // Insertar valores negativos
  BTree<int>* btree13 = new BTree<int>(3);
  for (int i = -10; i <= 10; i++) btree13->insert(i);
  ASSERT(btree13->minKey() == -10, "Error con valores negativos minKey");
  ASSERT(btree13->maxKey() == 10, "Error con valores negativos maxKey");
  ASSERT(btree13->size() == 21, "Error size con valores negativos");
  delete btree13;

  // Eliminar en orden aleatorio
  BTree<int>* btree14 = new BTree<int>(4);
  for (int i = 1; i <= 15; i++) btree14->insert(i);
  btree14->remove(8);
  btree14->remove(3);
  btree14->remove(12);
  btree14->remove(1);
  btree14->remove(15);
  ASSERT(btree14->check_properties(), "No cumple propiedades eliminacion aleatoria");
  delete btree14;

  // Insertar en orden, eliminar en orden inverso
  BTree<int>* btree15 = new BTree<int>(5);
  for (int i = 1; i <= 30; i++) btree15->insert(i);
  for (int i = 30; i >= 1; i--) {
    btree15->remove(i);
    if (btree15->size() > 0) {
      ASSERT(btree15->check_properties(), "No cumple propiedades durante eliminacion inversa");
    }
  }
  ASSERT(btree15->size() == 0, "Error al eliminar todo en orden inverso");
  delete btree15;


  // rangeSearch
  BTree<int>* btree16 = new BTree<int>(3);
  for (int i = 1; i <= 20; i++) btree16->insert(i);
  vector<int> range1 = btree16->rangeSearch(5, 10);
  ASSERT(range1.size() == 6 && range1[0] == 5 && range1[5] == 10, "Error rangeSearch basico");
  vector<int> range2 = btree16->rangeSearch(10, 5);
  ASSERT(range2.size() == 6, "Error rangeSearch invertido");
  vector<int> range3 = btree16->rangeSearch(30, 40);
  ASSERT(range3.size() == 0, "Error rangeSearch fuera de rango");
  vector<int> range4 = btree16->rangeSearch(15, 15);
  ASSERT(range4.size() == 1 && range4[0] == 15, "Error rangeSearch un elemento");
  delete btree16;


  // build_vector e insert/remove
  vector<int> vec1 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 20, 30, 45};
  BTree<int>* btree17 = BTree<int>::build_from_ordered_vector(vec1, 5);
  ASSERT(btree17->size() == 13, "Error size despues de build");
  ASSERT(btree17->check_properties(), "No cumple propiedades despues de build");
  btree17->insert(11);
  btree17->remove(5);
  ASSERT(btree17->size() == 13, "Error operaciones despues de build");
  ASSERT(btree17->check_properties(), "No cumple propiedades operaciones despues de build");
  delete btree17;

  // build_vector y rangeSearch
  vector<int> vec2;
  for (int i = 1; i <= 30; i++) vec2.push_back(i);
  BTree<int>* btree18 = BTree<int>::build_from_ordered_vector(vec2, 5);
  vector<int> range5 = btree18->rangeSearch(10, 20);
  ASSERT(range5.size() == 11 && range5[0] == 10 && range5[10] == 20, "Error rangeSearch despues de build");
  delete btree18;

  // Otros casos
  vector<int> vec4;
  BTree<int>* btree21 = BTree<int>::build_from_ordered_vector(vec4, 3);
  ASSERT(btree21->size() == 0, "Error build con vector vacio");
  delete btree21;
  vector<int> vec5 = {42};
  BTree<int>* btree22 = BTree<int>::build_from_ordered_vector(vec5, 3);
  ASSERT(btree22->size() == 1 && btree22->minKey() == 42, "Error build con un elemento");
  delete btree22;

  return 0;
}