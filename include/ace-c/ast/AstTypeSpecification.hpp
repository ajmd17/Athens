#ifndef AST_TYPE_SPECIFICATION_HPP
#define AST_TYPE_SPECIFICATION_HPP

#include <ace-c/ast/AstStatement.hpp>
#include <ace-c/SymbolType.hpp>

#include <string>
#include <memory>

class AstTypeSpecification: public AstStatement {
public:
    AstTypeSpecification(const std::string &left,
        const std::vector<std::shared_ptr<AstTypeSpecification>> &generic_params,
        const std::shared_ptr<AstTypeSpecification> &right,
        const SourceLocation &location);
    virtual ~AstTypeSpecification() = default;

    virtual void Visit(AstVisitor *visitor, Module *mod) override;
    virtual void Build(AstVisitor *visitor, Module *mod) override;
    virtual void Optimize(AstVisitor *visitor, Module *mod) override;
    virtual void Recreate(std::ostringstream &ss) override;

    inline const SymbolTypePtr_t &GetSymbolType() const { return m_symbol_type; }

private:
    std::string m_left;
    std::vector<std::shared_ptr<AstTypeSpecification>> m_generic_params;
    std::shared_ptr<AstTypeSpecification> m_right;

    /** Set while analyzing */
    SymbolTypePtr_t m_symbol_type;

    /** Is module access chained */
    bool m_is_chained;
};

#endif
