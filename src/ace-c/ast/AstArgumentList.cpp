#include <ace-c/ast/AstArgumentList.hpp>
#include <ace-c/AstVisitor.hpp>
#include <ace-c/emit/Instruction.hpp>

#include <common/instructions.hpp>
#include <common/my_assert.hpp>

AstArgumentList::AstArgumentList(
    const std::vector<std::shared_ptr<AstArgument>> &args,
    const SourceLocation &location)
    : AstExpression(location, ACCESS_MODE_LOAD),
      m_args(args)
{
}

void AstArgumentList::Visit(AstVisitor *visitor, Module *mod)
{
    ASSERT(visitor != nullptr);
    ASSERT(mod != nullptr);

    for (const std::shared_ptr<AstArgument> &arg : m_args) {
        ASSERT(arg != nullptr);
        arg->Visit(visitor, mod);
    }
}

void AstArgumentList::Build(AstVisitor *visitor, Module *mod)
{
    ASSERT(visitor != nullptr);
    ASSERT(mod != nullptr);

    for (const std::shared_ptr<AstArgument> &arg : m_args) {
        ASSERT(arg != nullptr);
        arg->Build(visitor, mod);
    }
}

void AstArgumentList::Optimize(AstVisitor *visitor, Module *mod)
{
    ASSERT(visitor != nullptr);
    ASSERT(mod != nullptr);

    for (const std::shared_ptr<AstArgument> &arg : m_args) {
        ASSERT(arg != nullptr);
        arg->Optimize(visitor, mod);
    }
}

void AstArgumentList::Recreate(std::ostringstream &ss)
{
    for (const std::shared_ptr<AstArgument> &arg : m_args) {
        ASSERT(arg != nullptr);
        arg->Recreate(ss);
    }
}

Pointer<AstStatement> AstArgumentList::Clone() const
{
    return CloneImpl();
}

int AstArgumentList::IsTrue() const
{
    return 1;
}

bool AstArgumentList::MayHaveSideEffects() const
{
    return false;
}

SymbolTypePtr_t AstArgumentList::GetSymbolType() const
{
    return SymbolType::Builtin::ANY;
}