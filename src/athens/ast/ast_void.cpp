#include <athens/ast/ast_void.hpp>

AstVoid::AstVoid(const SourceLocation &location)
    : AstConstant(location)
{
}

void AstVoid::Build(AstVisitor *visitor, Module *mod)
{
}

int AstVoid::IsTrue() const
{
    return false;
}

bool AstVoid::IsNumber() const
{
    return false;
}

a_int AstVoid::IntValue() const
{
    return 0;
}

a_float AstVoid::FloatValue() const
{
    return 0.0f;
}

std::shared_ptr<AstConstant> AstVoid::operator+(
        AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstVoid::operator-(
        AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstVoid::operator*(
        AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstVoid::operator/(
        AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstVoid::operator%(
        AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstVoid::operator^(
        AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstVoid::operator&(
        AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstVoid::operator|(
        AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstVoid::operator<<(
        AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstVoid::operator>>(
        AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstVoid::operator&&(
        AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstVoid::operator||(
        AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstVoid::Equals(
        AstConstant *right) const
{
    return nullptr;
}
