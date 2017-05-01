#ifndef PARSER_HPP
#define PARSER_HPP

#include <ace-c/TokenStream.hpp>
#include <ace-c/SourceLocation.hpp>
#include <ace-c/CompilationUnit.hpp>
#include <ace-c/AstIterator.hpp>
#include <ace-c/Keywords.hpp>
#include <ace-c/ObjectType.hpp>
#include <ace-c/ast/AstModuleDeclaration.hpp>
#include <ace-c/ast/AstDirective.hpp>
#include <ace-c/ast/AstVariableDeclaration.hpp>
#include <ace-c/ast/AstFunctionDefinition.hpp>
#include <ace-c/ast/AstFunctionExpression.hpp>
#include <ace-c/ast/AstArrayExpression.hpp>
#include <ace-c/ast/AstTypeDefinition.hpp>
#include <ace-c/ast/AstStatement.hpp>
#include <ace-c/ast/AstExpression.hpp>
#include <ace-c/ast/AstImport.hpp>
#include <ace-c/ast/AstLocalImport.hpp>
#include <ace-c/ast/AstInteger.hpp>
#include <ace-c/ast/AstFloat.hpp>
#include <ace-c/ast/AstString.hpp>
#include <ace-c/ast/AstBinaryExpression.hpp>
#include <ace-c/ast/AstUnaryExpression.hpp>
#include <ace-c/ast/AstFunctionCall.hpp>
#include <ace-c/ast/AstCallExpression.hpp>
#include <ace-c/ast/AstArgument.hpp>
#include <ace-c/ast/AstVariable.hpp>
#include <ace-c/ast/AstModuleAccess.hpp>
#include <ace-c/ast/AstMemberAccess.hpp>
#include <ace-c/ast/AstMember.hpp>
#include <ace-c/ast/AstArrayAccess.hpp>
#include <ace-c/ast/AstTrue.hpp>
#include <ace-c/ast/AstFalse.hpp>
#include <ace-c/ast/AstNil.hpp>
#include <ace-c/ast/AstBlock.hpp>
#include <ace-c/ast/AstIfStatement.hpp>
#include <ace-c/ast/AstWhileLoop.hpp>
#include <ace-c/ast/AstPrintStatement.hpp>
#include <ace-c/ast/AstTryCatch.hpp>
#include <ace-c/ast/AstTypeSpecification.hpp>
#include <ace-c/ast/AstTypeOfExpression.hpp>
#include <ace-c/ast/AstReturnStatement.hpp>

#include <string>

class Parser {
public:
    Parser(AstIterator *ast_iterator, TokenStream *token_stream,
        CompilationUnit *compilation_unit);
    Parser(const Parser &other);

    /** Generate an AST structure from the token stream */
    void Parse(bool expect_module_decl = true);

private:
    AstIterator *m_ast_iterator;
    TokenStream *m_token_stream;
    CompilationUnit *m_compilation_unit;

    Token Match(TokenClass token_class, bool read = false);
    Token MatchAhead(TokenClass token_class, int n);
    Token MatchKeyword(Keywords keyword, bool read = false);
    Token MatchKeywordAhead(Keywords keyword, int n);
    Token MatchOperator(const Operator *op, bool read = false);
    Token Expect(TokenClass token_class, bool read = false);
    Token ExpectKeyword(Keywords keyword, bool read = false);
    Token ExpectOperator(const Operator *op, bool read = false);
    Token MatchIdentifier(bool allow_keyword = false, bool read = false);
    Token ExpectIdentifier(bool allow_keyword = false, bool read = false);
    bool ExpectEndOfStmt();
    SourceLocation CurrentLocation() const;

    /** Skip past newlines and semicolons.
        Returns true if any were hit.
    */
    void SkipStatementTerminators();

    int OperatorPrecedence(const Operator *&out);

    std::shared_ptr<AstStatement> ParseStatement(bool top_level = false);
    std::shared_ptr<AstModuleDeclaration> ParseModuleDeclaration();
    std::shared_ptr<AstDirective> ParseDirective();
    std::shared_ptr<AstExpression> ParseTerm();
    std::shared_ptr<AstExpression> ParseParentheses();
    std::shared_ptr<AstInteger> ParseIntegerLiteral();
    std::shared_ptr<AstFloat> ParseFloatLiteral();
    std::shared_ptr<AstString> ParseStringLiteral();
    std::shared_ptr<AstIdentifier> ParseIdentifier(bool allow_keyword = false);
    std::shared_ptr<AstArgument> ParseArgument();
    std::shared_ptr<AstCallExpression> ParseCallExpression(std::shared_ptr<AstExpression> target);
    std::shared_ptr<AstFunctionCall> ParseFunctionCall(bool allow_keyword = false);
    std::shared_ptr<AstFunctionCall> ParseFunctionCallNoParams(bool allow_keyword = false);
    std::shared_ptr<AstModuleAccess> ParseModuleAccess();
    std::shared_ptr<AstMember> ParseMemberExpression(std::shared_ptr<AstExpression> target);
    std::shared_ptr<AstMemberAccess> ParseMemberAccess(std::shared_ptr<AstExpression> target);
    std::shared_ptr<AstArrayAccess> ParseArrayAccess(std::shared_ptr<AstExpression> target);
    std::shared_ptr<AstTrue> ParseTrue();
    std::shared_ptr<AstFalse> ParseFalse();
    std::shared_ptr<AstNil> ParseNil();
    std::shared_ptr<AstBlock> ParseBlock();
    std::shared_ptr<AstIfStatement> ParseIfStatement();
    std::shared_ptr<AstWhileLoop> ParseWhileLoop();
    std::shared_ptr<AstPrintStatement> ParsePrintStatement();
    std::shared_ptr<AstTryCatch> ParseTryCatchStatement();
    std::shared_ptr<AstExpression> ParseBinaryExpression(int expr_prec,
        std::shared_ptr<AstExpression> left);
    std::shared_ptr<AstExpression> ParseUnaryExpression();
    std::shared_ptr<AstExpression> ParseExpression();
    std::shared_ptr<AstTypeSpecification> ParseTypeSpecification();
#if 0
    std::shared_ptr<AstTypeContractExpression> ParseTypeContract();
    std::shared_ptr<AstTypeContractExpression> ParseTypeContractExpression();
    std::shared_ptr<AstTypeContractExpression> ParseTypeContractTerm();
    std::shared_ptr<AstTypeContractExpression> ParseTypeContractBinaryExpression(int expr_prec,
        std::shared_ptr<AstTypeContractExpression> left);
#endif
    std::shared_ptr<AstVariableDeclaration> ParseVariableDeclaration(bool require_keyword = true, bool allow_keyword_names = false);
    std::shared_ptr<AstFunctionDefinition> ParseFunctionDefinition(bool require_keyword = true);
    std::shared_ptr<AstFunctionExpression> ParseFunctionExpression(bool require_keyword = true,
        std::vector<std::shared_ptr<AstParameter>> params = {}, bool is_async = false);
    std::shared_ptr<AstArrayExpression> ParseArrayExpression();
    std::shared_ptr<AstExpression> ParseAsyncExpression();
    std::shared_ptr<AstExpression> ParseValueOfExpression();
    std::shared_ptr<AstTypeOfExpression> ParseTypeOfExpression();
    std::vector<std::shared_ptr<AstParameter>> ParseFunctionParameters();
    std::shared_ptr<AstTypeDefinition> ParseTypeDefinition();
    std::shared_ptr<AstImport> ParseImport();
    std::shared_ptr<AstLocalImport> ParseLocalImport();
    std::shared_ptr<AstReturnStatement> ParseReturnStatement();
};

#endif
