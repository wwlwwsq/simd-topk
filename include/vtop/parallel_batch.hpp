// simd-topk：基于线程池的并行批量查询（鸭子类型：Pool 只要有 submit 即可，
// 因此 bpool::ThreadPool 和 bpool::ShardedThreadPool 都能传进来）
#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "dataset.hpp"
#include "distance.hpp"
#include "topk.hpp"

namespace vtop {

// queries 指向 qnum 条连续查询向量（每条 dim 维）。返回第 i 条查询的 Top-K 下标。
template <class Pool>
std::vector<std::vector<uint32_t>> query_batch_parallel(
    Pool& pool, const Dataset& ds, const float* queries, size_t qnum, size_t k,
    DistMethod method = DistMethod::kAvx2) {
    // TODO(18) 每条查询一个任务：
    //   1. 距离缓冲怎么来？三选一并把理由写进 README：
    //      a) 每任务 make_unique<float[]>(n)     —— 简单但每查询一次堆分配
    //      b) 线程局部存储复用                   —— 零重复分配，注意 shutdown 前后生命周期
    //      c) 预分配 per-worker 槽位池           —— 折中
    //   2. dist[i] = l2_sq(query, ds.row(i), dim)（按 method 分发）
    //   3. topk_max_heap / topk_nth_element 取 k 个
    //   汇总：收集所有 future，逐个 get()
    (void)pool;
    (void)ds;
    (void)queries;
    (void)qnum;
    (void)k;
    (void)method;
    return {};
}

}  // namespace vtop
