#pragma once

#include <memory>
#include <functional>
#include <stdexcept>

#include "internal/binary_search_tree_container.hpp"

namespace algo {

template <typename T, typename U, typename Compare = std::less<T>, typename Node= internal::BinarySearchTreeNode<T, U>>
class BinarySearchTree {
public:
    BinarySearchTree() noexcept : root_(nullptr), sz_(0), comp_(Compare()) {}
    ~BinarySearchTree() { clear(); }

    BinarySearchTree(BinarySearchTree&& bst) noexcept = default;
    BinarySearchTree& operator=(BinarySearchTree&& bst) noexcept = default;

    BinarySearchTree(const BinarySearchTree& bst) = delete;
    BinarySearchTree& operator=(const BinarySearchTree& bst) = delete;

    template <typename Function>
    void inorder(Function&& func) const;

    template <typename Function>
    void inorder(Function&& func);

    const U& at(const T& key) const;
    U& at(const T& key);

    bool contains(const T& key) const { return search(key) != nullptr; }

    void add(const T& key, U value);

    void remove(const T& key);

    bool isBST() const;

    int size() const { return sz_; }

protected:
    using NodeType = std::unique_ptr<Node>;

    void clear();

    template <typename Function>
    void inorder(const Node* node, Function&& func) const;

    template <typename Function>
    void inorder(Node* node, Function&& func);

    void internal_add(NodeType&& node);
    NodeType internal_remove(Node* node);

    const Node* search(const T& key) const;
    Node* search(const T& key);

    Node* minimum(Node* node) const;
    Node* maximum(Node* node) const;

    void transplant(Node* u, Node* v);

    bool isBST(const Node* node) const;

    Node* root_;
    int sz_;
    Compare comp_;
};

template <typename T, typename U, typename Compare, typename Node>
void BinarySearchTree<T, U, Compare, Node>::clear() {
    while (root_ != nullptr) {
        internal_remove(root_);
    }
}

template <typename T, typename U, typename Compare, typename Node>
template <typename Function>
void BinarySearchTree<T, U, Compare, Node>::inorder(Function&& func) const {
    inorder(root_, std::forward<Function>(func));
}

template <typename T, typename U, typename Compare, typename Node>
template <typename Function>
void BinarySearchTree<T, U, Compare, Node>::inorder(Function&& func) {
    inorder(root_, std::forward<Function>(func));
}

template <typename T, typename U, typename Compare, typename Node>
template <typename Function>
void BinarySearchTree<T, U, Compare, Node>::inorder(const Node* node, Function&& func) const {
    if (node) {
        inorder(node->left(), std::forward<Function>(func));
        func(node->key(), node->value());
        inorder(node->right(), std::forward<Function>(func));
    }
}

template <typename T, typename U, typename Compare, typename Node>
template <typename Function>
void BinarySearchTree<T, U, Compare, Node>::inorder(Node* node, Function&& func) {
    if (node) {
        inorder(node->left(), std::forward<Function>(func));
        func(node->key(), node->value());
        inorder(node->right(), std::forward<Function>(func));
    }
}

template <typename T, typename U, typename Compare, typename Node>
const U& BinarySearchTree<T, U, Compare, Node>::at(const T& key) const {
    const Node* node = search(key);
    if (node == nullptr) {
        throw std::out_of_range("Key not found");
    }
    return node->value();
}

template <typename T, typename U, typename Compare, typename Node>
U& BinarySearchTree<T, U, Compare, Node>::at(const T& key) {
    Node* node = search(key);
    if (node == nullptr) {
        throw std::out_of_range("Key not found");
    }
    return node->value();
}

template <typename T, typename U, typename Compare, typename Node>
const Node* BinarySearchTree<T, U, Compare, Node>::search(const T& key) const {
    const Node* x = root_;
    while (x != nullptr) {
        if (comp_(key, x->key())) {
            x = x->left();
        } else if (comp_(x->key(), key)) {
            x = x->right();
        } else {
            // key == x->key()
            break;
        }
    }
    return x;
}

template <typename T, typename U, typename Compare, typename Node>
Node* BinarySearchTree<T, U, Compare, Node>::search(const T& key) {
    Node* x = root_;
    while (x != nullptr) {
        if (comp_(key, x->key())) {
            x = x->left();
        } else if (comp_(x->key(), key)) {
            x = x->right();
        } else {
            // key == x->key()
            break;
        }
    }
    return x;
}

template <typename T, typename U, typename Compare, typename Node>
void BinarySearchTree<T, U, Compare, Node>::add(const T& key, U value) {
    internal_add(std::make_unique<Node>(T{key}, std::move(value)));
}

template <typename T, typename U, typename Compare, typename Node>
void BinarySearchTree<T, U, Compare, Node>::internal_add(NodeType&& node) {
    Node* x = root_;
    Node* y = nullptr;
    while (x != nullptr) {
        y = x;
        if (comp_(node->key(), x->key())) {
            x = x->left();
        } else {
            x = x->right();
        }
    }
    if (y == nullptr) {
        root_ = node.get();
        root_->setParent(nullptr);
    } else if (comp_(node->key(), y->key())) {
        y->setLeft(node.get());
    } else {
        y->setRight(node.get());
    }
    node.release();
    sz_++;
}

template <typename T, typename U, typename Compare, typename Node>
void BinarySearchTree<T, U, Compare, Node>::remove(const T& key) {
    internal_remove(search(key));
}

template <typename T, typename U, typename Compare, typename Node>
void BinarySearchTree<T, U, Compare, Node>::transplant(Node* u, Node* v) {
    // Replace the subtree rooted at node u with the subtree rooted at node v.
    Node* parent = u->parent();
    if (parent == nullptr) {
        root_ = v;
        if (root_) {
            root_->setParent(nullptr);
        }
    } else if (u == parent->left()) {
        parent->setLeft(v);
    } else {
        parent->setRight(v);
    }
}

template <typename T, typename U, typename Compare, typename Node>
typename BinarySearchTree<T, U, Compare, Node>::NodeType BinarySearchTree<T, U, Compare, Node>::internal_remove(Node* z) {
    if (z == nullptr) {
        return nullptr;
    }

    if (z->left() == nullptr) {
        transplant(z, z->takeRight());
    } else if (z->right() == nullptr) {
        transplant(z, z->takeLeft());
    } else {
        Node* y = minimum(z->right());
        if (y != z->right()) {
            transplant(y, y->takeRight());
            y->setRight(z->takeRight());
        }
        transplant(z, y);
        y->setLeft(z->takeLeft());
    }

    sz_--;
    std::unique_ptr<Node> old_node(z);
    return old_node;
}

template <typename T, typename U, typename Compare, typename Node>
Node* BinarySearchTree<T, U, Compare, Node>::minimum(Node* node) const {
    while (node != nullptr && node->left() != nullptr) {
        node = node->left();
    }
    return node;
}

template <typename T, typename U, typename Compare, typename Node>
Node* BinarySearchTree<T, U, Compare, Node>::maximum(Node* node) const {
    while (node != nullptr && node->right() != nullptr) {
        node = node->right();
    }
    return node;
}

template <typename T, typename U, typename Compare, typename Node>
bool BinarySearchTree<T, U, Compare, Node>::isBST() const {
    return isBST(root_);
}

template <typename T, typename U, typename Compare, typename Node>
bool BinarySearchTree<T, U, Compare, Node>::isBST(const Node* node) const {
    if (node == nullptr) {
        return true;
    }

    if (node->left() != nullptr) {
        if (node->left()->key() > node->key()) {
            return false;
        }
        if (!isBST(node->left())) {
            return false;
        }
    }

    if (node->right() != nullptr) {
        if (comp_(node->right()->key(), node->key())) {
            return false;
        }
        if (!isBST(node->right())) {
            return false;
        }
    }

    return true;
}

}  // namespace algo