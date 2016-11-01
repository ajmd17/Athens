#include <ace-c/ast/ast_integer.hpp>
#include <ace-c/ast/ast_float.hpp>
#include <ace-c/ast/ast_null.hpp>
#include <ace-c/ast/ast_true.hpp>
#include <ace-c/ast/ast_false.hpp>
#include <ace-c/ast/ast_undefined.hpp>
#include <ace-c/ast_visitor.hpp>
#include <ace-c/emit/instruction.hpp>

#include <common/instructions.hpp>

#include <iostream>
#include <limits>
#include <cmath>

AstInteger::AstInteger(a_int value, const SourceLocation &location)
    : AstConstant(location),
      m_value(value)
{
}

void AstInteger::Build(AstVisitor *visitor, Module *mod)
{
    // get active register
    uint8_t rp = visitor->GetCompilationUnit()->GetInstructionStream().GetCurrentRegister();
    // load integer value into register
    visitor->GetCompilationUnit()->GetInstructionStream() <<
        Instruction<uint8_t, uint8_t, int32_t>(LOAD_I32, rp, m_value);
}

int AstInteger::IsTrue() const
{
    // any non-zero value is considered true
    return m_value != 0;
}

ObjectType AstInteger::GetObjectType() const
{
    return ObjectType::type_builtin_int;
}

bool AstInteger::IsNumber() const
{
    return true;
}

a_int AstInteger::IntValue() const
{
    return m_value;
}

a_float AstInteger::FloatValue() const
{
    return static_cast<a_float>(m_value);
}

std::shared_ptr<AstConstant> AstInteger::operator+(AstConstant *right) const
{
    if (!right->IsNumber()) {
        return nullptr;
    }

    // we have to determine weather or not to promote this to a float
    if (dynamic_cast<const AstFloat*>(right) != nullptr) {
        return std::shared_ptr<AstFloat>(
            new AstFloat(FloatValue() + right->FloatValue(), m_location));
    } else {
        return std::shared_ptr<AstInteger>(
            new AstInteger(IntValue() + right->IntValue(), m_location));
    }
}

std::shared_ptr<AstConstant> AstInteger::operator-(AstConstant *right) const
{
    if (!right->IsNumber()) {
        return nullptr;
    }

    // we have to determine weather or not to promote this to a float
    if (dynamic_cast<const AstFloat*>(right) != nullptr) {
        return std::shared_ptr<AstFloat>(
            new AstFloat(FloatValue() - right->FloatValue(), m_location));
    } else {
        return std::shared_ptr<AstInteger>(
            new AstInteger(IntValue() - right->IntValue(), m_location));
    }
}

std::shared_ptr<AstConstant> AstInteger::operator*(AstConstant *right) const
{
    if (!right->IsNumber()) {
        return nullptr;
    }

    // we have to determine weather or not to promote this to a float
    if (dynamic_cast<const AstFloat*>(right) != nullptr) {
        return std::shared_ptr<AstFloat>(
            new AstFloat(FloatValue() * right->FloatValue(), m_location));
    } else {
        return std::shared_ptr<AstInteger>(
            new AstInteger(IntValue() * right->IntValue(), m_location));
    }
}

std::shared_ptr<AstConstant> AstInteger::operator/(AstConstant *right) const
{
    if (!right->IsNumber()) {
        return nullptr;
    }

    // we have to determine weather or not to promote this to a float
    if (dynamic_cast<const AstFloat*>(right) != nullptr) {
        a_float result;
        a_float right_float = right->FloatValue();
        if (right_float == 0.0) {
            // division by zero, return Undefined
            return std::shared_ptr<AstUndefined>(new AstUndefined(m_location));
        } else {
            result = FloatValue() / right_float;
        }
        return std::shared_ptr<AstFloat>(
            new AstFloat(result, m_location));
    } else {
        a_int right_int = right->IntValue();
        if (right_int == 0) {
            // division by zero, return Undefined
            return std::shared_ptr<AstUndefined>(new AstUndefined(m_location));
        } else {
            return std::shared_ptr<AstInteger>(
                new AstInteger(IntValue() / right_int, m_location));
        }
    }
}

std::shared_ptr<AstConstant> AstInteger::operator%(AstConstant *right) const
{
    if (!right->IsNumber()) {
        return nullptr;
    }

    // we have to determine weather or not to promote this to a float
    if (dynamic_cast<const AstFloat*>(right) != nullptr) {
        a_float result;
        a_float right_float = right->FloatValue();
        if (right_float == 0.0) {
            // division by zero, return Undefined
            return std::shared_ptr<AstUndefined>(new AstUndefined(m_location));
        } else {
            result = std::fmod(FloatValue(), right_float);
        }
        return std::shared_ptr<AstFloat>(
            new AstFloat(result, m_location));
    } else {
        a_int right_int = right->IntValue();
        if (right_int == 0) {
            // division by zero, return Undefined
            return std::shared_ptr<AstUndefined>(new AstUndefined(m_location));
        } else {
            return std::shared_ptr<AstInteger>(
                new AstInteger(IntValue() % right_int, m_location));
        }
    }
}

std::shared_ptr<AstConstant> AstInteger::operator^(AstConstant *right) const
{
    // right must be integer
    if (!right->IsNumber() ||
         right->GetObjectType().ToString() != ObjectType::type_builtin_int.ToString()) {
        return nullptr;
    }

    return std::shared_ptr<AstInteger>(
        new AstInteger(IntValue() ^ right->IntValue(), m_location));
}

std::shared_ptr<AstConstant> AstInteger::operator&(AstConstant *right) const
{
    // right must be integer
    if (!right->IsNumber() ||
         right->GetObjectType().ToString() != ObjectType::type_builtin_int.ToString()) {
        return nullptr;
    }

    return std::shared_ptr<AstInteger>(
        new AstInteger(IntValue() & right->IntValue(), m_location));
}

std::shared_ptr<AstConstant> AstInteger::operator|(AstConstant *right) const
{
    // right must be integer
    if (!right->IsNumber() ||
         right->GetObjectType().ToString() != ObjectType::type_builtin_int.ToString()) {
        return nullptr;
    }

    return std::shared_ptr<AstInteger>(
        new AstInteger(IntValue() | right->IntValue(), m_location));
}

std::shared_ptr<AstConstant> AstInteger::operator<<(AstConstant *right) const
{
    // right must be integer
    if (!right->IsNumber() ||
         right->GetObjectType().ToString() != ObjectType::type_builtin_int.ToString()) {
        return nullptr;
    }

    return std::shared_ptr<AstInteger>(
        new AstInteger(IntValue() << right->IntValue(), m_location));
}

std::shared_ptr<AstConstant> AstInteger::operator>>(AstConstant *right) const
{
    // right must be integer
    if (!right->IsNumber() ||
         right->GetObjectType().ToString() != ObjectType::type_builtin_int.ToString()) {
        return nullptr;
    }

    return std::shared_ptr<AstInteger>(
        new AstInteger(IntValue() >> right->IntValue(), m_location));
}

std::shared_ptr<AstConstant> AstInteger::operator&&(AstConstant *right) const
{
    int this_true = IsTrue();
    int right_true = right->IsTrue();

    if (!right->IsNumber()) {
        // this operator is valid to compare against null
        AstNull *ast_null = dynamic_cast<AstNull*>(right);
        if (ast_null != nullptr) {
            // rhs is null, return false
            return std::shared_ptr<AstFalse>(
                new AstFalse(m_location));
        }
        return nullptr;
    }

    if (this_true == 1 && right_true == 1) {
        return std::shared_ptr<AstTrue>(new AstTrue(m_location));
    } else if (this_true == 0 && right_true == 0) {
        return std::shared_ptr<AstFalse>(new AstFalse(m_location));
    } else {
        // indeterminate
        return nullptr;
    }
}

std::shared_ptr<AstConstant> AstInteger::operator||(AstConstant *right) const
{
    int this_true = IsTrue();
    int right_true = right->IsTrue();

    if (!right->IsNumber()) {
        // this operator is valid to compare against null
        AstNull *ast_null = dynamic_cast<AstNull*>(right);
        if (ast_null != nullptr) {
            if (this_true == 1) {
                return std::shared_ptr<AstTrue>(new AstTrue(m_location));
            } else if (this_true == 0) {
                return std::shared_ptr<AstFalse>(new AstFalse(m_location));
            }
        }
        return nullptr;
    }

    if (this_true == 1 || right_true == 1) {
        return std::shared_ptr<AstTrue>(new AstTrue(m_location));
    } else if (this_true == 0 || right_true == 0) {
        return std::shared_ptr<AstFalse>(new AstFalse(m_location));
    } else {
        // indeterminate
        return nullptr;
    }
}

std::shared_ptr<AstConstant> AstInteger::Equals(AstConstant *right) const
{
    if (!right->IsNumber()) {
        return nullptr;
    }
    return std::shared_ptr<AstInteger>(
        new AstInteger(IntValue() == right->IntValue(), m_location));
}
