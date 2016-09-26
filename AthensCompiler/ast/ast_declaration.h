#ifndef AST_DECLARATION_H
#define AST_DECLARATION_H

#include "ast_statement.h"

#include <string>

class AstDeclaration : public AstStatement {
public:
    AstDeclaration(const std::string &name, const SourceLocation &location);
    virtual ~AstDeclaration() = default;

    inline const std::string &GetName() const { return m_name; }

protected:
    std::string m_name;
};

#endif