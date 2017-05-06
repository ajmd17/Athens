#ifndef ARRAY_HPP
#define ARRAY_HPP

#include <common/my_assert.hpp>

#include <ace-vm/Value.hpp>

namespace ace {
namespace vm {

class Array {
public:
    Array(int size = 0);
    Array(const Array &other);
    ~Array();

    Array &operator=(const Array &other);
    inline bool operator==(const Array &other) const { return this == &other; }

    inline int GetSize() const { return m_size; }
    inline Value &AtIndex(int index) { return m_buffer[index]; }
    inline const Value &AtIndex(int index) const { return m_buffer[index]; }
    inline void AtIndex(int index, const Value &value) { m_buffer[index] = value; }

    void Push(const Value &value);
    void PushMany(size_t n, Value **values);
    void Pop();

private:
    int m_size;
    int m_capacity;
    Value *m_buffer;
};

} // namespace vm
} // namespace ace

#endif