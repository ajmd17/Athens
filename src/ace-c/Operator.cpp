#include <ace-c/Operator.hpp>

#include <array>

const std::map<std::string, Operator> Operator::operator_objects = {
    { "+", Operator(OP_add, 11, ARITHMETIC) },
    { "-", Operator(OP_subtract, 11, ARITHMETIC) },
    { "*", Operator(OP_multiply, 12, ARITHMETIC) },
    { "/", Operator(OP_divide, 12, ARITHMETIC) },
    { "%", Operator(OP_modulus, 12, ARITHMETIC) },

    // Bitwise operators
    { "^", Operator(OP_bitwise_xor, 6, BITWISE) },
    { "&", Operator(OP_bitwise_and, 7, BITWISE) },
    { "|", Operator(OP_bitwise_or, 5, BITWISE) },
    { "<<", Operator(OP_bitshift_left, 10, BITWISE) },
    { ">>", Operator(OP_bitshift_right, 10, BITWISE) },

    // Logical operators
    { "&&", Operator(OP_logical_and, 4, LOGICAL) },
    { "||", Operator(OP_logical_or, 3, LOGICAL) },

    // Comparison operators
    { "==", Operator(OP_equals, 8, COMPARISON) },
    { "!=", Operator(OP_not_eql, 8, COMPARISON) },
    { "<", Operator(OP_less, 9, COMPARISON) },
    { ">", Operator(OP_greater, 9, COMPARISON) },
    { "<=", Operator(OP_less_eql, 9, COMPARISON) },
    { ">=", Operator(OP_greater_eql, 9, COMPARISON) },

    // Assignment operators
    { "=", Operator(OP_assign, 2, ASSIGNMENT, true) },
    { "+=", Operator(OP_add_assign, 2, ASSIGNMENT | ARITHMETIC, true) },
    { "-=", Operator(OP_subtract_assign, 2, ASSIGNMENT | ARITHMETIC, true) },
    { "*=", Operator(OP_multiply_assign, 2, ASSIGNMENT | ARITHMETIC, true) },
    { "/=", Operator(OP_divide_assign, 2, ASSIGNMENT | ARITHMETIC, true) },
    { "%=", Operator(OP_modulus_assign, 2, ASSIGNMENT | ARITHMETIC, true) },
    { "^=", Operator(OP_bitwise_xor_assign, 2, ASSIGNMENT | BITWISE, true) },
    { "&=", Operator(OP_bitwise_and_assign, 2, ASSIGNMENT | BITWISE, true) },
    { "|=", Operator(OP_bitwise_or_assign, 2, ASSIGNMENT | BITWISE, true) },

    // Unary operators
    { "!", Operator(OP_logical_not, 0, LOGICAL) },
    { "-", Operator(OP_negative, 0, ARITHMETIC) },
    { "+", Operator(OP_positive, 0, ARITHMETIC) },
    { "~", Operator(OP_bitwise_complement, 0, BITWISE) },
    { "++", Operator(OP_increment, 0, ASSIGNMENT | ARITHMETIC, true) },
    { "--", Operator(OP_decrement, 0, ASSIGNMENT | ARITHMETIC, true) }
};

Operator::Operator(Operators op_type,
    int precedence,
    int type,
    bool modifies_value)
    : m_op_type(op_type),
      m_precedence(precedence),
      m_type(type),
      m_modifies_value(modifies_value)
{
}

Operator::Operator(const Operator &other)
    : m_op_type(other.m_op_type),
      m_precedence(other.m_precedence),
      m_type(other.m_type),
      m_modifies_value(other.m_modifies_value)
{
}

