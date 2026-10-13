#!/usr/bin/env bash
# 全量基准：构建 Release 并跑三个基准，输出 markdown 表（结果粘进 README 数据表）
set -euo pipefail
cd "$(dirname "$0")/.."

cmake -S . -B build/report -DCMAKE_BUILD_TYPE=Release >/dev/null
cmake --build build/report -j >/dev/null

echo "== 距离方案 + TopK（n=200000, dim=128, reps=5）=="
./build/report/bench_topk --n 200000 --dim 128 --reps 5

echo
echo "== 最终大表（n=1000000，内存约 512MB，跑前确认内存够）=="
./build/report/bench_topk --n 1000000 --dim 128 --reps 5

if [ -x ./build/report/bench_parallel ]; then
    echo
    echo "== 并行批量查询（qnum=64, k=100, n=200000）=="
    ./build/report/bench_parallel 64 100
fi
