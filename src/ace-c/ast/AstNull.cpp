#include <ace-c/ast/AstNull.hpp>
#include <ace-c/ast/AstInteger.hpp>
#include <ace-c/ast/AstFalse.hpp>
#include <ace-c/ast/AstTrue.hpp>
#include <ace-c/AstVisitor.hpp>
#include <ace-c/Keywords.hpp>
#include <ace-c/emit/Instruction.hpp>

#include <common/instructions.hpp>

AstNull::AstNull(const SourceLocation &location)
    : AstConstant(location)
{
}

void AstNull::Build(AstVisitor *visitor, Module *mod)
{
    // get active register
    uint8_t rp = visitor->GetCompilationUnit()->GetInstructionStream().GetCurrentRegister();
    // load integer value into register
    visitor->GetCompilationUnit()->GetInstructionStream() <<
        Instruction<uint8_t, uint8_t>(LOAD_NULL, rp);
}

void AstNull::Recreate(std::ostringstream &ss)
{
    ss << Keyword::ToString(Keyword_null);
}

int AstNull::IsTrue() const
{
    return false;
}

bool AstNull::IsNumber() const
{
    /** Set it to be a number so we can perform logical operations */
    return false;
}

a_int AstNull::IntValue() const
{
    return 0;
}

a_float AstNull::FloatValue() const
{
    return 0.0f;
}

SymbolTypePtr_t AstNull::GetSymbolType() const
{
    return SymbolType::Builtin::ANY;
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
    return std::shared_ptr<AstFalse>(new AstFalse(m_location));
}

std::shared_ptr<AstConstant> AstNull::operator||(
        AstConstant *right) const
{
    if (!right->IsNumber()) {
        // this operator is valid to compare against null
        AstNull *ast_null = dynamic_cast<AstNull*>(right);
        if (ast_null != nullptr) {
            return std::shared_ptr<AstFalse>(new AstFalse(m_location));
        }
        return nullptr;
    }

    return std::shared_ptr<AstInteger>(
        new AstInteger(right->IntValue(), m_location));
}

std::shared_ptr<AstConstant> AstNull::operator<(AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstNull::operator>(AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstNull::operator<=(AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstNull::operator>=(AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstNull::Equals(AstConstant *right) const
{
    if (dynamic_cast<AstNull*>(right) != nullptr) {
        // only another null value should be equal
        return std::shared_ptr<AstTrue>(new AstTrue(m_location));
    }
    // other values never equal to null
    return std::shared_ptr<AstFalse>(new AstFalse(m_location));
}

std::shared_ptr<AstConstant> AstNull::operator-() const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstNull::operator~() const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstNull::operator!() const
{
    return std::shared_ptr<AstTrue>(new AstTrue(m_location));
}