#include "vtop/dataset.hpp"

#include <cstdlib>
#include <new>

#if defined(_WIN32)
#include <malloc.h>
#endif

namespace vtop {

namespace {

float* alloc_aligned(size_t count) {
    void* p = nullptr;
#if defined(_WIN32)
    p = _aligned_malloc(count * sizeof(float), 32);
#else
    const size_t bytes = (count * sizeof(float) + 31) & ~size_t(31);
    p = std::aligned_alloc(32, bytes);
#endif
    if (!p) throw std::bad_alloc();
    return static_cast<float*>(p);
}

void free_aligned(float* p) {
#if defined(_WIN32)
    _aligned_free(p);
#else
    std::free(p);
#endif
}

// splitmix64：种子确定性、跨平台一致
uint64_t splitmix64(uint64_t& x) {
    x += 0x9E3779B97F4A7C15ull;
    uint64_t z = x;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
}

}  // namespace

Dataset::Dataset(size_t n, size_t dim) : data_(alloc_aligned(n * dim)), n_(n), dim_(dim) {}

Dataset::~Dataset() { free_aligned(data_); }

Dataset::Dataset(Dataset&& o) noexcept : data_(o.data_), n_(o.n_), dim_(o.dim_) {
    o.data_ = nullptr;
    o.n_ = 0;
    o.dim_ = 0;
}

Dataset& Dataset::operator=(Dataset&& o) noexcept {
    if (this != &o) {
        free_aligned(data_);
        data_ = o.data_;
        n_ = o.n_;
        dim_ = o.dim_;
        o.data_ = nullptr;
        o.n_ = 0;
        o.dim_ = 0;
    }
    return *this;
}

Dataset make_dataset(size_t n, size_t dim, uint64_t seed) {
    Dataset ds(n, dim);
    uint64_t s = seed;
    float* p = ds.data();
    for (size_t i = 0; i < n * dim; ++i)
        p[i] = static_cast<float>(splitmix64(s) >> 40) * (1.0f / 16777216.0f);  // 24bit / 2^24 -> [0,1)
    return ds;
}

}  // namespace vtop
