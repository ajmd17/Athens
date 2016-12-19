#ifndef AST_MODULE_DECLARATION_HPP
#define AST_MODULE_DECLARATION_HPP

#include <ace-c/ast/ast_statement.hpp>
#include <ace-c/ast/ast_declaration.hpp>

#include <vector>
#include <memory>

class AstModuleDeclaration : public AstDeclaration {
public:
    AstModuleDeclaration(const std::string &name, const SourceLocation &location);

    inline void AddChild(const std::shared_ptr<AstStatement> &child)
        { m_children.push_back(child); }
    inline std::vector<std::shared_ptr<AstStatement>> &GetChildren()
        { return m_children; }
    inline const std::vector<std::shared_ptr<AstStatement>> &GetChildren() const
        { return m_children; }

    virtual void Visit(AstVisitor *visitor, Module *mod) override;
    virtual void Build(AstVisitor *visitor, Module *mod) override;
    virtual void Optimize(AstVisitor *visitor, Module *mod) override;

private:
    std::vector<std::shared_ptr<AstStatement>> m_children;
    Module *m_this_module;
};

#endif
