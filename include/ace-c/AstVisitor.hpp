#ifndef AST_VISITOR_HPP
#define AST_VISITOR_HPP

#include <ace-c/AstIterator.hpp>
#include <ace-c/ErrorList.hpp>
#include <ace-c/CompilerError.hpp>
#include <ace-c/CompilationUnit.hpp>

class AstVisitor {
public:
    AstVisitor(AstIterator *ast_iterator,
        CompilationUnit *compilation_unit);
    virtual ~AstVisitor() = default;

    inline AstIterator *GetIterator() const { return m_ast_iterator; }
    inline CompilationUnit *GetCompilationUnit() const { return m_compilation_unit; }

    /** If expr is false, the given error is added to the error list. */
    bool Assert(bool expr, const CompilerError &error);

    // void Visit(AstStatement *);

    // virtual void Visit(AstActionExpression *) = 0;
    // virtual void Visit(AstAliasDeclaration *) = 0;
    // virtual void Visit(AstArgument *) = 0;
    // virtual void Visit(AstArgumentList *) = 0;
    // virtual void Visit(AstArrayAccess *) = 0;
    // virtual void Visit(AstArrayExpression *) = 0;
    // virtual void Visit(AstBinaryExpression *) = 0;
    // virtual void Visit(AstBlock *) = 0;
    // virtual void Visit(AstCallExpression *) = 0;
    // virtual void Visit(AstDeclaration *) = 0;
    // virtual void Visit(AstDirective *) = 0;
    // virtual void Visit(AstEvent *) = 0;
    // virtual void Visit(AstFalse *) = 0;
    // virtual void Visit(AstFileImport *) = 0;
    // virtual void Visit(AstFloat *) = 0;
    // virtual void Visit(AstFunctionDefinition *) = 0;
    // virtual void Visit(AstFunctionExpression *) = 0;
    // virtual void Visit(AstHasExpression *) = 0;
    // virtual void Visit(AstIdentifier *) = 0;
    // virtual void Visit(AstIfStatement *) = 0;
    // virtual void Visit(AstImport *) = 0;
    // virtual void Visit(AstInteger *) = 0;
    // virtual void Visit(AstMember *) = 0;
    // virtual void Visit(AstModuleAccess *) = 0;
    // virtual void Visit(AstModuleDeclaration *) = 0;
    // virtual void Visit(AstModuleImport *) = 0;
    // virtual void Visit(AstModuleProperty *) = 0;
    // virtual void Visit(AstNewExpression *) = 0;
    // virtual void Visit(AstNil *) = 0;
    // virtual void Visit(AstNodeBuilder *) = 0;
    // virtual void Visit(AstObject *) = 0;
    // virtual void Visit(AstParameter *) = 0;
    // virtual void Visit(AstPrintStatement *) = 0;
    // virtual void Visit(AstReturnStatement *) = 0;
    // virtual void Visit(AstSelf *) = 0;
    // virtual void Visit(AstString *) = 0;
    // virtual void Visit(AstTrue *) = 0;
    // virtual void Visit(AstTryCatch *) = 0;
    // virtual void Visit(AstTupleExpression *) = 0;
    // virtual void Visit(AstTypeAlias *) = 0;
    // virtual void Visit(AstTypeDefinition *) = 0;
    // virtual void Visit(AstTypeOfExpression *) = 0;
    // virtual void Visit(AstTypeSpecification *) = 0;
    // virtual void Visit(AstUnaryExpression *) = 0;
    // virtual void Visit(AstUndefined *) = 0;
    // virtual void Visit(AstVariable *) = 0;
    // virtual void Visit(AstVariableDeclaration *) = 0;
    // virtual void Visit(AstWhileLoop *) = 0;
    // virtual void Visit(AstYieldStatement *) = 0;

protected:
    AstIterator *m_ast_iterator;
    CompilationUnit *m_compilation_unit;
};

#endif
