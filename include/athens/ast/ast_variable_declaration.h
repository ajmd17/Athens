#ifndef AST_VARIABLE_DECLARATION_H
#define AST_VARIABLE_DECLARATION_H

#include <athens/ast/ast_declaration.h>
#include <athens/ast/ast_expression.h>

#include <memory>

class AstVariableDeclaration : public AstDeclaration {
public:
    AstVariableDeclaration(const std::string &name, 
        const std::shared_ptr<AstExpression> &assignment,
        const SourceLocation &location);
    virtual ~AstVariableDeclaration() = default;

    virtual void Visit(AstVisitor *visitor);
    virtual void Build(AstVisitor *visitor) const;
    virtual void Optimize(AstVisitor *visitor);

protected:
    std::shared_ptr<AstExpression> m_assignment;
};

#endif