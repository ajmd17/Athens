#include <ace-c/IdentifierTable.hpp>
#include <ace-c/Configuration.hpp>

#include <common/my_assert.hpp>

#include <unordered_set>
#include <algorithm>

IdentifierTable::IdentifierTable()
    : m_identifier_index(0)
{
}

IdentifierTable::IdentifierTable(const IdentifierTable &other)
    : m_identifier_index(other.m_identifier_index),
      m_identifiers(other.m_identifiers)
{
}

int IdentifierTable::CountUsedVariables() const
{
    std::unordered_set<int> used_variables;
    
    for (auto &ident : m_identifiers) {
        if (!ace::compiler::Config::cull_unused_objects || ident->GetUseCount() > 0) {
            if (used_variables.find(ident->GetIndex()) == used_variables.end()) {
                used_variables.insert(ident->GetIndex());
            }
        }
    }
    
    return used_variables.size();
}

Identifier *IdentifierTable::AddAlias(const std::string &name, Identifier *aliasee)
{
    ASSERT(aliasee != nullptr);
    
    m_identifiers.push_back(std::shared_ptr<Identifier>(new Identifier(name,
        aliasee->GetIndex(), aliasee->GetFlags() | FLAG_ALIAS)));
    
    return m_identifiers.back().get();
}

Identifier *IdentifierTable::AddIdentifier(const std::string &name, int flags)
{
    m_identifiers.push_back(std::shared_ptr<Identifier>(new Identifier(name,
        m_identifier_index++, flags)));
    
    return m_identifiers.back().get();
}

Identifier *IdentifierTable::LookUpIdentifier(const std::string &name)
{
    for (auto &ident : m_identifiers) {
        if (ident->GetName() == name) {
            return ident.get();
        }
    }
    return nullptr;
}

SymbolTypePtr_t IdentifierTable::LookupSymbolType(const std::string &name) const
{
    for (auto &type : m_symbol_types) {
        if (type != nullptr && type->GetName() == name) {
            return type;
        }
    }
    return nullptr;
}

SymbolTypePtr_t IdentifierTable::LookupGenericInstance(const SymbolTypePtr_t &base,
    const std::vector<SymbolTypePtr_t> &params) const
{
    ASSERT(base != nullptr);
    ASSERT(base->GetTypeClass() == TYPE_GENERIC);

    for (auto &type : m_symbol_types) {
        if (type) {
            if (type->GetTypeClass() == TYPE_GENERIC_INSTANCE && 
                type->GetBaseType() == base) {

                // check params
                auto &other_params = type->GetGenericInstanceInfo().m_param_types;

                if (other_params.size() != params.size()) {
                    continue;
                }

                bool found = true;

                for (size_t i = 0; i < params.size(); i++) {
                    ASSERT(params[i] != nullptr);
                    ASSERT(type->GetGenericInstanceInfo().m_param_types[i] != nullptr);

                    if (!params[i]->TypeEqual(*type->GetGenericInstanceInfo().m_param_types[i])) {
                        found = false;
                        break;
                    }
                }

                if (found) {
                    return type;
                }
            }
        }
    }

    return nullptr;
}

void IdentifierTable::AddSymbolType(const SymbolTypePtr_t &type)
{
    m_symbol_types.push_back(type);
}