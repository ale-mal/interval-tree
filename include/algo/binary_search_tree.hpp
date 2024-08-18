#pragma once

#include <memory>
#include <functional>

namespace algo {
template <typename Key, typename Value>
class BinarySearchTreeNode final {
    static_assert(std::is_same_v<bool, decltype(std::declval<Key>() < std::declval<Key>())>, "Key type must support operator< and return a bool");

public:
    using Node = BinarySearchTreeNode<Key, Value>;

    template <typename... Args>
    explicit BinarySearchTreeNode(Key && key, Args&&... args)
        : left_(nullptr), right_(nullptr), parent_(nullptr), key_(std::move(key)), value_(std::forward<Args>(args)...) {}

    explicit BinarySearchTreeNode(Key&& key, Value&& value)
        : left_(nullptr), right_(nullptr), parent_(nullptr), key_(std::move(key)), value_(std::move(value)) {}

    const Node* left() const { return left_.get(); }
    Node* left() { return left_.get(); }

    void setLeft(std::unique_ptr<Node>&& node) { left_ = std::move(node); }

    const Node* right() const { return right_.get(); }
    Node* right() { return right_.get(); }

    void setRight(std::unique_ptr<Node>&& node) { right_ = std::move(node); }

    const Node* parent() const { return parent_; }
    Node* parent() { return parent_; }

    void setParent(Node* parent) { parent_ = parent; }

    const Key& key() const { return key_; }

    const Value& value() const { return value_; }
    Value& value() { return value_; }

    void setValue(Value&& value) { value_ = std::move(value); }

private:
    std::unique_ptr<Node> left_;
    std::unique_ptr<Node> right_;
    Node* parent_;
    Key key_;
    Value value_;
};

template <typename T, typename U>
class BinarySearchTree {
public:
    using NodeType = std::unique_ptr<BinarySearchTreeNode<T, U>>;

    template <typename Function>
    void inorder(Function&& func) const;

    template <typename Function>
    void inorder(Function&& func);

    U* searchIterative(const T& key) const;

    const NodeType& search(const T& key) const;

    void add(const T& key, U value);

    void remove(const T& key);

private:
    using Node = BinarySearchTreeNode<T, U>;

    template <typename Function>
    void inorder(Node* node, Function&& func) const;

    template <typename Function>
    void inorder(Node* node, Function&& func);

    const NodeType& searchNode(const T& key, const NodeType& node) const;

    void addNode(NodeType&& node);

    NodeType root_;
};

template <typename T, typename U>
template <typename Function>
void BinarySearchTree<T, U>::inorder(Function&& func) const {
    inorder(root_.get(), std::forward<Function>(func));
}

template <typename T, typename U>
template <typename Function>
void BinarySearchTree<T, U>::inorder(Function&& func) {
    inorder(root_.get(), std::forward<Function>(func));
}

template <typename T, typename U>
template <typename Function>
void BinarySearchTree<T, U>::inorder(Node* node, Function&& func) const {
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
U* BinarySearchTree<T, U>::searchIterative(const T& key) const {
    Node* x = root_.get();
    while (x != nullptr && key != x->key()) {
        if (key < x->key()) {
            x = x->left();
        } else {
            x = x->right();
        }
    }
    if (x == nullptr) {
        return nullptr;
    }
    return &x->value();
}

template <typename T, typename U>
const typename BinarySearchTree<T, U>::NodeType& BinarySearchTree<T, U>::search(const T& key) const {
    return searchNode(key, root_);
}

template <typename T, typename U>
const typename BinarySearchTree<T, U>::NodeType& BinarySearchTree<T, U>::searchNode(const T& key, const BinarySearchTree<T, U>::NodeType& node) const {
    if (node == nullptr || key == node->key()) {
        return node;
    }
    if (key < node->key()) {
        return searchNode(key, node->left());
    }
    return searchNode(key, node->right());
}

template <typename T, typename U>
void BinarySearchTree<T, U>::add(const T& key, U value) {
    addNode(std::make_unique<Node>(T{key}, std::move(value)));
}

template <typename T, typename U>
void BinarySearchTree<T, U>::addNode(NodeType&& node) {
    Node* x = root_.get();
    Node* y = nullptr;
    while (x != nullptr) {
        y = x;
        if (node->key() < x->key()) {
            x = x->left();
        } else {
            x = x->right();
        }
    }
    node->setParent(y);
    if (y == nullptr) {
        root_ = std::move(node);
    } else if (node->key() < y->key()) {
        y->setLeft(std::move(node));
    } else {
        y->setRight(std::move(node));
    }
}


}  // namespace algo