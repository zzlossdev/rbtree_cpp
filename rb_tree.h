#ifndef __RB_TREE_H__
#define __RB_TREE_H__

#include <cstdio>
#include <functional>
#include <string>

using namespace std;

enum RbNodeColor {
  Black = 0,
  Red = 1,
};

enum RbNodeDirection {
  NullChild = false, // node root is nobody's child
  LeftChild = false,
  RightChild = true,
};

template <class T> class RbNode : public T {
public:
  template <class... Args> RbNode(Args... args) : T{args...} {}

  void setNodeColor(RbNodeColor color) { addr_ = (addr_ & ~1) | color; }

  void setNodeChildColor(RbNodeDirection di, RbNodeColor color) {
    if (childs_[di] != nullptr)
      childs_[di]->setNodeColor(color);
  }

  bool isNodeColor(RbNodeColor color) { return (addr_ & 1) == color; }

  RbNodeColor getNodeColor() { return static_cast<RbNodeColor>(addr_ & 1); }

  RbNodeDirection getNodeDirection(RbNode<T> *parent) {
    return parent->childs_[LeftChild] == this ? LeftChild : RightChild;
  }

  void inheritNodeParent(RbNode<T> *node, RbNode<T> **root) {
    addr_ = node->addr_;
    auto *parent = node->getNodeParent();
    if (parent) {
      auto di = node->getNodeDirection(parent);
      parent->childs_[di] = this;
    } else {
      *root = this;
    }
  }

  // root will be updated
  void setNodeParent(RbNode<T> *parent, RbNodeColor color,
                     RbNode<T> **root = nullptr) {
    addr_ = reinterpret_cast<unsigned long>(parent);
    addr_ |= color;
    if (parent == nullptr && root != nullptr) {
      *root = this;
    }
  }

  void hookOldNodeChild(RbNode<T> *child, RbNodeDirection di) {
    childs_[di] = child;
    if (child) {
      child->addr_ = (child->addr_ & 1) | reinterpret_cast<unsigned long>(this);
    }
  }

  void setNodeChildWithoutColor(RbNode<T> *child, RbNodeDirection di) {
    childs_[di] = child;
    if (child) { // update child's parent
      child->addr_ = (child->addr_ & 1) | reinterpret_cast<unsigned long>(this);
    }
  }

  void setNodeChild(RbNode<T> *child, RbNodeDirection di,
                    RbNodeColor color = Black) {
    childs_[di] = child;
    if (child) { // update child's parent
      child->setNodeParent(this, color);
    }
  }

  RbNode<T> **getNodeChilds() { return childs_; }

  RbNode<T> *getNodeChild(RbNodeDirection di) { return childs_[di]; }

  RbNode<T> *getNodeParent() {
    return reinterpret_cast<RbNode<T> *>(addr_ & ~1);
  }

  RbNodeColor getChildColor(RbNodeDirection di) {
    auto child = childs_[di];
    if (!child)
      return Black;
    else
      return child->getNodeColor();
  }

  RbNode<T> *getNodeChildWithColor(RbNodeDirection di, RbNodeColor &color) {
    auto child = childs_[di];
    if (!child) {
      color = Black;
    } else {
      color = child->getNodeColor();
    }
    return child;
  }

  RbNode<T> *getTheOtherChildOfColor(RbNodeDirection di, RbNodeColor color) {
    di = static_cast<RbNodeDirection>(!di);
    auto child = childs_[di];

    if (!child || color != child->getNodeColor())
      return nullptr;
    else
      return child;
  }

  /*
   * Rotate from child(itself), so `di` is same direction with child:
   * left child go left, right child go right.
   * NOTE:
   * first: `parent` & `di` are handy, we don't need to recompute
   *   them from `this`
   * second: if we call this->InheritNodeParent() before
   *   this->RotateWithParent, then this->u_.parts.direction
   *   is changed already
   * `color` is the node's color, we already known, and will set it
   * to `parent`
   */
  void rotateWithParent(RbNode<T> *parent, RbNodeDirection di,
                        RbNodeColor color) {
    auto other = static_cast<RbNodeDirection>(!di);

    // both sides share the same pattern
    parent->setNodeChildWithoutColor(getNodeChild(other), di);
    setNodeChild(parent, other, color);
  }

private:
  unsigned long addr_;
  RbNode<T> *childs_[2] = {nullptr, nullptr};
};

template <class T, class Key> class RbTree {
public:
  RbTree(RbNode<T> *root = nullptr) : root_{root} {}
  RbTree &insertNode(RbNode<T> *node);
  RbTree &deleteNode(RbNode<T> *node);
  // return the found node or nullptr if non-exist
  RbNode<T> *search(Key key);

  /*
   * TODO:
   *  DFS(Depth First Traveral): Inorder, Preorder, Postorder
   *    recursive version & iterative version
   *  BFS(Breadth First Traversal): aka Level order Traversal
   *  Boundary Traveral
   *  Diagonal Traveral
   *  Zigzag(Spiral) Traversal
   */
  void traversalPreorder(RbNode<T> *node, function<void(RbNode<T> *)> func) {
    if (node != nullptr) {
      func(node);
      traversalPreorder(node->getNodeChild(LeftChild), func);
      traversalPreorder(node->getNodeChild(RightChild), func);
    }
  }
  void traversalPostorder(RbNode<T> *node, function<void(RbNode<T> *)> func) {
    if (node != nullptr) {
      traversalPostorder(node->getNodeChild(LeftChild), func);
      traversalPostorder(node->getNodeChild(RightChild), func);
      func(node);
    }
  }

  void traversalInorder(RbNode<T> *node, function<void(RbNode<T> *)> func) {
    if (node != nullptr) {
      traversalInorder(node->getNodeChild(LeftChild), func);
      func(node);
      traversalInorder(node->getNodeChild(RightChild), func);
    }
  }

  /*
   * NOTE: need impl a version of pyramid-sytle dump to output
   */
  void dumpTree() {
    traversalPreorder(root_, [](RbNode<T> *node) {
      auto p = node->getNodeParent();
      auto l = node->getNodeChild(LeftChild);
      auto r = node->getNodeChild(RightChild);
      printf("%c(%d): p: %s, l: %s, r: %s\n",
             node->isNodeColor(Black) ? 'B' : 'R', node->get(),
             p ? to_string(p->get()).c_str() : "nil",
             l ? to_string(l->get()).c_str() : "nil",
             r ? to_string(r->get()).c_str() : "nil");
    });
  }

  bool verifyProperties(RbNode<T> *node, int *blackCount,
                        int currentBlackCount);

  bool verifyProperties();

  bool verifyBST() {
    function<bool(RbNode<T> *, RbNode<T> *, RbNode<T> *)> checker =
        [&](RbNode<T> *node, RbNode<T> *left, RbNode<T> *right) {
          if (node != nullptr) {
            if (left != nullptr && less<T>{}(*node, *left))
              return false;
            if (right != nullptr && less<T>{}(*right, *node))
              return false;
            return checker(node->getNodeChild(RbNodeDirection::LeftChild), left,
                           node) &&
                   checker(node->getNodeChild(RbNodeDirection::RightChild),
                           node, right);
          } else
            return true;
        };

    return checker(root_, nullptr, nullptr);
  }

  bool verifyTree() {
    auto count = InitialBlackCounter;

    if (root_ == nullptr)
      return true;
    if (root_->isNodeColor(Red))
      return false;

    return verifyProperties(root_, &count, 0) && verifyBST();
  }

private:
  const static int InitialBlackCounter = -1;
  void insertRebalance(RbNode<T> *node);
  void deleteRebalance(RbNode<T> *node, RbNodeDirection di);
  RbNode<T> *root_;
};
#endif
