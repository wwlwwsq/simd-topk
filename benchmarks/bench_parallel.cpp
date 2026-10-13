// 基准二：并行批量查询吞吐 vs 线程数（复用 bounded-threadpool 的线程池）
// 用法: ./bench_parallel [qnum=64] [k=100]
// 需要 CMake 找到 bpool 头文件（VTOP_BPOOL_INCLUDE_DIR，默认指向兄弟目录）。
#include "bpool/thread_pool.hpp"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <vector>

#include "vtop/dataset.hpp"
#include "vtop/parallel_batch.hpp"

using namespace vtop;

int main(int argc, char** argv) {
    const size_t qnum = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 64;
    const size_t k = argc > 2 ? std::strtoull(argv[2], nullptr, 10) : 100;
    const size_t n = 200000, dim = 128;

    Dataset ds = make_dataset(n, dim, 42);
    Dataset qs = make_dataset(qnum, dim, 7);

    std::printf("| workers | M queries/s (n=%zu, k=%zu, qnum=%zu) |\n|---|---|---|\n", n, k, qnum);
    std::vector<int> ws{1, 2, 4, 8};
    const int hc = static_cast<int>(std::thread::hardware_concurrency());
    if (hc > 8) ws.push_back(hc);

    for (int w : ws) {
        bpool::PoolOptions o;
        o.num_workers = w;
        o.queue_capacity = 256;
        o.policy = bpool::RejectPolicy::kBlock;
        bpool::ThreadPool pool(o);
        const auto t0 = std::chrono::steady_clock::now();
        auto res = query_batch_parallel(pool, ds, qs.data(), qnum, k);
        const double dt = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
        (void)res;
        std::printf("| %d | %.3f |\n", w, double(qnum) / dt / 1e6);
    }
    return 0;
}
