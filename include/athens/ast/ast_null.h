#ifndef AST_NULL_H
#define AST_NULL_H

#include <athens/ast/ast_constant.h>

class AstNull : public AstConstant {
public:
    AstNull(const SourceLocation &location);

    virtual void Build(AstVisitor *visitor) const;
    virtual int IsTrue() const;
    virtual a_int IntValue() const;
    virtual a_float FloatValue() const;

    // Arithmetic operators
    virtual std::shared_ptr<AstConstant> operator+(
        const std::shared_ptr<AstConstant> &right) const;
    virtual std::shared_ptr<AstConstant> operator-(
        const std::shared_ptr<AstConstant> &right) const;
    virtual std::shared_ptr<AstConstant> operator*(
        const std::shared_ptr<AstConstant> &right) const;
    virtual std::shared_ptr<AstConstant> operator/(
        const std::shared_ptr<AstConstant> &right) const;
    virtual std::shared_ptr<AstConstant> operator%(
        const std::shared_ptr<AstConstant> &right) const;

    // Bitwise operators
    virtual std::shared_ptr<AstConstant> operator^(
        const std::shared_ptr<AstConstant> &right) const;
    virtual std::shared_ptr<AstConstant> operator&(
        const std::shared_ptr<AstConstant> &right) const;
    virtual std::shared_ptr<AstConstant> operator|(
        const std::shared_ptr<AstConstant> &right) const;
    virtual std::shared_ptr<AstConstant> operator<<(
        const std::shared_ptr<AstConstant> &right) const;
    virtual std::shared_ptr<AstConstant> operator>>(
        const std::shared_ptr<AstConstant> &right) const;
    
    // Logical operators
    virtual std::shared_ptr<AstConstant> operator&&(
        const std::shared_ptr<AstConstant> &right) const;
    virtual std::shared_ptr<AstConstant> operator||(
        const std::shared_ptr<AstConstant> &right) const;
};

#endif