#include <athens/ast/ast_binary_expression.h>
#include <athens/ast/ast_variable.h>
#include <athens/ast/ast_constant.h>
#include <athens/ast_visitor.h>
#include <athens/operator.h>

/** Attemps to reduce a variable that is const literal to the actual value. */
static void OptimizeSide(std::shared_ptr<AstExpression> &side)
{
    side->Optimize();

    auto side_as_var = std::dynamic_pointer_cast<AstVariable>(side);
    if (side_as_var != nullptr) {
        // the side is a variable, so we can further optimize by inlining,
        // only if it is const, and a literal.
        if (side_as_var->GetIdentifier() != nullptr) {
            if (side_as_var->GetIdentifier()->GetFlags() & (Flag_const | Flag_literal)) {
                // the variable is a const literal so we can inline it
                // set it to be the current value
                auto sp = side_as_var->GetIdentifier()->GetCurrentValue().lock();
                if (sp != nullptr) {
                    // yay! we were able to retrieve the value that
                    // the variable is set to, so now we can use that
                    // at compile-time rather than using a variable.
                    side = sp;
                }
            }
        }
    }
}

/** Attemps to evaluate the optimized expression at compile-time. */
static std::shared_ptr<AstConstant> ConstantFold(std::shared_ptr<AstExpression> &left, 
    std::shared_ptr<AstExpression> &right, const Operator *oper)
{
    auto left_as_constant = std::dynamic_pointer_cast<AstConstant>(left);
    auto right_as_constant = std::dynamic_pointer_cast<AstConstant>(right);

    std::shared_ptr<AstConstant> result(nullptr);

    if (left_as_constant != nullptr && right_as_constant != nullptr) {
        // perform operations on these constants
        if (oper == &Operator::operator_add) {
            result = (*left_as_constant) + right_as_constant;
        } else if (oper == &Operator::operator_subtract) {
            result = (*left_as_constant) - right_as_constant;
        } else if (oper == &Operator::operator_multiply) {
            result = (*left_as_constant) * right_as_constant;
        } else if (oper == &Operator::operator_divide) {
            result = (*left_as_constant) / right_as_constant;
        } else if (oper == &Operator::operator_modulus) {
            result = (*left_as_constant) % right_as_constant;
        } else if (oper == &Operator::operator_bitwise_xor) {
            result = (*left_as_constant) ^ right_as_constant;
        } else if (oper == &Operator::operator_bitwise_and) {
            result = (*left_as_constant) & right_as_constant;
        } else if (oper == &Operator::operator_bitshift_left) {
            result = (*left_as_constant) << right_as_constant;
        } else if (oper == &Operator::operator_bitshift_right) {
            result = (*left_as_constant) >> right_as_constant;
        } else if (oper == &Operator::operator_logical_and) {
            result = (*left_as_constant) && right_as_constant;
        } else if (oper == &Operator::operator_logical_or) {
            result = (*left_as_constant) || right_as_constant;
        }
        // don't have to worry about assignment operations,
        // because at this point both sides are const and literal.

        if (result == nullptr) {
            // an error has occured during compile-time evaluation
            // wat do ?
        }
    }
    
    // one or both of the sides are not a constant
    return result;
}

AstBinaryExpression::AstBinaryExpression(const std::shared_ptr<AstExpression> &left,
        const std::shared_ptr<AstExpression> &right,
        const Operator *op,
        const SourceLocation &location)
    : AstExpression(location),
      m_left(left),
      m_right(right),
      m_op(op)
{
}

void AstBinaryExpression::Visit(AstVisitor *visitor)
{
    m_left->Visit(visitor);
    m_right->Visit(visitor);

    if (m_op->ModifiesValue()) {
        auto left_as_var = std::dynamic_pointer_cast<AstVariable>(m_left);
        if (left_as_var != nullptr) {
            // make sure we are not modifying a const
            if (left_as_var->GetIdentifier() != nullptr) {
                visitor->Assert(!(left_as_var->GetIdentifier()->GetFlags() & Flag_const),
                    CompilerError(Level_fatal, Msg_const_modified, 
                        m_left->GetLocation(), left_as_var->GetName()));
            }
        }
    }
}

void AstBinaryExpression::Build(AstVisitor *visitor) const
{
    if (m_right != nullptr) {
        // the right side has not been optimized away
    }
}

void AstBinaryExpression::Optimize()
{
    OptimizeSide(m_left);
    OptimizeSide(m_right);

    // check that we can further optimize the
    // binary expression by optimizing away the right
    // side, and combining the resulting value into
    // the left side of the operation.
    auto constant_value = ConstantFold(m_left, m_right, m_op);
    if (constant_value != nullptr) {
        m_left = constant_value;
        m_right = nullptr;
    }
}

int AstBinaryExpression::IsTrue() const
{
    if (m_right != nullptr) {
        // the right was not optimized away,
        // therefore we cannot determine whether or
        // not this expression would be true or false.
        return -1;
    }

    return m_left->IsTrue();
}
