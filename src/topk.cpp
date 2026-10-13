#include "vtop/topk.hpp"

#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <utility>

namespace vtop {

// 参考实现（正确性基准，另两个方案与它对答案）
std::vector<uint32_t> topk_full_sort(const float* dist, size_t n, size_t k) {
    if (k > n) k = n;
    std::vector<uint32_t> idx(n);
    std::iota(idx.begin(), idx.end(), 0u);
    std::sort(idx.begin(), idx.end(),
              [dist](uint32_t x, uint32_t y) { return dist[x] < dist[y]; });
    idx.resize(k);
    return idx;
}

// TODO(16) 部分选择：
//   idx 数组 + std::nth_element(idx.begin(), idx.begin()+k-1, idx.end(), 按 dist 比较)
//   再对前 k 个 std::sort。复杂度 O(n) 选择 + O(k log k) 排序，K 远小于 n 时显著快于全排序。
std::vector<uint32_t> topk_nth_element(const float* dist, size_t n, size_t k) {
    (void)dist;
    (void)n;
    (void)k;
    throw std::logic_error("topk_nth_element: not implemented");
}

// TODO(17) 大小为 k 的大顶堆：
//   std::priority_queue<std::pair<float, uint32_t>, vector<...>, std::greater<>> min-heap?
//   不——要「当前最小 k 个」里的最大者做堆顶：用 less<>（pair 默认）即大顶堆。
//   前 k 个直接 push；之后 dist[i] < 堆顶则 pop + push。
//   结束后导出下标并按距离升序排列。复杂度 O(n log k)，一趟扫描，无需 idx 数组。
std::vector<uint32_t> topk_max_heap(const float* dist, size_t n, size_t k) {
    (void)dist;
    (void)n;
    (void)k;
    throw std::logic_error("topk_max_heap: not implemented");
}

}  // namespace vtop
