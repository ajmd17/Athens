#include <athens/ast/ast_module_declaration.hpp>
#include <athens/ast_visitor.hpp>

AstModuleDeclaration::AstModuleDeclaration(const std::string &name, const SourceLocation &location)
    : AstDeclaration(name, location)
{
}

void AstModuleDeclaration::Visit(AstVisitor *visitor)
{
    // make sure this module was not already declared/imported
    for (const std::unique_ptr<Module> &mod : visitor->GetCompilationUnit()->m_modules) {
        if (mod->GetName() == m_name) {
            visitor->GetCompilationUnit()->GetErrorList().AddError(
                CompilerError(Level_fatal, Msg_module_already_defined, m_location, m_name));
                
            break;
        }
    }

    // add this module to the compilation unit
    std::unique_ptr<Module> this_module(new Module(m_name, m_location));
    visitor->GetCompilationUnit()->m_modules.push_back(std::move(this_module));
}

void AstModuleDeclaration::Build(AstVisitor *visitor)
{
}

void AstModuleDeclaration::Optimize(AstVisitor *visitor)
{
}
