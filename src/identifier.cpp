#include <athens/identifier.h>

Identifier::Identifier(const std::string &name, int index, int flags)
    : m_name(name),
      m_index(index),
      m_stack_location(0),
      m_usecount(0),
      m_flags(flags)
{
}

Identifier::Identifier(const Identifier &other)
    : m_name(other.m_name),
      m_index(other.m_index),
      m_stack_location(other.m_stack_location),
      m_usecount(other.m_usecount),
      m_flags(other.m_flags),
      m_current_value(other.m_current_value)
{
}