#ifndef AST_VARIABLE_HPP
#define AST_VARIABLE_HPP

#include <athens/ast/ast_identifier.hpp>

class AstVariable : public AstIdentifier {
public:
    AstVariable(const std::string &name, const SourceLocation &location);
    virtual ~AstVariable() = default;

    virtual void Visit(AstVisitor *visitor) override;
    virtual void Build(AstVisitor *visitor) override;
    virtual void Optimize(AstVisitor *visitor) override;

    virtual int IsTrue() const override;
    virtual bool MayHaveSideEffects() const override;
};

#endif
