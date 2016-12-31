#include <ace-c/ast/AstArrayAccess.hpp>
#include <ace-c/emit/Instruction.hpp>
#include <ace-c/AstVisitor.hpp>
#include <ace-c/Compiler.hpp>
#include <ace-c/Module.hpp>

#include <common/instructions.hpp>
#include <common/my_assert.hpp>

AstArrayAccess::AstArrayAccess(const std::shared_ptr<AstExpression> &target,
    const std::shared_ptr<AstExpression> &index,
    const SourceLocation &location)
    : AstExpression(location),
      m_target(target),
      m_index(index),
      m_access_mode(ACCESS_MODE_LOAD)
{
}

void AstArrayAccess::Visit(AstVisitor *visitor, Module *mod)
{
    m_target->Visit(visitor, mod);
    m_index->Visit(visitor, mod);

    // TODO: check if target is an array
}

void AstArrayAccess::Build(AstVisitor *visitor, Module *mod)
{
    ASSERT(m_target != nullptr);
    ASSERT(m_index != nullptr);

    bool target_side_effects = m_target->MayHaveSideEffects();
    bool index_side_effects = m_index->MayHaveSideEffects();
    
    uint8_t rp;
    uint8_t r0, r1;

    Compiler::ExprInfo info {
        m_target.get(), m_index.get()
    };

    if (!index_side_effects) {
        Compiler::LoadLeftThenRight(visitor, mod, info);
        rp = visitor->GetCompilationUnit()->GetInstructionStream().GetCurrentRegister();

        r0 = rp - 1;
        r1 = rp;
    } else if (index_side_effects && !target_side_effects) {
        // load the index and store it
        Compiler::LoadRightThenLeft(visitor, mod, info);
        rp = visitor->GetCompilationUnit()->GetInstructionStream().GetCurrentRegister();

        r0 = rp;
        r1 = rp - 1;
    } else {
        // load target, store it, then load the index
        Compiler::LoadLeftAndStore(visitor, mod, info);
        rp = visitor->GetCompilationUnit()->GetInstructionStream().GetCurrentRegister();

        r0 = rp - 1;
        r1 = rp;
    }

    // do the operation
    if (m_access_mode == ACCESS_MODE_LOAD) {
        visitor->GetCompilationUnit()->GetInstructionStream() <<
            Instruction<uint8_t, uint8_t, uint8_t, uint8_t>(LOAD_ARRAYIDX, r0, r0, r1);
    }

    // unclaim register
    visitor->GetCompilationUnit()->GetInstructionStream().DecRegisterUsage();
}

void AstArrayAccess::Optimize(AstVisitor *visitor, Module *mod)
{
    ASSERT(m_target != nullptr);
    ASSERT(m_index != nullptr);

    m_target->Optimize(visitor, mod);
    m_index->Optimize(visitor, mod);
}

void AstArrayAccess::Recreate(std::ostringstream &ss)
{
    ASSERT(m_target != nullptr);
    ASSERT(m_index != nullptr);

    m_target->Recreate(ss);
    ss << "[";
    m_index->Recreate(ss);
    ss << "]";
}

int AstArrayAccess::IsTrue() const
{
    return -1;
}

bool AstArrayAccess::MayHaveSideEffects() const
{
    return m_target->MayHaveSideEffects() || m_index->MayHaveSideEffects() ||
        m_access_mode == ACCESS_MODE_STORE;
}

SymbolTypePtr_t AstArrayAccess::GetSymbolType() const
{
    ASSERT(m_target != nullptr);

    SymbolTypePtr_t target_type = m_target->GetSymbolType();

    if (target_type->GetTypeClass() == TYPE_ARRAY) {
        SymbolTypePtr_t held_type = SymbolType::Builtin::UNDEFINED;
        if (target_type->GetGenericInstanceInfo().m_param_types.size() == 1) {
            held_type = target_type->GetGenericInstanceInfo().m_param_types[0];
            // todo: tuple?
        }
        return held_type;
    }

    return SymbolType::Builtin::ANY;
}