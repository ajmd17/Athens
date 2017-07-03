#include <ace-c/ast/AstObject.hpp>
#include <ace-c/AstVisitor.hpp>
#include <ace-c/Module.hpp>
#include <ace-c/Configuration.hpp>

#include <ace-c/emit/BytecodeChunk.hpp>
#include <ace-c/emit/BytecodeUtil.hpp>

#include <common/instructions.hpp>
#include <common/my_assert.hpp>
#include <common/utf8.hpp>

#include <iostream>

AstObject::AstObject(const SymbolTypeWeakPtr_t &symbol_type,
    const SourceLocation &location)
    : AstExpression(location, ACCESS_MODE_LOAD),
      m_symbol_type(symbol_type)
{
}

void AstObject::Visit(AstVisitor *visitor, Module *mod)
{
    auto sp = m_symbol_type.lock();
    ASSERT(sp != nullptr);
}

std::unique_ptr<Buildable> AstObject::Build(AstVisitor *visitor, Module *mod)
{
    std::unique_ptr<BytecodeChunk> chunk = BytecodeUtil::Make<BytecodeChunk>();

    auto sp = m_symbol_type.lock();
    ASSERT(sp != nullptr);

    int static_id = sp->GetId();
    ASSERT(static_id != -1);

    // get active register
    uint8_t rp = visitor->GetCompilationUnit()->GetInstructionStream().GetCurrentRegister();
    // store object's register location
    uint8_t obj_reg = rp;
    // store the original register location of the object
    const uint8_t original_obj_reg = obj_reg;

    /*// load the type into a register
    visitor->GetCompilationUnit()->GetInstructionStream() <<
        Instruction<uint8_t, uint8_t, uint16_t>(LOAD_STATIC, obj_reg, static_id);

    if (!ace::compiler::Config::use_static_objects) {
        // fill with padding for LOAD_TYPE instruction
        visitor->GetCompilationUnit()->GetInstructionStream().GetPosition() += sp->GetName().length();
        visitor->GetCompilationUnit()->GetInstructionStream().GetPosition() += sizeof(uint16_t);
        for (size_t i = 0; i < sp->GetMembers().size(); i++) {
            visitor->GetCompilationUnit()->GetInstructionStream().GetPosition() += sizeof(uint16_t);
            visitor->GetCompilationUnit()->GetInstructionStream().GetPosition() += std::get<0>(sp->GetMembers()[i]).length();
        }
    }*/

    {
        auto instr_type = BytecodeUtil::Make<BuildableType>();
        instr_type->reg = obj_reg;
        instr_type->name = sp->GetName();

        for (const SymbolMember_t &mem : sp->GetMembers()) {
            instr_type->members.push_back(std::get<0>(mem));
        }

        chunk->Append(std::move(instr_type));
    }

    { // store newly allocated object in same register
        auto instr_new = BytecodeUtil::Make<RawOperation<>>();
        instr_new->opcode = NEW;
        instr_new->Accept<uint8_t>(obj_reg); // dst
        instr_new->Accept<uint8_t>(obj_reg); // src (holds type)
        chunk->Append(std::move(instr_new));
    }

    { // store the type on the stack
        auto instr_push = BytecodeUtil::Make<RawOperation<>>();
        instr_push->opcode = PUSH;
        instr_push->Accept<uint8_t>(obj_reg); // src
        chunk->Append(std::move(instr_push));
    }

    int obj_stack_loc = visitor->GetCompilationUnit()->GetInstructionStream().GetStackSize();
    // increment stack size for the type
    visitor->GetCompilationUnit()->GetInstructionStream().IncStackSize();

    // for each data member, load the default value
    int i = 0;
    for (const auto &dm : sp->GetMembers()) {
        ASSERT(std::get<1>(dm) != nullptr);
        ASSERT(std::get<1>(dm)->GetDefaultValue() != nullptr);

        // if there has not been an assignment provided,
        // use the default value of the members's type.
        if (std::get<2>(dm) != nullptr) {
            chunk->Append(std::get<2>(dm)->Build(visitor, mod));
        } else {
            // load the data member's default value.
            chunk->Append(std::get<1>(dm)->GetDefaultValue()->Build(visitor, mod));
        }

        // claim register for the data member
        visitor->GetCompilationUnit()->GetInstructionStream().IncRegisterUsage();
        // get register position
        rp = visitor->GetCompilationUnit()->GetInstructionStream().GetCurrentRegister();
        // set obj_reg for future data members
        obj_reg = rp;

        int stack_size = visitor->GetCompilationUnit()->GetInstructionStream().GetStackSize();
        int diff = stack_size - obj_stack_loc;
        ASSERT(diff == 1);

        { // load type from stack
            auto instr_load_offset = BytecodeUtil::Make<RawOperation<>>();
            instr_load_offset->opcode = LOAD_OFFSET;
            instr_load_offset->Accept<uint8_t>(obj_reg);
            instr_load_offset->Accept<uint16_t>(diff);
            chunk->Append(std::move(instr_load_offset));
        }

        { // store data member
            auto instr_mov_mem = BytecodeUtil::Make<RawOperation<>>();
            instr_mov_mem->opcode = MOV_MEM;
            instr_mov_mem->Accept<uint8_t>(obj_reg);
            instr_mov_mem->Accept<uint8_t>(i);
            instr_mov_mem->Accept<uint8_t>(rp - 1);
            chunk->Append(std::move(instr_mov_mem));
        }

        // unclaim register
        visitor->GetCompilationUnit()->GetInstructionStream().DecRegisterUsage();
        // get register position
        rp = visitor->GetCompilationUnit()->GetInstructionStream().GetCurrentRegister();

        // swap registers to move the object back to original register
        if (obj_reg != rp) {
            auto instr_mov_reg = BytecodeUtil::Make<RawOperation<>>();
            instr_mov_reg->opcode = MOV_REG;
            instr_mov_reg->Accept<uint8_t>(rp);
            instr_mov_reg->Accept<uint8_t>(obj_reg);
            chunk->Append(std::move(instr_mov_reg));
            
            obj_reg = rp;
        }

        i++;
    }

    { // pop the type from stack
        auto instr_pop = BytecodeUtil::Make<RawOperation<>>();
        instr_pop->opcode = POP;
        chunk->Append(std::move(instr_pop));
    }

    // decrement stack size for type
    visitor->GetCompilationUnit()->GetInstructionStream().DecStackSize();

    // check if we have to move the register value back to the original location
    if (obj_reg != original_obj_reg) {
        // move the value in obj_reg into the original location
        auto instr_mov_reg = BytecodeUtil::Make<RawOperation<>>();
        instr_mov_reg->opcode = MOV_REG;
        instr_mov_reg->Accept<uint8_t>(original_obj_reg);
        instr_mov_reg->Accept<uint8_t>(obj_reg);
        chunk->Append(std::move(instr_mov_reg));
    }

    return std::move(chunk);
}

void AstObject::Optimize(AstVisitor *visitor, Module *mod)
{
}

void AstObject::Recreate(std::ostringstream &ss)
{
    ss << "??";
}

/*void AstObject::SubstituteGenerics(AstVisitor *visitor, Module *mod, 
    const SymbolTypePtr_t &instance)
{
    if (auto sp = m_symbol_type.lock()) {
        if (sp->GetTypeClass() == TYPE_GENERIC_PARAMETER) {
            // substitute generic parameter
            auto base = instance->GetBaseType();
            ASSERT(base != nullptr);
            ASSERT(base->GetTypeClass() == TYPE_GENERIC);
            ASSERT(base->GetGenericInfo().m_params.size() == 
                sp->GetGenericInstanceInfo().m_param_types.size());

            size_t index = 0;
            bool type_found = false;

            for (auto &base_param : base->GetGenericInfo().m_params) {
                ASSERT(base_param != nullptr);

                if (base_param->GetName() == sp->GetName()) {
                    // substitute in supplied type
                    m_symbol_type = sp->GetGenericInstanceInfo().m_param_types[index];
                    type_found = true;
                    break;
                }

                index++;
            }

            ASSERT(type_found);
        }
    }
}*/

Pointer<AstStatement> AstObject::Clone() const
{
    return CloneImpl();
}

int AstObject::IsTrue() const
{
    return 1;
}

bool AstObject::MayHaveSideEffects() const
{
    return false;
}

SymbolTypePtr_t AstObject::GetSymbolType() const
{
    auto sp = m_symbol_type.lock();
    ASSERT(sp != nullptr);
    return sp;
}
