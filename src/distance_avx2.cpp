// 本翻译单元由 CMake 以 -mavx2 -mfma 编译；不要把其他函数放进这个文件。
#include "vtop/distance.hpp"

#if !defined(__AVX2__)
#error "distance_avx2.cpp 必须以 -mavx2 编译（见 CMakeLists 的 set_source_files_properties）"
#endif

#include <immintrin.h>
#include <stdexcept>

namespace vtop {

// TODO(15) AVX2 版，与 blocked8 逐位一致：
//   __m256 acc = _mm256_setzero_ps();
//   主循环 i 步进 8：
//     va = _mm256_loadu_ps(a + i)（数据已 32B 对齐，可用 _mm256_load_ps，想想差别）
//     d  = _mm256_sub_ps(va, vb)
//     注意：不要用 _mm256_fmadd_ps —— FMA 一次乘加会改变数值，与标量版不再逐位一致！
//     acc = _mm256_add_ps(acc, _mm256_mul_ps(d, d))
//   水平规约（与 blocked8 注释里的公式一一对应）：
//     __m128 lo = _mm256_castps256_ps128(acc);
//     __m128 hi = _mm256_extractf128_ps(acc, 1);
//     __m128 s  = _mm_add_ps(lo, hi);                  // lane0 = acc0+acc4 ...
//     s  = _mm_add_ps(s, _mm_movehl_ps(s, s));         // lane0 = (0+4)+(2+6)
//     s  = _mm_add_ss(s, _mm_shuffle_ps(s, s, _MM_SHUFFLE(1,1,1,1)));
//     return _mm_cvtss_f32(s);
//   进阶（做完一致性后再加）：新增 kAvx2Fma 方案对比 FMA 的精度差异与性能收益
float l2_sq_avx2(const float* a, const float* b, size_t dim) {
    (void)a;
    (void)b;
    (void)dim;
    throw std::logic_error("l2_sq_avx2: not implemented");
}

}  // namespace vtop
