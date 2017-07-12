#ifndef AST_BLOCK_EXPRESSION_HPP
#define AST_BLOCK_EXPRESSION_HPP

#include <ace-c/ast/AstExpression.hpp>
#include <ace-c/ast/AstBlock.hpp>

#include <string>
#include <vector>
#include <memory>

class AstBlockExpression : public AstExpression {
public:
    AstBlockExpression(
        const std::shared_ptr<AstBlock> &block,
        const SourceLocation &location);
    virtual ~AstBlockExpression() = default;

    inline const std::shared_ptr<AstBlock> &GetBlock() const
        { return m_block; }

    virtual void Visit(AstVisitor *visitor, Module *mod) override;
    virtual std::unique_ptr<Buildable> Build(AstVisitor *visitor, Module *mod) override;
    virtual void Optimize(AstVisitor *visitor, Module *mod) override;
    
    virtual Pointer<AstStatement> Clone() const override;

    virtual Tribool IsTrue() const override;
    virtual bool MayHaveSideEffects() const override;
    virtual SymbolTypePtr_t GetSymbolType() const override;

protected:
    std::shared_ptr<AstBlock> m_block;

    // set while analyzing
    SymbolTypePtr_t m_symbol_type;
    int m_num_locals;
    bool m_last_is_return;

    inline Pointer<AstBlockExpression> CloneImpl() const
    {
        return Pointer<AstBlockExpression>(new AstBlockExpression(
            CloneAstNode(m_block),
            m_location
        ));
    }
};

#endif
