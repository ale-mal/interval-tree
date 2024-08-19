#pragma once

#include <algorithm>
#include <utility>
#include <vector>

namespace algo {

class Interval {
public:
    bool overlap(const Interval& other) const {
        return low_ <= other.high_ && other.low_ <= high_;
    }

    int key() const {
        return low_;
    }

    int low_ = 0;
    int high_ = 0;
};

class IntervalTreeNode {
public:
    int key() const { return interval_.key(); }

    IntervalTreeNode* left_ = nullptr;
    IntervalTreeNode* right_ = nullptr;
    IntervalTreeNode* parent_ = nullptr;
    int max_ = 0;
    bool is_red_ = true;
    Interval interval_;
};

class IntervalTree {
public:
    void insert(Interval&& interval);

    IntervalTreeNode* search(const Interval& interval);

    void remove(IntervalTreeNode* node);

    template <typename Function>
    void inorder(Function&& func) const;

private:
    void rb_insert_fixup(IntervalTreeNode* node);
    void rb_delete_fixup(IntervalTreeNode* x);

    IntervalTreeNode* minimum(IntervalTreeNode* node);

    void transplant(IntervalTreeNode* u, IntervalTreeNode* v);

    void left_rotate(IntervalTreeNode* x);
    void right_rotate(IntervalTreeNode* x);

    template <typename Function>
    void inorder(IntervalTreeNode* node, Function&& func) const;

    IntervalTreeNode* root_ = nullptr;
};

void IntervalTree::insert(Interval&& interval) {
    auto newNode = new IntervalTreeNode();
    newNode->interval_ = std::move(interval);

    auto node = root_;
    IntervalTreeNode* parent = nullptr;
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
        root_ = newNode;
    } else if (newNode->key() < parent->key()) {
        parent->left_ = newNode;
    } else {
        parent->right_ = newNode;
    }

    rb_insert_fixup(newNode);
}

void IntervalTree::rb_insert_fixup(IntervalTreeNode* node) {
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

IntervalTreeNode* IntervalTree::search(const Interval& interval) {
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

void IntervalTree::remove(IntervalTreeNode* z) {
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
    }
    IntervalTreeNode* successor = nullptr;
    IntervalTreeNode* i = nullptr;

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

    IntervalTreeNode* y = z;
    bool y_original_color = y->is_red_;
    IntervalTreeNode* x;
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
}

void IntervalTree::rb_delete_fixup(IntervalTreeNode* x) {
    while (x && x != root_ && !x->is_red_) {
        if (x == x->parent_->left_) {
            IntervalTreeNode* w = x->parent_->right_;
            if (w->is_red_) {
                w->is_red_ = false;
                x->parent_->is_red_ = true;
                left_rotate(x->parent_);
                w = x->parent_->right_;
            }
            if (!w->left_->is_red_ && !w->right_->is_red_) {
                w->is_red_ = true;
                x = x->parent_;
            } else {
                if (!w->right_->is_red_) {
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
            IntervalTreeNode* w = x->parent_->left_;
            if (w->is_red_) {
                w->is_red_ = false;
                x->parent_->is_red_ = true;
                right_rotate(x->parent_);
                w = x->parent_->left_;
            }
            if (!w->right_->is_red_ && !w->left_->is_red_) {
                w->is_red_ = true;
                x = x->parent_;
            } else {
                if (!w->left_->is_red_) {
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

IntervalTreeNode* IntervalTree::minimum(IntervalTreeNode* node) {
    while (node->left_) {
        node = node->left_;
    }
    return node;
}

void IntervalTree::transplant(IntervalTreeNode* u, IntervalTreeNode* v) {
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

void IntervalTree::left_rotate(IntervalTreeNode* x) {
    IntervalTreeNode* y = x->right_;
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
}

void IntervalTree::right_rotate(IntervalTreeNode* x) {
    IntervalTreeNode* y = x->left_;
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
}

template <typename Function>
void IntervalTree::inorder(Function&& func) const {
    inorder(root_, std::forward<Function>(func));
}

template <typename Function>
void IntervalTree::inorder(IntervalTreeNode* node, Function&& func) const {
    if (node) {
        inorder(node->left_, std::forward<Function>(func));
        func(node->interval_);
        inorder(node->right_, std::forward<Function>(func));
    }
}

}

namespace test {
class Solution {
public:
    std::vector<std::vector<int>> merge(std::vector<std::vector<int>>& intervals) {
        algo::IntervalTree intervalTree;
        for (const auto& interval : intervals) {
            int low = interval[0], high = interval[1];
            for (;;) {
                auto rem = intervalTree.search(algo::Interval{low, high});
                if (!rem) {
                    break;
                }
                low = std::min(low, rem->interval_.low_);
                high = std::max(high, rem->interval_.high_);
                intervalTree.remove(rem);
            }
            intervalTree.insert(algo::Interval{low, high});
        }
        std::vector<std::vector<int>> res;
        intervalTree.inorder([&](const algo::Interval& x) {
            res.emplace_back(std::vector<int>{x.low_, x.high_});
        });
        return res;
    }
};
}

// [[1,3],[2,6],[8,10],[15,18]]
// [[1,6],[8,10],[15,18]]

// [[1,4],[4,5]]
// [[1,5]]

// [[2,3],[5,5],[2,2],[3,4],[3,4]]
// [[2,4],[5,5]]

// [[2,3],[4,5],[6,7],[8,9],[1,10]]
// [[1,10]]

// [[1,3],[0,2],[2,3],[4,6],[4,5],[5,5],[0,2],[3,3]]
// [[0,3],[4,6]]