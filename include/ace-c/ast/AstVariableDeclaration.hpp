#ifndef AST_VARIABLE_DECLARATION_HPP
#define AST_VARIABLE_DECLARATION_HPP

#include <ace-c/ast/AstDeclaration.hpp>
#include <ace-c/ast/AstExpression.hpp>
#include <ace-c/ast/AstTypeSpecification.hpp>

#include <memory>

class AstVariableDeclaration : public AstDeclaration {
public:
    AstVariableDeclaration(const std::string &name,
        const std::shared_ptr<AstTypeSpecification> &type_specification,
        const std::shared_ptr<AstExpression> &assignment,
        const SourceLocation &location);
    virtual ~AstVariableDeclaration() = default;

    inline const std::shared_ptr<AstExpression> &GetAssignment() const
        { return m_assignment; }

    virtual void Visit(AstVisitor *visitor, Module *mod) override;
    virtual void Build(AstVisitor *visitor, Module *mod) override;
    virtual void Optimize(AstVisitor *visitor, Module *mod) override;
    virtual void Recreate(std::ostringstream &ss) override;
    virtual Pointer<AstStatement> Clone() const override;

    bool m_assignment_already_visited;

protected:
    std::shared_ptr<AstTypeSpecification> m_type_specification;
    std::shared_ptr<AstExpression> m_assignment;

    // set while analyzing
    std::shared_ptr<AstExpression> m_real_assignment;

    inline Pointer<AstVariableDeclaration> CloneImpl() const
    {
        return Pointer<AstVariableDeclaration>(new AstVariableDeclaration(
            m_name,
            CloneAstNode(m_type_specification),
            CloneAstNode(m_assignment),
            m_location));
    }
};

#endif
