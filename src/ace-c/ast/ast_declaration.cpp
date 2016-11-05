#include <ace-c/ast/ast_declaration.hpp>
#include <ace-c/ast_visitor.hpp>
#include <ace-c/module.hpp>

AstDeclaration::AstDeclaration(const std::string &name, const SourceLocation &location)
    : AstStatement(location),
      m_name(name),
      m_identifier(nullptr)
{
}

void AstDeclaration::Visit(AstVisitor *visitor, Module *mod)
{
    CompilationUnit *compilation_unit = visitor->GetCompilationUnit();
    Scope &scope = mod->m_scopes.Top();

    // look up variable to make sure it doesn't already exist
    // only this scope matters, variables with the same name outside
    // of this scope are fine
    m_identifier = mod->LookUpIdentifier(m_name, true);
    if (m_identifier != nullptr) {
        // a collision was found, add an error
        compilation_unit->GetErrorList().AddError(
            CompilerError(Level_fatal, Msg_redeclared_identifier, m_location, m_name));
    } else {
        // add identifier
        m_identifier = scope.GetIdentifierTable().AddIdentifier(m_name);

        bool in_function = false;

        TreeNode<Scope> *top = mod->m_scopes.TopNode();
        while (top != nullptr) {
            if (top->m_value.GetScopeType() == SCOPE_TYPE_FUNCTION) {
                in_function = true;
                break;
            }
            top = top->m_parent;
        }

        if (in_function) {
            // set declared in function flag
            m_identifier->GetFlags() |= FLAG_DECLARED_IN_FUNCTION;
        }
    }
}

ObjectType AstDeclaration::GetObjectType() const
{
    if (m_identifier != nullptr) {
        return m_identifier->GetObjectType();
    }
    return ObjectType::type_builtin_undefined;
}
