#ifndef AST_TRUE_HPP
#define AST_TRUE_HPP

#include <ace-c/ast/AstConstant.hpp>

class AstTrue : public AstConstant {
public:
    AstTrue(const SourceLocation &location);

    virtual std::unique_ptr<Buildable> Build(AstVisitor *visitor, Module *mod) override;
    
    virtual Pointer<AstStatement> Clone() const override;

    virtual Tribool IsTrue() const override;
    virtual bool IsNumber() const override;
    virtual ace::aint32 IntValue() const override;
    virtual ace::afloat32 FloatValue() const override;
    virtual SymbolTypePtr_t GetSymbolType() const override;

    virtual std::shared_ptr<AstConstant> HandleOperator(Operators op_type, AstConstant *right) const override;

private:
    inline Pointer<AstTrue> CloneImpl() const
    {
        return Pointer<AstTrue>(new AstTrue(
            m_location
        ));
    }
};

#endif
