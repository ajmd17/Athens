#include <ace-c/ast/AstBinaryExpression.hpp>
#include <ace-c/ast/AstVariable.hpp>
#include <ace-c/ast/AstFalse.hpp>
#include <ace-c/Operator.hpp>
#include <ace-c/AstVisitor.hpp>
#include <ace-c/Optimizer.hpp>
#include <ace-c/Compiler.hpp>
#include <ace-c/SemanticAnalyzer.hpp>
#include <ace-c/Module.hpp>

#include <ace-c/type-system/BuiltinTypes.hpp>

#include <ace-c/emit/BytecodeChunk.hpp>
#include <ace-c/emit/BytecodeUtil.hpp>

#include <common/instructions.hpp>

AstBinaryExpression::AstBinaryExpression(const std::shared_ptr<AstExpression> &left,
    const std::shared_ptr<AstExpression> &right,
    const Operator *op,
    const SourceLocation &location)
    : AstExpression(location, ACCESS_MODE_LOAD),
      m_left(left),
      m_right(right),
      m_op(op)
{
}

void AstBinaryExpression::Visit(AstVisitor *visitor, Module *mod)
{
    ASSERT(m_left != nullptr);
    ASSERT(m_right != nullptr);

#if ACE_ENABLE_LAZY_DECLARATIONS
    // check for lazy declaration first
    if ((m_variable_declaration = CheckLazyDeclaration(visitor, mod))) {
        m_variable_declaration->Visit(visitor, mod);
        // return, our work here is done
        return;
    }
#endif

    m_left->Visit(visitor, mod);
    m_right->Visit(visitor, mod);

    SymbolTypePtr_t left_type = m_left->GetSymbolType();
    SymbolTypePtr_t left_type_unboxed = left_type;
    
    if (left_type->GetTypeClass() == TYPE_GENERIC_INSTANCE && left_type->IsBoxedType()) {
        left_type_unboxed = left_type->GetGenericInstanceInfo().m_generic_args[0].m_type;
    }

    ASSERT(left_type_unboxed != nullptr);

    SymbolTypePtr_t right_type = m_right->GetSymbolType();
    SymbolTypePtr_t right_type_unboxed = right_type;
    
    if (right_type->GetTypeClass() == TYPE_GENERIC_INSTANCE && right_type->IsBoxedType()) {
        right_type_unboxed = right_type->GetGenericInstanceInfo().m_generic_args[0].m_type;   
    }

    ASSERT(right_type_unboxed != nullptr);
    
    if (m_op->GetType() & BITWISE) {
        // no bitwise operators on floats allowed.
        visitor->Assert(
            (left_type_unboxed == BuiltinTypes::INT || left_type_unboxed == BuiltinTypes::ANY) &&
            (right_type_unboxed == BuiltinTypes::INT || right_type_unboxed == BuiltinTypes::ANY),
            CompilerError(
                LEVEL_ERROR,
                Msg_bitwise_operands_must_be_int, m_location,
                left_type_unboxed->GetName(),
                right_type_unboxed->GetName()
            )
        );
    }

    if (m_op->ModifiesValue()) {
        SemanticAnalyzer::Helpers::EnsureTypeAssignmentCompatibility(
            visitor,
            mod,
            left_type_unboxed,
            right_type_unboxed,
            m_location
        );
        
        // make sure we are not modifying a const
        if (left_type->IsConstType()) {
            visitor->GetCompilationUnit()->GetErrorList().AddError(CompilerError(
                LEVEL_ERROR,
                Msg_const_modified,
                m_left->GetLocation()
            ));
        }

        /*if (AstVariable *left_as_var = dynamic_cast<AstVariable*>(m_left.get())) {
            if (left_as_var->GetProperties().GetIdentifier() != nullptr) {
                if (left_as_var->GetProperties().GetIdentifier()->GetFlags() & FLAG_CONST) {
                    visitor->GetCompilationUnit()->GetErrorList().AddError(CompilerError(
                        LEVEL_ERROR,
                        Msg_const_modified,
                        m_left->GetLocation(),
                        left_as_var->GetName()
                    ));
                }
            }
        }*/
        
        // make sure left hand side is suitable for assignment
        if (!(m_left->GetAccessOptions() & AccessMode::ACCESS_MODE_STORE)) {
            visitor->GetCompilationUnit()->GetErrorList().AddError(CompilerError(
                LEVEL_ERROR,
                Msg_cannot_modify_rvalue,
                m_location
            ));
        }
    } else {
        // compare both sides because assignment does not matter in this case
        if (!left_type_unboxed->TypeCompatible(*right_type_unboxed, false)) {
            visitor->GetCompilationUnit()->GetErrorList().AddError(CompilerError(
                LEVEL_ERROR,
                Msg_mismatched_types,
                m_location,
                left_type_unboxed->GetName(),
                right_type_unboxed->GetName()
            ));
        }
    }
}

std::unique_ptr<Buildable> AstBinaryExpression::Build(AstVisitor *visitor, Module *mod)
{
#if ACE_ENABLE_LAZY_DECLARATIONS
    if (m_variable_declaration != nullptr) {
        return m_variable_declaration->Build(visitor, mod);
    }
#endif

    std::unique_ptr<BytecodeChunk> chunk = BytecodeUtil::Make<BytecodeChunk>();

    Compiler::ExprInfo info {
        m_left.get(),
        m_right.get()
    };

    if (m_op->GetType() == ARITHMETIC || m_op->GetType() == BITWISE) {
        uint8_t opcode = 0;

        switch (m_op->GetOperatorType()) {
            case Operators::OP_add:
                opcode = ADD;
                break;
            case Operators::OP_subtract:
                opcode = SUB;
                break;
            case Operators::OP_multiply:
                opcode = MUL;
                break;
            case Operators::OP_divide:
                opcode = DIV;
                break;
            case Operators::OP_modulus:
                opcode = MOD;
                break;
            case Operators::OP_bitwise_and:
                opcode = AND;
                break;
            case Operators::OP_bitwise_or:
                opcode = OR;
                break;
            case Operators::OP_bitwise_xor:
                opcode = XOR;
                break;
            case Operators::OP_bitshift_left:
                opcode = SHL;
                break;
            case Operators::OP_bitshift_right:
                opcode = SHR;
                break;
        }

        chunk->Append(Compiler::BuildBinOp(opcode, visitor, mod, info));

        visitor->GetCompilationUnit()->GetInstructionStream().DecRegisterUsage();
    } else if (m_op->GetType() == LOGICAL) {
        std::shared_ptr<AstExpression> first = nullptr;
        std::shared_ptr<AstExpression> second = nullptr;

        AstBinaryExpression *left_as_binop = dynamic_cast<AstBinaryExpression*>(m_left.get());
        AstBinaryExpression *right_as_binop = dynamic_cast<AstBinaryExpression*>(m_right.get());

        if (left_as_binop == nullptr && right_as_binop != nullptr) {
            first = m_right;
            second = m_left;
        } else {
            first = m_left;
            second = m_right;
        }

        if (m_op->GetOperatorType() == Operators::OP_logical_and) {
            uint8_t rp;

            LabelId false_label = chunk->NewLabel();
            LabelId true_label = chunk->NewLabel();

            rp = visitor->GetCompilationUnit()->GetInstructionStream().GetCurrentRegister();

            // TODO: There is some duplicated code in here, needs to be cleaned up.

            { // do first part of expression
                bool folded = false;
                // attempt to constant fold the values
                std::shared_ptr<AstExpression> tmp(new AstFalse(SourceLocation::eof));
                
                if (auto constant_folded = Optimizer::ConstantFold(first, tmp, Operators::OP_equals, visitor)) {
                    int folded_value = constant_folded->IsTrue();
                    folded = folded_value == 1 || folded_value == 0;

                    if (folded_value == 1) {
                        // value is equal to 0, therefore it is false.
                        // load the label address from static memory into register 0
                        chunk->Append(BytecodeUtil::Make<Jump>(Jump::JMP, false_label));
                    } else if (folded_value == 0) {
                        // do not jump at all, only accept the code that it is true
                    }
                }

                if (!folded) {
                    // load left-hand side into register 0
                    chunk->Append(first->Build(visitor, mod));

                    // since this is an AND operation, jump as soon as the lhs is determined to be false
                    rp = visitor->GetCompilationUnit()->GetInstructionStream().GetCurrentRegister();

                    // compare lhs to 0 (false)
                    chunk->Append(BytecodeUtil::Make<Comparison>(Comparison::CMPZ, rp));

                    // jump if they are equal: i.e the value is false
                    chunk->Append(BytecodeUtil::Make<Jump>(Jump::JE, false_label));
                }
            }

            // if we are at this point then lhs is true, so now test the rhs
            if (second != nullptr) {
                bool folded = false;
                // attempt to constant fold the values
                std::shared_ptr<AstExpression> tmp(new AstFalse(SourceLocation::eof));
                
                if (auto constant_folded = Optimizer::ConstantFold(second, tmp, Operators::OP_equals, visitor)) {
                    int folded_value = constant_folded->IsTrue();
                    folded = folded_value == 1 || folded_value == 0;

                    if (folded_value == 1) {
                        // value is equal to 0, therefore it is false.
                        // load the label address from static memory into register 0
                        chunk->Append(BytecodeUtil::Make<Jump>(Jump::JMP, false_label));
                    }
                }

                if (!folded) {
                    // load right-hand side into register 1
                    chunk->Append(second->Build(visitor, mod));

                    // get register position
                    rp = visitor->GetCompilationUnit()->GetInstructionStream().GetCurrentRegister();

                    // compare lhs to 0 (false)
                    chunk->Append(BytecodeUtil::Make<Comparison>(Comparison::CMPZ, rp));

                    // jump if they are equal: i.e the value is false
                    chunk->Append(BytecodeUtil::Make<Jump>(Jump::JE, false_label));
                }
            }

            // both values were true at this point so load the value 'true'
            chunk->Append(BytecodeUtil::Make<ConstBool>(rp, true));

            // jump to the VERY end (so we don't load 'false' value)
            chunk->Append(BytecodeUtil::Make<Jump>(Jump::JMP, false_label));

            chunk->Append(BytecodeUtil::Make<LabelMarker>(false_label));
            
            // here is where the value is false
            chunk->Append(BytecodeUtil::Make<ConstBool>(rp, false));

            chunk->Append(BytecodeUtil::Make<LabelMarker>(true_label));
        } else if (m_op->GetOperatorType() == Operators::OP_logical_or) {
            uint8_t rp;

            LabelId false_label = chunk->NewLabel();
            LabelId true_label = chunk->NewLabel();

            rp = visitor->GetCompilationUnit()->GetInstructionStream().GetCurrentRegister();

            { // do first part of expression
                bool folded = false;
                // attempt to constant fold the values
                std::shared_ptr<AstExpression> tmp(new AstFalse(SourceLocation::eof));
                
                if (auto constant_folded = Optimizer::ConstantFold(first, tmp, Operators::OP_equals, visitor)) {
                    int folded_value = constant_folded->IsTrue();
                    folded = folded_value == 1 || folded_value == 0;

                    if (folded_value == 1) {
                        // do not jump at all, we still have to test the second half of the expression
                    } else if (folded_value == 0) {
                        // jump to end, the value is true and we don't have to check the second half
                        chunk->Append(BytecodeUtil::Make<Jump>(Jump::JMP, true_label));
                    }
                }

                if (!folded) {
                    // load left-hand side into register 0
                    chunk->Append(first->Build(visitor, mod));
                    // since this is an OR operation, jump as soon as the lhs is determined to be anything but 0
                    rp = visitor->GetCompilationUnit()->GetInstructionStream().GetCurrentRegister();

                    // compare lhs to 0 (false)
                    chunk->Append(BytecodeUtil::Make<Comparison>(Comparison::CMPZ, rp));
                    chunk->Append(BytecodeUtil::Make<Jump>(Jump::JNE, true_label));
                }
            }

            // if we are at this point then lhs is true, so now test the rhs
            if (second != nullptr) {
                bool folded = false;
                { // attempt to constant fold the values
                    std::shared_ptr<AstExpression> tmp(new AstFalse(SourceLocation::eof));
                    
                    if (auto constant_folded = Optimizer::ConstantFold(second, tmp, Operators::OP_equals, visitor)) {
                        Tribool folded_value = constant_folded->IsTrue();

                        if (folded_value == Tribool::True()) {
                            // value is equal to 0
                            folded = true;
                        } else if (folded_value == Tribool::False()) {
                            folded = true;
                            // value is equal to 1 so jump to end
                            chunk->Append(BytecodeUtil::Make<Jump>(Jump::JMP, true_label));
                        }
                    }
                }

                if (!folded) {
                    // load right-hand side into register 1
                    chunk->Append(second->Build(visitor, mod));
                    // get register position
                    rp = visitor->GetCompilationUnit()->GetInstructionStream().GetCurrentRegister();

                    // compare rhs to 0 (false)
                    chunk->Append(BytecodeUtil::Make<Comparison>(Comparison::CMPZ, rp));

                    // jump if they are equal: i.e the value is true
                    chunk->Append(BytecodeUtil::Make<Jump>(Jump::JNE, true_label));
                }
            }

            // no values were true at this point so load the value 'false'
            chunk->Append(BytecodeUtil::Make<ConstBool>(rp, false));

            // jump to the VERY end (so we don't load 'true' value)
            chunk->Append(BytecodeUtil::Make<Jump>(Jump::JMP, false_label));
            chunk->Append(BytecodeUtil::Make<LabelMarker>(true_label));

            // here is where the value is true
            chunk->Append(BytecodeUtil::Make<ConstBool>(rp, true));
            chunk->Append(BytecodeUtil::Make<LabelMarker>(false_label));
        }
    } else if (m_op->GetType() == COMPARISON) {
        uint8_t rp;
        
        Jump::JumpClass jump_class;
        
        bool swapped = false;

        switch (m_op->GetOperatorType()) {
            case Operators::OP_equals:
                jump_class = Jump::JNE;
                break;
            case Operators::OP_not_eql:
                jump_class = Jump::JE;
                break;
            case Operators::OP_less:
                jump_class = Jump::JGE;
                break;
            case Operators::OP_less_eql:
                jump_class = Jump::JG;
                break;
            case Operators::OP_greater:
                jump_class = Jump::JGE;
                swapped = true;
                break;
            case Operators::OP_greater_eql:
                jump_class = Jump::JG;
                swapped = true;
                break;
        }

        AstBinaryExpression *left_as_binop = dynamic_cast<AstBinaryExpression*>(m_left.get());
        AstBinaryExpression *right_as_binop = dynamic_cast<AstBinaryExpression*>(m_right.get());

        if (m_right != nullptr) {
            uint8_t r0, r1;

            LabelId true_label = chunk->NewLabel();
            LabelId false_label = chunk->NewLabel();

            if (left_as_binop == nullptr && right_as_binop != nullptr) {
                // if the right hand side is a binary operation,
                // we should build in the rhs first in order to
                // transverse the parse tree.
                chunk->Append(Compiler::LoadRightThenLeft(visitor, mod, info));
                rp = visitor->GetCompilationUnit()->GetInstructionStream().GetCurrentRegister();

                r0 = rp;
                r1 = rp - 1;
            } else if (m_right != nullptr && m_right->MayHaveSideEffects()) {
                // lhs must be temporary stored on the stack,
                // to avoid the rhs overwriting it.
                if (m_left->MayHaveSideEffects()) {
                    chunk->Append(Compiler::LoadLeftAndStore(visitor, mod, info));
                    rp = visitor->GetCompilationUnit()->GetInstructionStream().GetCurrentRegister();

                    r0 = rp - 1;
                    r1 = rp;
                } else {
                    // left  doesn't have side effects,
                    // so just evaluate right without storing the lhs.
                    chunk->Append(Compiler::LoadRightThenLeft(visitor, mod, info));
                    rp = visitor->GetCompilationUnit()->GetInstructionStream().GetCurrentRegister();

                    r0 = rp;
                    r1 = rp - 1;
                }
            } else {
                // normal usage, load left into register 0,
                // then load right into register 1.
                // rinse and repeat.
                chunk->Append(Compiler::LoadLeftThenRight(visitor, mod, info));
                rp = visitor->GetCompilationUnit()->GetInstructionStream().GetCurrentRegister();

                r0 = rp - 1;
                r1 = rp;
            }

            if (swapped) {
                std::swap(r0, r1);
            }

            // perform operation
            chunk->Append(BytecodeUtil::Make<Comparison>(Comparison::CMP, r0, r1));

            visitor->GetCompilationUnit()->GetInstructionStream().DecRegisterUsage();
            rp = visitor->GetCompilationUnit()->GetInstructionStream().GetCurrentRegister();

            // jump if they are equal
            chunk->Append(BytecodeUtil::Make<Jump>(jump_class, true_label));
            
            // values are not equal at this point
            chunk->Append(BytecodeUtil::Make<ConstBool>(rp, true));

            // jump to the false label, the value is false at this point
            chunk->Append(BytecodeUtil::Make<Jump>(Jump::JMP, false_label));
            
            chunk->Append(BytecodeUtil::Make<LabelMarker>(true_label));

            // values are equal
            chunk->Append(BytecodeUtil::Make<ConstBool>(rp, false));

            chunk->Append(BytecodeUtil::Make<LabelMarker>(false_label));
        } else {
            // load left-hand side into register
            // right-hand side has been optimized away
            chunk->Append(m_left->Build(visitor, mod));
        }
    } else if (m_op->GetType() & ASSIGNMENT) {
        uint8_t rp;

        if (m_op->GetOperatorType() == Operators::OP_assign) {
            // load right-hand side into register 0
            chunk->Append(m_right->Build(visitor, mod));
            visitor->GetCompilationUnit()->GetInstructionStream().IncRegisterUsage();
        } else {
            // assignment/operation
            uint8_t opcode = 0;

            switch (m_op->GetOperatorType()) {
                case Operators::OP_add_assign:
                    opcode = ADD;
                    break;
                case Operators::OP_subtract_assign:
                    opcode = SUB;
                    break;
                case Operators::OP_multiply_assign:
                    opcode = MUL;
                    break;
                case Operators::OP_divide_assign:
                    opcode = DIV;
                    break;
                case Operators::OP_modulus_assign:
                    opcode = MOD;
                    break;
            }

            chunk->Append(Compiler::BuildBinOp(opcode, visitor, mod, info));
        }

        rp = visitor->GetCompilationUnit()->GetInstructionStream().GetCurrentRegister();

        if (m_left->GetAccessOptions() & AccessMode::ACCESS_MODE_STORE) {
            m_left->SetAccessMode(AccessMode::ACCESS_MODE_STORE);
            chunk->Append(m_left->Build(visitor, mod));
        }

        visitor->GetCompilationUnit()->GetInstructionStream().DecRegisterUsage();
    }

    return std::move(chunk);
}

void AstBinaryExpression::Optimize(AstVisitor *visitor, Module *mod)
{
#if ACE_ENABLE_LAZY_DECLARATIONS
    if (m_variable_declaration != nullptr) {
        m_variable_declaration->Optimize(visitor, mod);
        return;
    }
#endif

    ASSERT(m_left != nullptr);
    ASSERT(m_right != nullptr);

    m_left->Optimize(visitor, mod);
    m_left = Optimizer::OptimizeExpr(m_left, visitor, mod);

    m_right->Optimize(visitor, mod);
    m_right = Optimizer::OptimizeExpr(m_right, visitor, mod);
    
    // check that we can further optimize the
    // binary expression by optimizing away the right
    // side, and combining the resulting value into
    // the left side of the operation.
    auto constant_value = Optimizer::ConstantFold(
        m_left,
        m_right,
        m_op->GetOperatorType(),
        visitor
    );

    if (constant_value != nullptr) {
        // compile-time evaluation was successful
        m_left = constant_value;
        m_right = nullptr;
    }
}

Pointer<AstStatement> AstBinaryExpression::Clone() const
{
    return CloneImpl();
}

Tribool AstBinaryExpression::IsTrue() const
{
    // if (m_member_access) {
    //     return m_member_access->IsTrue();
    // } else {
        if (m_right != nullptr) {
            // the right was not optimized away,
            // therefore we cannot determine whether or
            // not this expression would be true or false.
            return Tribool::Indeterminate();
        }

        return m_left->IsTrue();
    //}
}

bool AstBinaryExpression::MayHaveSideEffects() const
{
    // if (m_member_access) {
    //     return m_member_access->MayHaveSideEffects();
    // } else {
        bool left_side_effects = m_left->MayHaveSideEffects();
        bool right_side_effects = false;

        if (m_right != nullptr) {
            right_side_effects = m_right->MayHaveSideEffects();
        }

        if (m_op->ModifiesValue()) {
            return true;
        }

        return left_side_effects || right_side_effects;
    //}
}

SymbolTypePtr_t AstBinaryExpression::GetSymbolType() const
{
    // if (m_member_access) {
    //     return m_member_access->GetSymbolType();
    // } else {
        ASSERT(m_left != nullptr);

        SymbolTypePtr_t l_type_ptr = m_left->GetSymbolType();
        ASSERT(l_type_ptr != nullptr);

        if (m_right != nullptr) {
            // the right was not optimized away,
            // return type promotion
            SymbolTypePtr_t r_type_ptr = m_right->GetSymbolType();

            ASSERT(r_type_ptr != nullptr);

            return SymbolType::TypePromotion(l_type_ptr, r_type_ptr, true);
        } else {
            // right was optimized away, return only left type
            return l_type_ptr;
        }
    //}
}

#if ACE_ENABLE_LAZY_DECLARATIONS
std::shared_ptr<AstVariableDeclaration> AstBinaryExpression::CheckLazyDeclaration(AstVisitor *visitor, Module *mod)
{
    if (m_op->GetOperatorType() == Operators::OP_assign) {
        if (AstVariable *left_as_var = dynamic_cast<AstVariable*>(m_left.get())) {
            std::string var_name = left_as_var->GetName();
            // lookup variable name
            if (mod->LookUpIdentifier(var_name, false)) {
                return nullptr;
            }
            // not found as variable name
            // look up in the global module
            if (visitor->GetCompilationUnit()->GetGlobalModule()->LookUpIdentifier(var_name, false)) {
                return nullptr;
            }

            // check all modules for one with the same name
            if (visitor->GetCompilationUnit()->LookupModule(var_name)) {
                return nullptr;
            }

            return std::shared_ptr<AstVariableDeclaration>(new AstVariableDeclaration(
                var_name,
                nullptr,
                m_right,
                false,
                m_left->GetLocation()
            ));
        }
    }

    return nullptr;
}
#endif