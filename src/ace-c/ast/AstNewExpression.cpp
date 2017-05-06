#include <ace-c/ast/AstNewExpression.hpp>
#include <ace-c/AstVisitor.hpp>
#include <ace-c/Module.hpp>

#include <common/my_assert.hpp>
#include <common/utf8.hpp>

AstNewExpression::AstNewExpression(
    const std::shared_ptr<AstTypeSpecification> &type_expr,
    const std::vector<std::shared_ptr<AstArgument>> &args,
    const SourceLocation &location)
    : AstExpression(location, ACCESS_MODE_LOAD),
      m_type_expr(type_expr),
      m_args(args)
{
}

void AstNewExpression::Visit(AstVisitor *visitor, Module *mod)
{
    ASSERT(visitor != nullptr);
    ASSERT(mod != nullptr);

    ASSERT(m_type_expr != nullptr);
    m_type_expr->Visit(visitor, mod);

    for (auto &arg : m_args) {
      arg->Visit(visitor, mod);
    }

    SymbolTypePtr_t object_type = m_type_expr->GetSymbolType();
    ASSERT(object_type != nullptr);
    // get default value
    auto object_value = object_type->GetDefaultValue();
    ASSERT(object_value != nullptr);

    m_object_value = object_value;
}

void AstNewExpression::Build(AstVisitor *visitor, Module *mod)
{
    ASSERT(m_type_expr != nullptr);
    m_type_expr->Build(visitor, mod);

    for (size_t i = 0; i < m_args.size(); i++) {
        ASSERT(m_args[i] != nullptr);
        m_args[i]->Build(visitor, mod);
    }

    // build in the value
    ASSERT(m_object_value != nullptr);
    m_object_value->Build(visitor, mod);
}

void AstNewExpression::Optimize(AstVisitor *visitor, Module *mod)
{
    ASSERT(m_type_expr != nullptr);
    m_type_expr->Optimize(visitor, mod);

    for (size_t i = 0; i < m_args.size(); i++) {
        ASSERT(m_args[i] != nullptr);
        m_args[i]->Optimize(visitor, mod);
    }

    // build in the value
    ASSERT(m_object_value != nullptr);
    m_object_value->Optimize(visitor, mod);
}

void AstNewExpression::Recreate(std::ostringstream &ss)
{
    ASSERT(m_type_expr != nullptr);

    ss << "new ";
    m_type_expr->Recreate(ss);
    ss << "(";
    for (size_t i = 0; i < m_args.size(); i++) {
        m_args[i]->Recreate(ss);
        if (i != m_args.size() - 1) {
            ss << ", ";
        }
    }
    ss << ")";
}

Pointer<AstStatement> AstNewExpression::Clone() const
{
    return CloneImpl();
}

int AstNewExpression::IsTrue() const
{
    ASSERT(m_object_value != nullptr);
    return m_object_value->IsTrue();
}

bool AstNewExpression::MayHaveSideEffects() const
{
    return true;
}

SymbolTypePtr_t AstNewExpression::GetSymbolType() const
{
    ASSERT(m_object_value != nullptr);
    return m_object_value->GetSymbolType();
}
