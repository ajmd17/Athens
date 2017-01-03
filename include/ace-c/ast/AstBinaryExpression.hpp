#ifndef AST_BINARY_EXPRESSION_HPP
#define AST_BINARY_EXPRESSION_HPP

#include <ace-c/ast/AstExpression.hpp>
#include <ace-c/ast/AstMemberAccess.hpp>
#include <ace-c/ast/AstVariableDeclaration.hpp>
#include <ace-c/Operator.hpp>

class AstBinaryExpression : public AstExpression {
public:
    AstBinaryExpression(const std::shared_ptr<AstExpression> &left,
        const std::shared_ptr<AstExpression> &right,
        const Operator *op,
        const SourceLocation &location);

    inline const std::shared_ptr<AstExpression> &GetLeft() const { return m_left; }
    inline const std::shared_ptr<AstExpression> &GetRight() const { return m_right; }

    virtual void Visit(AstVisitor *visitor, Module *mod) override;
    virtual void Build(AstVisitor *visitor, Module *mod) override;
    virtual void Optimize(AstVisitor *visitor, Module *mod) override;
    virtual void Recreate(std::ostringstream &ss) override;
    virtual Pointer<AstStatement> Clone() const override;

    virtual int IsTrue() const override;
    virtual bool MayHaveSideEffects() const override;
    virtual SymbolTypePtr_t GetSymbolType() const override;

private:
    std::shared_ptr<AstExpression> m_left;
    std::shared_ptr<AstExpression> m_right;
    const Operator *m_op;

    // if the operator is overloaded and it is actually a function call
    std::shared_ptr<AstMemberAccess> m_member_access;

    // if the expression is lazy declaration
    std::shared_ptr<AstVariableDeclaration> m_variable_declaration;

    std::shared_ptr<AstVariableDeclaration> CheckLazyDeclaration(AstVisitor *visitor, Module *mod);

    inline std::shared_ptr<AstBinaryExpression> CloneImpl() const
    {
        return std::shared_ptr<AstBinaryExpression>(
            new AstBinaryExpression(
                CloneAstNode(m_left),
                CloneAstNode(m_right),
                m_op, m_location));
    }
};

#endif
