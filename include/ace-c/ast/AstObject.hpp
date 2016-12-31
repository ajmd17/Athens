#ifndef AST_OBJECT_HPP
#define AST_OBJECT_HPP

#include <ace-c/ast/AstExpression.hpp>

class AstObject : public AstExpression {
public:
    AstObject(const SymbolTypePtr_t &symbol_type, const SourceLocation &location);
    virtual ~AstObject() = default;

    virtual void Visit(AstVisitor *visitor, Module *mod) override;
    virtual void Build(AstVisitor *visitor, Module *mod) override;
    virtual void Optimize(AstVisitor *visitor, Module *mod) override;
    virtual void Recreate(std::ostringstream &ss) override;

    virtual int IsTrue() const override;
    virtual bool MayHaveSideEffects() const override;
    virtual SymbolTypePtr_t GetSymbolType() const override;

private:
    SymbolTypePtr_t m_symbol_type;
};

#endif
