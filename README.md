# simd-topk

SoA 布局向量库 + 平方 L2 精确 Top-K：三种 Top-K 方案对比、手写 AVX2 向量化（与标量分块版**逐位一致**）、复用 [bounded-threadpool](../bounded-threadpool) 并行批量查询。
对应 JD 关键词「性能调优 / 技术方案选型 / 高并发」。

## 构建

```bash
cmake -S . -B build && cmake --build build -j
./build/test_consistency              # 一致性测试（骨架阶段预期 FAILED: not implemented）
./build/bench_topk --n 200000         # 距离方案 + TopK 方案
./build/bench_parallel 64 100         # 并行查询（需 ../bounded-threadpool 存在）
bash scripts/gen_report.sh            # 全量基准出表
```

内存预算：n=1,000,000 × dim=128 × float32 ≈ **512 MB**。开发默认 20 万，最终数据表用 100 万。

## 为什么 blocked8 和 avx2 能逐位一致（面试高频考点）

浮点加法不满足结合律：`scalar_naive` 顺序累加，和向量化结果只能做相对误差比较；
`scalar_blocked8` 用 8 路累加器，且水平规约顺序与 AVX2 的
`_mm_add_ps → _mm_movehl_ps → _mm_add_ss` 完全一一对应，因此两者**逐位相等**——
这是 SIMD 结果正确性验证的标准做法。
注意 AVX2 里不能顺手用 FMA：一次乘加会改变数值，与标量版不再逐位一致（FMA 可作为进阶第四方案，对比精度与性能，面试加分）。

## TODO 实现顺序

TODO(14) blocked8 → TODO(15) avx2（此时 test_avx2_bit_exact 转绿）
→ TODO(16) nth_element、TODO(17) max_heap（test_topk_agreement 转绿）
→ TODO(18) parallel_batch（bench_parallel 出数据）。

## 数据表（实测填入）

### 距离方案（n=1M, dim=128, reps=5 取中位）

| method | M vec/s | 相对 naive 加速比 |
|---|---|---|
| scalar_naive | | 1.0x |
| scalar_blocked8 | | |
| avx2 | | |

### Top-K 方案（n=1M，预计算距离）

| k | full_sort_ms | nth_ms | heap_ms |
|---|---|---|---|
| 10 | | | |
| 100 | | | |
| 1000 | | | |

### 并行批量查询（qnum=64, k=100, n=200k）

| workers | M queries/s | 加速比 |
|---|---|---|
| 1 | | 1.0x |
| 8 | | |

### 优化记录

| 日期 | 假设 | 改动 | 数据（前 -> 后） | 工具 |
|---|---|---|---|---|
| | | | | |

## 简历 bullet（数据出来再填）

- 实现 SoA 布局向量库与平方 L2 精确 Top-K，对比全排序/nth_element/大顶堆方案，K=100 时最优方案较全排序耗时降低 __%
- 手写 AVX2 批量距离计算（8 路累加 + 顺序对应规约），与标量分块实现逐位一致，单核吞吐提升 __ 倍
- 基于自研线程池的并行批量查询，8 线程 QPS 提升 __ 倍；perf/gdb 定位并记录优化过程
