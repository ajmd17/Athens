#include <ace-c/ast/AstVariable.hpp>
#include <ace-c/AstVisitor.hpp>
#include <ace-c/ast/AstConstant.hpp>
#include <ace-c/ast/AstInteger.hpp>
#include <ace-c/Scope.hpp>
#include <ace-c/emit/Instruction.hpp>

#include <common/instructions.hpp>
#include <common/my_assert.hpp>

#include <iostream>

AstVariable::AstVariable(const std::string &name, const SourceLocation &location)
    : AstIdentifier(name, location)
{
}

void AstVariable::Visit(AstVisitor *visitor, Module *mod)
{
    AstIdentifier::Visit(visitor, mod);

    ASSERT(m_properties.GetIdentifierType() != IDENTIFIER_TYPE_UNKNOWN);

    switch (m_properties.GetIdentifierType()) {
        case IDENTIFIER_TYPE_VARIABLE: {
            ASSERT(m_properties.GetIdentifier() != nullptr);

            if (m_properties.GetIdentifier()->GetFlags() & IdentifierFlags::FLAG_ALIAS) {
                const std::shared_ptr<AstExpression> &current_value = m_properties.GetIdentifier()->GetCurrentValue();
                ASSERT(current_value != nullptr);

                // set access options for this variable based on those of the current value
                AstExpression::m_access_options = current_value->GetAccessOptions();

                // if alias, accept the current value instead
                current_value->Visit(visitor, mod);
            } else {
                m_properties.GetIdentifier()->IncUseCount();

                if (m_properties.IsInFunction()) {
                    if (m_properties.IsInPureFunction()) {
                        // check if pure function - in a pure function, only variables from this scope may be used
                        if (!mod->LookUpIdentifierDepth(m_name, m_properties.GetDepth())) {
                            // add error that the variable must be passed as a parameter
                            visitor->GetCompilationUnit()->GetErrorList().AddError(CompilerError(
                                LEVEL_ERROR,
                                Msg_pure_function_scope,
                                m_location,
                                m_name
                            ));
                        }
                    }

                    // NOTE: if we are in a function, and the variable we are loading is declared in a separate function,
                    // we will show an error message saying that the variable must be passed as a parameter to be captured.
                    // the reason for this is that any variables owned by the parent function will be immediately popped from the stack
                    // when the parent function returns. That will mean the variables used here will reference garbage.
                    // In the near feature, it'd be possible to automatically make a copy of those variables referenced and store them
                    // on the stack of /this/ function.
                    if (m_properties.GetIdentifier()->GetFlags() & FLAG_DECLARED_IN_FUNCTION) {
                        // lookup the variable by depth to make sure it was declared in the current function
                        // we do this to make sure it was declared in this scope.
                        if (!mod->LookUpIdentifierDepth(m_name, m_properties.GetDepth())) {
                            Scope *function_scope = m_properties.GetFunctionScope();
                            ASSERT(function_scope != nullptr);

                            function_scope->AddClosureCapture(
                                m_name,
                                m_properties.GetIdentifier()
                            );

                            // closures are objects with a method named '$invoke',
                            // because we are in the '$invoke' method currently,
                            // we use the variable as 'self.<variable name>'
                            m_closure_member_access.reset(new AstMember(
                                m_name,
                                std::shared_ptr<AstVariable>(new AstVariable(
                                    "self",
                                    m_location
                                )),
                                m_location
                            ));

                            m_closure_member_access->Visit(visitor, mod);

                            // we're in closure, lookup variable __closure_locals and get array access
                            /*m_closure_array_access = std::shared_ptr<AstArrayAccess>(new AstArrayAccess(
                                std::shared_ptr<AstVariable>(new AstVariable(
                                    "__closure_locals",
                                    m_location
                                )),
                                std::shared_ptr<AstInteger>(new AstInteger(
                                    0, // TODO
                                    m_location
                                )),
                                m_location
                            ));

                            m_closure_array_access->Visit(visitor, mod);*/
                            
                            // add error that the variable must be passed as a parameter
                            /*visitor->GetCompilationUnit()->GetErrorList().AddError(CompilerError(
                                LEVEL_ERROR,
                                Msg_closure_capture_must_be_parameter,
                                m_location,
                                m_name
                            ));*/
                        }
                    }
                }
            }

            break;
        }
        case IDENTIFIER_TYPE_MODULE:
            visitor->GetCompilationUnit()->GetErrorList().AddError(CompilerError(
                LEVEL_ERROR,
                Msg_identifier_is_module,
                m_location,
                m_name
            ));
            break;
        case IDENTIFIER_TYPE_TYPE:
            visitor->GetCompilationUnit()->GetErrorList().AddError(CompilerError(
                LEVEL_ERROR,
                Msg_identifier_is_type,
                m_location,
                m_name
            ));
            break;
        case IDENTIFIER_TYPE_NOT_FOUND:
            visitor->GetCompilationUnit()->GetErrorList().AddError(CompilerError(
                LEVEL_ERROR,
                Msg_undeclared_identifier,
                m_location,
                m_name,
                mod->GenerateFullModuleName()
            ));
            break;
        default:
            break;
    }
}

void AstVariable::Build(AstVisitor *visitor, Module *mod)
{
    if (m_closure_member_access != nullptr) {
        m_closure_member_access->SetAccessMode(m_access_mode);
        m_closure_member_access->Build(visitor, mod);
    } else {
        ASSERT(m_properties.GetIdentifier() != nullptr);

        // if alias
        if (m_properties.GetIdentifier()->GetFlags() & IdentifierFlags::FLAG_ALIAS) {
            const std::shared_ptr<AstExpression> &current_value = m_properties.GetIdentifier()->GetCurrentValue();
            ASSERT(current_value != nullptr);

            // if alias, accept the current value instead
            const AccessMode current_access_mode = current_value->GetAccessMode();
            current_value->SetAccessMode(m_access_mode);
            current_value->Build(visitor, mod);
            // reset access mode
            current_value->SetAccessMode(current_access_mode);
        } else {
            int stack_size = visitor->GetCompilationUnit()->GetInstructionStream().GetStackSize();
            int stack_location = m_properties.GetIdentifier()->GetStackLocation();
            int offset = stack_size - stack_location;

            // get active register
            uint8_t rp = visitor->GetCompilationUnit()->GetInstructionStream().GetCurrentRegister();

            if (!(m_properties.GetIdentifier()->GetFlags() & FLAG_DECLARED_IN_FUNCTION)) {
                // load globally, rather than from offset.
                if (m_access_mode == ACCESS_MODE_LOAD) {
                    // load stack value at index into register
                    visitor->GetCompilationUnit()->GetInstructionStream() <<
                        Instruction<uint8_t, uint8_t, uint16_t>(LOAD_INDEX, rp, (uint16_t)stack_location);
                } else if (m_access_mode == ACCESS_MODE_STORE) {
                    // store the value at the index into this local variable
                    visitor->GetCompilationUnit()->GetInstructionStream() <<
                        Instruction<uint8_t, uint16_t, uint8_t>(MOV_INDEX, (uint16_t)stack_location, rp - 1);
                }
            } else {
                if (m_access_mode == ACCESS_MODE_LOAD) {
                    // load stack value at offset value into register
                    visitor->GetCompilationUnit()->GetInstructionStream() <<
                        Instruction<uint8_t, uint8_t, uint16_t>(LOAD_OFFSET, rp, (uint16_t)offset);
                } else if (m_access_mode == ACCESS_MODE_STORE) {
                    // store the value at (rp - 1) into this local variable
                    visitor->GetCompilationUnit()->GetInstructionStream() <<
                        Instruction<uint8_t, uint16_t, uint8_t>(MOV_OFFSET, (uint16_t)offset, rp - 1);
                }
            }
        }
    }
}

void AstVariable::Optimize(AstVisitor *visitor, Module *mod)
{
}

void AstVariable::Recreate(std::ostringstream &ss)
{
    ss << m_name;
}

Pointer<AstStatement> AstVariable::Clone() const
{
    return CloneImpl();
}

int AstVariable::IsTrue() const
{
    if (m_properties.GetIdentifier()) {
        // we can only check if this is true during
        // compile time if it is const literal
        if (m_properties.GetIdentifier()->GetFlags() & FLAG_CONST) {
            if (auto *constant = dynamic_cast<AstConstant*>(m_properties.GetIdentifier()->GetCurrentValue().get())) {
                return constant->IsTrue();
            }
        }
    }

    return -1;
}

bool AstVariable::MayHaveSideEffects() const
{
    // a simple variable reference does not cause side effects
    return false;
}
