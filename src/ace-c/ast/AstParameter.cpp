#include <ace-c/ast/AstParameter.hpp>
#include <ace-c/AstVisitor.hpp>

#include <ace-c/type-system/BuiltinTypes.hpp>

#include <ace-c/emit/BytecodeChunk.hpp>
#include <ace-c/emit/BytecodeUtil.hpp>

#include <common/my_assert.hpp>

AstParameter::AstParameter(const std::string &name, 
    const std::shared_ptr<AstTypeSpecification> &type_spec, 
    const std::shared_ptr<AstExpression> &default_param, 
    bool is_variadic,
    bool is_const,
    const SourceLocation &location)
    : AstDeclaration(name, location),
      m_type_spec(type_spec),
      m_default_param(default_param),
      m_is_variadic(is_variadic),
      m_is_const(is_const)
{
}

void AstParameter::Visit(AstVisitor *visitor, Module *mod)
{
    AstDeclaration::Visit(visitor, mod);

    // params are `Any` by default
    SymbolTypePtr_t symbol_type = BuiltinTypes::ANY;

    if (m_type_spec != nullptr) {
        m_type_spec->Visit(visitor, mod);

        ASSERT(m_type_spec->GetSymbolType() != nullptr);
        symbol_type = m_type_spec->GetSymbolType();
    }

    if (m_default_param != nullptr) {
        m_default_param->Visit(visitor, mod);

        ASSERT(m_default_param->GetSymbolType() != nullptr);
        const SymbolTypePtr_t default_param_type = m_default_param->GetSymbolType();

        // make sure the types are compatible
        if (m_type_spec != nullptr && !symbol_type->TypeCompatible(*default_param_type, true)) {
            visitor->GetCompilationUnit()->GetErrorList().AddError(CompilerError(
                LEVEL_ERROR,
                Msg_arg_type_incompatible,
                m_default_param->GetLocation(),
                symbol_type->GetName(),
                default_param_type->GetName()
            ));
        } else {
            symbol_type = default_param_type;
        }
    }

    // if variadic, then make it array of whatever type it is
    if (m_is_variadic) {
        symbol_type = SymbolType::GenericInstance(
            BuiltinTypes::VAR_ARGS,
            GenericInstanceTypeInfo {
                {
                    { m_name, symbol_type }
                }
            }
        );
    }

    if (m_identifier != nullptr) {
        m_identifier->SetSymbolType(symbol_type);

        if (m_is_const) {
            m_identifier->SetFlags(m_identifier->GetFlags() | IdentifierFlags::FLAG_CONST);
        }

        if (m_default_param != nullptr) {
            m_identifier->SetCurrentValue(m_default_param);
        }
    }
}

std::unique_ptr<Buildable> AstParameter::Build(AstVisitor *visitor, Module *mod)
{
    std::unique_ptr<BytecodeChunk> chunk = BytecodeUtil::Make<BytecodeChunk>();

    ASSERT(m_identifier != nullptr);

    // get current stack size
    const int stack_location = visitor->GetCompilationUnit()->GetInstructionStream().GetStackSize();
    // set identifier stack location
    m_identifier->SetStackLocation(stack_location);

    if (m_default_param != nullptr) {
        /*m_default_param->Build(visitor, mod);

        // get active register
        uint8_t rp = visitor->GetCompilationUnit()->GetInstructionStream().GetCurrentRegister();

        // add instruction to store on stack
        visitor->GetCompilationUnit()->GetInstructionStream() <<
            Instruction<uint8_t, uint8_t>(PUSH, rp);*/
    }


    // increment stack size
    visitor->GetCompilationUnit()->GetInstructionStream().IncStackSize();

    return std::move(chunk);
}

void AstParameter::Optimize(AstVisitor *visitor, Module *mod)
{
}

Pointer<AstStatement> AstParameter::Clone() const
{
    return CloneImpl();
}
