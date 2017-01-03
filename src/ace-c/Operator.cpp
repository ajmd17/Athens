#include <ace-c/Operator.hpp>

#include <array>

const Operator
    // Arithmetic operators
    Operator::operator_add("+", 11, ARITHMETIC),
    Operator::operator_subtract("-", 11, ARITHMETIC),
    Operator::operator_multiply("*", 12, ARITHMETIC),
    Operator::operator_divide("/", 12, ARITHMETIC),
    Operator::operator_modulus("%", 12, ARITHMETIC),

    // Bitwise operators
    Operator::operator_bitwise_xor("^", 6, BITWISE),
    Operator::operator_bitwise_and("&", 7, BITWISE),
    Operator::operator_bitwise_or("|", 5, BITWISE),
    Operator::operator_bitshift_left("<<", 10, BITWISE),
    Operator::operator_bitshift_right(">>", 10, BITWISE),

    // Logical operators
    Operator::operator_logical_and("&&", 4, LOGICAL),
    Operator::operator_logical_or("||", 3, LOGICAL),

    // Comparison operators
    Operator::operator_equals("==", 8, COMPARISON),
    Operator::operator_not_eql("!=", 8, COMPARISON),
    Operator::operator_less("<", 9, COMPARISON),
    Operator::operator_greater(">", 9, COMPARISON),
    Operator::operator_less_eql("<=", 9, COMPARISON),
    Operator::operator_greater_eql(">=", 9, COMPARISON),

    // Assignment operators
    Operator::operator_assign("=", 2, ASSIGNMENT, true),
    Operator::operator_add_assign("+=", 2, ASSIGNMENT | ARITHMETIC, true),
    Operator::operator_subtract_assign("-=", 2, ASSIGNMENT | ARITHMETIC, true),
    Operator::operator_multiply_assign("*=", 2, ASSIGNMENT | ARITHMETIC, true),
    Operator::operator_divide_assign("/=", 2, ASSIGNMENT | ARITHMETIC, true),
    Operator::operator_modulus_assign("%=", 2, ASSIGNMENT | ARITHMETIC, true),
    Operator::operator_bitwise_xor_assign("^=", 2, ASSIGNMENT | BITWISE, true),
    Operator::operator_bitwise_and_assign("&=", 2, ASSIGNMENT | BITWISE, true),
    Operator::operator_bitwise_or_assign("|=", 2, ASSIGNMENT | BITWISE, true),

    // Unary operators
    Operator::operator_logical_not("!", 0, LOGICAL),
    Operator::operator_negative("-", 0, ARITHMETIC),
    Operator::operator_positive("+", 0, ARITHMETIC),
    Operator::operator_bitwise_complement("~", 0, BITWISE),
    Operator::operator_increment("++", 0, ASSIGNMENT | ARITHMETIC, true),
    Operator::operator_decrement("--", 0, ASSIGNMENT | ARITHMETIC, true);

bool Operator::IsOperator(const std::string &str)
{
    const Operator *tmp = nullptr;
    return IsOperator(str, tmp);
}

bool Operator::IsOperator(const std::string &str, const Operator *&out)
{
    return IsBinaryOperator(str, out) || IsUnaryOperator(str, out);
}

bool Operator::IsBinaryOperator(const std::string &str)
{
    const Operator *tmp = nullptr;
    return IsBinaryOperator(str, tmp);
}

bool Operator::IsBinaryOperator(const std::string &str, const Operator *&out)
{
    const Operator *binary_operators[] = {
        &operator_add,
        &operator_subtract,
        &operator_multiply,
        &operator_divide,
        &operator_modulus,
        &operator_bitwise_xor,
        &operator_bitwise_and,
        &operator_bitwise_or,
        &operator_bitshift_left,
        &operator_bitshift_right,
        &operator_logical_and,
        &operator_logical_or,
        &operator_equals,
        &operator_not_eql,
        &operator_less,
        &operator_greater,
        &operator_less_eql,
        &operator_greater_eql,
        &operator_assign,
        &operator_add_assign,
        &operator_subtract_assign,
        &operator_multiply_assign,
        &operator_divide_assign,
        &operator_bitwise_xor_assign,
        &operator_bitwise_and_assign,
        &operator_bitwise_or_assign
    };

    size_t num_binary_operators =
        sizeof(binary_operators) / sizeof(binary_operators[0]);

    for (int i = 0; i < num_binary_operators; i++) {
        auto oper = binary_operators[i];
        if (str == oper->ToString()) {
            out = oper;
            return true;
        }
    }

    return false;
}

bool Operator::IsUnaryOperator(const std::string &str)
{
    const Operator *tmp = nullptr;
    return IsUnaryOperator(str, tmp);
}

bool Operator::IsUnaryOperator(const std::string &str, const Operator *&out)
{
    const Operator *unary_operators[] = {
        &operator_logical_not,
        &operator_negative,
        &operator_positive,
        &operator_bitwise_complement,
        &operator_increment,
        &operator_decrement
    };

    size_t num_unary_operators = sizeof(unary_operators) /
        sizeof(unary_operators[0]);
    for (int i = 0; i < num_unary_operators; i++) {
        auto oper = unary_operators[i];
        if (str == oper->ToString()) {
            out = oper;
            return true;
        }
    }

    return false;
}

Operator::Operator(const std::string &str,
    int precedence, int type, bool modifies_value)
    : m_str(str),
      m_precedence(precedence),
      m_type(type),
      m_modifies_value(modifies_value)
{
}
