#include <ace-c/emit/cppgen/CppGenerator.hpp>

#include <common/instructions.hpp>
#include <common/my_assert.hpp>

std::ostream &operator<<(std::ostream &os, CppGenerator &cpp_gen)
{
    os << "/* Generated by ace2cpp */\n";
    os << "#include <ace-vm/VM.hpp>\n\n";
    os << "using namespace ace;\n";
    os << "using namespace ace::vm;\n";
    os << "static VM vm;\n";
    os << "int main(int argc, char *argv[]) {\n";
    os << cpp_gen.m_cpp_ss.rdbuf();
    os << "}";
    return os;
}

CppGenerator::CppGenerator()
{
}

CppGenerator &CppGenerator::operator<<(InstructionStream &is)
{
    std::ostringstream ss;
    ss << is;
    
    std::string str = ss.str();

    ace::vm::BytecodeStream bs(str.data(), str.size());
    return operator<<(bs);
}

CppGenerator &CppGenerator::operator<<(ace::vm::BytecodeStream &bs)
{
    while (!bs.Eof()) {
        uint8_t code;
        bs.Read(&code);

        switch (code) {
            case STORE_STATIC_STRING: {
                uint32_t len;
                bs.Read(&len);

                char *str = new char[len + 1];
                str[len] = '\0';
                bs.Read(str, len);

                AddInstructionCall("StoreStaticString", len, str);

                delete[] str;

                break;
            }
            case STORE_STATIC_ADDRESS: {
                uint32_t val;
                bs.Read(&val);

                AddInstructionCall("StoreStaticAddress", val);

                break;
            }
            case STORE_STATIC_FUNCTION: {
                uint32_t addr;
                bs.Read(&addr);

                uint8_t nargs;
                bs.Read(&nargs);

                uint8_t flags;
                bs.Read(&flags);

                AddInstructionCall("StoreStaticFunction", addr, nargs, flags);

                break;
            }
            case STORE_STATIC_TYPE: {
                uint16_t type_name_len;
                bs.Read(&type_name_len);

                std::vector<uint8_t> type_name;
                type_name.resize(type_name_len + 1);
                type_name[type_name_len] = '\0';
                bs.Read(&type_name[0], type_name_len);

                uint16_t size;
                bs.Read(&size);

                std::vector<std::vector<uint8_t>> names;
                names.resize(size);
                
                for (int i = 0; i < size; i++) {
                    uint16_t len;
                    bs.Read(&len);

                    names[i].resize(len + 1);
                    names[i][len] = '\0';
                    bs.Read(&names[i][0], len);
                }

                

                break;
            }
            case LOAD_I32: {
                uint8_t reg;
                bs.Read(&reg);

                int32_t val;
                bs.Read(&val);

                break;
            }
            case LOAD_I64: {
                uint8_t reg;
                bs.Read(&reg);

                int64_t val;
                bs.Read(&val);

                break;
            }
            case LOAD_F32: {
                uint8_t reg;
                bs.Read(&reg);

                float val;
                bs.Read(&val);
                
                break;
            }
            case LOAD_F64: {
                uint8_t reg;
                bs.Read(&reg);

                double val;
                bs.Read(&val);
                
                break;
            }
            case LOAD_OFFSET: {
                uint8_t reg;
                bs.Read(&reg);

                uint16_t offset;
                bs.Read(&offset);
                
                break;
            }
            case LOAD_INDEX: {
                uint8_t reg;
                bs.Read(&reg);

                uint16_t idx;
                bs.Read(&idx);
                
                break;
            }
            case LOAD_STATIC: {
                uint8_t reg;
                bs.Read(&reg);

                uint16_t index;
                bs.Read(&index);

                break;
            }
            case LOAD_STRING: {
                uint8_t reg;
                bs.Read(&reg);

                // get string length
                uint32_t len;
                bs.Read(&len);

                // read string based on length
                char *str = new char[len + 1];
                bs.Read(str, len);
                str[len] = '\0';




                delete[] str;

                break;
            }
            case LOAD_ADDR: {
                uint8_t reg;
                bs.Read(&reg);

                uint32_t val;
                bs.Read(&val);

                m_cpp_ss << "addr_" << val << ":\n";

                break;
            }
            case LOAD_FUNC: {
                uint8_t reg;
                bs.Read(&reg);

                uint32_t addr;
                bs.Read(&addr);

                uint8_t nargs;
                bs.Read(&nargs);

                uint8_t flags;
                bs.Read(&flags);

                break;
            }
            case LOAD_TYPE: {
                uint8_t reg;
                bs.Read(&reg);

                uint16_t type_name_len;
                bs.Read(&type_name_len);

                std::vector<uint8_t> type_name;
                type_name.resize(type_name_len + 1);
                type_name[type_name_len] = '\0';
                bs.Read(&type_name[0], type_name_len);

                uint16_t size;
                bs.Read(&size);

                std::vector<std::vector<uint8_t>> names;
                names.resize(size);
                
                for (int i = 0; i < size; i++) {
                    uint16_t len;
                    bs.Read(&len);

                    names[i].resize(len + 1);
                    names[i][len] = '\0';
                    bs.Read(&names[i][0], len);
                }

                break;
            }
            case LOAD_MEM: {
                uint8_t reg;
                bs.Read(&reg);

                uint8_t src;
                bs.Read(&src);

                uint8_t idx;
                bs.Read(&idx);

                break;
            }
            case LOAD_MEM_HASH: {
                uint8_t reg;
                bs.Read(&reg);

                uint8_t src;
                bs.Read(&src);

                uint32_t hash;
                bs.Read(&hash);

                break;
            }
            case LOAD_ARRAYIDX: {
                uint8_t reg;
                bs.Read(&reg);

                uint8_t src;
                bs.Read(&src);

                uint8_t idx;
                bs.Read(&idx);

                break;
            }
            case LOAD_NULL: {
                uint8_t reg;
                bs.Read(&reg);

                break;
            }
            case LOAD_TRUE: {
                uint8_t reg;
                bs.Read(&reg);

                break;
            }
            case LOAD_FALSE: {
                uint8_t reg;
                bs.Read(&reg);

                break;
            }
            case MOV_OFFSET: {
                uint16_t dst;
                bs.Read(&dst);

                uint8_t src;
                bs.Read(&src);

                break;
            }
            case MOV_INDEX: {
                uint16_t dst;
                bs.Read(&dst);

                uint8_t src;
                bs.Read(&src);

                break;
            }
            case MOV_MEM: {
                uint8_t reg;
                bs.Read(&reg);

                uint8_t idx;
                bs.Read(&idx);

                uint8_t src;
                bs.Read(&src);

                break;
            }
            case MOV_MEM_HASH: {
                uint8_t reg;
                bs.Read(&reg);

                uint32_t hash;
                bs.Read(&hash);

                uint8_t src;
                bs.Read(&src);

                break;
            }
            case MOV_ARRAYIDX: {
                uint8_t reg;
                bs.Read(&reg);

                uint32_t idx;
                bs.Read(&idx);

                uint8_t src;
                bs.Read(&src);

                break;
            }
            case MOV_REG: {
                uint8_t dst;
                bs.Read(&dst);

                uint8_t src;
                bs.Read(&src);

                break;
            }
            case HAS_MEM_HASH: {
                uint8_t reg;
                bs.Read(&reg);

                uint8_t src;
                bs.Read(&src);

                uint32_t hash;
                bs.Read(&hash);

                break;
            }
            case PUSH: {
                uint8_t src;
                bs.Read(&src);

                break;
            }
            case POP: {

                break;
            }
            case POP_N: {
                uint8_t n;
                bs.Read(&n);

                break;
            }
            case PUSH_ARRAY: {
                uint8_t dst;
                bs.Read(&dst);

                uint8_t src;
                bs.Read(&src);

                break;
            }
            case ECHO: {
                uint8_t reg;
                bs.Read(&reg);

                break;
            }
            case ECHO_NEWLINE: {

                break;
            }
            case JMP: {
                uint8_t addr;
                bs.Read(&addr);

                break;
            }
            case JE: {
                uint8_t addr;
                bs.Read(&addr);

                break;
            }
            case JNE: {
                uint8_t addr;
                bs.Read(&addr);

                break;
            }
            case JG: {
                uint8_t addr;
                bs.Read(&addr);

                break;
            }
            case JGE: {
                uint8_t addr;
                bs.Read(&addr);

                break;
            }
            case CALL: {
                uint8_t func;
                bs.Read(&func);

                uint8_t argc;
                bs.Read(&argc);

                break;
            }
            case RET: {

                break;
            }
            case BEGIN_TRY: {
                uint8_t reg;
                bs.Read(&reg);

                break;
            }
            case END_TRY: {
                
                break;
            }
            case NEW: {
                uint8_t dst;
                bs.Read(&dst);

                uint8_t type;
                bs.Read(&type);

                break;
            }
            case NEW_ARRAY: {
                uint8_t dst;
                bs.Read(&dst);

                uint32_t size;
                bs.Read(&size);

                break;
            }
            case CMP: {
                uint8_t lhs;
                bs.Read(&lhs);

                uint8_t rhs;
                bs.Read(&rhs);

                break;
            }
            case CMPZ: {
                uint8_t lhs;
                bs.Read(&lhs);

                break;
            }
            case ADD: {
                uint8_t lhs;
                bs.Read(&lhs);

                uint8_t rhs;
                bs.Read(&rhs);

                uint8_t dst;
                bs.Read(&dst);

                break;
            }
            case SUB: {
                uint8_t lhs;
                bs.Read(&lhs);

                uint8_t rhs;
                bs.Read(&rhs);

                uint8_t dst;
                bs.Read(&dst);

                break;
            }
            case MUL: {
                uint8_t lhs;
                bs.Read(&lhs);

                uint8_t rhs;
                bs.Read(&rhs);

                uint8_t dst;
                bs.Read(&dst);

                break;
            }
            case DIV: {
                uint8_t lhs;
                bs.Read(&lhs);

                uint8_t rhs;
                bs.Read(&rhs);

                uint8_t dst;
                bs.Read(&dst);

                break;
            }
            case MOD: {
                uint8_t lhs;
                bs.Read(&lhs);

                uint8_t rhs;
                bs.Read(&rhs);

                uint8_t dst;
                bs.Read(&dst);

                break;
            }
            case NEG: {
                uint8_t reg;
                bs.Read(&reg);

                break;
            }
            case EXIT: {

                break;
            }
            default:
                // unrecognized instruction
                break;
        }
    }

    return *this;
}