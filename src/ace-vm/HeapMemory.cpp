#include <ace-vm/HeapMemory.hpp>
#include <iostream>

namespace ace {
namespace vm {

std::ostream &operator<<(std::ostream &os, const Heap &heap)
{
    HeapNode *tmp_head = heap.m_head;
    while (tmp_head) {
        os  << tmp_head->value.GetId() << '\t'
            << tmp_head->value.GetFlags() << '\t'
            << '\n';

        tmp_head = tmp_head->before;
    }
    return os;
}

Heap::Heap()
    : m_head(nullptr),
      m_num_objects(0)
{
}

Heap::~Heap()
{
    // purge the heap on destructor
    Purge();
}

void Heap::Purge()
{
    // clean up all allocated objects
    while (m_head) {
        HeapNode *tmp = m_head;
        m_head = tmp->before;
        delete tmp;

        m_num_objects--;
    }
}

HeapValue *Heap::Alloc()
{
    HeapNode *node = new HeapNode;

    if (m_head) {
        m_head->after = node;
    }
    
    node->before = m_head;
    m_head = node;

    m_num_objects++;

    return &m_head->value;
}

void Heap::Sweep()
{
    HeapNode *last = m_head;
    while (last) {
        if (!(last->value.GetFlags() & GC_MARKED)) {
            // unmarked object, so delete it

            HeapNode *after = last->after;
            HeapNode *before = last->before;

            if (before) {
                before->after = after;
            }

            if (after) {
                // removing an item from the middle, so
                // make the nodes to the other sides now
                // point to each other
                after->before = before;
            } else {
                // since there are no nodes after this,
                // set the head to be this node here
                m_head = before;
            }

            // delete the middle node
            delete last;
            last = before;

            // decrement number of currently allocated
            // objects
            m_num_objects--;

        } else {
            // the object is currently marked, so
            // we unmark it for the next time
            last->value.GetFlags() &= ~GC_MARKED;
            last = last->before;
        }
    }
}

} // namespace vm
} // namespace ace
