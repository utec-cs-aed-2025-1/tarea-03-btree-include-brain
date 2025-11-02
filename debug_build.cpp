#include <iostream>
#include <vector>
#include "btree.h"

using namespace std;

// Función helper para imprimir la estructura del árbol
template<typename TK>
void printTree(Node<TK>* node, int level = 0, const string& prefix = "ROOT: ") {
    if (!node) return;

    cout << string(level * 2, ' ') << prefix << "[";
    for (int i = 0; i < node->count; ++i) {
        if (i > 0) cout << ", ";
        cout << node->keys[i];
    }
    cout << "] (count=" << node->count << ", leaf=" << (node->leaf ? "YES" : "NO") << ")" << endl;

    if (!node->leaf) {
        for (int i = 0; i <= node->count; ++i) {
            printTree(node->children[i], level + 1, "Child[" + to_string(i) + "]: ");
        }
    }
}

int main() {
    vector<int> elements = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};

    cout << "Building B-tree with M=4 from vector: ";
    for (int x : elements) cout << x << " ";
    cout << "\n\n";

    BTree<int>* tree = BTree<int>::build_from_ordered_vector(elements, 4);

    cout << "Tree structure:\n";
    printTree(tree->getRoot());

    cout << "\ntoString result: " << tree->toString(",") << endl;
    cout << "Expected:        1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20" << endl;

    cout << "\nTree size: " << tree->size() << " (expected 20)" << endl;
    cout << "Check properties: " << (tree->check_properties() ? "PASS" : "FAIL") << endl;

    delete tree;
    return 0;
}
