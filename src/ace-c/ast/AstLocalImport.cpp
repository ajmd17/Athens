#include <ace-c/ast/AstLocalImport.hpp>
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

AstLocalImport::AstLocalImport(const std::string &path, const SourceLocation &location)
    : AstImport(location),
      m_path(path)
{
}

void AstLocalImport::Visit(AstVisitor *visitor, Module *mod)
{
    // find the folder which the current file is in
    std::string dir;
    size_t index = m_location.GetFileName().find_last_of("/\\");
    if (index != std::string::npos) {
        dir = m_location.GetFileName().substr(0, index) + "/";
    }

    // create relative path
    std::string filepath = dir + m_path;

    // parse into vector
    std::vector<std::string> path_vec = str_util::split_path(filepath);
    // canonicalize the vector
    path_vec = str_util::canonicalize_path(path_vec);
    // put it back into a string
    std::string canon_path = str_util::path_to_str(path_vec);

    // first, check if the file has already been imported somewhere in this compilation unit
    auto it = visitor->GetCompilationUnit()->m_imported_modules.find(canon_path);
    if (it != visitor->GetCompilationUnit()->m_imported_modules.end()) {
        // imported file found, so just re-open all
        // modules that belong to the file into this scope
        for (std::shared_ptr<Module> mod : it->second) {
            // add this module to the compilation unit
            visitor->GetCompilationUnit()->m_module_tree.Open(mod.get());

            TreeNode<Module*> *new_link = visitor->GetCompilationUnit()->m_module_tree.TopNode();

            // copy all nested modules

            std::function<void(TreeNode<Module*>*)> copy_nodes = 
                [visitor, &copy_nodes](TreeNode<Module*> *link) {
                    ASSERT(link != nullptr);
                    for (auto *sibling : link->m_siblings) {
                        visitor->GetCompilationUnit()->m_module_tree.Open(sibling->m_value);
                        copy_nodes(sibling);
                        visitor->GetCompilationUnit()->m_module_tree.Close();
                    }
                };

            copy_nodes(mod->GetImportTreeLink());

            // close module
            visitor->GetCompilationUnit()->m_module_tree.Close();
        }
    } else {
        // file hasn't been imported, so open it
        std::ifstream file(filepath, std::ios::in | std::ios::ate);

        if (!file.is_open()) {
            visitor->GetCompilationUnit()->GetErrorList().AddError(CompilerError(
                Level_fatal, Msg_could_not_open_file, m_location, filepath));
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

void AstLocalImport::Recreate(std::ostringstream &ss)
{
    m_ast_iterator.ResetPosition();
    Minifier minifier(&m_ast_iterator);
    minifier.Minify(ss);
}
