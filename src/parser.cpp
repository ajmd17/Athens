#include <athens/parser.h>

#include <memory>
#include <cstdlib>

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

const Token *Parser::MatchAhead(TokenType type, int n) 
{
    const Token *peek = m_token_stream->Peek(n);
    if (peek != nullptr && peek->GetType() == type) {
        return peek;
    }
    return nullptr;
}

const Token *Parser::Match(TokenType type, bool read)
{
    const Token *peek = m_token_stream->Peek();
    if (peek != nullptr && peek->GetType() == type) {
        if (read) {
            m_token_stream->Next();
        }
        return peek;
    }
    return nullptr;
}

const Token *Parser::MatchKeyword(Keywords keyword, bool read)
{
    const Token *peek = m_token_stream->Peek();
    if (peek != nullptr && peek->GetType() == Token_keyword) {
        std::string str = Keyword::ToString(keyword);
        if (peek->GetValue() == str) {
            if (read) {
                m_token_stream->Next();
            }
            return peek;
        }
    }
    return nullptr;
}

const Token *Parser::Expect(TokenType type, bool read)
{
    const Token *token = Match(type, read);
    if (token == nullptr) {
        SourceLocation location = m_token_stream->Peek()->GetLocation();
        if (read) {
            m_token_stream->Next();
        }

        ErrorMessage error_msg;
        std::string error_str;

        switch (type) {
        case Token_identifier:
            error_msg = Msg_expected_identifier;
            break;
        default:
            error_msg = Msg_expected_token;
            error_str = Token::TokenTypeToString(type);
        }

        CompilerError error(Level_fatal, 
            error_msg, location, error_str);

        m_compilation_unit->GetErrorList().AddError(error);
    }

    return token;
}

const Token *Parser::ExpectKeyword(Keywords keyword, bool read)
{
    const Token *token = MatchKeyword(keyword, read);
    if (token == nullptr) {
        SourceLocation location = m_token_stream->Peek()->GetLocation();
        if (read) {
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

        CompilerError error(Level_fatal,
            error_msg, location, error_str);
        
        m_compilation_unit->GetErrorList().AddError(error);
    }

    return token;
}

const SourceLocation &Parser::CurrentLocation() const
{
    const Token *peek = m_token_stream->Peek();
    return peek != nullptr ? peek->GetLocation() : SourceLocation::eof;
}

void Parser::Parse()
{
    std::shared_ptr<AstModuleDeclaration> module_ast(nullptr);

    // all source code files must start with module declaration
    const Token *module_decl = ExpectKeyword(Keyword_module, true);
    if (module_decl != nullptr) {
        const Token *module_name = Expect(Token_identifier, true);
        if (module_name != nullptr) {
            module_ast.reset(new AstModuleDeclaration(
                module_name->GetValue(), module_decl->GetLocation()));

            m_ast_iterator->Push(module_ast);
        }
    }

    while (m_token_stream->HasNext()) {
        SourceLocation location(CurrentLocation());

        // read semicolon tokens
        if (Match(Token_semicolon, true)) {
            continue;
        }

        std::shared_ptr<AstStatement> statement(ParseStatement());

        if (statement != nullptr) {
            m_ast_iterator->Push(statement);
        } else {
            // expression or statement could not be evaluated
            CompilerError error(Level_fatal,
                Msg_illegal_expression, location);

            m_compilation_unit->GetErrorList().AddError(error);

            // skip ahead to avoid endlessly looping
            m_token_stream->Next();
        }
    }
}

int Parser::OperatorPrecedence(const Operator *&out)
{
    out = nullptr;
    const Token *token = m_token_stream->Peek();
    
    if (token != nullptr && token->GetType() == Token_operator) {
        if (!Operator::IsBinaryOperator(token->GetValue(), out)) {
            // internal error: operator not defined
            CompilerError error(Level_fatal,
                Msg_internal_error, token->GetLocation());

            m_compilation_unit->GetErrorList().AddError(error);
        }
    }

    if (out != nullptr) {
        return out->GetPrecedence();
    } else {
        return -1;
    }
}

std::shared_ptr<AstStatement> Parser::ParseStatement()
{
    if (Match(Token_keyword, false)) {
        if (MatchKeyword(Keyword_import, false)) {
            return ParseImport();
        } else if (MatchKeyword(Keyword_var, false)) {
            return ParseVariableDeclaration();
        }
    } else if (Match(Token_open_brace, false)) {
        return ParseBlock();
    }
    
    return ParseExpression(true);
}

std::shared_ptr<AstExpression> Parser::ParseTerm()
{
    const Token *token = m_token_stream->Peek();
    if (token == nullptr) {
        m_token_stream->Next();
        return nullptr;
    }

    if (Match(Token_open_parenthesis)) {
        return ParseParentheses();
    } else if (Match(Token_integer_literal)) {
        return ParseIntegerLiteral();
    } else if (Match(Token_float_literal)) {
        return ParseFloatLiteral();
    } else if (Match(Token_string_literal)) {
        return ParseStringLiteral();
    } else if (Match(Token_identifier)) {
        return ParseIdentifier();
    } else if (MatchKeyword(Keyword_true)) {
        return ParseTrue();
    } else if (MatchKeyword(Keyword_false)) {
        return ParseFalse();
    } else if (MatchKeyword(Keyword_null)) {
        return ParseNull();
    } else if (MatchKeyword(Keyword_func)) {
        return nullptr;//ParseFunctionExpression();
    } else if (Match(Token_operator)) {
        return nullptr;//ParseUnaryExpression(); 
    } else {
        CompilerError error(Level_fatal, Msg_unexpected_token, 
            token->GetLocation(), token->GetValue());
        m_compilation_unit->GetErrorList().AddError(error);

        m_token_stream->Next();
        
        return nullptr;
    }
}

std::shared_ptr<AstExpression> Parser::ParseParentheses()
{
    Expect(Token_open_parenthesis, true);
    std::shared_ptr<AstExpression> expr(ParseExpression());
    Expect(Token_close_parenthesis, true);
    return expr;
}

std::shared_ptr<AstInteger> Parser::ParseIntegerLiteral()
{
    const Token *token = Expect(Token_integer_literal, true);
    a_int value = (a_int)atoll(token->GetValue().c_str());
    return std::shared_ptr<AstInteger>(
        new AstInteger(value, token->GetLocation()));
}

std::shared_ptr<AstFloat> Parser::ParseFloatLiteral()
{
    const Token *token = Expect(Token_float_literal, true);
    a_float value = (a_float)atof(token->GetValue().c_str());
    return std::shared_ptr<AstFloat>(
        new AstFloat(value, token->GetLocation()));
}

std::shared_ptr<AstString> Parser::ParseStringLiteral()
{
    const Token *token = Expect(Token_string_literal, true);
    return std::shared_ptr<AstString>(
        new AstString(token->GetValue(), token->GetLocation()));
}

std::shared_ptr<AstExpression> Parser::ParseIdentifier()
{
    const Token *token = Expect(Token_identifier, false);
    if (MatchAhead(Token_open_parenthesis, 1)) {
        // function call
        return ParseFunctionCall();
    } else {
        // read identifier token
        m_token_stream->Next();

        // return variable
        return std::shared_ptr<AstVariable>(
                new AstVariable(token->GetValue(), token->GetLocation()));
    }
}

std::shared_ptr<AstFunctionCall> Parser::ParseFunctionCall()
{
    const Token *token = Expect(Token_identifier, true);
    Expect(Token_open_parenthesis, true);

    std::vector<std::shared_ptr<AstExpression>> args;

    if (!Match(Token_close_parenthesis, false)) {
        while (true) {
            std::shared_ptr<AstExpression> expr = ParseExpression();
            if (expr == nullptr) {
                return nullptr;
            }

            args.push_back(expr);

            if (Match(Token_close_parenthesis, true)) {
                // stop reading function arguments
                break;
            } else if (!Expect(Token_comma, true)) {
                // unexpected token
                return nullptr;
            }
        }
    }
    
    return std::shared_ptr<AstFunctionCall>(
            new AstFunctionCall(token->GetValue(), args, token->GetLocation()));
}

std::shared_ptr<AstTrue> Parser::ParseTrue()
{
    const Token *token = ExpectKeyword(Keyword_true, true);
    return std::shared_ptr<AstTrue>(
        new AstTrue(token->GetLocation()));
}

std::shared_ptr<AstFalse> Parser::ParseFalse()
{
    const Token *token = ExpectKeyword(Keyword_false, true);
    return std::shared_ptr<AstFalse>(
        new AstFalse(token->GetLocation()));
}

std::shared_ptr<AstNull> Parser::ParseNull()
{
    const Token *token = ExpectKeyword(Keyword_null, true);
    return std::shared_ptr<AstNull>(
        new AstNull(token->GetLocation()));
}

std::shared_ptr<AstBlock> Parser::ParseBlock()
{
    const Token *token = Expect(Token_open_brace, true);
    if (token != nullptr) {
        std::shared_ptr<AstBlock> block(new AstBlock(token->GetLocation()));
        while (!Match(Token_close_brace, true)) {
            // read semicolon tokens
            if (!Match(Token_semicolon, true)) {
                block->AddChild(ParseStatement());
            }
        }
        
        return block;
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
        const Token *token = Expect(Token_operator, true);

        std::shared_ptr<AstExpression> right = ParseTerm();
        if (right == nullptr) {
            return nullptr;
        }

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
            new AstBinaryExpression(left, right, op, 
                token->GetLocation()));
    }
    
    return nullptr;
}

std::shared_ptr<AstExpression> Parser::ParseExpression(bool standalone)
{
    std::shared_ptr<AstExpression> term = ParseTerm();
    if (term == nullptr) {
        return nullptr;
    }

    if (Match(Token_operator, false)) {
        std::shared_ptr<AstExpression> bin_expr = ParseBinaryExpression(0, term);
        if (bin_expr == nullptr) {
            return nullptr;
        }
        term = bin_expr;
    }
    
    term->m_is_standalone = standalone;
    return term;
}

std::shared_ptr<AstVariableDeclaration> Parser::ParseVariableDeclaration()
{
    const Token *token = ExpectKeyword(Keyword_var, true);
    const Token *identifier = Expect(Token_identifier, true);

    if (identifier != nullptr) {
        std::shared_ptr<AstExpression> assignment(nullptr);

        const Token *op = Match(Token_operator, true);
        if (op != nullptr) {
            if (op->GetValue() == Operator::operator_assign.ToString()) {
                // read assignment expression
                assignment = ParseExpression();
            } else {
                // unexpected operator
                CompilerError error(Level_fatal,
                    Msg_illegal_operator, op->GetLocation());

                m_compilation_unit->GetErrorList().AddError(error);
            }
        } else {
            // assign variable to null
            assignment.reset(new AstNull(CurrentLocation()));
        }

        return std::shared_ptr<AstVariableDeclaration>(
            new AstVariableDeclaration(identifier->GetValue(), assignment, token->GetLocation()));
    }

    return nullptr;
}

std::shared_ptr<AstImport> Parser::ParseImport()
{
    if (ExpectKeyword(Keyword_import, true)) {
        if (Match(Token_string_literal, false)) {
            return ParseLocalImport();
        } else {
            // TODO: handle other types of imports here
        }
    }

    return nullptr;
}

std::shared_ptr<AstLocalImport> Parser::ParseLocalImport()
{
    SourceLocation location(CurrentLocation());

    const Token *file = Expect(Token_string_literal, true);
    if (file != nullptr) {
        std::shared_ptr<AstLocalImport> result(
            new AstLocalImport(file->GetValue(), location));

        return result;
    }

    return nullptr;
}