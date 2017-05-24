#include <ace-c/ast/AstImport.hpp>
#include <ace-c/AstVisitor.hpp>
#include <ace-c/Optimizer.hpp>
#include <ace-c/Compiler.hpp>
#include <ace-c/SourceFile.hpp>
#include <ace-c/Lexer.hpp>
#include <ace-c/Parser.hpp>
#include <ace-c/SemanticAnalyzer.hpp>

#include <common/str_util.hpp>

#include <fstream>
#include <iostream>
#include <functional>

AstImport::AstImport(const SourceLocation &location)
    : AstStatement(location)
{
}

void AstImport::CopyModules(
    AstVisitor *visitor,
    std::shared_ptr<Module> &mod,
    bool check_lookup,
    bool update_tree_link)
{
    ASSERT(visitor != nullptr);
    ASSERT(mod != nullptr);

    if (check_lookup) {
        if (visitor->GetCompilationUnit()->LookupModule(mod->GetName())) {
            visitor->GetCompilationUnit()->GetErrorList().AddError(CompilerError(
                LEVEL_ERROR,
                Msg_module_already_defined,
                mod->GetLocation(),
                mod->GetName()
            ));
        }
    }

    // add this module to the compilation unit
    visitor->GetCompilationUnit()->m_module_tree.Open(mod.get());

    if (update_tree_link) {
        mod->SetImportTreeLink(visitor->GetCompilationUnit()->m_module_tree.TopNode());
    }

    // function to copy nested modules 
    std::function<void(TreeNode<Module*>*)> copy_nodes =

    [visitor, &copy_nodes, &check_lookup, &update_tree_link]
    (TreeNode<Module*> *link) {
        ASSERT(link != nullptr);
        ASSERT(link->m_value != nullptr);

        for (auto *sibling : link->m_siblings) {
            visitor->GetCompilationUnit()->m_module_tree.Open(sibling->m_value);

            if (update_tree_link) {
                sibling->m_value->SetImportTreeLink(sibling);
            }

            if (check_lookup) {
                if (visitor->GetCompilationUnit()->LookupModule(sibling->m_value->GetName())) {
                    visitor->GetCompilationUnit()->GetErrorList().AddError(CompilerError(
                        LEVEL_ERROR,
                        Msg_module_already_defined,
                        sibling->m_value->GetLocation(),
                        sibling->m_value->GetName()
                    ));
                }
            }

            copy_nodes(sibling);

            visitor->GetCompilationUnit()->m_module_tree.Close();
        }
    };

    // copy all nested modules
    copy_nodes(mod->GetImportTreeLink());

    // close module
    visitor->GetCompilationUnit()->m_module_tree.Close();
}

bool AstImport::TryOpenFile(const std::string &path, std::ifstream &is)
{
    is.open(path, std::ios::in | std::ios::ate);
    return is.is_open();
}

void AstImport::PerformImport(
    AstVisitor *visitor,
    Module *mod,
    const std::string &filepath)
{
    ASSERT(visitor != nullptr);
    ASSERT(mod != nullptr);

    // parse path into vector
    std::vector<std::string> path_vec = str_util::split_path(filepath);
    // canonicalize the vector
    path_vec = str_util::canonicalize_path(path_vec);
    // put it back into a string
    const std::string canon_path = str_util::path_to_str(path_vec);

    // first, check if the file has already been imported somewhere in this compilation unit
    const auto it = visitor->GetCompilationUnit()->m_imported_modules.find(canon_path);
    if (it != visitor->GetCompilationUnit()->m_imported_modules.end()) {
        // imported file found, so just re-open all
        // modules that belong to the file into this scope
        for (std::shared_ptr<Module> &mod : it->second) {
            AstImport::CopyModules(visitor, mod);
        }
    } else {
        // file hasn't been imported, so open it
        std::ifstream file;

        if (!TryOpenFile(filepath, file)) {
            visitor->GetCompilationUnit()->GetErrorList().AddError(CompilerError(
                LEVEL_ERROR,
                Msg_could_not_open_file,
                m_location,
                filepath
            ));
        } else {
            // get number of bytes
            size_t max = file.tellg();
            // seek to beginning
            file.seekg(0, std::ios::beg);
            // load stream into file buffer
            SourceFile source_file(filepath, max);
            file.read(source_file.GetBuffer(), max);

            // use the lexer and parser on this file buffer
            TokenStream token_stream;
            Lexer lexer(SourceStream(&source_file), &token_stream, visitor->GetCompilationUnit());
            lexer.Analyze();

            Parser parser(&m_ast_iterator, &token_stream, visitor->GetCompilationUnit());
            parser.Parse();

            SemanticAnalyzer semantic_analyzer(&m_ast_iterator, visitor->GetCompilationUnit());
            semantic_analyzer.Analyze();
        }
    }
}

void AstImport::Build(AstVisitor *visitor, Module *mod)
{
    m_ast_iterator.ResetPosition();

    // compile the imported module
    Compiler compiler(&m_ast_iterator, visitor->GetCompilationUnit());
    compiler.Compile();
}

void AstImport::Optimize(AstVisitor *visitor, Module *mod)
{
    m_ast_iterator.ResetPosition();

    // optimize the imported module
    Optimizer optimizer(&m_ast_iterator, visitor->GetCompilationUnit());
    optimizer.Optimize();
}
