#include <ace-vm/object.hpp>

Object::Object(int size)
    : m_size(size),
      m_members(new Member[size])
{
}

Object::Object(const Object &other)
    : m_size(other.m_size),
      m_members(new Member[other.m_size])
{
    // copy all members
    for (int i = 0; i < m_size; i++) {
        m_members[i] = other.m_members[i];
    }
}

Object::~Object()
{
    delete[] m_members;
}

Object &Object::operator=(const Object &other)
{
    if (m_size != other.m_size) {
        // size is not equal, so delete the members and resize
        if (m_members != nullptr) {
            delete[] m_members;
        }

        m_size = other.m_size;
        m_members = new Member[other.m_size];
    }

    // copy all members
    for (int i = 0; i < m_size; i++) {
        m_members[i] = other.m_members[i];
    }

    return *this;
}
