#ifndef AST_TYPE_CONTRACT_HPP
#define AST_TYPE_CONTRACT_HPP

#include <ace-c/ast/ast_identifier.hpp>
#include <ace-c/ast/ast_type_specification.hpp>
#include <ace-c/object_type.hpp>
#include <ace-c/operator.hpp>
#include <ace-c/ast_visitor.hpp>
#include <ace-c/type_contract.hpp>

#include <string>
#include <memory>

class AstTypeContractExpression : public AstStatement {
public:
    AstTypeContractExpression(const SourceLocation &location);
    virtual ~AstTypeContractExpression() = default;

    virtual void Visit(AstVisitor *visitor, Module *mod) override = 0;
    virtual void Build(AstVisitor *visitor, Module *mod) override = 0;
    virtual void Optimize(AstVisitor *visitor, Module *mod) override = 0;

    virtual bool Satisfies(AstVisitor *visitor, const ObjectType &object_type) const = 0;
};

class AstTypeContractTerm : public AstTypeContractExpression {
public:
    AstTypeContractTerm(const std::string &type_contract_operation,
        const std::shared_ptr<AstTypeSpecification> &type_spec,
        const SourceLocation &location);
    virtual ~AstTypeContractTerm() = default;

    virtual void Visit(AstVisitor *visitor, Module *mod) override;
    virtual void Build(AstVisitor *visitor, Module *mod) override;
    virtual void Optimize(AstVisitor *visitor, Module *mod) override;

    virtual bool Satisfies(AstVisitor *visitor, const ObjectType &object_type) const override;

private:
    std::string m_type_contract_operation;
    std::shared_ptr<AstTypeSpecification> m_type_spec;

    TypeContract::Type m_type;
};

class AstTypeContractBinaryExpression: public AstTypeContractExpression {
public:
    AstTypeContractBinaryExpression(const std::shared_ptr<AstTypeContractExpression> &left,
        const std::shared_ptr<AstTypeContractExpression> &right,
        const Operator *op,
        const SourceLocation &location);
    virtual ~AstTypeContractBinaryExpression() = default;

    virtual void Visit(AstVisitor *visitor, Module *mod) override;
    virtual void Build(AstVisitor *visitor, Module *mod) override;
    virtual void Optimize(AstVisitor *visitor, Module *mod) override;

    virtual bool Satisfies(AstVisitor *visitor, const ObjectType &object_type) const override;

private:
    std::shared_ptr<AstTypeContractExpression> m_left;
    std::shared_ptr<AstTypeContractExpression> m_right;
    const Operator *m_op;
};

#endif