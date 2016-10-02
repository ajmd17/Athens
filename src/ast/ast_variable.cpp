#include <athens/ast/ast_variable.h>
#include <athens/ast_visitor.h>
#include <athens/ast/ast_constant.h>

AstVariable::AstVariable(const std::string &name, const SourceLocation &location)
    : AstExpression(location),
      m_name(name),
      m_identifier(nullptr)
{
}

void AstVariable::Visit(AstVisitor *visitor) 
{
    // make sure that the variable exists
    std::unique_ptr<Module> &mod = visitor->GetCompilationUnit()->CurrentModule();
    Scope &scope = mod->m_scopes.Top();

    // the variable must exist in the active scope or a parent scope
    m_identifier = mod->LookUpIdentifier(m_name, false);
    if (m_identifier == nullptr) {
        visitor->GetCompilationUnit()->GetErrorList().AddError(
            CompilerError(Level_fatal, Msg_undeclared_identifier, m_location, m_name));
    } else {
        m_identifier->IncUseCount();
    }
}

void AstVariable::Build(AstVisitor *visitor) const
{
}

void AstVariable::Optimize()
{
}

int AstVariable::IsTrue() const
{
    if (m_identifier != nullptr) {
        // we can only check if this is true during
        // compile time if it is const literal
        if (m_identifier->GetFlags() & (Flag_const)) {
            auto value_sp = m_identifier->GetCurrentValue().lock();
            auto constant_sp = std::dynamic_pointer_cast<AstConstant>(value_sp);
            if (constant_sp != nullptr) {
                return constant_sp->IsTrue();
            }
        }
    }
    
    return -1;
}