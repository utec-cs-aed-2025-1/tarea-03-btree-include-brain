#include <iostream>
#include "btree.h"
#include "tester.h"

using namespace std;

int main() {
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

  return 0;
}
