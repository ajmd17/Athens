#ifndef AST_FLOAT_H
#define AST_FLOAT_H

#include <athens/ast/ast_constant.h>

class AstFloat : public AstConstant {
public:
    AstFloat(a_float value, const SourceLocation &location);

    virtual void Build(AstVisitor *visitor) override;
    virtual int IsTrue() const override;
    virtual bool IsNumber() const override;
    virtual a_int IntValue() const override;
    virtual a_float FloatValue() const override;

    // Arithmetic operators
    virtual std::shared_ptr<AstConstant> operator+(
        AstConstant *right) const override;
    virtual std::shared_ptr<AstConstant> operator-(
        AstConstant *right) const override;
    virtual std::shared_ptr<AstConstant> operator*(
        AstConstant *right) const override;
    virtual std::shared_ptr<AstConstant> operator/(
        AstConstant *right) const override;
    virtual std::shared_ptr<AstConstant> operator%(
        AstConstant *right) const override;

    // Bitwise operators
    virtual std::shared_ptr<AstConstant> operator^(
        AstConstant *right) const override;
    virtual std::shared_ptr<AstConstant> operator&(
        AstConstant *right) const override;
    virtual std::shared_ptr<AstConstant> operator|(
        AstConstant *right) const override;
    virtual std::shared_ptr<AstConstant> operator<<(
        AstConstant *right) const override;
    virtual std::shared_ptr<AstConstant> operator>>(
        AstConstant *right) const override;
    
    // Logical operators
    virtual std::shared_ptr<AstConstant> operator&&(
        AstConstant *right) const override;
    virtual std::shared_ptr<AstConstant> operator||(
        AstConstant *right) const override;

    virtual std::shared_ptr<AstConstant> Equals(AstConstant *right) const override;

private:
    a_float m_value;
};

#endif