#include <ace-c/ast/AstTrue.hpp>
#include <ace-c/ast/AstFalse.hpp>
#include <ace-c/ast/AstInteger.hpp>
#include <ace-c/AstVisitor.hpp>
#include <ace-c/Keywords.hpp>
#include <ace-c/emit/Instruction.hpp>

#include <common/instructions.hpp>

AstTrue::AstTrue(const SourceLocation &location)
    : AstConstant(location)
{
}

void AstTrue::Build(AstVisitor *visitor, Module *mod)
{
    // get active register
    uint8_t rp = visitor->GetCompilationUnit()->GetInstructionStream().GetCurrentRegister();
    // load value into register
    visitor->GetCompilationUnit()->GetInstructionStream() <<
        Instruction<uint8_t, uint8_t>(LOAD_TRUE, rp);
}

void AstTrue::Recreate(std::ostringstream &ss)
{
    ss << Keyword::ToString(Keyword_true);
}

int AstTrue::IsTrue() const
{
    return true;
}

bool AstTrue::IsNumber() const
{
    return false;
}

a_int AstTrue::IntValue() const
{
    return 1;
}

a_float AstTrue::FloatValue() const
{
    return 1.0f;
}

SymbolTypePtr_t AstTrue::GetSymbolType() const
{
    return SymbolType::Builtin::BOOLEAN;
}

std::shared_ptr<AstConstant> AstTrue::operator+(AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstTrue::operator-(AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstTrue::operator*(AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstTrue::operator/(AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstTrue::operator%(AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstTrue::operator^(AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstTrue::operator&(AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstTrue::operator|(AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstTrue::operator<<(AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstTrue::operator>>(AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstTrue::operator&&(AstConstant *right) const
{
    int right_true = right->IsTrue();
    if (right_true == 1) {
        return std::shared_ptr<AstTrue>(new AstTrue(m_location));
    } else if (right_true == 0) {
        return std::shared_ptr<AstFalse>(new AstFalse(m_location));
    } else {
        return nullptr;
    }
}

std::shared_ptr<AstConstant> AstTrue::operator||(AstConstant *right) const
{
    return std::shared_ptr<AstTrue>(new AstTrue(m_location));
}

std::shared_ptr<AstConstant> AstTrue::operator<(AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstTrue::operator>(AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstTrue::operator<=(AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstTrue::operator>=(AstConstant *right) const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstTrue::Equals(AstConstant *right) const
{
    if (dynamic_cast<AstTrue*>(right) != nullptr) {
        return std::shared_ptr<AstTrue>(new AstTrue(m_location));
    }
    return std::shared_ptr<AstFalse>(new AstFalse(m_location));
}

std::shared_ptr<AstConstant> AstTrue::operator-() const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstTrue::operator~() const
{
    return nullptr;
}

std::shared_ptr<AstConstant> AstTrue::operator!() const
{
    return std::shared_ptr<AstFalse>(new AstFalse(m_location));
}
