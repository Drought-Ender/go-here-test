#include "types.h"
#include "ObjectMgr.h"
#include "math.h"

// Adapted from https://github.com/woodfrog/FibonacciHeap/tree/master

namespace Drought {
template <typename T>
struct FibonacciNode {
    FibonacciNode(T obj) {
        mObject = obj;
        mPrev = this;
        mNext = this;
        mDegree = 0;
        mMark = false;
        mChild = nullptr;
        mParent = nullptr;
    }
    int mDegree; 
    FibonacciNode<T>* mParent; 
    FibonacciNode<T>* mChild; 
    FibonacciNode<T>* mPrev; 
    FibonacciNode<T>* mNext;
    T mObject;
    bool mMark;
    int mIndex;
};

template <typename T>
struct FibonacciEvalStruct
{
    virtual f32 eval(T) const = 0;
};


template <typename T>
struct FibonacciHeap
{
    typedef FibonacciNode<T> Node;
    Node* mRoot;
    int mNodeCount;
    
    const FibonacciEvalStruct<T>* mEval;

    FibonacciHeap(FibonacciEvalStruct<T>* func) : mEval(func) {
        mRoot = nullptr;
        mNodeCount = 0;
    }

    ~FibonacciHeap() {
        _clear(mRoot);
    }

    Node* insert(T object);
    void merge(FibonacciHeap<T>& other);

    T extractMin();

    void decreaseKey(Node* x, T key);
    void decreaseKey(Node* x);

    void deleteNode(Node* x);

    bool _compareKeys(T first, T second);

    Node* _create_node(T obj);
    void _insert_node(Node* node);
    void _remove_from_circular_list(Node* x);
    Node* _merge(Node* a, Node* b);
    void _make_child(Node* child, Node* parent);
    void _consolidate();
    void _unparent_all(Node* x);
    Node* _extract_min_node();
    void _decrease_key(Node* x, T key);
    void _cut(Node* x, Node* y);
    void _cascading_cut(Node* y);
    void _clear(Node* x);

    
    private:

    
};



template <typename T>
FibonacciNode<T>* FibonacciHeap<T>::insert(T key) {
    Node* node = _create_node(key);
    _insert_node(node);
    return node;
}

template <typename T>
void FibonacciHeap<T>::merge(FibonacciHeap<T>& other) {
    mRoot = _merge(mRoot, other.mRoot);
    mNodeCount += other.mNodeCount;
    // clear other for saftey
    other.mRoot      = nullptr;
    other.mNodeCount = 0;
}

template <typename T>
T FibonacciHeap<T>::extractMin() {
    Node* minNode = _extract_min_node();
    T ret = minNode->mObject;
    delete minNode;
    return ret;
}

template <typename T>
void FibonacciHeap<T>::decreaseKey(Node* x, T key) {
    _decrease_key(x, key);
}

template <typename T>
void FibonacciHeap<T>::decreaseKey(Node* x) {
    decreaseKey(x, x->mObject);
}


template <typename T>
void FibonacciHeap<T>::deleteNode(Node* node) {

}

template <typename T>
FibonacciNode<T>* FibonacciHeap<T>::_create_node(T key) {
    return new Node(key);
}


template <typename T>
void FibonacciHeap<T>::_insert_node(Node* node) {
    mRoot = _merge(mRoot, node);
    mNodeCount++;
}

template <typename T>
void FibonacciHeap<T>::_remove_from_circular_list(Node* node) {
    if (node->mNext == node) return;

    Node* prevSibling = node->mPrev;
    Node* nextSibling = node->mNext;

    prevSibling->mNext = nextSibling;
    nextSibling->mPrev = prevSibling;

    node->mNext = node->mPrev = node;
}

template <typename T>
bool FibonacciHeap<T>::_compareKeys(T first, T second) {
    return mEval->eval(first) > mEval->eval(second);
}


template <typename T>
FibonacciNode<T>* FibonacciHeap<T>::_merge(Node* a, Node* b) {
    if (!a) return b;
    if (!b) return a;

    if (_compareKeys(a->mObject, b->mObject)) {
        Node* temp = a;
        a = b;
        b = temp;
    }
    Node* aNext = a->mNext;
    Node* bPrev = b->mPrev;

    a->mNext = b;
    b->mPrev = a;

    aNext->mPrev = bPrev;
    bPrev->mNext = aNext;
    return a;
}



template <typename T>
FibonacciNode<T>* FibonacciHeap<T>::_extract_min_node() {
    Node* min = mRoot;
    if (!min) {
        return nullptr;
    }

    _unparent_all(min->mChild);
    _merge(min, min->mChild);

    if (min == min->mNext) {
        mRoot = nullptr;
    }
    else {
        mRoot = min->mNext;
    }
    _remove_from_circular_list(min);
    
    if (mRoot) {
        _consolidate();
    }

    mNodeCount--;

    return min;
}

template <typename T>
void FibonacciHeap<T>::_unparent_all(Node* x) {
    if (!x) return;

    Node* y = x;
    do {
        y->mParent = nullptr;
        y = y->mNext;
    } while (y != x);
}

#define GOLDEN_RATIO (1.618033988749)

template <typename T>
void FibonacciHeap<T>::_consolidate() {
    int Dn = (int)(log10(mNodeCount) / log10(GOLDEN_RATIO));

    Node** A = new Node*[Dn + 1];

    for (size_t i = 0; i < Dn + 1; i++) {
        A[i] = nullptr;
    }

    

    u32 baseNodeCount = 0;
    Node** allNodes = new Node*[mNodeCount];

    Node* node = mRoot;
    do {
        allNodes[baseNodeCount++] = node;
        node = node->mNext;
    } while (node != mRoot);

    for (u32 i = 0; i < baseNodeCount; i++) {
        Node* e = allNodes[i];
        int d = e->mDegree;
        _remove_from_circular_list(e);
        while (A[d])
        {
            Node* tmp = A[d];
            if (_compareKeys(e->mObject, tmp->mObject)) {
                Node* tmptmp = tmp;
                tmp = e;
                e = tmptmp;
            }
            _make_child(tmp, e);
            A[d++] = nullptr;
        }
        A[e->mDegree] = e;
        mRoot = e;
    }
    for (u32 i = 0; i < Dn + 1; i++) {
        if (A[i] && A[i] != mRoot) {
            _merge(mRoot, A[i]);
        }
    }
    Node* flag = mRoot;
    Node* iter = flag;
    do {
        if (_compareKeys(mRoot->mObject, iter->mObject)) {
            mRoot = iter;
        }
        iter = iter->mNext;
    } while (iter != flag);
    
    delete[] A;
    delete[] allNodes;
}

template <typename T>
void FibonacciHeap<T>::_make_child(Node* child, Node* parent) {
    child->mParent = parent;
    parent->mChild = _merge(parent->mChild, child);
    parent->mDegree++;
    child->mMark = false;
}

template <typename T>
void FibonacciHeap<T>::_decrease_key(Node* node, T key) {
    node->mObject = key;
    Node* y = node->mParent;

    if (y && _compareKeys(y->mObject, node->mObject)) {
        _cut(x, y);
        _cascading_cut(y);
    }

    if (_compareKeys(mRoot->mObject, node->mObject)) {
        mRoot = node;
    }
}

template <typename T>
void FibonacciHeap<T>::_cut(Node* x, Node* y) {
    y->mChild = (x == x->mNext) ? nullptr : x->mNext;
    _remove_from_circular_list(x);
    y->mDegree--;
    _merge(mRoot, x);
    
    x->mParent = nullptr;
    x->mMark   = false;
}

template <typename T>
void FibonacciHeap<T>::_cascading_cut(Node* y) {
    Node* z = y->mParent;
    if (z) {
        if (!y->mMark) {
            y->mMark = true;
        }
        else {
            _cut(y, z);
            _cascading_cut(z);
        }
    }
}

template <typename T>
void FibonacciHeap<T>::_clear(Node* x) {
    if (x) {
        Node* t1 = x;
        do {
            Node* t2 = t1;
            t1 = t1->mNext;
            _clear(t2->mChild);
            delete t2;
        } while (t1 != x);
    }
}

} // namespace Drought