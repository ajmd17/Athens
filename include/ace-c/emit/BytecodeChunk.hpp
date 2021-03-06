#ifndef BYTECODE_CHUNK_HPP
#define BYTECODE_CHUNK_HPP

#include <ace-c/emit/Instruction.hpp>

#include <common/my_assert.hpp>

#include <vector>
#include <memory>

struct BytecodeChunk final : public Buildable {
    std::vector<LabelInfo> labels;

    BytecodeChunk();
    BytecodeChunk(const BytecodeChunk &other) = delete;
    virtual ~BytecodeChunk() = default;

    inline void Append(std::unique_ptr<Buildable> buildable)
    {
        if (buildable != nullptr) {
            buildables.push_back(std::move(buildable));
        }
    }

    inline LabelId NewLabel()
    {
        LabelId index = labels.size();
        labels.emplace_back();
        return index;
    }

    /*inline void MarkLabel(LabelId label_id)
    {
        ASSERT(label_id < labels.size());
        labels[label_id].position = chunk_size;
    }*/
    
    std::vector<std::unique_ptr<Buildable>> buildables;
};

#endif