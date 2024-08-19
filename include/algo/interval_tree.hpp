#pragma once

#include <algorithm>
#include <utility>
#include <vector>
#include <cassert>
#include <iostream>
#include <limits>

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
    IntervalTree() = default;
    ~IntervalTree();

    void cleanup();

    void insert(Interval&& interval);

    IntervalTreeNode* search(const Interval& interval);

    void remove(IntervalTreeNode* node);

    template <typename Function>
    void inorder(Function&& func) const;

    std::string print() const;

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

IntervalTree::~IntervalTree() {
    cleanup();
}

void IntervalTree::cleanup() {
    while (root_) {
        remove(root_);
    }
}

void IntervalTree::insert(Interval&& interval) {
    auto newNode = new IntervalTreeNode();
    newNode->interval_ = std::move(interval);
    newNode->max_ = newNode->interval_.high_;

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
    } else {
        z->max_ = std::numeric_limits<int>::min();
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

    delete z;
}

void IntervalTree::rb_delete_fixup(IntervalTreeNode* x) {
    while (x && x != root_ && !x->is_red_) {
        if (x == x->parent_->left_) {
            IntervalTreeNode* w = x->parent_->right_;
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
            IntervalTreeNode* w = x->parent_->left_;
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

    y->max_ = x->max_;
    x->max_ = x->interval_.high_;
    if (x->left_ && x->left_->max_ > x->max_) {
        x->max_ = x->left_->max_;
    }
    if (x->right_ && x->right_->max_ > x->max_) {
        x->max_ = x->right_->max_;
    }
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

    y->max_ = x->max_;
    x->max_ = x->interval_.high_;
    if (x->left_ && x->left_->max_ > x->max_) {
        x->max_ = x->left_->max_;
    }
    if (x->right_ && x->right_->max_ > x->max_) {
        x->max_ = x->right_->max_;
    }
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

std::string IntervalTree::print() const {
    std::string res;
    inorder([&](const Interval& interval) {
        res += std::to_string(interval.low_) + " " + std::to_string(interval.high_) + "\n";
    });
    return res;
}

}  // namespace algo

namespace test {
class Solution {
public:
    std::vector<std::vector<int>> merge(std::vector<std::vector<int>>& intervals) {
        algo::IntervalTree intervalTree;
        for (const auto& interval : intervals) {
            int low = interval[0], high = interval[1];
            for (;;) {
                if (low == high && low == 3) {
                    int a = 5;
                }
                auto rem = intervalTree.search(algo::Interval{low, high});
                if (!rem) {
                    break;
                }
                low = std::min(low, rem->interval_.low_);
                high = std::max(high, rem->interval_.high_);
                intervalTree.remove(rem);
            }
            intervalTree.insert(algo::Interval{low, high});
            auto treeStr = intervalTree.print();
            std::cout << treeStr << std::endl;
        }
        std::vector<std::vector<int>> res;
        intervalTree.inorder([&](const algo::Interval& x) {
            res.emplace_back(std::vector<int>{x.low_, x.high_});
        });
        return res;
    }
};

bool test(const std::vector<std::vector<int>>& intervals, const std::vector<std::vector<int>>& expected) {
    Solution solution;
    auto actual = solution.merge(const_cast<std::vector<std::vector<int>>&>(intervals));
    return actual == expected;
}

bool testIntervalTree() {
    std::vector<std::tuple<std::vector<std::vector<int>>, std::vector<std::vector<int>>>> data;
    {
        std::vector<std::vector<int>> intervals;
        std::vector<std::vector<int>> expected;
        data.emplace_back(intervals, expected);
    }
    {
        std::vector<std::vector<int>> intervals{{1, 3}, {2, 6}, {8, 10}, {15, 18}};
        std::vector<std::vector<int>> expected{{1, 6}, {8, 10}, {15, 18}};
        data.emplace_back(intervals, expected);
    }
    {
        std::vector<std::vector<int>> intervals{{1, 4}, {4, 5}};
        std::vector<std::vector<int>> expected{{1, 5}};
        data.emplace_back(intervals, expected);
    }
    {
        std::vector<std::vector<int>> intervals{{2, 3}, {5, 5}, {2, 2}, {3, 4}, {3, 4}};
        std::vector<std::vector<int>> expected{{2, 4}, {5, 5}};
        data.emplace_back(intervals, expected);
    }
    {
        std::vector<std::vector<int>> intervals{{2, 3}, {4, 5}, {6, 7}, {8, 9}, {1, 10}};
        std::vector<std::vector<int>> expected{{1, 10}};
        data.emplace_back(intervals, expected);
    }
    {
        std::vector<std::vector<int>> intervals{{1, 3}, {0, 2}, {2, 3}, {4, 6}, {4, 5}, {5, 5}, {0, 2}, {3, 3}};
        std::vector<std::vector<int>> expected{{0, 3}, {4, 6}};
        data.emplace_back(intervals, expected);
    }
    {
        std::vector<std::vector<int>> intervals{{9,11},{430,435},{56,56},{323,330},{47,51},{354,358},{194,202},{286,290},{149,158},{121,127},{208,212},{271,278},{69,78},{33,33},{359,360},{386,394},{84,90},{175,177},{78,79},{241,248},{267,272},{164,165},{113,115},{107,112},{384,392},{291,293},{204,207},{231,234},{352,356},{96,100},{77,79},{284,287},{150,152},{5,5},{163,171},{409,409},{193,196},{243,250},{228,228},{274,276},{78,83},{56,60},{480,489},{259,264},{255,260},{249,251},{189,194},{198,199},{197,202},{123,123},{154,157},{142,149},{106,111},{6,10},{235,235},{298,303},{346,352},{299,307},{345,346},{363,363},{266,268},{433,441},{350,353},{499,506},{38,41},{408,410},{156,156},{392,396},{436,444},{301,304},{31,32},{41,48},{158,160},{407,410},{103,104},{104,106},{235,244},{30,35},{372,373},{133,133},{4,7},{455,457},{443,450},{479,480},{245,247},{255,261},{83,91},{5,6},{340,343},{97,101},{36,37},{166,167},{442,448},{357,363},{77,79},{428,432},{235,238},{298,306},{230,237},{262,270},{409,418},{456,459},{17,21},{86,93},{79,82}};
        std::vector<std::vector<int>> expected{{4,11},{17,21},{30,35},{36,37},{38,51},{56,60},{69,93},{96,101},{103,112},{113,115},{121,127},{133,133},{142,160},{163,171},{175,177},{189,202},{204,207},{208,212},{228,228},{230,251},{255,278},{284,290},{291,293},{298,307},{323,330},{340,343},{345,363},{372,373},{384,396},{407,418},{428,450},{455,459},{479,489},{499,506}};
        data.emplace_back(intervals, expected);
    }
    {
        std::vector<std::vector<int>> intervals{{260,266},{263,268},{411,411},{467,473},{259,263},{263,267},{302,307},{409,413},{462,468},{489,490},{490,492},{356,357},{467,474},{450,451},{337,344},{263,270},{455,463},{462,468},{298,307},{135,143},{436,436},{112,112},{316,320},{16,16},{230,235},{411,411},{329,329},{183,191},{29,32},{399,406},{43,45},{464,472},{392,399},{365,366},{208,210},{101,107},{102,107},{439,439},{439,444},{253,254},{313,319},{241,243},{136,142},{291,297},{262,262},{385,390},{205,214},{303,310},{128,128},{140,149},{224,225},{37,46},{307,313},{301,309},{432,436},{120,127},{71,74},{239,248},{490,492},{77,79},{425,427},{409,409},{312,316},{79,81},{437,439},{333,334},{165,166},{192,194},{347,351},{71,72},{48,51},{228,237},{256,256},{179,187},{202,210},{202,209},{71,72},{109,114},{28,37},{27,32},{113,114},{478,486},{232,233},{103,104},{165,171},{63,68},{69,72},{185,190},{463,469},{276,282},{324,332},{293,300},{442,447},{482,482},{444,446},{287,296},{223,228},{187,194},{436,436},{90,93},{316,319},{19,24}};
        std::vector<std::vector<int>> expected{{16,16},{19,24},{27,46},{48,51},{63,68},{69,74},{77,81},{90,93},{101,107},{109,114},{120,127},{128,128},{135,149},{165,171},{179,194},{202,214},{223,237},{239,248},{253,254},{256,256},{259,270},{276,282},{287,320},{324,332},{333,334},{337,344},{347,351},{356,357},{365,366},{385,390},{392,406},{409,413},{425,427},{432,436},{437,447},{450,451},{455,474},{478,486},{489,492}};
        data.emplace_back(intervals, expected);
    }
    {
        std::vector<std::vector<int>> intervals{{93,94},{173,192},{459,470},{211,212},{280,287},{37,38},{223,223},{404,413},{448,462},{265,267},{58,67},{269,272},{126,136},{403,422},{360,364},{229,242},{437,452},{164,180},{258,273},{318,328},{163,170},{111,129},{387,394},{159,175},{421,425},{250,269},{25,37},{195,198},{202,220},{482,496},{394,398},{219,230},{171,175},{228,237},{259,277},{319,333},{105,107},{364,376},{150,153},{180,183},{300,311},{255,260},{303,305},{329,346},{400,403},{272,278},{227,230},{477,495},{227,244},{179,198},{307,325},{465,477},{281,291},{257,260},{353,370},{58,71},{220,233},{331,334},{115,115},{233,240},{175,180},{13,28},{200,203},{5,13},{252,257},{119,119},{183,187},{324,340},{266,267},{451,451},{371,381},{125,136},{323,331},{467,478},{320,320},{158,174},{57,61},{251,268},{219,235},{37,48},{293,309},{31,40},{93,109},{145,164},{289,298},{331,331},{459,468},{252,266},{117,128},{425,443},{271,287},{286,294},{460,469},{437,456},{145,160},{2,13},{483,497},{120,136},{242,259},{436,439},{66,85},{44,49},{428,436},{12,18},{219,228},{336,338},{225,227},{170,187}};
        std::vector<std::vector<int>> expected{{2,49},{57,85},{93,109},{111,136},{145,198},{200,346},{353,381},{387,398},{400,497}};
        data.emplace_back(intervals, expected);
    }
    for (int i = 0; i < data.size(); i++) {
        std::cout << "Test case " << i + 1 << std::endl;
        if (!test(std::get<0>(data[i]), std::get<1>(data[i]))) {
            std::cout << "Failed test case " << i + 1 << std::endl;
            return false;
        } else {
            std::cout << "Passed test case " << i + 1 << std::endl;
        }
    }
    return true;
}

}  // namespace test