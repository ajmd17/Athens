#ifndef AST_INTEGER_HPP
#define AST_INTEGER_HPP

#include <athens/ast/ast_constant.hpp>

#include <cstdint>

class AstInteger : public AstConstant {
public:
    AstInteger(a_int value, const SourceLocation &location);

    virtual void Build(AstVisitor *visitor, Module *mod) override;

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
    a_int m_value;
};

#endif
