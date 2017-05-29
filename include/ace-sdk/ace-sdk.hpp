#ifndef ACE_SDK_HPP
#define ACE_SDK_HPP

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

namespace ace {
namespace vm {
// forward declarations
struct InstructionHandler;
struct Value;
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
