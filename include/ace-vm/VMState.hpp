#ifndef VM_STATE_HPP
#define VM_STATE_HPP

#include <ace-vm/StackMemory.hpp>
#include <ace-vm/StaticMemory.hpp>
#include <ace-vm/HeapMemory.hpp>
#include <ace-vm/Exception.hpp>
#include <ace-vm/BytecodeStream.hpp>

#define GC_THRESHOLD_MUL 2
#define GC_THRESHOLD_MIN 20
#define GC_THRESHOLD_MAX 1000

#define VM_MAX_THREADS 8
#define VM_NUM_REGISTERS 8

namespace ace {
namespace vm {

// forward declaration
class VM;

struct Registers {
    Value m_reg[VM_NUM_REGISTERS];
    int m_flags = 0;

    inline Value &operator[](uint8_t index) { return m_reg[index]; }
    inline void ResetFlags() { m_flags = 0; }
};

struct ExceptionState {
    // incremented each time BEGIN_TRY is encountered,
    // decremented each time END_TRY is encountered
    int m_try_counter = 0;

    // set to true when an exception occurs,
    // set to false when handled in BEGIN_TRY
    bool m_exception_occured = false;

    inline bool HasExceptionOccurred() const { return m_exception_occured; }
    inline void Reset() { m_try_counter = 0; m_exception_occured = false; }
};

struct ExecutionThread {
    friend struct VMState;

    Stack m_stack;
    ExceptionState m_exception_state;
    Registers m_regs;

    int m_func_depth = 0;

    inline Stack &GetStack() { return m_stack; }
    inline ExceptionState &GetExceptionState() { return m_exception_state; }
    inline Registers &GetRegisters() { return m_regs; }
    inline int GetId() const { return m_id; }

private:
    int m_id;
};

struct VMState {
    VMState();
    VMState(const VMState &other) = delete;
    ~VMState();

    ExecutionThread *m_threads[VM_MAX_THREADS];
    Heap m_heap;
    StaticMemory m_static_memory;
    non_owning_ptr<BytecodeStream> m_bs;
    non_owning_ptr<VM> m_vm;

    bool good = true;
    int m_max_heap_objects = GC_THRESHOLD_MIN;

    /** Reset the state of the VM, destroying all heap objects,
        stack objects and exception flags, etc.
     */
    void Reset();

    void ThrowException(ExecutionThread *thread, const Exception &exception);
    HeapValue *HeapAlloc(ExecutionThread *thread);
    void GC();

    /** Add a thread */
    ExecutionThread *CreateThread();
    /** Destroy thread with ID */
    void DestroyThread(int id);
    /** Returns true if there is enough space to create a new thread */
    inline bool CanCreateThread() const { return m_num_threads < VM_MAX_THREADS; }
    /** Get the number of threads currently in use */
    inline int GetNumThreads() const { return m_num_threads; }

    inline Heap &GetHeap() { return m_heap; }
    inline StaticMemory &GetStaticMemory() { return m_static_memory; }
    inline non_owning_ptr<BytecodeStream> &GetBytecodeStream() { return m_bs; }
    inline const non_owning_ptr<BytecodeStream> &GetBytecodeStream() const { return m_bs; }
    inline void SetBytecodeStream(const non_owning_ptr<BytecodeStream> &bs) { m_bs = bs; }

private:
    int m_num_threads = 0;
};

} // namespace vm
} // namespace ace

#endif
