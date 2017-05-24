#include <ace-c/Parser.hpp>

#include <common/utf8.hpp>

#include <memory>
#include <cstdlib>
#include <cstdio>
#include <sstream>
#include <iostream>

Parser::Parser(AstIterator *ast_iterator, TokenStream *token_stream,
    CompilationUnit *compilation_unit)
    : m_ast_iterator(ast_iterator),
      m_token_stream(token_stream),
      m_compilation_unit(compilation_unit)
{
}

Parser::Parser(const Parser &other)
    : m_ast_iterator(other.m_ast_iterator),
      m_token_stream(other.m_token_stream),
      m_compilation_unit(other.m_compilation_unit)
{
}

Token Parser::Match(TokenClass token_class, bool read)
{
    Token peek = m_token_stream->Peek();
    
    if (peek && peek.GetTokenClass() == token_class) {
        if (read && m_token_stream->HasNext()) {
            m_token_stream->Next();
        }
        
        return peek;
    }
    
    return Token::EMPTY;
}

Token Parser::MatchAhead(TokenClass token_class, int n)
{
    Token peek = m_token_stream->Peek(n);
    
    if (peek && peek.GetTokenClass() == token_class) {
        return peek;
    }
    
    return Token::EMPTY;
}

Token Parser::MatchKeyword(Keywords keyword, bool read)
{
    Token peek = m_token_stream->Peek();
    
    if (peek && peek.GetTokenClass() == TK_KEYWORD) {
        std::string str = Keyword::ToString(keyword);
        if (peek.GetValue() == str) {
            if (read && m_token_stream->HasNext()) {
                m_token_stream->Next();
            }
            
            return peek;
        }
    }
    
    return Token::EMPTY;
}

Token Parser::MatchKeywordAhead(Keywords keyword, int n)
{
    Token peek = m_token_stream->Peek(n);
    
    if (peek && peek.GetTokenClass() == TK_KEYWORD) {
        std::string str = Keyword::ToString(keyword);
        if (peek.GetValue() == str) {
            return peek;
        }
    }
    
    return Token::EMPTY;
}

Token Parser::MatchOperator(const Operator *op, bool read)
{
    Token peek = m_token_stream->Peek();
    
    if (peek && peek.GetTokenClass() == TK_OPERATOR) {
        std::string str = op->ToString();
        if (peek.GetValue() == str) {
            if (read && m_token_stream->HasNext()) {
                m_token_stream->Next();
            }
            return peek;
        }
    }
    
    return Token::EMPTY;
}

Token Parser::Expect(TokenClass token_class, bool read)
{
    Token token = Match(token_class, read);
    
    if (!token) {
        SourceLocation location = CurrentLocation();

        ErrorMessage error_msg;
        std::string error_str;

        switch (token_class) {
            case TK_IDENT:
                error_msg = Msg_expected_identifier;
                break;
            default:
                error_msg = Msg_expected_token;
                error_str = Token::TokenTypeToString(token_class);
        }

        CompilerError error(LEVEL_ERROR,
            error_msg, location, error_str);

        m_compilation_unit->GetErrorList().AddError(error);
    }

    return token;
}

Token Parser::ExpectKeyword(Keywords keyword, bool read)
{
    Token token = MatchKeyword(keyword, read);
    
    if (!token) {
        SourceLocation location = CurrentLocation();
        if (read && m_token_stream->HasNext()) {
            m_token_stream->Next();
        }

        ErrorMessage error_msg;
        std::string error_str;

        switch (keyword) {
            case Keyword_module:
                error_msg = Msg_expected_module;
                break;
            default:
                error_msg = Msg_expected_token;
                error_str = Keyword::ToString(keyword);
        }

        CompilerError error(LEVEL_ERROR,
            error_msg, location, error_str);

        m_compilation_unit->GetErrorList().AddError(error);
    }

    return token;
}

Token Parser::ExpectOperator(const Operator *op, bool read)
{
    Token token = MatchOperator(op, read);

    if (!token) {
        SourceLocation location = CurrentLocation();
        if (read && m_token_stream->HasNext()) {
            m_token_stream->Next();
        }

        ErrorMessage error_msg = Msg_expected_token;
        std::string error_str = op->ToString();

        CompilerError error(LEVEL_ERROR,
            error_msg, location, error_str);

        m_compilation_unit->GetErrorList().AddError(error);
    }

    return token;
}

Token Parser::MatchIdentifier(bool allow_keyword, bool read)
{
    Token ident = Match(TK_IDENT, read);

    if (!ident) {
        Token kw = Match(TK_KEYWORD, read);
        
        if (kw) {
            if (allow_keyword) {
                return kw;
            }
            // keyword may not be used as an identifier here.
            CompilerError error(LEVEL_ERROR,
                Msg_keyword_cannot_be_used_as_identifier, 
                kw.GetLocation(),
                kw.GetValue()
            );
            m_compilation_unit->GetErrorList().AddError(error);
        }

        return Token::EMPTY;
    }

    return ident;
}

Token Parser::ExpectIdentifier(bool allow_keyword, bool read)
{
    Token kw = Match(TK_KEYWORD, read);

    if (!kw) {
        // keyword not found, so must be identifier
        return Expect(TK_IDENT, read);
    }

    // handle ident as keyword
    if (allow_keyword) {
        return kw;
    }
    
    CompilerError error(LEVEL_ERROR,
        Msg_keyword_cannot_be_used_as_identifier, 
        kw.GetLocation(),
        kw.GetValue()
    );
    m_compilation_unit->GetErrorList().AddError(error);

    return Token::EMPTY;
}

bool Parser::ExpectEndOfStmt()
{
    SourceLocation location = CurrentLocation();

    if (!Match(TK_NEWLINE, true) && !Match(TK_SEMICOLON, true)) {
        CompilerError error(LEVEL_ERROR, Msg_expected_end_of_statement, location);
        m_compilation_unit->GetErrorList().AddError(error);
        return false;
    }

    return true;
}

SourceLocation Parser::CurrentLocation() const
{
    if (m_token_stream->GetSize() != 0 && !m_token_stream->HasNext()) {
        return m_token_stream->Last().GetLocation();
    }

    return m_token_stream->Peek().GetLocation();
}

void Parser::SkipStatementTerminators()
{
    // read past statement terminator tokens
    while (Match(TK_SEMICOLON, true) || Match(TK_NEWLINE, true));
}

void Parser::Parse(bool expect_module_decl)
{
    if (expect_module_decl) {
        if (std::shared_ptr<AstModuleDeclaration> module_ast = ParseModuleDeclaration()) {
            m_ast_iterator->Push(module_ast);
            
            while (m_token_stream->HasNext()) {
                if (Match(TK_SEMICOLON, true) || Match(TK_NEWLINE, true)) {
                    continue;
                }

                // check for modules declared after the first
                if (MatchKeyword(Keyword_module, false)) {
                    m_ast_iterator->Push(ParseModuleDeclaration());
                } else {
                    SourceLocation location = CurrentLocation();

                    // call ParseStatement() to read to next
                    ParseStatement();

                    // error, statement outside of module
                    CompilerError error(LEVEL_ERROR, Msg_statement_outside_module, location);
                    m_compilation_unit->GetErrorList().AddError(error);
                }
            }
        }
    } else {
        while (m_token_stream->HasNext()) {
            SourceLocation location = CurrentLocation();

            // skip statement terminator tokens
            if (Match(TK_SEMICOLON, true) || Match(TK_NEWLINE, true)) {
                continue;
            }

            std::shared_ptr<AstStatement> stmt;

            // check for module declaration
            if (MatchKeyword(Keyword_module, false)) {
                stmt = ParseModuleDeclaration();
            } else {
                stmt = ParseStatement();
            }
            
            if (stmt) {
                m_ast_iterator->Push(stmt);
            } else {
                // skip ahead to avoid endlessly looping
                if (m_token_stream->HasNext()) {
                    m_token_stream->Next();
                }
            }
        }
    }
}

int Parser::OperatorPrecedence(const Operator *&out)
{
    out = nullptr;
    
    Token token = m_token_stream->Peek();

    if (token && token.GetTokenClass() == TK_OPERATOR) {
        if (!Operator::IsBinaryOperator(token.GetValue(), out)) {
            // internal error: operator not defined
            CompilerError error(LEVEL_ERROR,
                Msg_internal_error, token.GetLocation());

            m_compilation_unit->GetErrorList().AddError(error);
        }
    }

    if (out) {
        return out->GetPrecedence();
    }
    
    return -1;
}

std::shared_ptr<AstStatement> Parser::ParseStatement(bool top_level)
{
    std::shared_ptr<AstStatement> res;

    if (Match(TK_KEYWORD, false)) {
        if (MatchKeyword(Keyword_module, false)) {
            auto module_decl = ParseModuleDeclaration();
            
            if (top_level) {
                res = module_decl;
            } else {
                // module may not be declared in a block
                CompilerError error(LEVEL_ERROR,
                    Msg_module_declared_in_block, m_token_stream->Next().GetLocation());
                m_compilation_unit->GetErrorList().AddError(error);

                res = nullptr;
            }
        } else if (MatchKeyword(Keyword_use, false)) {
            res = ParseDirective();
        } else if (MatchKeyword(Keyword_import, false)) {
            res = ParseImport();
        } else if (MatchKeyword(Keyword_let, false)) {
            res = ParseVariableDeclaration(true);
        } else if (MatchKeyword(Keyword_function, false)) {
            if (MatchAhead(TK_IDENT, 1)) {
                res = ParseFunctionDefinition();
            } else {
                res = ParseFunctionExpression();
            }
        } else if (MatchKeyword(Keyword_async, false)) {
            res = ParseAsyncExpression();
        } else if (MatchKeyword(Keyword_pure, false)) {
            res = ParsePureExpression();
        } else if (MatchKeyword(Keyword_impure)) {
            res = ParseImpureExpression();
        } else if (MatchKeyword(Keyword_type, false)) {
            res = ParseTypeDefinition();
        } else if (MatchKeyword(Keyword_if, false)) {
            res = ParseIfStatement();
        } else if (MatchKeyword(Keyword_while, false)) {
            res = ParseWhileLoop();
        } else if (MatchKeyword(Keyword_print, false)) {
            res = ParsePrintStatement();
        } else if (MatchKeyword(Keyword_try, false)) {
            res = ParseTryCatchStatement();
        } else if (MatchKeyword(Keyword_return, false)) {
            res = ParseReturnStatement();
        } else {
            res = ParseExpression();
        }
    } else if (Match(TK_IDENT, false) && MatchAhead(TK_COLON, 1)) {
        res = ParseVariableDeclaration(false);
    } else if (Match(TK_OPEN_BRACE, false)) {
        res = ParseBlock();
    } else {
        res = ParseExpression();
    }

    if (res != nullptr && m_token_stream->HasNext()) {
        ExpectEndOfStmt();
    }

    return res;
}

std::shared_ptr<AstModuleDeclaration> Parser::ParseModuleDeclaration()
{
    if (Token module_decl = ExpectKeyword(Keyword_module, true)) {
        if (Token module_name = Expect(TK_IDENT, true)) {
            // expect open brace
            if (Expect(TK_OPEN_BRACE, true)) {
                std::shared_ptr<AstModuleDeclaration> module_ast(
                    new AstModuleDeclaration(module_name.GetValue(), module_decl.GetLocation()));

                // build up the module declaration with statements
                while (m_token_stream->HasNext() && !Match(TK_CLOSE_BRACE, false)) {
                    // skip statement terminator tokens
                    if (!Match(TK_SEMICOLON, true) && !Match(TK_NEWLINE, true)) {

                        // parse at top level, to allow for nested modules
                        module_ast->AddChild(ParseStatement(true));
                    }
                }

                // expect close brace
                if (Expect(TK_CLOSE_BRACE, true)) {
                    return module_ast;
                }
            }
        }
    }

    return nullptr;
}

std::shared_ptr<AstDirective> Parser::ParseDirective()
{
    if (Token token = ExpectKeyword(Keyword_use, true)) {
        if (Token ident = Expect(TK_IDENT, true)) {
            // the arguments will be held in an array expression
            std::vector<std::string> args;

            if (Match(TK_OPEN_BRACKET)) {
                if (Token token = Expect(TK_OPEN_BRACKET, true)) {
                    
                    while (true) {
                        if (Match(TK_CLOSE_BRACKET, false)) {
                            break;
                        }

                        if (Token arg = Expect(TK_STRING, true)) {
                            args.push_back(arg.GetValue());
                        }

                        if (!Match(TK_COMMA, true)) {
                            break;
                        }
                    }

                    Expect(TK_CLOSE_BRACKET, true);
                }
            }

            return std::shared_ptr<AstDirective>(new AstDirective(
                ident.GetValue(),
                args,
                token.GetLocation()
            ));
        }
    }

    return nullptr;
}

std::shared_ptr<AstExpression> Parser::ParseTerm()
{
    Token token = m_token_stream->Peek();
    
    if (!token) {
        CompilerError error(LEVEL_ERROR, Msg_unexpected_eof, CurrentLocation());
        m_compilation_unit->GetErrorList().AddError(error);
        if (m_token_stream->HasNext()) {
            m_token_stream->Next();
        }
        return nullptr;
    }

    std::shared_ptr<AstExpression> expr;

    if (Match(TK_OPEN_PARENTH)) {
        expr = ParseParentheses();
    } else if (Match(TK_OPEN_BRACKET)) {
        expr = ParseArrayExpression();
    } else if (Match(TK_INTEGER)) {
        expr = ParseIntegerLiteral();
    } else if (Match(TK_FLOAT)) {
        expr = ParseFloatLiteral();
    } else if (Match(TK_STRING)) {
        if (MatchAhead(TK_FAT_ARROW, 1)) {
            expr = ParseActionExpression();
        } else {
            expr = ParseStringLiteral();
        }
    } else if (Match(TK_IDENT)) {
        if (MatchAhead(TK_DOUBLE_COLON, 1)) {
            expr = ParseModuleAccess();
        } else {
            expr = ParseIdentifier();
        }
    } else if (MatchKeyword(Keyword_self)) {
        expr = ParseIdentifier(true);
    }  else if (MatchKeyword(Keyword_true)) {
        expr = ParseTrue();
    } else if (MatchKeyword(Keyword_false)) {
        expr = ParseFalse();
    } else if (MatchKeyword(Keyword_nil)) {
        expr = ParseNil();
    } else if (MatchKeyword(Keyword_new)) {
        expr = ParseNewExpression();
    } else if (MatchKeyword(Keyword_function)) {
        expr = ParseFunctionExpression();
    } else if (MatchKeyword(Keyword_async)) {
        expr = ParseAsyncExpression();
    } else if (MatchKeyword(Keyword_pure)) {
        expr = ParsePureExpression();
    } else if (MatchKeyword(Keyword_impure)) {
        expr = ParseImpureExpression();
    } else if (MatchKeyword(Keyword_valueof)) {
        expr = ParseValueOfExpression();
    } else if (MatchKeyword(Keyword_typeof)) {
        expr = ParseTypeOfExpression();
    } else if (Match(TK_OPERATOR)) {
        expr = ParseUnaryExpression();
    } else {
        CompilerError error(LEVEL_ERROR, Msg_unexpected_token,
            token.GetLocation(), token.GetValue());
        m_compilation_unit->GetErrorList().AddError(error);

        if (m_token_stream->HasNext()) {
            m_token_stream->Next();
        }

        return nullptr;
    }

    while (expr != nullptr &&
           (Match(TK_DOT) ||
            Match(TK_OPEN_BRACKET) ||
            Match(TK_OPEN_PARENTH) ||
            MatchKeyword(Keyword_has)))
    {
        if (MatchKeyword(Keyword_has)) {
            expr = ParseHasExpression(expr);
        }
        if (Match(TK_DOT)) {
            expr = ParseMemberExpression(expr);
        }
        if (Match(TK_OPEN_BRACKET)) {
            expr = ParseArrayAccess(expr);
        }
        if (Match(TK_OPEN_PARENTH)) {
            expr = ParseCallExpression(expr);
        }
    }

    return expr;
}

std::shared_ptr<AstExpression> Parser::ParseParentheses()
{
    SourceLocation location = CurrentLocation();
    std::shared_ptr<AstExpression> expr;
    size_t before_pos = m_token_stream->GetPosition();

    Expect(TK_OPEN_PARENTH, true);

    if (!Match(TK_CLOSE_PARENTH) && !Match(TK_IDENT) && !MatchKeyword(Keyword_self)) {
        expr = ParseExpression();
        Expect(TK_CLOSE_PARENTH, true);
    } else {
        if (Match(TK_CLOSE_PARENTH, true)) {
            // if '()' found, it is a function with empty parameters
            // allow ParseFunctionParameters() to handle parentheses
            m_token_stream->SetPosition(before_pos);
            expr = ParseFunctionExpression(false, ParseFunctionParameters());
        } else {
            expr = ParseExpression();

            if (Match(TK_COMMA) || Match(TK_COLON) || Match(TK_ELLIPSIS) ||
                (Match(TK_CLOSE_PARENTH) && MatchAhead(TK_COLON, 1))) {
                // go back to before open '(' found, 
                // to allow ParseFunctionParameters() to handle it
                m_token_stream->SetPosition(before_pos);

                // parse function parameters
                expr = ParseFunctionExpression(false, ParseFunctionParameters());
            } else {
                Expect(TK_CLOSE_PARENTH, true);

                if (Match(TK_OPEN_BRACE, true)) {
                    // if '{' found after ')', it is a function
                    m_token_stream->SetPosition(before_pos);
                    expr = ParseFunctionExpression(false, ParseFunctionParameters());
                }
            }
        }
    }

/*
    // if it's a function expression
    bool is_function_expr = false;

    std::vector<std::shared_ptr<AstParameter>> parameters;
    bool found_variadic = false;

    while (Match(Token::Token_comma, true)) {
        is_function_expr = true;

        Token param_token = Match(Token::Token_identifier, true);
        if (!param_token) {
            break;
        }

        // TODO make use of the type specification
        std::shared_ptr<AstTypeSpecification> type_spec;
        std::shared_ptr<AstTypeContractExpression> type_contract;

        // check if parameter type has been declared
        if (Match(Token::Token_colon, true)) {
            // type contracts are denoted by <angle brackets>
            if (MatchOperator(&Operator::operator_less, false)) {
                // parse type contracts
                type_contract = ParseTypeContract();
            } else {
                type_spec = ParseTypeSpecification();
            }
        }


        if (found_variadic) {
            // found another parameter after variadic
            m_compilation_unit->GetErrorList().AddError(
                CompilerError(LEVEL_ERROR, Msg_argument_after_varargs, param_token.GetLocation()));
        }

        bool is_variadic = false;
        if (Match(Token::Token_ellipsis, true)) {
            is_variadic = true;
            found_variadic = true;
        }

        auto param = std::shared_ptr<AstParameter>(
            new AstParameter(param_token.GetValue(), is_variadic, param_token.GetLocation()));

        if (type_contract) {
            param->SetTypeContract(type_contract);
        }

        parameters.push_back(param);
    }*/

    /*if (is_function_expr) {
        std::shared_ptr<AstTypeSpecification> return_type_spec;

        // return type specification
        if (Match(Token::Token_colon, true)) {
            // read return type for functions
            return_type_spec = ParseTypeSpecification();
        }

        if (auto block = ParseBlock()) {
            return std::shared_ptr<AstFunctionExpression>(
                new AstFunctionExpression(parameters, return_type_spec, block, location));
        }
    }*/

    return expr;
}

std::shared_ptr<AstInteger> Parser::ParseIntegerLiteral()
{
    if (Token token = Expect(TK_INTEGER, true)) {
        std::istringstream ss(token.GetValue());
        ace::aint32 value;
        ss >> value;
        return std::shared_ptr<AstInteger>(
            new AstInteger(value, token.GetLocation()));
    }

    return nullptr;
}

std::shared_ptr<AstFloat> Parser::ParseFloatLiteral()
{
    if (Token token = Expect(TK_FLOAT, true)) {
        ace::afloat32 value = std::atof(token.GetValue().c_str());
        return std::shared_ptr<AstFloat>(
            new AstFloat(value, token.GetLocation()));
    }

    return nullptr;
}

std::shared_ptr<AstString> Parser::ParseStringLiteral()
{
    if (Token token = Expect(TK_STRING, true)) {
        return std::shared_ptr<AstString>(
            new AstString(token.GetValue(), token.GetLocation()));
    }

    return nullptr;
}

std::shared_ptr<AstIdentifier> Parser::ParseIdentifier(bool allow_keyword)
{
    if (Token token = ExpectIdentifier(allow_keyword, false)) {
        /*if (MatchAhead(TK_OPEN_PARENTH, 1)) {
            // function call
            return ParseFunctionCall(allow_keyword);
        }
        // allow identifiers with identifiers directly after to be used as functions
        // i.e: select x from y would be parsed as:
        //      select(x).from(y)
        else if (MatchAhead(TK_IDENT, 1) || MatchAhead(TK_STRING, 1)) {
            return ParseFunctionCallNoParams(allow_keyword);
        }
        // return a variable
        else {*/
            // read identifier token
            if (m_token_stream->HasNext()) {
                m_token_stream->Next();
            }

            // return variable
            return std::shared_ptr<AstVariable>(new AstVariable(
                token.GetValue(), token.GetLocation()
            ));
        //}
    }

    return nullptr;
}

std::shared_ptr<AstArgument> Parser::ParseArgument()
{
    SourceLocation location = CurrentLocation();

    bool is_named_arg = false;
    std::string arg_name;

    // check for name: value expressions (named arguments)
    if (Match(TK_IDENT)) {
        if (MatchAhead(TK_COLON, 1)) {
            // named argument
            is_named_arg = true;
            Token name_token = Expect(TK_IDENT, true);
            arg_name = name_token.GetValue();

            // read the colon
            Expect(TK_COLON, true);
        }
    }

    if (auto expr = ParseExpression()) {
        return std::shared_ptr<AstArgument>(
            new AstArgument(
                expr,
                is_named_arg,
                arg_name,
                location
            )
        );
    } else {
        CompilerError error(LEVEL_ERROR, Msg_illegal_expression, location);
        m_compilation_unit->GetErrorList().AddError(error);
        return nullptr;
    }
}

std::vector<std::shared_ptr<AstArgument>> Parser::ParseArguments()
{
    std::vector<std::shared_ptr<AstArgument>> args;

    Expect(TK_OPEN_PARENTH, true);

    while (!Match(TK_CLOSE_PARENTH, false)) {
        auto arg = ParseArgument();
        if (arg == nullptr) {
            break;
        }

        args.push_back(arg);

        if (!Match(TK_COMMA, true)) {
            // unexpected token
            break;
        }
    }

    Expect(TK_CLOSE_PARENTH, true);

    return args;
}

std::shared_ptr<AstCallExpression> Parser::ParseCallExpression(std::shared_ptr<AstExpression> target)
{
    return std::shared_ptr<AstCallExpression>(new AstCallExpression(
        target,
        ParseArguments(),
        target->GetLocation()
    ));
}

std::shared_ptr<AstModuleAccess> Parser::ParseModuleAccess()
{
    Token token = Expect(TK_IDENT, true);
    Expect(TK_DOUBLE_COLON, true);

    std::shared_ptr<AstExpression> expr;

    if (MatchAhead(TK_DOUBLE_COLON, 1)) {
        expr = ParseModuleAccess();
    } else {
        expr = ParseIdentifier(true);
    }

    if (!expr) {
        return nullptr;
    }

    return std::shared_ptr<AstModuleAccess>(new AstModuleAccess(
        token.GetValue(),
        expr,
        token.GetLocation()
    ));
}

std::shared_ptr<AstMember> Parser::ParseMemberExpression(std::shared_ptr<AstExpression> target)
{
    Expect(TK_DOT, true);

    if (Token ident = ExpectIdentifier(true, true)) {
        return std::shared_ptr<AstMember>(new AstMember(
            ident.GetValue(),
            target,
            ident.GetLocation()
        ));
    }

    return nullptr;
}

std::shared_ptr<AstArrayAccess> Parser::ParseArrayAccess(std::shared_ptr<AstExpression> target)
{
    if (Token token = Expect(TK_OPEN_BRACKET, true)) {
        auto expr = ParseExpression();
        Expect(TK_CLOSE_BRACKET, true);
        if (expr) {
            return std::shared_ptr<AstArrayAccess>(
                new AstArrayAccess(target, expr, token.GetLocation()));
        }
    }

    return nullptr;
}

std::shared_ptr<AstHasExpression> Parser::ParseHasExpression(std::shared_ptr<AstExpression> target)
{
    if (Token token = ExpectKeyword(Keyword_has, true)) {
        if (Token field = Expect(TK_STRING, true)) {
            return std::shared_ptr<AstHasExpression>(new AstHasExpression(
                target,
                field.GetValue(),
                target->GetLocation()
            ));
        }
    }

    return nullptr;
}

std::shared_ptr<AstActionExpression> Parser::ParseActionExpression()
{
    if (Token action_name = Expect(TK_STRING, true)) {
        if (Expect(TK_FAT_ARROW, true)) {
            // TODO: args
            if (auto target = ParseExpression()) {
                return std::shared_ptr<AstActionExpression>(new AstActionExpression(
                    action_name.GetValue(),
                    target,
                    {},
                    target->GetLocation()
                ));
            }
        }
    }

    return nullptr;
}

std::shared_ptr<AstNewExpression> Parser::ParseNewExpression()
{
    if (Token token = ExpectKeyword(Keyword_new, true)) {
        if (auto type_spec = ParseTypeSpecification()) {
            std::vector<std::shared_ptr<AstArgument>> args;

            if (Match(TK_OPEN_PARENTH, false)) {
                // parse args
                args = ParseArguments();
            }

            return std::shared_ptr<AstNewExpression>(new AstNewExpression(
                type_spec,
                args,
                token.GetLocation()
            ));
        }
    }

    return nullptr;
}

std::shared_ptr<AstTrue> Parser::ParseTrue()
{
    if (Token token = ExpectKeyword(Keyword_true, true)) {
        return std::shared_ptr<AstTrue>(new AstTrue(token.GetLocation()));
    }
    return nullptr;
}

std::shared_ptr<AstFalse> Parser::ParseFalse()
{
    if (Token token = ExpectKeyword(Keyword_false, true)) {
        return std::shared_ptr<AstFalse>(new AstFalse(token.GetLocation()));
    }
    return nullptr;
}

std::shared_ptr<AstNil> Parser::ParseNil()
{
    if (Token token = ExpectKeyword(Keyword_nil, true)) {
        return std::shared_ptr<AstNil>(new AstNil(token.GetLocation()));
    }
    return nullptr;
}

std::shared_ptr<AstBlock> Parser::ParseBlock()
{
    if (Token token = Expect(TK_OPEN_BRACE, true)) {
        std::shared_ptr<AstBlock> block(new AstBlock(token.GetLocation()));

        while (!Match(TK_CLOSE_BRACE, true)) {
            // skip statement terminator tokens
            if (!Match(TK_SEMICOLON, true) && !Match(TK_NEWLINE, true)) {
                block->AddChild(ParseStatement());
            }
        }

        return block;
    }

    return nullptr;
}

std::shared_ptr<AstIfStatement> Parser::ParseIfStatement()
{
    if (Token token = ExpectKeyword(Keyword_if, true)) {
        SourceLocation cond_location = CurrentLocation();

        std::shared_ptr<AstExpression> conditional = ParseExpression();
        if (!conditional) {
            return nullptr;
        }

        std::shared_ptr<AstBlock> block = ParseBlock();
        if (!block) {
            return nullptr;
        }

        std::shared_ptr<AstBlock> else_block = nullptr;
        // parse else statement if the "else" keyword is found
        Token else_token = MatchKeyword(Keyword_else, true);
        if (else_token) {
            // check for "if" keyword for else-if
            if (MatchKeyword(Keyword_if, false)) {
                else_block = std::shared_ptr<AstBlock>(new AstBlock(else_token.GetLocation()));
                else_block->AddChild(ParseIfStatement());
            } else {
                // parse block after "else keyword
                else_block = ParseBlock();
            }
        }

        return std::shared_ptr<AstIfStatement>(
            new AstIfStatement(conditional, block, else_block,
                token.GetLocation()));
    }

    return nullptr;
}

std::shared_ptr<AstWhileLoop> Parser::ParseWhileLoop()
{
    if (Token token = ExpectKeyword(Keyword_while, true)) {
        SourceLocation cond_location = CurrentLocation();
        std::shared_ptr<AstExpression> conditional = ParseExpression();
        std::shared_ptr<AstBlock> block = ParseBlock();

        if (!conditional) {
            CompilerError error(LEVEL_ERROR, Msg_illegal_expression, cond_location);
            m_compilation_unit->GetErrorList().AddError(error);
            return nullptr;
        }

        if (!block) {
            return nullptr;
        }

        return std::shared_ptr<AstWhileLoop>(
            new AstWhileLoop(conditional, block, token.GetLocation()));
    }

    return nullptr;
}

std::shared_ptr<AstPrintStatement> Parser::ParsePrintStatement()
{
    if (Token token = ExpectKeyword(Keyword_print, true)) {
        std::vector<std::shared_ptr<AstExpression>> arguments;

        while (true) {
            SourceLocation loc = CurrentLocation();

            if (auto expr = ParseExpression()) {
                arguments.push_back(expr);
            } else {
                // expression or statement could not be evaluated
                CompilerError error(LEVEL_ERROR, Msg_illegal_expression, loc);
                m_compilation_unit->GetErrorList().AddError(error);
            }

            if (!Match(TK_COMMA, true)) {
                break;
            }
        }

        return std::shared_ptr<AstPrintStatement>(
            new AstPrintStatement(arguments, token.GetLocation()));
    }

    return nullptr;
}

std::shared_ptr<AstTryCatch> Parser::ParseTryCatchStatement()
{
    if (Token token = ExpectKeyword(Keyword_try, true)) {
        std::shared_ptr<AstBlock> try_block(ParseBlock());
        std::shared_ptr<AstBlock> catch_block(nullptr);

        if (ExpectKeyword(Keyword_catch, true)) {
            // TODO: Add exception argument
            catch_block = ParseBlock();
        }

        if (try_block == nullptr || catch_block == nullptr) {
            return nullptr;
        }

        return std::shared_ptr<AstTryCatch>(
            new AstTryCatch(try_block, catch_block, token.GetLocation()));
    }

    return nullptr;
}

std::shared_ptr<AstExpression> Parser::ParseBinaryExpression(int expr_prec,
    std::shared_ptr<AstExpression> left)
{
    while (true) {
        // get precedence
        const Operator *op = nullptr;
        int precedence = OperatorPrecedence(op);
        if (precedence < expr_prec) {
            return left;
        }

        // read the operator token
        Token token = Expect(TK_OPERATOR, true);

        if (auto right = ParseTerm()) {
            // next part of expression's precedence
            const Operator *next_op = nullptr;
            int next_prec = OperatorPrecedence(next_op);
            if (precedence < next_prec) {
                right = ParseBinaryExpression(precedence + 1, right);
                if (right == nullptr) {
                    return nullptr;
                }
            }

            left = std::shared_ptr<AstBinaryExpression>(
                new AstBinaryExpression(left, right, op, token.GetLocation()));
        }
    }

    return nullptr;
}

std::shared_ptr<AstExpression> Parser::ParseUnaryExpression()
{
    // read the operator token
    if (Token token = Expect(TK_OPERATOR, true)) {
        const Operator *op = nullptr;
        if (Operator::IsUnaryOperator(token.GetValue(), op)) {
            if (auto term = ParseTerm()) {
                return std::shared_ptr<AstUnaryExpression>(
                    new AstUnaryExpression(term, op, token.GetLocation()));
            }
            
            return nullptr;
        } else {
            // internal error: operator not defined
            CompilerError error(LEVEL_ERROR,
                Msg_illegal_operator, token.GetLocation(), token.GetValue());
            m_compilation_unit->GetErrorList().AddError(error);
        }
    }

    return nullptr;
}

std::shared_ptr<AstExpression> Parser::ParseExpression()
{
    if (auto term = ParseTerm()) {
        if (Match(TK_OPERATOR, false)) {
            if (auto bin_expr = ParseBinaryExpression(0, term)) {
                term = bin_expr;
            } else {
                return nullptr;
            }
        }

        return term;
    }

    return nullptr;
}

std::shared_ptr<AstTypeSpecification> Parser::ParseTypeSpecification()
{
    if (Token left = Expect(TK_IDENT, true)) {
        std::string left_name = left.GetValue();

        std::shared_ptr<AstTypeSpecification> right;
        // module access of type
        if (Match(TK_DOUBLE_COLON, true)) {
            // read next part
            right = ParseTypeSpecification();
        }

        // check generics
        std::vector<std::shared_ptr<AstTypeSpecification>> generic_params;
        if (MatchOperator(&Operator::operator_less, true)) {
            do {
                if (std::shared_ptr<AstTypeSpecification> generic_param = ParseTypeSpecification()) {
                    generic_params.push_back(generic_param);
                }
            } while (Match(TK_COMMA, true));

            ExpectOperator(&Operator::operator_greater, true);
        }

        while (Match(TK_OPEN_BRACKET) || Match(TK_QUESTION_MARK)) {
            // question mark at the end of a type is syntactical sugar for `Maybe(T)`
            if (Match(TK_QUESTION_MARK, true)) {
                std::shared_ptr<AstTypeSpecification> inner(new AstTypeSpecification(
                    left_name,
                    generic_params,
                    right,
                    left.GetLocation()
                ));

                left_name = SymbolType::Builtin::MAYBE->GetName();
                generic_params = { inner };
                right = nullptr;
            } else if (Match(TK_OPEN_BRACKET, true)) {
                // array braces at the end of a type are syntactical sugar for `Array(T)`
                std::shared_ptr<AstTypeSpecification> inner(new AstTypeSpecification(
                    left_name,
                    generic_params,
                    right,
                    left.GetLocation()
                ));

                left_name = SymbolType::Builtin::ARRAY->GetName();
                generic_params = { inner };
                right = nullptr;

                Expect(TK_CLOSE_BRACKET, true);
            }
        }

        return std::shared_ptr<AstTypeSpecification>(new AstTypeSpecification(
            left_name,
            generic_params,
            right,
            left.GetLocation()
        ));
    }

    return nullptr;
}

#if 0
std::shared_ptr<AstTypeContractExpression> Parser::ParseTypeContract()
{
    std::shared_ptr<AstTypeContractExpression> expr;

    // we have to use ExpectOperator for angle brackets
    if (Token token = ExpectOperator(&Operator::operator_less, true)) {
        expr = ParseTypeContractExpression();
    }

    ExpectOperator(&Operator::operator_greater, true);

    return expr;
}

std::shared_ptr<AstTypeContractExpression> Parser::ParseTypeContractExpression()
{
    if (auto term = ParseTypeContractTerm()) {
        if (Match(TK_OPERATOR, false)) {
            if (auto expr = ParseTypeContractBinaryExpression(0, term)) {
                term = expr;
            } else {
                return nullptr;
            }
        }

        return term;
    }

    return nullptr;
}


std::shared_ptr<AstTypeContractExpression> Parser::ParseTypeContractTerm()
{
    if (Token contract_prop = Expect(TK_IDENT, true)) {
        // read the contract parameter here
        return std::shared_ptr<AstTypeContractTerm>(new AstTypeContractTerm(
            contract_prop.GetValue(), ParseTypeSpecification(), contract_prop.GetLocation()));
    }

    return nullptr;
}

std::shared_ptr<AstTypeContractExpression> Parser::ParseTypeContractBinaryExpression(int expr_prec,
    std::shared_ptr<AstTypeContractExpression> left)
{
    while (true) {
        // check for end of contract
        if (MatchOperator(&Operator::operator_greater, false)) {
            return left;
        }

        // get precedence
        const Operator *op = nullptr;
        int precedence = OperatorPrecedence(op);
        if (precedence < expr_prec) {
            return left;
        }

        // read the operator token
        Token token = Expect(TK_OPERATOR, false);
        Token token_op = MatchOperator(&Operator::operator_bitwise_or, true);
        if (!token) {
            return nullptr;
        }
        if (!token_op) {
            // try and operator
            token_op = MatchOperator(&Operator::operator_bitwise_and, true);
        }
        if (!token_op) {
            CompilerError error(LEVEL_ERROR, Msg_invalid_type_contract_operator, token.GetLocation(), token.GetValue());
            m_compilation_unit->GetErrorList().AddError(error);
            return nullptr;
        }

        auto right = ParseTypeContractTerm();
        if (!right) {
            return nullptr;
        }

        // next part of expression's precedence
        const Operator *next_op = nullptr;
        int next_prec = OperatorPrecedence(next_op);
        if (precedence < next_prec) {
            right = ParseTypeContractBinaryExpression(precedence + 1, right);
            if (!right) {
                return nullptr;
            }
        }

        left = std::shared_ptr<AstTypeContractBinaryExpression>(
            new AstTypeContractBinaryExpression(left, right, op, token.GetLocation()));
    }

    return nullptr;
}

#endif

std::shared_ptr<AstVariableDeclaration> Parser::ParseVariableDeclaration(
    bool require_keyword,
    bool allow_keyword_names)
{
    SourceLocation location = CurrentLocation();

    if (require_keyword) {
        if (!ExpectKeyword(Keyword_let, true)) {
            return nullptr;
        }
    } else {
        // match and read in the case that it is found
        MatchKeyword(Keyword_let, true);
    }

    if (Token identifier = ExpectIdentifier(allow_keyword_names, true)) {
        std::shared_ptr<AstTypeSpecification> type_spec;

        if (Match(TK_COLON, true)) {
            // read object type
            type_spec = ParseTypeSpecification();
        }

        std::shared_ptr<AstExpression> assignment;

        if (Token op = Match(TK_OPERATOR, true)) {
            if (op.GetValue() == Operator::operator_assign.ToString()) {
                // read assignment expression
                SourceLocation expr_location = CurrentLocation();
                assignment = ParseExpression();
                if (!assignment) {
                    CompilerError error(LEVEL_ERROR, Msg_illegal_expression, expr_location);
                    m_compilation_unit->GetErrorList().AddError(error);
                }
            } else {
                // unexpected operator
                CompilerError error(LEVEL_ERROR,
                    Msg_illegal_operator, op.GetLocation());

                m_compilation_unit->GetErrorList().AddError(error);
            }
        }

        return std::shared_ptr<AstVariableDeclaration>(new AstVariableDeclaration(
            identifier.GetValue(),
            type_spec,
            assignment,
            location
        ));
    }

    return nullptr;
}

std::shared_ptr<AstFunctionDefinition> Parser::ParseFunctionDefinition(bool require_keyword)
{
    SourceLocation location = CurrentLocation();

    if (require_keyword) {
        if (!ExpectKeyword(Keyword_function, true)) {
            return nullptr;
        }
    } else {
        // match and read in the case that it is found
        MatchKeyword(Keyword_function, true);
    }

    if (Token identifier = Expect(TK_IDENT, true)) {
        if (auto expr = ParseFunctionExpression(false, ParseFunctionParameters())) {
            return std::shared_ptr<AstFunctionDefinition>(new AstFunctionDefinition(
                identifier.GetValue(),
                expr,
                location
            ));
        }
    }

    return nullptr;
}

std::shared_ptr<AstFunctionExpression> Parser::ParseFunctionExpression(
    bool require_keyword,
    std::vector<std::shared_ptr<AstParameter>> params,
    bool is_async,
    bool is_pure)
{
    Token token = require_keyword ? ExpectKeyword(Keyword_function, true) : Token::EMPTY;
    SourceLocation location = token ? token.GetLocation() : CurrentLocation();

    if (require_keyword || !token) {
        if (require_keyword) {
            // read params
            params = ParseFunctionParameters();
        }

        std::shared_ptr<AstTypeSpecification> type_spec;

        if (Match(TK_COLON, true)) {
            // read return type for functions
            type_spec = ParseTypeSpecification();
        }

        // parse function block
        if (auto block = ParseBlock()) {
            return std::shared_ptr<AstFunctionExpression>(new AstFunctionExpression(
                params,
                type_spec,
                block,
                is_async,
                is_pure,
                location
            ));
        }
    }

    return nullptr;
}

std::shared_ptr<AstArrayExpression> Parser::ParseArrayExpression()
{
    if (Token token = Expect(TK_OPEN_BRACKET, true)) {
        std::vector<std::shared_ptr<AstExpression>> members;

        while (true) {
            if (Match(TK_CLOSE_BRACKET, false)) {
                break;
            }

            if (auto expr = ParseExpression()) {
                members.push_back(expr);
            }

            if (!Match(TK_COMMA, true)) {
                break;
            }
        }

        Expect(TK_CLOSE_BRACKET, true);

        return std::shared_ptr<AstArrayExpression>(
            new AstArrayExpression(members, token.GetLocation()));
    }

    return nullptr;
}

std::shared_ptr<AstExpression> Parser::ParseAsyncExpression()
{
    if (Token token = ExpectKeyword(Keyword_async, true)) {
        MatchKeyword(Keyword_function, true); // skip 'function' keyword if found
        // for now, only functions are supported.
        return ParseFunctionExpression(
            false, ParseFunctionParameters(), true, false
        );
    }

    return nullptr;
}

std::shared_ptr<AstExpression> Parser::ParsePureExpression()
{
    if (Token token = ExpectKeyword(Keyword_pure, true)) {
        MatchKeyword(Keyword_function, true); // skip 'function' keyword if found
        return ParseFunctionExpression(
            false, ParseFunctionParameters(), false, true
        );
    }

    return nullptr;
}

std::shared_ptr<AstExpression> Parser::ParseImpureExpression()
{
    if (Token token = ExpectKeyword(Keyword_impure, true)) {
        MatchKeyword(Keyword_function, true); // skip 'function' keyword if found
        return ParseFunctionExpression(
            false, ParseFunctionParameters(), false, false
        );
    }

    return nullptr;
}

std::shared_ptr<AstExpression> Parser::ParseValueOfExpression()
{
    if (Token token = ExpectKeyword(Keyword_valueof, true)) {
        std::shared_ptr<AstExpression> expr;

        if (!MatchAhead(TK_DOUBLE_COLON, 1)) {
            Token ident = Expect(TK_IDENT, true);
            expr.reset(
                new AstVariable(ident.GetValue(), token.GetLocation())
            );
        } else {
            do {
                Token ident = Expect(TK_IDENT, true);
                expr.reset(
                    new AstModuleAccess(ident.GetValue(), expr, ident.GetLocation())
                );
            } while (Match(TK_DOUBLE_COLON, true));
        }

        return expr;
    }

    return nullptr;
}

std::shared_ptr<AstTypeOfExpression> Parser::ParseTypeOfExpression()
{
    SourceLocation location = CurrentLocation();

    Token token = ExpectKeyword(Keyword_typeof, true);
    
    if (token) {
        SourceLocation expr_location = CurrentLocation();
        if (auto term = ParseTerm()) {
            return std::shared_ptr<AstTypeOfExpression>(
                new AstTypeOfExpression(term, location)
            );
        } else {
            CompilerError error(LEVEL_ERROR, Msg_illegal_expression, expr_location);
            m_compilation_unit->GetErrorList().AddError(error);
        }
    }

    return nullptr;
}

std::vector<std::shared_ptr<AstParameter>> Parser::ParseFunctionParameters()
{
    std::vector<std::shared_ptr<AstParameter>> parameters;

    if (Match(TK_OPEN_PARENTH, true)) {
        bool found_variadic = false;
        bool keep_reading = true;

        while (keep_reading) {
            Token token = Token::EMPTY;

            if (Match(TK_CLOSE_PARENTH, false)) {
                keep_reading = false;
            } else if ((token = MatchKeyword(Keyword_self, true)) ||
                       (token = Expect(TK_IDENT, true)))
            {
                std::shared_ptr<AstTypeSpecification> type_spec;
                std::shared_ptr<AstExpression> default_param;

                // check if parameter type has been declared
                if (Match(TK_COLON, true)) {
                    type_spec = ParseTypeSpecification();
                }

                if (found_variadic) {
                    // found another parameter after variadic
                    CompilerError error(
                        LEVEL_ERROR,
                        Msg_argument_after_varargs,
                        token.GetLocation()
                    );

                    m_compilation_unit->GetErrorList().AddError(error);
                }

                // if this parameter is variadic
                bool is_variadic = false;

                if (Match(TK_ELLIPSIS, true)) {
                    is_variadic = true;
                    found_variadic = true;
                }

                // check for default assignment
                if (MatchOperator(&Operator::operator_assign, true)) {
                    default_param = ParseExpression();
                }

                parameters.push_back(std::shared_ptr<AstParameter>(new AstParameter(
                    token.GetValue(),
                    type_spec,
                    default_param,
                    is_variadic,
                    token.GetLocation()
                )));

                if (!Match(TK_COMMA, true)) {
                    keep_reading = false;
                }
            } else {
                keep_reading = false;
            }
        }

        Expect(TK_CLOSE_PARENTH, true);
    }

    return parameters;
}

std::shared_ptr<AstStatement> Parser::ParseTypeDefinition()
{
    if (Token token = ExpectKeyword(Keyword_type, true)) {

        // type names may not be a keyword
        if (Token identifier = ExpectIdentifier(false, true)) {
            std::vector<std::string> generic_params;
            std::vector<std::shared_ptr<AstVariableDeclaration>> members;
            std::vector<std::shared_ptr<AstEvent>> events;

            // parse generic parameters after '<'
            if (MatchOperator(&Operator::operator_less, true)) {
                while (Token ident = ExpectIdentifier(false, true)) {
                    generic_params.push_back(ident.GetValue());
                    if (!Match(TK_COMMA, true)) {
                        break;
                    }
                }
                ExpectOperator(&Operator::operator_greater, true);
            }

            // check type alias
            if (Token op = Match(TK_OPERATOR, false)) {
                if (op.GetValue() == Operator::operator_assign.ToString()) {
                    // read the operator
                    if (m_token_stream->HasNext()) {
                        m_token_stream->Next();
                    }

                    if (auto aliasee = ParseTypeSpecification()) {

                        return std::shared_ptr<AstTypeAlias>(new AstTypeAlias(
                            identifier.GetValue(), 
                            aliasee,
                            identifier.GetLocation()
                        ));
                    } else {
                        return nullptr;
                    }
                }
            }

            if (Expect(TK_OPEN_BRACE, true)) {
                while (!Match(TK_CLOSE_BRACE, true)) {
                    // check for events
                    if (MatchKeyword(Keyword_on, true)) {
                        // match a string
                        if (Token event_key = Expect(TK_STRING, true)) {
                            // expect =>
                            if (Expect(TK_FAT_ARROW, true)) {
                                // read function expression
                                MatchKeyword(Keyword_function, true); // skip 'function' keyword if found
                                if (auto event_trigger = ParseFunctionExpression(false, ParseFunctionParameters())) {
                                    events.push_back(std::shared_ptr<AstEvent>(new AstEvent(
                                        event_key.GetValue(),
                                        event_trigger,
                                        event_key.GetLocation()
                                    )));
                                }
                            }
                        }
                    } else if (MatchIdentifier(true, false)) {
                        // do not require declaration keyword for data members.
                        // also, data members may be keywords.

                        // if parentheses matched, it will be a function
                        
                        if (MatchAhead(TK_OPEN_PARENTH, 1)) {
                            // read the identifier token
                            Token identifier = ExpectIdentifier(true, true);

                            MatchKeyword(Keyword_function, true); // skip 'function' keyword if found
                            if (auto expr = ParseFunctionExpression(
                                false, ParseFunctionParameters()
                            )) {
                                // first, read the identifier
                                std::shared_ptr<AstVariableDeclaration> member(new AstVariableDeclaration(
                                    identifier.GetValue(),
                                    nullptr,
                                    expr,
                                    identifier.GetLocation()
                                ));

                                members.push_back(member);
                            }
                        } else {
                            members.push_back(ParseVariableDeclaration(false, true));
                        }
                    } else {
                        // error; unexpected token
                        CompilerError error(LEVEL_ERROR, Msg_unexpected_token,
                            m_token_stream->Peek().GetLocation(),
                            m_token_stream->Peek().GetValue());
                            
                        m_compilation_unit->GetErrorList().AddError(error);

                        if (m_token_stream->HasNext()) {
                            m_token_stream->Next();
                        }
                    }


                    ExpectEndOfStmt();
                    SkipStatementTerminators();
                }

                return std::shared_ptr<AstTypeDefinition>(new AstTypeDefinition(
                    identifier.GetValue(), 
                    generic_params,
                    members,
                    events,
                    token.GetLocation()
                ));
            }
        }
    }

    return nullptr;
}

std::shared_ptr<AstImport> Parser::ParseImport()
{
    if (ExpectKeyword(Keyword_import, true)) {
        if (Match(TK_STRING, false)) {
            return ParseFileImport();
        } else if (Match(TK_IDENT, false)) {
            return ParseModuleImport();
        }
    }

    return nullptr;
}

std::shared_ptr<AstFileImport> Parser::ParseFileImport()
{
    const SourceLocation location = CurrentLocation();

    if (Token file = Expect(TK_STRING, true)) {
        std::shared_ptr<AstFileImport> result(new AstFileImport(
            file.GetValue(),
            location
        ));

        return result;
    }

    return nullptr;
}

std::shared_ptr<AstModuleImport> Parser::ParseModuleImport()
{
    const SourceLocation location = CurrentLocation();

    if (auto mod_access = ParseModuleAccess()) {
        return std::shared_ptr<AstModuleImport>(new AstModuleImport(
            mod_access,
            location
        ));
    }

    return nullptr;
}

std::shared_ptr<AstReturnStatement> Parser::ParseReturnStatement()
{
    SourceLocation location = CurrentLocation();

    Token token = ExpectKeyword(Keyword_return, true);
    
    if (token) {
        SourceLocation expr_location = CurrentLocation();
        auto expr = ParseExpression();
        if (!expr) {
            CompilerError error(LEVEL_ERROR, Msg_illegal_expression, expr_location);
            m_compilation_unit->GetErrorList().AddError(error);
        }
        return std::shared_ptr<AstReturnStatement>(new AstReturnStatement(expr, location));
    }

    return nullptr;
}
