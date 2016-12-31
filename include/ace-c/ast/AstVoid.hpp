#ifndef AST_VOID_HPP
#define AST_VOID_HPP

#if 0

#include <ace-c/ast/AstConstant.hpp>

class AstVoid : public AstConstant {
public:
    AstVoid(const SourceLocation &location);

    virtual void Build(AstVisitor *visitor, Module *mod) override;
    virtual void Recreate(std::ostringstream &ss) override;

    virtual int IsTrue() const override;
    virtual bool IsNumber() const override;
    virtual a_int IntValue() const override;
    virtual a_float FloatValue() const override;

    // Arithmetic operators
    virtual std::shared_ptr<AstConstant> operator+(AstConstant *right) const override;
    virtual std::shared_ptr<AstConstant> operator-(AstConstant *right) const override;
    virtual std::shared_ptr<AstConstant> operator*(AstConstant *right) const override;
    virtual std::shared_ptr<AstConstant> operator/(AstConstant *right) const override;
    virtual std::shared_ptr<AstConstant> operator%(AstConstant *right) const override;

    // Bitwise operators
    virtual std::shared_ptr<AstConstant> operator^(AstConstant *right) const override;
    virtual std::shared_ptr<AstConstant> operator&(AstConstant *right) const override;
    virtual std::shared_ptr<AstConstant> operator|(AstConstant *right) const override;
    virtual std::shared_ptr<AstConstant> operator<<(AstConstant *right) const override;
    virtual std::shared_ptr<AstConstant> operator>>(AstConstant *right) const override;

    // Logical operators
    virtual std::shared_ptr<AstConstant> operator&&(AstConstant *right) const override;
    virtual std::shared_ptr<AstConstant> operator||(AstConstant *right) const override;

    // Comparison operators
    virtual std::shared_ptr<AstConstant> operator<(AstConstant *right) const override;
    virtual std::shared_ptr<AstConstant> operator>(AstConstant *right) const override;
    virtual std::shared_ptr<AstConstant> operator<=(AstConstant *right) const override;
    virtual std::shared_ptr<AstConstant> operator>=(AstConstant *right) const override;
    virtual std::shared_ptr<AstConstant> Equals(AstConstant *right) const override;
    
    // Unary operators
    virtual std::shared_ptr<AstConstant> operator-() const override;
    virtual std::shared_ptr<AstConstant> operator~() const override;
    virtual std::shared_ptr<AstConstant> operator!() const override;
};

#endif

#endif
