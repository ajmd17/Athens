#include <ace-c/Optimizer.hpp>
#include <ace-c/ast/AstModuleDeclaration.hpp>
#include <ace-c/ast/AstBinaryExpression.hpp>
#include <ace-c/ast/AstVariable.hpp>
#include <ace-c/ast/AstConstant.hpp>

#include <common/my_assert.hpp>

std::shared_ptr<AstConstant> Optimizer::ConstantFold(
    std::shared_ptr<AstExpression> &left,
    std::shared_ptr<AstExpression> &right,
    Operators op_type,
    AstVisitor *visitor)
{
    AstConstant *left_as_constant  = dynamic_cast<AstConstant*>(left.get());
    AstConstant *right_as_constant = dynamic_cast<AstConstant*>(right.get());

    std::shared_ptr<AstConstant> result;

    if (left_as_constant != nullptr && right_as_constant != nullptr) {
        result = left_as_constant->HandleOperator(op_type, right_as_constant);

        /*// perform operations on these constants
        switch (op_type) {
            case Operators::OP_add:
                result = (*left_as_constant) + right_as_constant;
                break;
            case Operators::OP_subtract:
                result = (*left_as_constant) - right_as_constant;
                break;
            case Operators::OP_multiply:
                result = (*left_as_constant) * right_as_constant;
                break;
            case Operators::OP_divide:
                result = (*left_as_constant) / right_as_constant;
                break;
            case Operators::OP_modulus:
                result = (*left_as_constant) % right_as_constant;
                break;
            case Operators::OP_bitwise_xor:
                result = (*left_as_constant) ^ right_as_constant;
                break;
            case Operators::OP_bitwise_and:
                result = (*left_as_constant) & right_as_constant;
                break;
            case Operators::OP_bitshift_left:
                result = (*left_as_constant) << right_as_constant;
                break;
            case Operators::OP_bitshift_right:
                result = (*left_as_constant) >> right_as_constant;
                break;
            case Operators::OP_logical_and:
                result = (*left_as_constant) && right_as_constant;
                break;
            case Operators::OP_logical_or:
                result = (*left_as_constant) || right_as_constant;
                break;
            case Operators::OP_less:
                result = (*left_as_constant) < right_as_constant;
                break;
            case Operators::OP_greater:
                result = (*left_as_constant) > right_as_constant;
                break;
            case Operators::OP_less_eql:
                result = (*left_as_constant) <= right_as_constant;
                break;
            case Operators::OP_greater_eql:
                result = (*left_as_constant) >= right_as_constant;
                break;
            case Operators::OP_equals:
                result = left_as_constant->Equals(right_as_constant);
                break;
        }*/

        // don't have to worry about assignment operations,
        // because at this point both sides are const and literal.
    }

    // one or both of the sides are not a constant
    return result;
}

std::shared_ptr<AstExpression> Optimizer::OptimizeExpr(
    const std::shared_ptr<AstExpression> &expr,
    AstVisitor *visitor,
    Module *mod)
{
    ASSERT(expr != nullptr);
    expr->Optimize(visitor, mod);

    if (const AstVariable *expr_as_var = dynamic_cast<AstVariable*>(expr.get())) {
        // the side is a variable, so we can further optimize by inlining,
        // only if it is const, and a literal.
        if (const Identifier *ident = expr_as_var->GetProperties().GetIdentifier()) {
            const SymbolTypePtr_t &identifier_type = ident->GetSymbolType();
            const std::shared_ptr<AstExpression> &current_value = ident->GetCurrentValue();

            if (current_value != nullptr) {
                ASSERT(identifier_type != nullptr);
                
                if (identifier_type->IsConstType()) {
                    // decrement use count because it would have been incremented by Visit()
                    ident->DecUseCount();
                    return Optimizer::OptimizeExpr(current_value, visitor, mod);
                }
            }
        }
    } else if (AstBinaryExpression *expr_as_binop = dynamic_cast<AstBinaryExpression*>(expr.get())) {
        if (expr_as_binop->GetRight() == nullptr) {
            // right side has been optimized away, to just left side
            return Optimizer::OptimizeExpr(expr_as_binop->GetLeft(), visitor, mod);
        }
    }

    return expr;
}

Optimizer::Optimizer(AstIterator *ast_iterator, CompilationUnit *compilation_unit)
    : AstVisitor(ast_iterator, compilation_unit)
{
}

Optimizer::Optimizer(const Optimizer &other)
    : AstVisitor(other.m_ast_iterator, other.m_compilation_unit)
{
}

void Optimizer::Optimize(bool expect_module_decl)
{
    /*if (expect_module_decl) {
        if (m_ast_iterator->HasNext()) {
            std::shared_ptr<AstStatement> first_stmt = m_ast_iterator->Next();

            if (AstModuleDeclaration *mod_decl = dynamic_cast<AstModuleDeclaration*>(first_stmt.get())) {
                // all files must begin with a module declaration
                mod_decl->Optimize(this, nullptr);
                OptimizeInner();
            }
        }
    } else {
        OptimizeInner();
    }*/

    OptimizeInner();
}

void Optimizer::OptimizeInner()
{
    Module *mod = m_compilation_unit->GetCurrentModule();
    ASSERT(mod != nullptr);
    
    while (m_ast_iterator->HasNext()) {
        m_ast_iterator->Next()->Optimize(this, mod);
    }
}
