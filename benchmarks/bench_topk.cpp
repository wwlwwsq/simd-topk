// 基准一：距离方案吞吐（naive / blocked8 / avx2）+ Top-K 三方案耗时（K 扫描）
// 用法: ./bench_topk [--n 200000] [--dim 128] [--k 100] [--reps 5]
// 输出 markdown 表行，粘进 README 数据表。
#include "vtop/dataset.hpp"
#include "vtop/distance.hpp"
#include "vtop/topk.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

using namespace vtop;
using clk = std::chrono::steady_clock;

namespace {

struct Args {
    size_t n = 200000;
    size_t dim = 128;
    size_t k = 100;
    int reps = 5;
};

Args parse(int argc, char** argv) {
    Args a;
    for (int i = 1; i + 1 < argc; i += 2) {
        if (!std::strcmp(argv[i], "--n")) a.n = std::strtoull(argv[i + 1], nullptr, 10);
        else if (!std::strcmp(argv[i], "--dim")) a.dim = std::strtoull(argv[i + 1], nullptr, 10);
        else if (!std::strcmp(argv[i], "--k")) a.k = std::strtoull(argv[i + 1], nullptr, 10);
        else if (!std::strcmp(argv[i], "--reps")) a.reps = std::atoi(argv[i + 1]);
    }
    return a;
}

double median(std::vector<double> v) {
    std::sort(v.begin(), v.end());
    return v[v.size() / 2];
}

bool has_avx2() {
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
    return __builtin_cpu_supports("avx2");
#else
    return false;
#endif
}

}  // namespace

int main(int argc, char** argv) {
    const Args a = parse(argc, argv);
    Dataset ds = make_dataset(a.n, a.dim, 42);
    Dataset qs = make_dataset(1, a.dim, 7);
    const float* q = qs.row(0);
    volatile double sink = 0;

    // ---- 1. 距离方案吞吐 ----
    std::printf("| method | M vec/s (n=%zu, dim=%zu) |\n|---|---|---|\n", a.n, a.dim);
    struct Plan { const char* name; DistMethod m; };
    const Plan plans[] = {
        {"scalar_naive", DistMethod::kScalarNaive},
        {"scalar_blocked8", DistMethod::kScalarBlocked8},
        {"avx2", DistMethod::kAvx2},
    };
    for (const auto& pl : plans) {
        if (pl.m == DistMethod::kAvx2 && !has_avx2()) {
            std::printf("| avx2 | SKIP（CPU 不支持） |\n");
            continue;
        }
        std::vector<double> ts;
        for (int r = 0; r < a.reps; ++r) {
            const auto t0 = clk::now();
            double s = 0;
            for (size_t i = 0; i < ds.num_vectors(); ++i) {
                float d = 0.f;
                switch (pl.m) {
                    case DistMethod::kScalarNaive:
                        d = l2_sq_scalar_naive(q, ds.row(i), ds.dim());
                        break;
                    case DistMethod::kScalarBlocked8:
                        d = l2_sq_scalar_blocked8(q, ds.row(i), ds.dim());
                        break;
                    default:
                        d = l2_sq_avx2(q, ds.row(i), ds.dim());
                        break;
                }
                s += d;
            }
            sink = s;
            ts.push_back(std::chrono::duration<double>(clk::now() - t0).count());
        }
        std::printf("| %s | %.2f |\n", pl.name, double(a.n) / median(ts) / 1e6);
    }

    // ---- 2. Top-K 方案（预计算一次距离数组，消除距离计算干扰）----
    std::vector<float> dist(a.n);
    for (size_t i = 0; i < a.n; ++i) dist[i] = l2_sq_scalar_naive(q, ds.row(i), ds.dim());

    std::printf("\n| k | full_sort_ms | nth_ms | heap_ms |\n|---|---|---|---|\n");
    for (size_t k : {size_t(10), size_t(100), size_t(1000)}) {
        if (k > a.n) continue;
        std::vector<double> t_sort, t_nth, t_heap;
        for (int r = 0; r < a.reps; ++r) {
            auto t0 = clk::now();
            auto r1 = topk_full_sort(dist.data(), a.n, k);
            (void)r1;
            t_sort.push_back(std::chrono::duration<double, std::milli>(clk::now() - t0).count());

            t0 = clk::now();
            auto r2 = topk_nth_element(dist.data(), a.n, k);
            (void)r2;
            t_nth.push_back(std::chrono::duration<double, std::milli>(clk::now() - t0).count());

            t0 = clk::now();
            auto r3 = topk_max_heap(dist.data(), a.n, k);
            (void)r3;
            t_heap.push_back(std::chrono::duration<double, std::milli>(clk::now() - t0).count());
        }
        std::printf("| %zu | %.2f | %.2f | %.2f |\n", k, median(t_sort), median(t_nth), median(t_heap));
    }
    (void)sink;
    return 0;
}
