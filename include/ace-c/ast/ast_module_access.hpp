#ifndef AST_MODULE_ACCESS_HPP
#define AST_MODULE_ACCESS_HPP

#include <ace-c/ast/ast_expression.hpp>

#include <string>
#include <vector>
#include <memory>

class AstModuleAccess : public AstExpression {
public:
    AstModuleAccess(const std::string &target,
        const std::shared_ptr<AstExpression> &expr,
        const SourceLocation &location);
    virtual ~AstModuleAccess() = default;

    inline const Module *GetModule() const { return m_mod_access; }
    inline const std::string &GetTarget() const { return m_target; }
    inline const std::shared_ptr<AstExpression> &GetExpression() const { return m_expr; }
    inline void SetChained(bool is_chained) { m_is_chained = is_chained; }

    void PerformLookup(AstVisitor *visitor, Module *mod);

    virtual void Visit(AstVisitor *visitor, Module *mod) override;
    virtual void Build(AstVisitor *visitor, Module *mod) override;
    virtual void Optimize(AstVisitor *visitor, Module *mod) override;
    virtual void Recreate(std::ostringstream &ss) override;

    virtual int IsTrue() const override;
    virtual bool MayHaveSideEffects() const override;
    virtual ObjectType GetObjectType() const override;

private:
    std::string m_target;
    std::shared_ptr<AstExpression> m_expr;
    Module *m_mod_access;
    // is this module access chained to another before it?
    bool m_is_chained;
    bool m_looked_up;
};

#endif
