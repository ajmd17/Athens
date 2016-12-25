#ifndef AST_TYPE_SPECIFICATION_HPP
#define AST_TYPE_SPECIFICATION_HPP

#include <ace-c/ast/ast_identifier.hpp>
#include <ace-c/object_type.hpp>

#include <string>
#include <memory>

class AstTypeSpecification: public AstStatement {
public:
    AstTypeSpecification(const std::string &left,
        const std::shared_ptr<AstTypeSpecification> &right,
        const SourceLocation &location);
    virtual ~AstTypeSpecification() = default;

    virtual void Visit(AstVisitor *visitor, Module *mod) override;
    virtual void Build(AstVisitor *visitor, Module *mod) override;
    virtual void Optimize(AstVisitor *visitor, Module *mod) override;
    virtual void Recreate(std::ostringstream &ss) override;

    inline const ObjectType &GetObjectType() const { return m_object_type; }

private:
    std::string m_left;
    std::shared_ptr<AstTypeSpecification> m_right;

    /** Set while analyzing */
    ObjectType m_object_type;

    /** Is module access chained */
    bool m_is_chained;
};

#endif
