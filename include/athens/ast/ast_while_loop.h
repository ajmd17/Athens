#ifndef AST_WHILE_LOOP_H
#define AST_WHILE_LOOP_H

#include <athens/ast/ast_statement.h>
#include <athens/ast/ast_expression.h>
#include <athens/ast/ast_block.h>

#include <memory>

class AstWhileLoop : public AstStatement {
public:
    AstWhileLoop(const std::shared_ptr<AstExpression> &conditional, 
        const std::shared_ptr<AstBlock> &block,
        const SourceLocation &location);
    virtual ~AstWhileLoop() = default;

    virtual void Visit(AstVisitor *visitor) override;
    virtual void Build(AstVisitor *visitor) override;
    virtual void Optimize(AstVisitor *visitor) override;

private:
    std::shared_ptr<AstExpression> m_conditional;
    std::shared_ptr<AstBlock> m_block;
};

#endif