#include <athens/ast/ast_null.h>
#include <athens/ast/ast_integer.h>
#include <athens/ast/ast_void.h>
#include <athens/ast/ast_false.h>
#include <athens/ast/ast_true.h>
#include <athens/ast_visitor.h>
#include <athens/emit/instruction.h>

#include <common/instructions.h>

AstNull::AstNull(const SourceLocation &location)
    : AstConstant(location)
{
}

void AstNull::Build(AstVisitor *visitor)
{
    // get active register
    uint8_t rp = visitor->GetCompilationUnit()->GetInstructionStream().GetCurrentRegister();
    // load integer value into register
    visitor->GetCompilationUnit()->GetInstructionStream() << 
        Instruction<uint8_t, uint8_t>(LOAD_NULL, rp);
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

    if (!right->IsNumber()) {
        // this operator is valid to compare against null
        AstNull *ast_null = dynamic_cast<AstNull*>(right);
        if (ast_null != nullptr) {
            return std::shared_ptr<AstFalse>(new AstFalse(m_location));
        }
        return nullptr;
    }

    return std::shared_ptr<AstFalse>(
        new AstFalse(m_location));
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
        new AstInteger(0 || right->IntValue(), m_location));
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