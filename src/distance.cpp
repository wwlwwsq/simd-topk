#include "vtop/distance.hpp"

#include <stdexcept>

namespace vtop {

// 朴素基线：顺序累加。浮点加法不满足结合律，
// 它的结果与 blocked8/avx2 只能做相对误差比较（测试即如此设计）——面试可讲的点。
float l2_sq_scalar_naive(const float* a, const float* b, size_t dim) {
    float s = 0.f;
    for (size_t i = 0; i < dim; ++i) {
        const float d = a[i] - b[i];
        s += d * d;
    }
    return s;
}

// TODO(14) 8 路分块累加（与 AVX2 版规约顺序严格一致，才能逐位相等）：
//   float acc[8] = {};  // 全 0
//   主循环 i 步进 8：acc[j] += (a[i+j]-b[i+j])^2，j = 0..7
//   尾部（dim % 8 != 0 时）单独顺序累加——本项目 dim=128 用不到，但接口别埋雷
//   水平规约（顺序与 avx2 的 intrinsics 一一对应，见 distance_avx2.cpp）：
//     ((acc[0]+acc[4]) + (acc[2]+acc[6])) + ((acc[1]+acc[5]) + (acc[3]+acc[7]))
float l2_sq_scalar_blocked8(const float* a, const float* b, size_t dim) {
    (void)a;
    (void)b;
    (void)dim;
    throw std::logic_error("l2_sq_scalar_blocked8: not implemented");
}

}  // namespace vtop
