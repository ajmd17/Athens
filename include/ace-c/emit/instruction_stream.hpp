#ifndef INSTRUCTION_STREAM_HPP
#define INSTRUCTION_STREAM_HPP

#include <ace-c/emit/instruction.hpp>
#include <ace-c/emit/static_object.hpp>

#include <vector>
#include <ostream>
#include <cstdint>

class InstructionStream {
    friend std::ostream &operator<<(std::ostream &os, InstructionStream instruction_stream);
public:
    InstructionStream();
    InstructionStream(const InstructionStream &other);

    inline size_t GetPosition() const { return m_position; }

    inline const std::vector<Instruction<>> &GetData() const { return m_data; }

    inline uint8_t GetCurrentRegister() const { return m_register_counter; }
    inline void IncRegisterUsage() { m_register_counter++; }
    inline void DecRegisterUsage() { m_register_counter--; }

    inline int GetStackSize() const { return m_stack_size; }
    inline void IncStackSize() { m_stack_size++; }
    inline void DecStackSize() { m_stack_size--; }

    inline int NewStaticId() { return m_static_id++; }

    inline void AddStaticObject(const StaticObject &static_object) { m_static_objects.push_back(static_object); }
    inline int FindStaticObject(const StaticObject &static_object)
    {
        for (const StaticObject &so : m_static_objects) {
            if (so == static_object) {
                return so.m_id;
            }
        }
        // not found
        return -1;
    }

    InstructionStream &operator<<(const Instruction<> &instruction);

private:
    size_t m_position;
    std::vector<Instruction<>> m_data;
    // incremented and decremented each time a register
    // is used/unused
    uint8_t m_register_counter;
    // incremented each time a variable is pushed,
    // decremented each time a stack frame is closed
    int m_stack_size;
    // the current static object id
    int m_static_id;

    std::vector<StaticObject> m_static_objects;
};

#endif
