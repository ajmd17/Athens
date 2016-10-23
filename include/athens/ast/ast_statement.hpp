#ifndef AST_STATEMENT_HPP
#define AST_STATEMENT_HPP

#include <athens/source_location.hpp>

#include <memory>

// Forward declarations
class AstVisitor;
class Module;

class AstStatement {
    friend class AstIterator;
public:
    AstStatement(const SourceLocation &location);
    virtual ~AstStatement() = default;

    inline SourceLocation &GetLocation() { return m_location; }
    inline const SourceLocation &GetLocation() const { return m_location; }

    virtual void Visit(AstVisitor *visitor, Module *mod) = 0;
    virtual void Build(AstVisitor *visitor, Module *mod) = 0;
    virtual void Optimize(AstVisitor *visitor, Module *mod) = 0;

protected:
    SourceLocation m_location;
};

#endif
