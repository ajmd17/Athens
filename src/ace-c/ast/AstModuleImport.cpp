#include <ace-c/ast/AstModuleImport.hpp>
#include <ace-c/SourceFile.hpp>
#include <ace-c/Lexer.hpp>
#include <ace-c/Parser.hpp>
#include <ace-c/SemanticAnalyzer.hpp>
#include <ace-c/Optimizer.hpp>
#include <ace-c/Minifier.hpp>

#include <common/str_util.hpp>

#include <fstream>
#include <iostream>
#include <functional>

AstModuleImportPart::AstModuleImportPart(
    const std::string &left,
    const std::vector<std::shared_ptr<AstModuleImportPart>> &right_parts,
    const SourceLocation &location)
    : AstStatement(location),
      m_left(left),
      m_right_parts(right_parts),
      m_pull_in_modules(true)
{
}

void AstModuleImportPart::Visit(AstVisitor *visitor, Module *mod)
{
    ASSERT(visitor != nullptr);
    ASSERT(mod != nullptr);

    if (Module *this_module = mod->LookupNestedModule(m_left)) {
        if (m_pull_in_modules && m_right_parts.empty()) {
            // pull this module into scope
            AstImport::CopyModules(
                visitor,
                this_module,
                true,
                false
            );
        } else {
            // get nested items
            for (const std::shared_ptr<AstModuleImportPart> &part : m_right_parts) {
                ASSERT(part != nullptr);
                part->Visit(visitor, this_module);
            }
        }
    } else {
        std::cout << "could not find nested module " << m_left << "\n";
    }
}

void AstModuleImportPart::Build(AstVisitor *visitor, Module *mod)
{
}

void AstModuleImportPart::Optimize(AstVisitor *visitor, Module *mod)
{
}

void AstModuleImportPart::Recreate(std::ostringstream &ss)
{
}

Pointer<AstStatement> AstModuleImportPart::Clone() const
{
    return CloneImpl();
}


AstModuleImport::AstModuleImport(
    const std::vector<std::shared_ptr<AstModuleImportPart>> &parts,
    const SourceLocation &location)
    : AstImport(location),
      m_parts(parts)
{
}

void AstModuleImport::Visit(AstVisitor *visitor, Module *mod)
{
    ASSERT(!m_parts.empty());

    const std::shared_ptr<AstModuleImportPart> &first = m_parts[0];
    ASSERT(first != nullptr);

    bool opened = false;

    // already imported into this module, set opened to true
    if (mod->LookupNestedModule(first->GetLeft()) != nullptr) {
        opened = true;
    }

    // if this is not a direct import (i.e `import range`),
    // we will allow duplicates in imports like `import range::{_Detail_}`
    // and we won't import the 'range' module again
    if (first->GetParts().empty() || !opened) {
        // find the folder which the current file is in
        std::string current_dir;
        const size_t index = m_location.GetFileName().find_last_of("/\\");
        if (index != std::string::npos) {
            current_dir = m_location.GetFileName().substr(0, index) + "/";
        }

        std::ifstream file;
        std::string found_path;

        // iterate through library paths to try and find a file
        for (const std::string &scan_path : mod->GetScanPaths()) {
            const std::string &filename = first->GetLeft();

            // create relative path
            const std::string path = current_dir + scan_path + "/";
            const std::string ext = ".ace";

            found_path = path + filename + ext;
            if (AstImport::TryOpenFile(found_path, file)) {
                opened = true;
                break;
            }

            // try it without extension
            found_path = path + filename;
            if (AstImport::TryOpenFile(found_path, file)) {
                opened = true;
                break;
            }
        }

        AstImport::PerformImport(
            visitor,
            mod,
            found_path
        );

        // do not pull module into scope for single imports
        // i.e `import range` will just import the file
        if (first->GetParts().empty()) {
            first->SetPullInModules(false);
        }
    }

    if (opened) {
        for (const std::shared_ptr<AstModuleImportPart> &part : m_parts) {
            ASSERT(part != nullptr);
            part->Visit(visitor, mod);
        }
    } else {
        visitor->GetCompilationUnit()->GetErrorList().AddError(CompilerError(
            LEVEL_ERROR,
            Msg_could_not_find_module,
            m_location,
            first->GetLeft()
        ));
    }
}

void AstModuleImport::Recreate(std::ostringstream &ss)
{
    m_ast_iterator.ResetPosition();
    Minifier minifier(&m_ast_iterator);
    minifier.Minify(ss);
}

Pointer<AstStatement> AstModuleImport::Clone() const
{
    return CloneImpl();
}