#ifndef ACE_SDK_HPP
#define ACE_SDK_HPP

#include <stdint.h>

#ifndef __cplusplus
#error Ace SDK requires a C++ compiler
#endif

#ifdef _WIN32
#define ACE_EXPORT extern "C" __declspec(dllexport)
#else
#define ACE_EXPORT extern "C"
#endif

#define ACE_PARAMS ace::sdk::Params

#define ACE_FUNCTION(name) ACE_EXPORT void name(ace::sdk::Params params)

#define ACE_CHECK_ARGS(cmp, amt) \
    do { \
        if (!(params.nargs cmp amt)) { \
            params.handler->state->ThrowException(params.handler->thread, \
                ace::vm::Exception::InvalidArgsException((#cmp != "==") ? (#cmp " " #amt) : (#amt), params.nargs)); \
            return; \
        } \
    } while (false)

#define ACE_RETURN(value) \
    do { \
        params.handler->thread->GetRegisters()[0] = value; \
        return; \
    } while (false)

namespace utf {
    // forward declaration
    class Utf8String;
} // namespace utf

namespace ace {
namespace sdk {

// forward declaration
struct Params;

}
}

namespace ace {
namespace vm {

// forward declarations
struct InstructionHandler;
struct ExecutionThread;
struct VMState;
struct Value;

}
}

// native typedefs
typedef void(*NativeFunctionPtr_t)(ace::sdk::Params);
typedef void(*NativeInitializerPtr_t)(ace::vm::VMState*, ace::vm::ExecutionThread *thread, ace::vm::Value*);

namespace ace {
namespace vm {

typedef uint32_t bc_address_t;
typedef uint8_t bc_reg_t;

class HeapValue;
    
struct Value {
    enum ValueType {
        /* These first four types are listed in order of precedence */
        I32,
        I64,
        F32,
        F64,

        BOOLEAN,

        CONST_STRING,

        HEAP_POINTER,
        FUNCTION,
        NATIVE_FUNCTION,
        ADDRESS,
        FUNCTION_CALL,
        TRY_CATCH_INFO
    } m_type;

    union ValueData {
        int32_t i32;
        int64_t i64;
        float f;
        double d;

        bool b;

        const char *c_str;

        HeapValue *ptr;

        struct {
            bc_address_t m_addr;
            uint8_t m_nargs;
            uint8_t m_flags;
        } func;

        NativeFunctionPtr_t native_func;
        
        struct {
            bc_address_t addr;
            int32_t varargs_push;
        } call;

        bc_address_t addr;

        struct {
            bc_address_t catch_address;
        } try_catch_info;
    } m_value;

    Value() = default;
    explicit Value(const Value &other);

    inline Value::ValueType GetType()  const { return m_type; }
    inline Value::ValueData GetValue() const { return m_value; }

    inline bool GetInteger(int64_t *out) const
    {
        switch (m_type) {
            case I32: *out = m_value.i32; return true;
            case I64: *out = m_value.i64; return true;
            default:                      return false;
        }
    }

    inline bool GetFloatingPoint(double *out) const
    {
        switch (m_type) {
            case F32: *out = m_value.f; return true;
            case F64: *out = m_value.d; return true;
            default:                    return false;
        }
    }

    inline bool GetNumber(double *out) const
    {
        switch (m_type) {
            case I32: *out = m_value.i32; return true;
            case I64: *out = m_value.i64; return true;
            case F32: *out = m_value.f;   return true;
            case F64: *out = m_value.d;   return true;
            default:                      return false;
        }
    }

    void Mark();

    const char *GetTypeString() const;
    utf::Utf8String ToString() const;
    void ToRepresentation(utf::Utf8String &out_str,
        bool add_type_name = true) const;
};

} // namespace vm

namespace sdk {

struct Params {
    vm::InstructionHandler *handler;
    vm::Value **args;
    int nargs;
};

} // namespace sdk
} // namespace ace

#endif
