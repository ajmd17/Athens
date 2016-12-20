#include <ace-c/ast/ast_module_declaration.hpp>
#include <ace-c/ast_visitor.hpp>

#include <common/my_assert.hpp>
#include <common/str_util.hpp>

AstModuleDeclaration::AstModuleDeclaration(const std::string &name, const SourceLocation &location)
    : AstDeclaration(name, location)
{
}

void AstModuleDeclaration::PerformLookup(AstVisitor *visitor)
{
    // make sure this module was not already declared/imported
    if (visitor->GetCompilationUnit()->LookupModule(m_name)) {
        visitor->GetCompilationUnit()->GetErrorList().AddError(
            CompilerError(Level_fatal, Msg_module_already_defined, m_location, m_name));
    } else {
        m_module.reset(new Module(m_name, m_location));
    }
}

void AstModuleDeclaration::Visit(AstVisitor *visitor, Module *mod)
{
    if (!m_module) {
        PerformLookup(visitor);
    }

    if (m_module) {
        // add this module to the compilation unit
        visitor->GetCompilationUnit()->m_module_tree.Open(m_module.get());
        // set the link to the module in the tree
        m_module->SetImportTreeLink(visitor->GetCompilationUnit()->m_module_tree.TopNode());

        // add this module to list of imported modules,
        // but only if mod == nullptr, that way we don't add nested modules
        if (!mod) {
            // parse filename
            std::vector<std::string> path = str_util::split_path(m_location.GetFileName());
            path = str_util::canonicalize_path(path);
            // change it back to string
            std::string canon_path = str_util::path_to_str(path);

            // map filepath to module
            auto it = visitor->GetCompilationUnit()->m_imported_modules.find(canon_path);
            if (it != visitor->GetCompilationUnit()->m_imported_modules.end()) {
                it->second.push_back(m_module);
            } else {
                visitor->GetCompilationUnit()->m_imported_modules[canon_path] = { m_module };
            }
        }

        // update current module
        mod = m_module.get();
        ASSERT(mod == visitor->GetCompilationUnit()->GetCurrentModule());

        // visit all children
        for (auto &child : m_children) {
            if (child) {
                child->Visit(visitor, mod);
            }
        }

        // close this module
        visitor->GetCompilationUnit()->m_module_tree.Close();
    }
}

void AstModuleDeclaration::Build(AstVisitor *visitor, Module *mod)
{
    ASSERT(m_module != nullptr);

    // build all children
    for (auto &child : m_children) {
        if (child) {
            child->Build(visitor, m_module.get());
        }
    }
}

void AstModuleDeclaration::Optimize(AstVisitor *visitor, Module *mod)
{
    ASSERT(m_module != nullptr);

    // optimize all children
    for (auto &child : m_children) {
        if (child) {
            child->Optimize(visitor, m_module.get());
        }
    }
}
