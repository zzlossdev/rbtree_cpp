#include "rb_tree.h"
#include <stack>

using namespace std;

template <class T, class Key>
RbTree<T, Key> &RbTree<T, Key>::insertNode(RbNode<T> *node) {
  RbNode<T> *parent = nullptr, *p;
  RbNodeDirection di = LeftChild;

  p = root_;

  while (p) {
    parent = p;
    di = static_cast<RbNodeDirection>(less<T>{}(*p, *node));
    p = p->getNodeChild(di);
  }

  if (parent) {
    parent->setNodeChild(node, di, Red);
    insertRebalance(node);
  } else { // root node
    node->setNodeParent(nullptr, Black, &root_);
  }

#ifdef _TC_ENABLE
  if (!verifyTree())
    dumpTree();
#endif

  return *this;
}

template <class T, class Key>
void RbTree<T, Key>::insertRebalance(RbNode<T> *node) {
  RbNode<T> *p, *gp;
  RbNodeDirection nd, pd;

  while (true) {
    p = node->getNodeParent();

    if (p == nullptr) { // node is root
      // recursive routine may set root to Red
      node->setNodeParent(nullptr, Black, &root_);
      break;
    }

    if (p->getNodeColor() == Black) { // done
      break;
    }

    gp = p->getNodeParent();

    nd = node->getNodeDirection(p);
    pd = p->getNodeDirection(gp);
    auto uncle = gp->getTheOtherChildOfColor(pd, Red);
    if (uncle) {
      /*
       * case 1a: B(g)           R(g)
       *        /   \          /   \
       *      R(p)  R(u) ->  B(p)  B(u)
       *      /              /
       *    R(n)           R(n)
       *
       * case 1b: B(g)           R(g)
       *        /   \          /   \
       *      R(u)  R(p) ->  B(u)  B(p)
       *             \              \
       *             R(n)           R(n)
       *
       * case 1c: B(g)           R(g)
       *        /   \          /   \
       *      R(p)  R(u) ->  B(p)  B(u)
       *        \              \
       *       R(n)           R(n)
       *
       * case 1d: B(g)           R(g)
       *        /   \          /   \
       *      R(u)  R(p) ->  B(u)  B(p)
       *            /              /
       *           R(n)           R(n)
       *
       * black node descending, we should move to grand parent to
       * resolve potential violations
       */
      p->setNodeColor(Black);
      uncle->setNodeColor(Black);
      node = gp;
      node->setNodeColor(Red);
      continue;
    } else {
      if (nd != pd) {
        /*
         * case 2a: B(g)           B(g)
         *        /   \          /   \
         *      R(p)  B(u) ->  R(n)  B(u)
         *        \            /
         *       R(n)        R(p)
         *
         * case 2b: B(g)           B(g)
         *        /   \          /   \
         *      B(u)  R(p) ->  B(u)  R(n)
         *            /               \
         *          R(n)              R(p)
         *
         * interchange node and parent then pass it to case 3a or 3b
         */
        node->rotateWithParent(p, nd, Red);

        // flip direction for case 3a or 3b
        nd = pd;
        p = node;
      }

      /*
       * case 3a: B(g)           B(p)
       *        /   \          /   \
       *      R(p)  B(u) ->  R(n)  R(g)
       *      /                     \
       *    R(n)                    B(u)
       *
       * case 3b: B(g)           B(p)
       *        /   \          /   \
       *      B(u)  R(p) ->  R(g)  R(n)
       *             \       /
       *             R(n)  B(u)
       * to keep the counts of black node on each direction,
       * we need to rotate at grand parent
       */

      p->inheritNodeParent(gp, &root_); // Parent | Direction | Color

      p->rotateWithParent(gp, nd, Red);

      break;
    }
  }
}

template <class T, class Key>
RbTree<T, Key> &RbTree<T, Key>::deleteNode(RbNode<T> *node) {
  RbNode<T> *fix = nullptr;
  auto left = node->getNodeChild(LeftChild),
       right = node->getNodeChild(RightChild);
  auto di = LeftChild;

  if (left == nullptr || right == nullptr) {
    /*
     * case 1: if node only have one child, then the child is red
     * and node itself is black. as long as we change the child to
     * black, and take over the node, then we break nothing
     */
    if (left) { // case 1a
      left->inheritNodeParent(node, &root_);
    } else if (right) { // case 1b
      right->inheritNodeParent(node, &root_);
    } else {
      auto p = node->getNodeParent();
      if (p != nullptr) {
        di = node->getNodeDirection(p);
        p->setNodeChild(nullptr, di);
        if (node->isNodeColor(Black)) {
          fix = p;
        }
      } else {
        root_ = nullptr;
      }
    }
  } else {
    RbNode<T> *farLeft, *nearRight, *x = right;

    do {
      farLeft = x;
    } while ((x = x->getNodeChild(LeftChild)));

    nearRight = farLeft->getNodeChild(RightChild);
    auto needFix = farLeft->isNodeColor(Black) && nearRight == nullptr;

    if (farLeft != right) {
      /*
       * case 3a:  X(n)        X(s)
       *          /  \        /  \
       *         l    r  ->  l   r
       *            /           /
       *           x           x
       *          /
       *        Y(s)
       *         \
       *         nil
       * if Y == Black, then we should reblance the tree from x
       *
       * case 3b:  X(n)         X(s)
       *          /  \         /  \
       *         l   r        l    r
       *           /              /
       *          x      ->      x
       *         /              /
       *      B(s)            B(ss)
       *         \
       *         R(ss)
       * we let s inherit n, ss change to black, then
       * there's no harm on all pathes
       */

      fix = farLeft->getNodeParent();
      fix->setNodeChild(nearRight, LeftChild, Black);
      farLeft->hookOldNodeChild(right, RightChild);
    } else {
      /*
       * case 4a:  X(n)          X(s)
       *          /   \         /  \
       *         l    Y(s) ->  l   nil
       *             / \
       *           nil  nil
       * if Y == Black, then we should reblance the tree from x
       *
       * case 4b:  X(n)           X(s)
       *          /   \          /   \
       *         l   B(s)  ->   l    B(r)
       *             / \
       *           nil  R(r)
       */
      fix = farLeft;
      di = RightChild;
      if (nearRight) {
        nearRight->setNodeColor(Black);
      }
    }

    if (!needFix) {
      fix = nullptr;
    }

    farLeft->inheritNodeParent(node, &root_);
    farLeft->hookOldNodeChild(left, LeftChild);
  }

  if (fix != nullptr) {
    deleteRebalance(fix, di);
  }

#ifdef _TC_ENABLE
  if (!verifyTree())
    dumpTree();
#endif
  return *this;
}

template <class T, class Key>
void RbTree<T, Key>::deleteRebalance(RbNode<T> *parent, RbNodeDirection nd) {
  RbNode<T> *node;
  while (true) {
    auto sd = static_cast<RbNodeDirection>(!nd);
    RbNodeColor color;
    /*
     * sibling always exist in the loop, because node is black and
     * one black shorter than sibling side
     */
    auto s = parent->getNodeChildWithColor(sd, color);

    if (color == Red) {
      /*
       * case 1a:    B(p)               B(s)
       *            /   \              /   \
       *         B(n)  R(s)    ->    R(p)  B(r)
       *              /  \           /  \
       *            B(l) B(r)      B(n) B(l) <- this is the new sibling
       *
       * case 1b:
       *          B(p)              B(s)
       *         /   \             /   \
       *       R(s)  B(n)   ->  B(l)  R(p)
       *       /  \                  /  \
       *     B(l) B(r)             B(r) B(n)
       *
       * shift a red node to the other branch, harmless
       * turn case 1a -> case 2a, case 1b -> case 2b
       */
      s->inheritNodeParent(parent, &root_);
      s->rotateWithParent(parent, sd, Red);
      s = parent->getNodeChild(sd);
    }

    auto sc = s->getNodeChilds();
    RbNodeColor scc[2] = {
        sc[LeftChild] == nullptr ? Black : sc[LeftChild]->getNodeColor(),
        sc[RightChild] == nullptr ? Black : sc[RightChild]->getNodeColor(),
    };

    if (scc[LeftChild] == Black && scc[RightChild] == Black) {
      /*
       * case 2a:    X(p)               X(p) <- this is the new node
       *            /   \              /   \
       *         B(n)  B(s)    ->   B(n)   R(s) <- turn sibling's color to red
       *               / \                / \
       *            B(l) B(r)          B(l) B(r)
       *
       * case 2b:   X(p)               X(p)
       *           /   \              /   \
       *        B(s)   B(n)   ->    R(s)  B(n)
       *        /  \               /  \
       *     B(l) B(r)          B(l) B(r)
       *
       * sibling side reduce a black counter, then both sides are even
       */
      s->setNodeColor(Red);
      node = parent;
      if (node != root_ && node->isNodeColor(RbNodeColor::Black)) {
        nd = node->getNodeDirection(node->getNodeParent());
        parent = node->getNodeParent();
        continue;
      } else {
        break;
      }
    } else {
      if (scc[sd] == Black) {
        /*
         * case 3a:   X(p)            X(p)
         *           /  \            /   \
         *        B(n)  B(s)   ->  B(n)  B(l) <- new sibling
         *             / \                \
         *          R(l) B(r)             R(s)
         *                                 \
         *                                 B(r)
         *
         * case 3a:   X(p)            X(p)
         *           /  \            /   \
         *        B(s)  B(n)   ->  B(r)  B(n)
         *        / \              /
         *     B(l) R(r)         R(s)
         *                       /
         *                     B(l)
         */
        sc[nd]->inheritNodeParent(s, &root_);
        sc[nd]->rotateWithParent(s, nd, Red);
        s = s->getNodeParent();
      }
      /*
       * case 4a:  X(p)              X(s)
       *          /   \             /   \
       *       B(n)   B(s)   ->  B(p)  B(r)
       *             / \         /  \
       *          X(l) R(r)    B(n) X(l)
       *
       * case 4b:  X(p)              X(s)
       *          /   \             /   \
       *       B(s)   B(n)   ->  B(l)  B(p)
       *       / \                     / \
       *    R(l) X(r)               X(r) B(n)
       *
       * both sides are even, next node is the root
       */
      s->inheritNodeParent(parent, &root_);
      s->rotateWithParent(parent, sd, Black);
      s->setNodeChildColor(sd, Black);
      node = root_;
      break;
    }
  }
  node->setNodeColor(Black);
}

template <class T, class Key> bool RbTree<T, Key>::verifyProperties() {
  if (root_ == nullptr)
    return true;
  if (!root_->isNodeColor(Black))
    return false;

  stack<pair<RbNode<T> *, int>> s;
  s.push({root_, 0});
  int pathBlackCount = InitialBlackCounter;

  while (!s.empty()) {
    auto [node, blackCount] = s.top();
    s.pop();

    if (node == nullptr) {
      if (pathBlackCount == InitialBlackCounter) {
        pathBlackCount = blackCount;
      } else if (pathBlackCount != blackCount) {
        return false;
      }
      continue;
    }

    if (node->isNodeColor(Red)) {
      if (node->getChildColor(LeftChild) == Red ||
          node->getChildColor(RightChild) == Red) {
        return false;
      }
    } else {
      blackCount++;
    }

    auto childs = node->getNodeChilds();
    if (childs[RightChild] != nullptr)
      s.push({childs[RightChild], blackCount});
    if (childs[LeftChild] != nullptr)
      s.push({childs[LeftChild], blackCount});
  }

  return true;
}

template <class T, class Key>
bool RbTree<T, Key>::verifyProperties(RbNode<T> *node, int *blackCount,
                                      int currentBlackCount) {

  if (node == nullptr) {
    if (*blackCount == InitialBlackCounter) {
      *blackCount = currentBlackCount;
    }
    return *blackCount == currentBlackCount;
  }

  if (node->isNodeColor(Red)) {
    if (node->getChildColor(LeftChild) == Red ||
        node->getChildColor(RightChild) == Red)
      return false;
  } else {
    currentBlackCount++;
  }

  return verifyProperties(node->getNodeChild(LeftChild), blackCount,
                          currentBlackCount) &&
         verifyProperties(node->getNodeChild(RightChild), blackCount,
                          currentBlackCount);
}

#ifdef _TC_ENABLE

#include "file_stream.h"
#include "testcase.h"
#include <iostream>

namespace {
class Test {
public:
  Test() {}
  Test(int key) : key_{key} {}

  int get() { return key_; }

  void set(int key) { key_ = key; }

  bool operator<(const Test &other) const { return key_ < other.key_; }

private:
  int key_;
};

class TestcaseRbTree : public TestcaseBase {
public:
  virtual void testRoutine() override {
    RbTree<Test, int> tree;
    RbNode<Test> a{1}, b{3}, c{8}, d{6}, e{5}, f{10}, g{-1}, h{158}, i{10},
        j{166}, k{-56}, l{28}, m{-158};

    int array[1000];
    RbNode<Test> nodes[1000];

    FileStream fs("/dev/urandom");

    tree.insertNode(&a)
        .insertNode(&b)
        .insertNode(&c)
        .insertNode(&d)
        .insertNode(&e)
        .insertNode(&f)
        .insertNode(&g)
        .insertNode(&h)
        .insertNode(&j)
        .insertNode(&k)
        .insertNode(&i)
        .insertNode(&m)
        .insertNode(&l);

    tree.dumpTree();

    tree.deleteNode(&h)
        .deleteNode(&i)
        .deleteNode(&g)
        .deleteNode(&l)
        .deleteNode(&f)
        .deleteNode(&e)
        .deleteNode(&a)
        .deleteNode(&b)
        .deleteNode(&c)
        .deleteNode(&m)
        .deleteNode(&d)
        .deleteNode(&j)
        .deleteNode(&k);

    fs.loadBlocks(array, 0, 1000);

    for (int i = 0; i < 1000; i++) {
      nodes[i].set(array[i]);
      tree.insertNode(&nodes[i]);
    }

    if (!tree.verifyTree()) {
      tree.dumpTree();
    } else {
      std::cout << "red black verified!" << endl;
    }

    for (int i = 0; i < 1000; i++) {
      tree.deleteNode(&nodes[i]);
    }
  }
};
} // namespace
INIT_CASE(TestcaseRbTree)
#endif
