#ifndef AST_WHILE_LOOP_H
#define AST_WHILE_LOOP_H

#include <athens/ast/ast_statement.h>
#include <athens/ast/ast_expression.h>
#include <athens/ast/ast_block.h>

#include <memory>

class AstWhileLoop : public AstStatement {
public:
    AstWhileLoop(std::unique_ptr<AstExpression> &&conditional, 
        std::unique_ptr<AstBlock> &&block,
        const SourceLocation &location);
    virtual ~AstWhileLoop() = default;

    virtual void Visit(AstVisitor *visitor);

private:
    std::unique_ptr<AstExpression> m_conditional;
    std::unique_ptr<AstBlock> m_block;
};

#endif