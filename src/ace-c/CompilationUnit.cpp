#include <ace-c/CompilationUnit.hpp>
#include <ace-c/Configuration.hpp>
#include <ace-c/emit/StaticObject.hpp>
#include <ace-c/emit/NamesPair.hpp>
#include <ace-c/Configuration.hpp>

#include <ace-c/type-system/BuiltinTypes.hpp>

#include <common/my_assert.hpp>

#include <iostream>

CompilationUnit::CompilationUnit()
    : m_module_index(0),
      m_global_module(new Module(
          ace::compiler::Config::global_module_name,
          SourceLocation::eof
      ))
{
    m_global_module->SetImportTreeLink(m_module_tree.TopNode());

    Scope &top = m_global_module->m_scopes.Top();
    top.GetIdentifierTable().AddSymbolType(BuiltinTypes::ANY);
    top.GetIdentifierTable().AddSymbolType(BuiltinTypes::INT);
    top.GetIdentifierTable().AddSymbolType(BuiltinTypes::FLOAT);
    top.GetIdentifierTable().AddSymbolType(BuiltinTypes::NUMBER);
    top.GetIdentifierTable().AddSymbolType(BuiltinTypes::BOOLEAN);
    top.GetIdentifierTable().AddSymbolType(BuiltinTypes::STRING);
    top.GetIdentifierTable().AddSymbolType(BuiltinTypes::FUNCTION);
    top.GetIdentifierTable().AddSymbolType(BuiltinTypes::ARRAY);
    top.GetIdentifierTable().AddSymbolType(BuiltinTypes::TUPLE);
    top.GetIdentifierTable().AddSymbolType(BuiltinTypes::MAYBE);
    top.GetIdentifierTable().AddSymbolType(BuiltinTypes::CONST_TYPE);
    top.GetIdentifierTable().AddSymbolType(BuiltinTypes::NULL_TYPE);
    top.GetIdentifierTable().AddSymbolType(BuiltinTypes::EVENT);
    top.GetIdentifierTable().AddSymbolType(BuiltinTypes::EVENT_ARRAY);
    top.GetIdentifierTable().AddSymbolType(BuiltinTypes::BLOCK_TYPE);
    top.GetIdentifierTable().AddSymbolType(BuiltinTypes::CLOSURE_TYPE);

    m_module_tree.TopNode()->m_value = m_global_module.get();
}

CompilationUnit::~CompilationUnit()
{
}

void CompilationUnit::RegisterType(SymbolTypePtr_t &type_ptr)
{
    std::vector<NamesPair_t> names;

    for (auto &mem : type_ptr->GetMembers()) {
        std::string mem_name = std::get<0>(mem);

        names.push_back({
            mem_name.size(),
            std::vector<uint8_t>(mem_name.begin(), mem_name.end())
        });
    }

    // mangle the type name
    const size_t len = type_ptr->GetName().length();

    ASSERT(type_ptr->GetMembers().size() < ace::compiler::Config::max_data_members);

    // create static object
    StaticTypeInfo st;
    st.m_size = type_ptr->GetMembers().size();
    st.m_names = names;
    st.m_name = new char[len + 1];
    st.m_name[len] = '\0';
    std::strcpy(st.m_name, type_ptr->GetName().c_str());

    int id;

    StaticObject so(st);
    int found_id = m_instruction_stream.FindStaticObject(so);
    if (found_id == -1) {
        so.m_id = m_instruction_stream.NewStaticId();
        m_instruction_stream.AddStaticObject(so);
        id = so.m_id;
    } else {
        id = found_id;
    }

    delete[] st.m_name;

    type_ptr->SetId(id);
}

Module *CompilationUnit::LookupModule(const std::string &name)
{
    TreeNode<Module*> *top = m_module_tree.TopNode();

    while (top != nullptr) {
        if (top->m_value != nullptr && top->m_value->GetName() == name) {
            return top->m_value;
        }

        // look up module names in the top module's siblings
        for (auto &sibling : top->m_siblings) {
            if (sibling != nullptr && sibling->m_value != nullptr) {
                if (sibling->m_value->GetName() == name) {
                    return sibling->m_value;
                }
            }
        }

        top = top->m_parent;
    }

    return nullptr;
}