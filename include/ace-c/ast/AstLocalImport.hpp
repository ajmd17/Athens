#ifndef AST_LOCAL_IMPORT_HPP
#define AST_LOCAL_IMPORT_HPP

#include <ace-c/ast/AstImport.hpp>

#include <string>

class AstLocalImport : public AstImport {
public:
    AstLocalImport(const std::string &path, const SourceLocation &location);

    inline const std::string &GetPath() const { return m_path; }

    virtual void Visit(AstVisitor *visitor, Module *mod) override;
    virtual void Recreate(std::ostringstream &ss) override;
    virtual Pointer<AstStatement> Clone() const override;

protected:
    std::string m_path;

    inline Pointer<AstLocalImport> CloneImpl() const
    {
        return Pointer<AstLocalImport>(new AstLocalImport(
            m_path,
            m_location));
    }
};

#endif