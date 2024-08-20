#pragma once

#include <algorithm>
#include <utility>
#include <vector>
#include <cassert>
#include <iostream>
#include <limits>

namespace algo {

template <typename T>
struct Interval {
    bool overlap(const Interval& other) const {
        // to make it half open change less_equal ('<=') to less ('<')
        return low_ <= other.high_ && other.low_ <= high_;
    }

    T key() const {
        return low_;
    }

    T low_ = 0;
    T high_ = 0;

};

template <typename K, typename V>
class IntervalTreeNode {
public:
    K key() const { return interval_.key(); }

    IntervalTreeNode<K, V>* left_ = nullptr;
    IntervalTreeNode<K, V>* right_ = nullptr;
    IntervalTreeNode<K, V>* parent_ = nullptr;
    K max_;
    bool is_red_ = true;
    Interval<K> interval_;
    V value_;
};

template <typename K, typename V>
class IntervalTree {
public:
    using Node = IntervalTreeNode<K, V>;

    IntervalTree() noexcept : root_(nullptr) {}
    ~IntervalTree() { cleanup(); }

    IntervalTree(IntervalTree&& tree) noexcept = default;
    IntervalTree& operator=(IntervalTree&& tree) noexcept = default;

    IntervalTree(const IntervalTree& tree) = delete;
    IntervalTree& operator=(const IntervalTree& tree) = delete;

    void cleanup();

    void insert(Interval<K>&& interval, V value);

    Node* search(const Interval<K>& interval);

    void remove(Node* node);

    template <typename Function>
    void inorder(Function&& func) const;

    template <typename Function>
    void walk(const Interval<K>& interval, Function&& func) const;

    std::string print() const;

private:
    using NodeType = std::unique_ptr<Node>;

    void internal_add(NodeType&& node);

    void rb_insert_fixup(Node* node);
    void rb_delete_fixup(Node* x);

    Node* minimum(Node* node);

    void transplant(Node* u, Node* v);

    void left_rotate(Node* x);
    void right_rotate(Node* x);

    template <typename Function>
    void inorder(Node* node, Function&& func) const;

    template <typename Function>
    void walk(Node* node, const Interval<K>& interval, Function&& func) const;

    Node* root_ = nullptr;
};

template <typename K, typename V>
void IntervalTree<K, V>::cleanup() {
    while (root_) {
        remove(root_);
    }
}

template <typename K, typename V>
void IntervalTree<K, V>::insert(Interval<K>&& interval, V value) {
    auto newNode = std::make_unique<IntervalTreeNode<K, V>>();
    newNode->interval_ = std::move(interval);
    newNode->max_ = newNode->interval_.high_;
    newNode->value_ = std::move(value);
    internal_add(std::move(newNode));
}

template <typename K, typename V>
void IntervalTree<K, V>::internal_add(NodeType&& newNode) {
    auto node = root_;
    Node* parent = nullptr;
    while (node) {
        parent = node;
        node->max_ = std::max(node->max_, newNode->interval_.high_);
        if (newNode->key() < node->key()) {
            node = node->left_;
        } else {
            node = node->right_;
        }
    }

    newNode->parent_ = parent;
    if (parent == nullptr) {
        root_ = newNode.get();
    } else if (newNode->key() < parent->key()) {
        parent->left_ = newNode.get();
    } else {
        parent->right_ = newNode.get();
    }

    rb_insert_fixup(newNode.get());

    newNode.release();
}

template <typename K, typename V>
void IntervalTree<K, V>::rb_insert_fixup(Node* node) {
    while (node->parent_ && node->parent_->is_red_) {
        if (node->parent_ == node->parent_->parent_->left_) {
            auto uncle = node->parent_->parent_->right_;
            if (uncle && uncle->is_red_) {
                node->parent_->is_red_ = false;
                uncle->is_red_ = false;
                node->parent_->parent_->is_red_ = true;
                node = node->parent_->parent_;
            } else {
                if (node == node->parent_->right_) {
                    node = node->parent_;
                    left_rotate(node);
                }
                node->parent_->is_red_ = false;
                if (node->parent_->parent_) {
                    node->parent_->parent_->is_red_ = true;
                    right_rotate(node->parent_->parent_);
                }
            }
        } else {
            auto uncle = node->parent_->parent_->left_;
            if (uncle && uncle->is_red_) {
                node->parent_->is_red_ = false;
                uncle->is_red_ = false;
                node->parent_->parent_->is_red_ = true;
                node = node->parent_->parent_;
            } else {
                if (node == node->parent_->left_) {
                    node = node->parent_;
                    right_rotate(node);
                }
                node->parent_->is_red_ = false;
                if (node->parent_->parent_) {
                    node->parent_->parent_->is_red_ = true;
                    left_rotate(node->parent_->parent_);
                }
            }
        }
    }
    root_->is_red_ = false;
}

template <typename K, typename V>
typename IntervalTree<K, V>::Node* IntervalTree<K, V>::search(const Interval<K>& interval) {
    auto node = root_;
    while (node && !node->interval_.overlap(interval)) {
        if (node->left_ && node->left_->max_ >= interval.low_) {
            node = node->left_;
        } else {
            node = node->right_;
        }
    }
    return node;
}

template <typename K, typename V>
void IntervalTree<K, V>::remove(Node* z) {
    if (!z) {
        return;
    }

    bool has_two_children = false;
    if (z->left_ && z->right_) {
        z->max_ = std::max(z->left_->max_, z->right_->max_);
    } else if (z->left_) {
        z->max_ = z->left_->max_;
    } else if (z->right_) {
        z->max_ = z->right_->max_;
    } else {
        z->max_ = std::numeric_limits<K>::lowest();
    }
    Node* successor = nullptr;
    Node* i = nullptr;

    if (z->left_ && z->right_) {
        successor = minimum(z->right_);
        i = successor->parent_;
        has_two_children = true;
    } else {
        i = z->parent_;
    }

    while (i) {
        i->max_ = i->interval_.high_;
        if (i->left_) {
            i->max_ = std::max(i->max_, i->left_->max_);
        }
        if (i->right_) {
            i->max_ = std::max(i->max_, i->right_->max_);
        }
        i = i->parent_;
    }

    if (has_two_children) {
        successor->max_ = z->max_;
    }

    Node* y = z;
    bool y_original_color = y->is_red_;
    Node* x;
    if (z->left_ == nullptr) {
        x = z->right_;
        transplant(z, z->right_);
    } else if (z->right_ == nullptr) {
        x = z->left_;
        transplant(z, z->left_);
    } else {
        y = minimum(z->right_);
        y_original_color = y->is_red_;
        x = y->right_;
        if (y != z->right_) {
            transplant(y, y->right_);
            y->right_ = z->right_;
            y->right_->parent_ = y;
        } else {
            if (x) {
                x->parent_ = y;
            }
        }
        transplant(z, y);
        y->left_ = z->left_;
        y->left_->parent_ = y;
        y->is_red_ = z->is_red_;
    }

    if (y_original_color == false) {
        rb_delete_fixup(x);
    }

    std::unique_ptr<Node> old_node(z);
}

template <typename K, typename V>
void IntervalTree<K, V>::rb_delete_fixup(Node* x) {
    while (x && x != root_ && !x->is_red_) {
        if (x == x->parent_->left_) {
            Node* w = x->parent_->right_;
            if (!w) {
                x = x->parent_;
                continue;
            }
            if (w->is_red_) {
                w->is_red_ = false;
                x->parent_->is_red_ = true;
                left_rotate(x->parent_);
                w = x->parent_->right_;
            }
            if (!w || ((!w->left_ || !w->left_->is_red_) && (!w->right_ || !w->right_->is_red_))) {
                if (w) {
                    w->is_red_ = true;
                }
                x = x->parent_;
            } else {
                if (!w->right_ || !w->right_->is_red_) {
                    w->left_->is_red_ = false;
                    w->is_red_ = true;
                    right_rotate(w);
                    w = x->parent_->right_;
                }
                w->is_red_ = x->parent_->is_red_;
                x->parent_->is_red_ = false;
                w->right_->is_red_ = false;
                left_rotate(x->parent_);
                x = root_;
            }
        } else {
            Node* w = x->parent_->left_;
            if (!w) {
                x = x->parent_;
                continue;
            }
            if (w->is_red_) {
                w->is_red_ = false;
                x->parent_->is_red_ = true;
                right_rotate(x->parent_);
                w = x->parent_->left_;
            }
            if (!w || ((!w->right_ || !w->right_->is_red_) && (!w->left_ || !w->left_->is_red_))) {
                if (w) {
                    w->is_red_ = true;
                }
                x = x->parent_;
            } else {
                if (!w->left_ || !w->left_->is_red_) {
                    w->right_->is_red_ = false;
                    w->is_red_ = true;
                    left_rotate(w);
                    w = x->parent_->left_;
                }
                w->is_red_ = x->parent_->is_red_;
                x->parent_->is_red_ = false;
                w->left_->is_red_ = false;
                right_rotate(x->parent_);
                x = root_;
            }
        }
    }
    if (x) {
        x->is_red_ = false;
    }
}

template <typename K, typename V>
typename IntervalTree<K, V>::Node* IntervalTree<K, V>::minimum(Node* node) {
    while (node->left_) {
        node = node->left_;
    }
    return node;
}

template <typename K, typename V>
void IntervalTree<K, V>::transplant(Node* u, Node* v) {
    if (u->parent_ == nullptr) {
        root_ = v;
    } else if (u == u->parent_->left_) {
        u->parent_->left_ = v;
    } else {
        u->parent_->right_ = v;
    }
    if (v) {
        v->parent_ = u->parent_;
    }
}

template <typename K, typename V>
void IntervalTree<K, V>::left_rotate(Node* x) {
    Node* y = x->right_;
    x->right_ = y->left_;
    if (y->left_) {
        y->left_->parent_ = x;
    }
    y->parent_ = x->parent_;
    if (x->parent_ == nullptr) {
        root_ = y;
    } else if (x == x->parent_->left_) {
        x->parent_->left_ = y;
    } else {
        x->parent_->right_ = y;
    }
    y->left_ = x;
    x->parent_ = y;

    y->max_ = x->max_;
    x->max_ = x->interval_.high_;
    if (x->left_ && x->left_->max_ > x->max_) {
        x->max_ = x->left_->max_;
    }
    if (x->right_ && x->right_->max_ > x->max_) {
        x->max_ = x->right_->max_;
    }
}

template <typename K, typename V>
void IntervalTree<K, V>::right_rotate(Node* x) {
    Node* y = x->left_;
    x->left_ = y->right_;
    if (y->right_) {
        y->right_->parent_ = x;
    }
    y->parent_ = x->parent_;
    if (x->parent_ == nullptr) {
        root_ = y;
    } else if (x == x->parent_->right_) {
        x->parent_->right_ = y;
    } else {
        x->parent_->left_ = y;
    }
    y->right_ = x;
    x->parent_ = y;

    y->max_ = x->max_;
    x->max_ = x->interval_.high_;
    if (x->left_ && x->left_->max_ > x->max_) {
        x->max_ = x->left_->max_;
    }
    if (x->right_ && x->right_->max_ > x->max_) {
        x->max_ = x->right_->max_;
    }
}

template <typename K, typename V>
template <typename Function>
void IntervalTree<K, V>::inorder(Function&& func) const {
    inorder(root_, std::forward<Function>(func));
}

template <typename K, typename V>
template <typename Function>
void IntervalTree<K, V>::inorder(Node* node, Function&& func) const {
    if (node) {
        inorder(node->left_, std::forward<Function>(func));
        func(node->interval_, node->value_);
        inorder(node->right_, std::forward<Function>(func));
    }
}

template <typename K, typename V>
std::string IntervalTree<K, V>::print() const {
    std::string res;
    inorder([&](const auto& interval, const auto& value) {
        res += std::to_string(interval.low_) + " " + std::to_string(interval.high_) + "\n";
    });
    return res;
}


template <typename K, typename V>
template <typename Function>
void IntervalTree<K, V>::walk(const Interval<K>& interval, Function&& func) const {
    walk(root_, interval, std::forward<Function>(func));
}


template <typename K, typename V>
template <typename Function>
void IntervalTree<K, V>::walk(Node* node, const Interval<K>& interval, Function&& func) const {
    if (node == nullptr) {
        return;
    }
    if (node->left_ && interval.low_ < node->left_->max_) {
        walk(node->left_, interval, std::forward<Function>(func));
    }
    if (node->interval_.overlap(interval)) {
        func(node->interval_, node->value_);
    }
    if (node->right_ && interval.low_ < node->right_->max_) {
        walk(node->right_, interval, std::forward<Function>(func));
    }
}

}  // namespace algo
