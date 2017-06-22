#include <ace-c/ast/AstTryCatch.hpp>
#include <ace-c/AstVisitor.hpp>
#include <ace-c/Compiler.hpp>
#include <ace-c/emit/Instruction.hpp>
#include <ace-c/emit/StaticObject.hpp>
#include <ace-c/Keywords.hpp>
#include <ace-c/Configuration.hpp>

#include <common/instructions.hpp>
#include <common/my_assert.hpp>

AstTryCatch::AstTryCatch(const std::shared_ptr<AstBlock> &try_block,
    const std::shared_ptr<AstBlock> &catch_block,
    const SourceLocation &location)
    : AstStatement(location),
      m_try_block(try_block),
      m_catch_block(catch_block)
{
}

void AstTryCatch::Visit(AstVisitor *visitor, Module *mod)
{
    // accept the try block
    m_try_block->Visit(visitor, mod);
    // accept the catch block
    m_catch_block->Visit(visitor, mod);
}

std::unique_ptr<Buildable> AstTryCatch::Build(AstVisitor *visitor, Module *mod)
{
    std::unique_ptr<BytecodeChunk> chunk = BytecodeUtil::Make<BytecodeChunk>();

    uint8_t rp;

    // the label to jump to the very end
    LabelId end_label = chunk->NewLabel();

    // the label to jump to the catch-block
    LabelId catch_label = chunk->NewLabel();

    { // send the instruction to enter the try-block
        auto instr_begin_try = BytecodeUtil::Make<BuildableTryCatch>();
        instr_begin_try->catch_label_id = catch_label;
        chunk->Append(std::move(instr_begin_try));
    }

    // try block increases stack size to hold the data about the catch block
    visitor->GetCompilationUnit()->GetInstructionStream().IncStackSize();

    // build the try-block
    chunk->Append(m_try_block->Build(visitor, mod));

    { // send the instruction to end the try-block
        auto instr_end_try = BytecodeUtil::Make<RawOperation<>>();
        instr_end_try->opcode = END_TRY;
        chunk->Append(std::move(instr_end_try));
    }
    
    // decrease stack size for the try block
    visitor->GetCompilationUnit()->GetInstructionStream().DecStackSize();

    { // jump to the end, as to not execute the catch-block
        auto instr_jmp = BytecodeUtil::Make<Jump>();
        instr_jmp->opcode = JMP;
        instr_jmp->label_id = end_label;
        chunk->Append(std::move(instr_jmp));
    }

    // set the label's position to where the catch-block would be
    chunk->MarkLabel(catch_label);

    // exception was thrown, pop all local variables from the try-block
    chunk->Append(Compiler::PopStack(visitor, m_try_block->NumLocals()));

    // build the catch-block
    chunk->Append(m_catch_block->Build(visitor, mod));

    chunk->MarkLabel(end_label);

    return std::move(chunk);
}

void AstTryCatch::Optimize(AstVisitor *visitor, Module *mod)
{
    // optimize the try block
    m_try_block->Optimize(visitor, mod);
    // optimize the catch block
    m_catch_block->Optimize(visitor, mod);
}

void AstTryCatch::Recreate(std::ostringstream &ss)
{
    ASSERT(m_try_block != nullptr && m_catch_block != nullptr);
    
    ss << Keyword::ToString(Keyword_try);
    m_try_block->Recreate(ss);
    ss << Keyword::ToString(Keyword_catch);
    m_catch_block->Recreate(ss);
}

Pointer<AstStatement> AstTryCatch::Clone() const
{
    return CloneImpl();
}
