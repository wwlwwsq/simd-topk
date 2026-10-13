// simd-topk：Top-K 三方案（技术选型对比的载体）
// 契约：返回距离最小的 k 个下标，按距离升序；k >= n 时返回全部。
// 三方案的下标在并列处可不同，但距离序列必须一致（测试校验后者）。
#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace vtop {

std::vector<uint32_t> topk_full_sort(const float* dist, size_t n, size_t k);
std::vector<uint32_t> topk_nth_element(const float* dist, size_t n, size_t k);
std::vector<uint32_t> topk_max_heap(const float* dist, size_t n, size_t k);

}  // namespace vtop
