#pragma once
#include <memory>
#include <vector>
#include <queue>

namespace ermerchant {

template<typename T, size_t PoolSize = 1024>
class MemoryPool {
private:
    struct Block {
        std::array<T, PoolSize> data;
        std::queue<size_t> freeIndices;
    };
    std::vector<std::unique_ptr<Block>> blocks;

public:
    T* allocate() {
        for (auto& block : blocks) {
            if (!block->freeIndices.empty()) {
                size_t index = block->freeIndices.front();
                block->freeIndices.pop();
                return &block->data[index];
            }
        }

        auto newBlock = std::make_unique<Block>();
        for (size_t i = 0; i < PoolSize; i++) {
            newBlock->freeIndices.push(i);
        }

        blocks.push_back(std::move(newBlock));
        return allocate();
    }

    void deallocate(T* ptr) {
        for (auto& block : blocks) {
            if (ptr >= &block->data[0] && ptr < &block->data[PoolSize]) {
                size_t index = ptr - &block->data[0];
                block->freeIndices.push(index);
                return;
            }
        }
    }
};

} // namespace ermerchant
