// simd-topk：平方欧氏距离，三种实现
// 一致性契约（对应 tests/test_consistency.cpp）：
//   naive    vs blocked8 / avx2 : 浮加顺序不同，只做相对误差比较（1e-5）
//   blocked8 vs avx2            : 必须【逐位相等】——两者用相同的 8 路累加与规约顺序
#pragma once

#include <cstddef>

namespace vtop {

enum class DistMethod {
    kScalarNaive,
    kScalarBlocked8,
    kAvx2,
};

float l2_sq_scalar_naive(const float* a, const float* b, size_t dim);
float l2_sq_scalar_blocked8(const float* a, const float* b, size_t dim);
float l2_sq_avx2(const float* a, const float* b, size_t dim);  // 要求 dim % 8 == 0

}  // namespace vtop
