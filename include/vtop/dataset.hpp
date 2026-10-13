// simd-topk：SoA 连续存储的向量集
#pragma once

#include <cstddef>
#include <cstdint>

namespace vtop {

// 第 i 个向量位于 data() + i * dim()，整块 32 字节对齐（AVX2 友好）
class Dataset {
public:
    Dataset() = default;
    Dataset(size_t n, size_t dim);  // 只分配，不初始化
    ~Dataset();
    Dataset(Dataset&&) noexcept;
    Dataset& operator=(Dataset&&) noexcept;
    Dataset(const Dataset&) = delete;
    Dataset& operator=(const Dataset&) = delete;

    float* data() { return data_; }
    const float* data() const { return data_; }
    float* row(size_t i) { return data_ + i * dim_; }
    const float* row(size_t i) const { return data_ + i * dim_; }
    size_t num_vectors() const { return n_; }
    size_t dim() const { return dim_; }

private:
    float* data_ = nullptr;
    size_t n_ = 0;
    size_t dim_ = 0;
};

// 生成 [0,1) 均匀分布合成数据（splitmix64，跨平台可复现，同 seed 同数据）
Dataset make_dataset(size_t n, size_t dim, uint64_t seed);

}  // namespace vtop
