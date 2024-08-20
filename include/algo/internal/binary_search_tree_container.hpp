#pragma once

#include <utility>

namespace algo::internal {

template <typename Key, typename Value>
class BinarySearchTreeNode final {
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
    Value& value() { return value_; }

    void setValue(Value&& value) { value_ = std::move(value); }

protected:
    Node* left_;
    Node* right_;
    Node* parent_;
    Key key_;
    Value value_;
};

}  // namespace algo::internal