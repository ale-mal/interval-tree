#pragma once

#include <memory>
#include <functional>

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

    explicit BinarySearchTreeNode(Key&& key, Value&& value)
        : left_(nullptr), right_(nullptr), parent_(nullptr), key_(std::move(key)), value_(std::move(value)) {}

    const std::unique_ptr<Node>& left() const { return left_; }
    std::unique_ptr<Node>& left() { return left_; }

    const std::unique_ptr<Node>& right() const { return right_; }
    std::unique_ptr<Node>& right() { return right_; }

    void setLeft(std::unique_ptr<Node>&& node) { left_ = std::move(node); if (left_) { left_->setParent(this); } }
    void setRight(std::unique_ptr<Node>&& node) { right_ = std::move(node); if (right_) { right_->setParent(this); } }

    std::unique_ptr<Node> takeLeft() { return std::move(left_); }
    std::unique_ptr<Node> takeRight() { return std::move(right_); }

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

    U* search(const T& key) const;

    void add(const T& key, U value);

    NodeType remove(const T& key);

    const NodeType& min_element() const { return minimum(root_); }
    const NodeType& max_element() const { return maximum(root_); }

    bool isBST() const;

private:
    using Node = BinarySearchTreeNode<T, U>;

    template <typename Function>
    void inorder(const NodeType& node, Function&& func) const;

    template <typename Function>
    void inorder(NodeType& node, Function&& func);

    void addNode(NodeType&& node);

    const NodeType& find(const T& key, const NodeType& node) const;

    const NodeType& minimum(const NodeType& node) const;
    const NodeType& maximum(const NodeType& node) const;

    NodeType transplant(const NodeType& u, NodeType&& v);

    bool isBST(const NodeType& node) const;

    NodeType root_;
};

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
void BinarySearchTree<T, U>::inorder(const NodeType& node, Function&& func) const {
    if (node) {
        inorder(node->left(), std::forward<Function>(func));
        func(node->key(), node->value());
        inorder(node->right(), std::forward<Function>(func));
    }
}

template <typename T, typename U>
template <typename Function>
void BinarySearchTree<T, U>::inorder(NodeType& node, Function&& func) {
    if (node) {
        inorder(node->left(), std::forward<Function>(func));
        func(node->key(), node->value());
        inorder(node->right(), std::forward<Function>(func));
    }
}

template <typename T, typename U>
U* BinarySearchTree<T, U>::search(const T& key) const {
    Node* x = root_.get();
    while (x != nullptr && !(key == x->key())) {
        if (key < x->key()) {
            x = x->left().get();
        } else {
            x = x->right().get();
        }
    }
    if (x == nullptr) {
        return nullptr;
    }
    return &x->value();
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
            x = x->left().get();
        } else {
            x = x->right().get();
        }
    }
    if (y == nullptr) {
        root_ = std::move(node);
        root_->setParent(nullptr);
    } else if (node->key() < y->key()) {
        y->setLeft(std::move(node));
    } else {
        y->setRight(std::move(node));
    }
}

template <typename T, typename U>
const typename BinarySearchTree<T, U>::NodeType& BinarySearchTree<T, U>::find(const T& key, const NodeType& node) const {
    if (node == nullptr || key == node->key()) {
        return node;
    }
    if (key < node->key()) {
        return find(key, node->left());
    }
    return find(key, node->right());
}

template <typename T, typename U>
const typename BinarySearchTree<T, U>::NodeType& BinarySearchTree<T, U>::minimum(const NodeType& node) const {
    if (node == nullptr || node->left() == nullptr) {
        return node;
    }
    return minimum(node->left());
}

template <typename T, typename U>
const typename BinarySearchTree<T, U>::NodeType& BinarySearchTree<T, U>::maximum(const NodeType& node) const {
    if (node == nullptr || node->right() == nullptr) {
        return node;
    }
    return maximum(node->right());
}

template <typename T, typename U>
typename BinarySearchTree<T, U>::NodeType BinarySearchTree<T, U>::transplant(const NodeType& u, NodeType&& v) {
    // Replace the subtree rooted at node u with the subtree rooted at node v.
    Node* parent = u->parent();
    if (parent == nullptr) {
        auto old_root = std::move(root_);
        root_ = std::move(v);
        if (root_) {
            root_->setParent(nullptr);
        }
        return old_root;
    }

    NodeType old_subtree;
    if (u == parent->left()) {
        old_subtree = std::move(parent->takeLeft());
        parent->setLeft(std::move(v));
    } else {
        old_subtree = std::move(parent->takeRight());
        parent->setRight(std::move(v));
    }
    old_subtree->setParent(nullptr);
    return old_subtree;
}

template <typename T, typename U>
typename BinarySearchTree<T, U>::NodeType BinarySearchTree<T, U>::remove(const T& key) {
    const auto& z = find(key, root_);
    if (z == nullptr) {
        return nullptr;
    }

    if (z->left() == nullptr) {
        return transplant(z, z->takeRight());
    } else if (z->right() == nullptr) {
        return transplant(z, z->takeLeft());
    } else {
        const auto& y = minimum(z->right());
        if (y != z->right()) {
            auto old_y = transplant(y, y->takeRight());
            old_y->setRight(z->takeRight());
            old_y->setLeft(z->takeLeft());
            return transplant(z, std::move(old_y));
        } else {
            y->setLeft(z->takeLeft());
            return transplant(z, z->takeRight());
        }
    }
    return nullptr;
}

template <typename T, typename U>
bool BinarySearchTree<T, U>::isBST() const {
    return isBST(root_);
}

template <typename T, typename U>
bool BinarySearchTree<T, U>::isBST(const NodeType& node) const {
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