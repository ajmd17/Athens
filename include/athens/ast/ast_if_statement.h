#ifndef AST_IF_STATEMENT_H
#define AST_IF_STATEMENT_H

#include <athens/ast/ast_statement.h>
#include <athens/ast/ast_expression.h>
#include <athens/ast/ast_block.h>

#include <memory>

class AstIfStatement : public AstStatement {
public:
    AstIfStatement(const std::shared_ptr<AstExpression> &conditional,
        const std::shared_ptr<AstBlock> &block,
        const std::shared_ptr<AstBlock> &else_block,
        const SourceLocation &location);
    virtual ~AstIfStatement() = default;

    virtual void Visit(AstVisitor *visitor) override;
    virtual void Build(AstVisitor *visitor) override;
    virtual void Optimize(AstVisitor *visitor) override;

private:
    std::shared_ptr<AstExpression> m_conditional;
    std::shared_ptr<AstBlock> m_block;
    std::shared_ptr<AstBlock> m_else_block;
};

#endif
