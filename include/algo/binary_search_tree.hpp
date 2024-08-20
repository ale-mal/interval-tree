#pragma once

#include <memory>
#include <functional>
#include <stdexcept>

namespace algo {
template <typename Key, typename Value>
class BinarySearchTreeNode final {
    static_assert(std::is_same_v<bool, decltype(std::declval<Key>() < std::declval<Key>())>, "Key type must support operator< and return a bool");
    static_assert(std::is_same_v<bool, decltype(std::declval<Key>() == std::declval<Key>())>, "Key type must support operator== and return a bool");

public:
    using Node = BinarySearchTreeNode<Key, Value>;

    template <typename... Args>
    explicit BinarySearchTreeNode(Key && key, Args&&... args)
        : left_(nullptr), right_(nullptr), parent_(nullptr), key_(std::move(key)), value_(std::forward<Args>(args)...) {}

    const Node* left() const { return left_; }
    Node* left() { return left_; }

    const Node* right() const { return right_; }
    Node* right() { return right_; }

    void setLeft(Node* node) {
        left_ = node;
        if (left_) {
            left_->setParent(this);
        }
    }

    void setRight(Node* node) {
        right_ = node;
        if (right_) {
            right_->setParent(this);
        }
    }

    Node* takeLeft() {
        auto old_left = left_;
        left_ = nullptr;
        if (old_left) {
            old_left->setParent(nullptr);
        }
        return old_left;
    }

    Node* takeRight() {
        auto old_right = right_;
        right_ = nullptr;
        if (old_right) {
            old_right->setParent(nullptr);
        }
        return old_right;
    }

    const Node* parent() const { return parent_; }
    Node* parent() { return parent_; }

    void setParent(Node* parent) { parent_ = parent; }

    const Key& key() const { return key_; }

    const Value& value() const { return value_; }
    Value& mutable_value() { return value_; }

    void setValue(Value&& value) { value_ = std::move(value); }

private:
    Node* left_;
    Node* right_;
    Node* parent_;
    Key key_;
    Value value_;
};

template <typename T, typename U>
class BinarySearchTree {
public:
    using Node = BinarySearchTreeNode<T, U>;

    BinarySearchTree() noexcept : root_(nullptr) {}
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

    const Node* min_element() const { return minimum(root_); }
    const Node* max_element() const { return maximum(root_); }

    bool isBST() const;

private:
    using NodeType = std::unique_ptr<Node>;

    void clear();

    template <typename Function>
    void inorder(const Node* node, Function&& func) const;

    template <typename Function>
    void inorder(Node* node, Function&& func);

    void internal_add(NodeType&& node);
    NodeType internal_remove(Node* node);

    const Node* search(const T& key) const;
    Node* mutable_search(const T& key);

    Node* minimum(Node* node) const;
    Node* maximum(Node* node) const;

    void transplant(Node* u, Node* v);

    bool isBST(const Node* node) const;

    Node* root_;
};

template <typename T, typename U>
void BinarySearchTree<T, U>::clear() {
    while (root_ != nullptr) {
        internal_remove(root_);
    }
}

template <typename T, typename U>
template <typename Function>
void BinarySearchTree<T, U>::inorder(Function&& func) const {
    inorder(root_, std::forward<Function>(func));
}

template <typename T, typename U>
template <typename Function>
void BinarySearchTree<T, U>::inorder(Function&& func) {
    inorder(root_, std::forward<Function>(func));
}

template <typename T, typename U>
template <typename Function>
void BinarySearchTree<T, U>::inorder(const Node* node, Function&& func) const {
    if (node) {
        inorder(node->left(), std::forward<Function>(func));
        func(node->key(), node->value());
        inorder(node->right(), std::forward<Function>(func));
    }
}

template <typename T, typename U>
template <typename Function>
void BinarySearchTree<T, U>::inorder(Node* node, Function&& func) {
    if (node) {
        inorder(node->left(), std::forward<Function>(func));
        func(node->key(), node->value());
        inorder(node->right(), std::forward<Function>(func));
    }
}

template <typename T, typename U>
const U& BinarySearchTree<T, U>::at(const T& key) const {
    const Node* node = search(key);
    if (node == nullptr) {
        throw std::out_of_range("Key not found");
    }
    return node->value();
}

template <typename T, typename U>
U& BinarySearchTree<T, U>::at(const T& key) {
    Node* node = mutable_search(key);
    if (node == nullptr) {
        throw std::out_of_range("Key not found");
    }
    return node->mutable_value();
}

template <typename T, typename U>
const typename BinarySearchTree<T, U>::Node* BinarySearchTree<T, U>::search(const T& key) const {
    const Node* x = root_;
    while (x != nullptr && !(key == x->key())) {
        if (key < x->key()) {
            x = x->left();
        } else {
            x = x->right();
        }
    }
    return x;
}

template <typename T, typename U>
typename BinarySearchTree<T, U>::Node* BinarySearchTree<T, U>::mutable_search(const T& key) {
    Node* x = root_;
    while (x != nullptr && !(key == x->key())) {
        if (key < x->key()) {
            x = x->left();
        } else {
            x = x->right();
        }
    }
    return x;
}

template <typename T, typename U>
void BinarySearchTree<T, U>::add(const T& key, U value) {
    internal_add(std::make_unique<Node>(T{key}, std::move(value)));
}

template <typename T, typename U>
void BinarySearchTree<T, U>::internal_add(NodeType&& node) {
    Node* x = root_;
    Node* y = nullptr;
    while (x != nullptr) {
        y = x;
        if (node->key() < x->key()) {
            x = x->left();
        } else {
            x = x->right();
        }
    }
    if (y == nullptr) {
        root_ = node.get();
        root_->setParent(nullptr);
    } else if (node->key() < y->key()) {
        y->setLeft(node.get());
    } else {
        y->setRight(node.get());
    }
    node.release();
}

template <typename T, typename U>
void BinarySearchTree<T, U>::remove(const T& key) {
    internal_remove(mutable_search(key));
}

template <typename T, typename U>
void BinarySearchTree<T, U>::transplant(Node* u, Node* v) {
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

template <typename T, typename U>
typename BinarySearchTree<T, U>::NodeType BinarySearchTree<T, U>::internal_remove(Node* z) {
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

    std::unique_ptr<Node> old_node(z);
    return old_node;
}

template <typename T, typename U>
typename BinarySearchTree<T, U>::Node* BinarySearchTree<T, U>::minimum(Node* node) const {
    while (node != nullptr && node->left() != nullptr) {
        node = node->left();
    }
    return node;
}

template <typename T, typename U>
typename BinarySearchTree<T, U>::Node* BinarySearchTree<T, U>::maximum(Node* node) const {
    while (node != nullptr && node->right() != nullptr) {
        node = node->right();
    }
    return node;
}

template <typename T, typename U>
bool BinarySearchTree<T, U>::isBST() const {
    return isBST(root_);
}

template <typename T, typename U>
bool BinarySearchTree<T, U>::isBST(const Node* node) const {
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
        if (node->right()->key() < node->key()) {
            return false;
        }
        if (!isBST(node->right())) {
            return false;
        }
    }

    return true;
}

}  // namespace algo