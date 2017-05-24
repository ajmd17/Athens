#include <ace-c/ast/AstIdentifier.hpp>
#include <ace-c/AstVisitor.hpp>
#include <ace-c/Module.hpp>

#include <common/my_assert.hpp>

#include <iostream>

AstIdentifier::AstIdentifier(const std::string &name, const SourceLocation &location)
    : AstExpression(location, ACCESS_MODE_LOAD | ACCESS_MODE_STORE),
      m_name(name)
{
}

void AstIdentifier::PerformLookup(AstVisitor *visitor, Module *mod)
{
    // the variable must exist in the active scope or a parent scope
    if ((m_properties.m_identifier = mod->LookUpIdentifier(m_name, false))) {
        m_properties.SetIdentifierType(IDENTIFIER_TYPE_VARIABLE);
    } else if ((m_properties.m_identifier = visitor->GetCompilationUnit()->GetGlobalModule()->LookUpIdentifier(m_name, false))) {
        // if the identifier was not found,
        // look in the global module to see if it is a global function.
        m_properties.SetIdentifierType(IDENTIFIER_TYPE_VARIABLE);
    } else if (visitor->GetCompilationUnit()->LookupModule(m_name)) {
        m_properties.SetIdentifierType(IDENTIFIER_TYPE_MODULE);
    } else if (mod->LookupSymbolType(m_name)) {
        m_properties.SetIdentifierType(IDENTIFIER_TYPE_TYPE);
    } else {
        // nothing was found
        m_properties.SetIdentifierType(IDENTIFIER_TYPE_NOT_FOUND);
    }
}

void AstIdentifier::CheckInFunction(AstVisitor *visitor, Module *mod)
{
    m_properties.m_depth = 0;
    TreeNode<Scope> *top = mod->m_scopes.TopNode();
    
    while (top != nullptr) {
        m_properties.m_depth++;
        if (top->m_value.GetScopeType() == SCOPE_TYPE_FUNCTION) {
            m_properties.m_is_in_function = true;
            break;
        } else if (top->m_value.GetScopeType() == SCOPE_TYPE_PURE_FUNCTION) {
            m_properties.m_is_in_function = true;
            m_properties.m_is_in_pure_function = true;
            break;
        }
        top = top->m_parent;
    }
}

void AstIdentifier::Visit(AstVisitor *visitor, Module *mod)
{
    if (m_properties.GetIdentifierType() == IDENTIFIER_TYPE_UNKNOWN) {
        PerformLookup(visitor, mod);
    }

    CheckInFunction(visitor, mod);
}

SymbolTypePtr_t AstIdentifier::GetSymbolType() const
{
    if (m_properties.GetIdentifier()) {
        return m_properties.GetIdentifier()->GetSymbolType();
    }

    return SymbolType::Builtin::UNDEFINED;
}

int AstIdentifier::GetStackOffset(int stack_size) const
{
    ASSERT(m_properties.GetIdentifier() != nullptr);
    return stack_size - m_properties.GetIdentifier()->GetStackLocation();
}
