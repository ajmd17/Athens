#ifndef AST_ARRAY_EXPRESSION_HPP
#define AST_ARRAY_EXPRESSION_HPP

#include <ace-c/ast/AstExpression.hpp>

#include <memory>
#include <vector>

class AstArrayExpression : public AstExpression {
public:
    AstArrayExpression(const std::vector<std::shared_ptr<AstExpression>> &members,
        const SourceLocation &location);
    virtual ~AstArrayExpression() = default;

    virtual void Visit(AstVisitor *visitor, Module *mod) override;
    virtual void Build(AstVisitor *visitor, Module *mod) override;
    virtual void Optimize(AstVisitor *visitor, Module *mod) override;
    virtual void Recreate(std::ostringstream &ss) override;

    virtual int IsTrue() const override;
    virtual bool MayHaveSideEffects() const override;
    virtual SymbolTypePtr_t GetSymbolType() const override;

protected:
    std::vector<std::shared_ptr<AstExpression>> m_members;
    SymbolTypePtr_t m_held_type;
};

#endif