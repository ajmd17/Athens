#include <ace-c/ast/AstTypeExpression.hpp>
#include <ace-c/ast/AstNil.hpp>
#include <ace-c/ast/AstObject.hpp>
#include <ace-c/ast/AstArrayExpression.hpp>
#include <ace-c/AstVisitor.hpp>
#include <ace-c/Keywords.hpp>
#include <ace-c/Module.hpp>
#include <ace-c/Configuration.hpp>

#include <ace-c/type-system/BuiltinTypes.hpp>

#include <ace-c/emit/BytecodeChunk.hpp>
#include <ace-c/emit/BytecodeUtil.hpp>

#include <common/hasher.hpp>
#include <common/my_assert.hpp>

AstTypeExpression::AstTypeExpression(
    const std::string &name,
    const std::shared_ptr<AstTypeSpecification> &base_specification,
    const std::vector<std::shared_ptr<AstVariableDeclaration>> &members,
    const std::vector<std::shared_ptr<AstVariableDeclaration>> &static_members,
    const SourceLocation &location)
    : AstExpression(location, ACCESS_MODE_LOAD),
      m_name(name),
      m_base_specification(base_specification),
      m_members(members),
      m_static_members(static_members),
      m_num_members(0)
{
}

void AstTypeExpression::Visit(AstVisitor *visitor, Module *mod)
{   
    ASSERT(visitor != nullptr && mod != nullptr);

    // ===== INSTANCE DATA MEMBERS =====

    // open the scope for data members
    mod->m_scopes.Open(Scope(SCOPE_TYPE_TYPE_DEFINITION, 0));

    std::vector<SymbolMember_t> member_types;

    for (const auto &mem : m_members) {
        if (mem != nullptr) {
            mem->Visit(visitor, mod);

            ASSERT(mem->GetIdentifier() != nullptr);

            std::string mem_name = mem->GetName();
            SymbolTypePtr_t mem_type = mem->GetIdentifier()->GetSymbolType();
            
            member_types.push_back(std::make_tuple(
                mem_name,
                mem_type,
                mem->GetRealAssignment()
            ));
        }
    }

    // close the scope for data members
    mod->m_scopes.Close();

    SymbolTypePtr_t prototype_type = SymbolType::Object(
        m_name + "Instance", // Prototype type
        member_types,
        BuiltinTypes::OBJECT
    );

    // TODO: allow custom bases (which would have to extend Type somewhere)
    SymbolTypePtr_t base_type = BuiltinTypes::TYPE_TYPE;

    std::vector<SymbolMember_t> static_members;

    // check if one with the name $proto already exists.
    bool proto_found = false;
    bool base_found = false;

    for (const auto &mem : m_static_members) {
        ASSERT(mem != nullptr);

        if (mem->GetName() == "$proto") {
            proto_found = true;
        } else if (mem->GetName() == "base") {
            base_found = true;
        }

        if (proto_found && base_found) {
            break; // no need to keep searching
        }
    }

    if (!proto_found) { // no custom '$proto' member, add default.
        static_members.push_back(SymbolMember_t {
            "$proto",
            prototype_type,
            std::shared_ptr<AstTypeObject>(new AstTypeObject(
                prototype_type,
                nullptr,
                m_location
            ))
        });
    }

    if (!base_found) { // no custom 'base' member, add default
        static_members.push_back(SymbolMember_t {
            "base",
            base_type,
            std::shared_ptr<AstTypeObject>(new AstTypeObject(
                base_type,
                nullptr,
                m_location
            ))
        });
    }

    // ===== STATIC DATA MEMBERS ======

    // open the scope for static data members
    mod->m_scopes.Open(Scope(SCOPE_TYPE_TYPE_DEFINITION, 0));

    for (const auto &mem : m_static_members) {
        ASSERT(mem != nullptr);
        mem->Visit(visitor, mod);

        std::string mem_name = mem->GetName();

        ASSERT(mem->GetIdentifier() != nullptr);
        SymbolTypePtr_t mem_type = mem->GetIdentifier()->GetSymbolType();
        
        static_members.push_back(SymbolMember_t {
            mem_name,
            mem_type,
            mem->GetRealAssignment()
        });
    }

    // close the scope for static data members
    mod->m_scopes.Close();

    m_symbol_type = SymbolType::Extend(
        m_name,
        base_type,
        static_members
    );

    m_expr.reset(new AstTypeObject(
        m_symbol_type,
        nullptr, // prototype - TODO
        m_location
    ));

    m_expr->Visit(visitor, mod);
}

std::unique_ptr<Buildable> AstTypeExpression::Build(AstVisitor *visitor, Module *mod)
{
    ASSERT(m_expr != nullptr);
    auto buildable = m_expr->Build(visitor, mod);

    return std::move(buildable);
}

void AstTypeExpression::Optimize(AstVisitor *visitor, Module *mod)
{
    ASSERT(m_expr != nullptr);
    m_expr->Optimize(visitor, mod);
}

Pointer<AstStatement> AstTypeExpression::Clone() const
{
    return CloneImpl();
}

bool AstTypeExpression::IsLiteral() const
{
    return false;
}

Tribool AstTypeExpression::IsTrue() const
{
    return Tribool::True();
}

bool AstTypeExpression::MayHaveSideEffects() const
{
    return true;
}

SymbolTypePtr_t AstTypeExpression::GetExprType() const
{
    ASSERT(m_expr != nullptr);
    return m_expr->GetExprType();
}

const AstExpression *AstTypeExpression::GetValueOf() const
{
    ASSERT(m_expr != nullptr);
    return m_expr->GetValueOf();
}