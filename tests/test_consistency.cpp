// 一致性测试：AVX2 与 blocked8 逐位一致、Top-K 三方案距离序列一致。
// 骨架阶段预期 FAILED（not implemented）；全绿 = 数值正确性有据可查。
#include "vtop/dataset.hpp"
#include "vtop/distance.hpp"
#include "vtop/topk.hpp"

#include <cmath>
#include <cstdio>
#include <stdexcept>
#include <vector>

using namespace vtop;

static int g_fail = 0;
static int g_total = 0;

#define CHECK(cond)                                                                  \
    do {                                                                             \
        if (!(cond)) {                                                               \
            ++g_fail;                                                                \
            std::printf("    FAILED %s:%d  CHECK(%s)\n", __FILE__, __LINE__, #cond); \
        }                                                                            \
    } while (0)

#define RUN(fn)                                                            \
    do {                                                                   \
        ++g_total;                                                         \
        std::printf("[ RUN      ] %s\n", #fn);                             \
        try {                                                              \
            fn();                                                          \
        } catch (const std::exception& e) {                                \
            ++g_fail;                                                      \
            std::printf("    FAILED (exception) %s: %s\n", #fn, e.what()); \
        }                                                                  \
    } while (0)

static bool close_rel(float a, float b, float eps) {
    return std::fabs(a - b) <= eps * std::fmax(1.f, std::fmax(std::fabs(a), std::fabs(b)));
}

static bool cpu_has_avx2() {
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
    return __builtin_cpu_supports("avx2");
#else
    return false;
#endif
}

static void test_dataset_basics() {
    Dataset ds = make_dataset(100, 128, 1);
    CHECK(ds.num_vectors() == 100);
    CHECK(ds.dim() == 128);
    bool in_range = true;
    for (size_t i = 0; i < 100 * 128; ++i)
        if (ds.data()[i] < 0.f || ds.data()[i] >= 1.f) { in_range = false; break; }
    CHECK(in_range);
    CHECK(l2_sq_scalar_naive(ds.row(3), ds.row(3), 128) == 0.f);
}

static void test_blocked8_vs_naive_approx() {
    Dataset a = make_dataset(8, 128, 2);
    Dataset b = make_dataset(8, 128, 3);
    for (size_t i = 0; i < 8; ++i)
        CHECK(close_rel(l2_sq_scalar_blocked8(a.row(i), b.row(i), 128),
                        l2_sq_scalar_naive(a.row(i), b.row(i), 128), 1e-5f));
}

static void test_avx2_bit_exact() {
    if (!cpu_has_avx2()) {
        std::printf("    SKIP: CPU 不支持 AVX2\n");
        return;
    }
    Dataset a = make_dataset(64, 128, 4);
    Dataset b = make_dataset(64, 128, 5);
    for (size_t i = 0; i < 64; ++i) {
        const float x = l2_sq_avx2(a.row(i), b.row(i), 128);
        const float y = l2_sq_scalar_blocked8(a.row(i), b.row(i), 128);
        if (x != y) {  // 必须逐位相等
            CHECK(false);
            return;
        }
    }
    CHECK(true);
}

static void test_topk_agreement() {
    Dataset ds = make_dataset(2048, 1, 6);  // 一维向量集，直接当任意距离数组用
    const float* dist = ds.data();
    const size_t n = 2048;
    for (size_t k : {size_t(1), size_t(10), size_t(100), n}) {
        const auto ref = topk_full_sort(dist, n, k);
        CHECK(ref.size() == k);
        for (size_t i = 1; i < ref.size(); ++i)
            CHECK(dist[ref[i - 1]] <= dist[ref[i]]);  // 升序

        const auto check_one = [&](const std::vector<uint32_t>& r) {
            CHECK(r.size() == k);
            for (size_t i = 1; i < r.size(); ++i)
                CHECK(dist[r[i - 1]] <= dist[r[i]]);
            for (size_t i = 0; i < r.size(); ++i)  // 距离序列与参考一致（下标可差）
                CHECK(close_rel(dist[r[i]], dist[ref[i]], 1e-6f));
        };
        check_one(topk_nth_element(dist, n, k));
        check_one(topk_max_heap(dist, n, k));
    }
}

int main() {
    RUN(test_dataset_basics);
    RUN(test_blocked8_vs_naive_approx);
    RUN(test_avx2_bit_exact);
    RUN(test_topk_agreement);
    std::printf("\n%d/%d passed\n", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
