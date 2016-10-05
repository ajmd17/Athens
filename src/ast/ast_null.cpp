#include <athens/ast/ast_null.h>
#include <athens/ast/ast_integer.h>
#include <athens/ast/ast_void.h>

AstNull::AstNull(const SourceLocation &location)
    : AstConstant(location)
{
}

void AstNull::Build(AstVisitor *visitor) const
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
        const std::shared_ptr<AstConstant> &right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstNull::operator-(
        const std::shared_ptr<AstConstant> &right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstNull::operator*(
        const std::shared_ptr<AstConstant> &right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstNull::operator/(
        const std::shared_ptr<AstConstant> &right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstNull::operator%(
        const std::shared_ptr<AstConstant> &right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstNull::operator^(
        const std::shared_ptr<AstConstant> &right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstNull::operator&(
        const std::shared_ptr<AstConstant> &right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstNull::operator|(
        const std::shared_ptr<AstConstant> &right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstNull::operator<<(
        const std::shared_ptr<AstConstant> &right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstNull::operator>>(
        const std::shared_ptr<AstConstant> &right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstNull::operator&&(
        const std::shared_ptr<AstConstant> &right) const
{
    // logical operations still work, so that we can do
    // things like testing for null in an if statement.
    return std::shared_ptr<AstInteger>(
        new AstInteger(IntValue() && right->IntValue(), m_location));
}

std::shared_ptr<AstConstant> AstNull::operator||(
        const std::shared_ptr<AstConstant> &right) const
{
    return std::shared_ptr<AstInteger>(
        new AstInteger(IntValue() || right->IntValue(), m_location));
}