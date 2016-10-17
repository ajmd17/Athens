#include <athens/ast/ast_null.h>
#include <athens/ast/ast_integer.h>
#include <athens/ast/ast_void.h>
#include <athens/ast/ast_false.h>
#include <athens/ast/ast_true.h>

AstNull::AstNull(const SourceLocation &location)
    : AstConstant(location)
{
}

void AstNull::Build(AstVisitor *visitor)
{
}

int AstNull::IsTrue() const
{
    return false;
}

bool AstNull::IsNumber() const
{
    /** Set it to be a number so we can perform logical operations */
    return true;
}

a_int AstNull::IntValue() const
{
    return 0;
}

a_float AstNull::FloatValue() const
{
    return 0.0f;
}

std::shared_ptr<AstConstant> AstNull::operator+(
        AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstNull::operator-(
        AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstNull::operator*(
        AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstNull::operator/(
        AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstNull::operator%(
        AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstNull::operator^(
        AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstNull::operator&(
        AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstNull::operator|(
        AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstNull::operator<<(
        AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstNull::operator>>(
        AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstNull::operator&&(
        AstConstant *right) const
{
    // logical operations still work, so that we can do
    // things like testing for null in an if statement.
    return std::shared_ptr<AstInteger>(
        new AstInteger(IntValue() && right->IntValue(), m_location));
}

std::shared_ptr<AstConstant> AstNull::operator||(
        AstConstant *right) const
{
    return std::shared_ptr<AstInteger>(
        new AstInteger(IntValue() || right->IntValue(), m_location));
}

std::shared_ptr<AstConstant> AstNull::Equals(AstConstant *right) const
{
    if (dynamic_cast<AstNull*>(right) != nullptr) {
        return std::shared_ptr<AstTrue>(new AstTrue(m_location));
    }
    return std::shared_ptr<AstFalse>(new AstFalse(m_location));
}