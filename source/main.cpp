#include "algo/binary_search_tree.hpp"
#include "algo/interval_tree.hpp"
#include "test/interval_tree.hpp"

#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <random>
#include <ctime>
#include <cassert>

int printBST(const algo::BinarySearchTree<int, int>& bst) {
    int count = 0;
    bst.inorder([&](const int& key, const int& value) {
        std::cout << key << " ";
        count++;
    });
    std::cout << std::endl;
    return count;
}

bool testBST() {
    algo::BinarySearchTree<int, int> bst;

    int sz = 8;
    std::vector<int> array1(sz);
    std::iota(array1.begin(), array1.end(), 0);  // Fills array with 0, 1, 2, ..., 7
    for (auto& val : array1) {
        val *= 13;  // Multiply each element by 13 to match the step of 13 in the Python code
    }

    // Seed for random number generator
    std::random_device rd;
    std::mt19937 g(rd());

    std::shuffle(array1.begin(), array1.end(), g);

    for (int value : array1) {
        bst.add(value, value);
    }

    int total = printBST(bst);
    int prev_total = total;
    if (total != sz) {
        assert(false && "Incorrect number of elements in the tree");
        return false;
    }
    if (!bst.isBST()) {
        assert(false && "Tree is not a BST");
        return false;
    }

    bst.remove(39);
    total = printBST(bst);
    if (total != prev_total - 1) {
        assert(false && "Incorrect number of elements in the tree");
        return false;
    }
    prev_total = total;
    if (!bst.isBST()) {
        assert(false && "Tree is not a BST");
        return false;
    }

    bst.remove(52);
    total = printBST(bst);
    if (total != prev_total - 1) {
        assert(false && "Incorrect number of elements in the tree");
        return false;
    }
    prev_total = total;
    if (!bst.isBST()) {
        assert(false && "Tree is not a BST");
        return false;
    }

    bst.remove(78);
    total = printBST(bst);
    if (total != prev_total - 1) {
        assert(false && "Incorrect number of elements in the tree");
        return false;
    }
    prev_total = total;
    if (!bst.isBST()) {
        assert(false && "Tree is not a BST");
        return false;
    }

    bst.remove(65);
    total = printBST(bst);
    if (total != prev_total - 1) {
        assert(false && "Incorrect number of elements in the tree");
        return false;
    }
    prev_total = total;
    if (!bst.isBST()) {
        assert(false && "Tree is not a BST");
        return false;
    }

    return true;
}

bool testBSTAll() {
    int total = 1000;
    int passed = 0;
    for (int i = 0; i < total; i++) {
        if (testBST()) {
            passed++;
        }
    }
    std::cout << "Passed " << passed << " out of " << total << " tests." << std::endl;
    return passed == total;
}

int main() {
    if (!testBSTAll()) {
        return 1;
    }
    if (!test::testIntervalTree()) {
        return 1;
    }
    std::cout << "All tests passed." << std::endl;

    return 0;
}