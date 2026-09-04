// 4-3  同じ仕事を3段パイプラインに組んで測る（4-2 は直列だった）
#include <iostream>
#include <iomanip>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include "cq.h"
using namespace std::chrono;
using TP = steady_clock::time_point;

// ---- 仕事の中身。4-2 とまったく同じ ----
long calib = 0;
std::atomic<long> sink{0};        // 3本から足すので atomic（volatile では守れない）
long burn(long n) { long s = 0; for (long i = 0; i < n; i++) s += (i * 2654435761u) % 7; return s; }
void cpu_ms(int ms) { sink.fetch_add(burn(calib * ms), std::memory_order_relaxed); }   // CPU が計算する
void ext_ms(int ms) { std::this_thread::sleep_for(milliseconds(ms)); }                 // 外部にまかせる
void calibrate() {
    long n = 100000;
    for (;;) {
        TP t = steady_clock::now(); sink.fetch_add(burn(n), std::memory_order_relaxed);
        auto us = duration_cast<microseconds>(steady_clock::now() - t).count();
        if (us > 30000) { calib = n * 1000 / us; return; }
        n *= 2;
    }
}

const int N = 33, WARMUP = 3;      // 立ち上がりの3個は捨てる（作法④）
const std::size_t CAP = 4;

// 段ごとの記録。各スレッドが自分専用に持つので、鍵は要らない（作法②）
struct Stat { long long pop_us = 0, work_us = 0, push_us = 0; };

// t から今までの時間(us)を返し、t を進める（作法③）
long long lap(TP& t) {
    TP n = steady_clock::now();
    long long us = duration_cast<microseconds>(n - t).count();
    t = n;
    return us;
}

int main() {
    calibrate();
    ConcurrentQueue<int> q1(CAP), q2(CAP);      // Read→Infer、Infer→Show
    Stat sr, si, ss;
    TP tstart;

    std::thread rd([&] {
        for (int i = 0; i < N; i++) {
            TP t = steady_clock::now();
            cpu_ms(13);                  sr.work_us += (i < WARMUP ? 0 : lap(t));   // read
            q1.push(i);                  sr.push_us += (i < WARMUP ? 0 : lap(t));   // 入れ待ち
        }
    });
    std::thread in([&] {
        for (int i = 0; i < N; i++) {
            TP t = steady_clock::now();
            int v = q1.pop();            si.pop_us  += (i < WARMUP ? 0 : lap(t));   // 取り出し待ち
            cpu_ms(9); ext_ms(2); cpu_ms(2);
                                         si.work_us += (i < WARMUP ? 0 : lap(t));   // pre+dpu+post
            q2.push(v);                  si.push_us += (i < WARMUP ? 0 : lap(t));   // 入れ待ち
        }
    });
    std::thread sh([&] {
        for (int i = 0; i < N; i++) {
            TP t = steady_clock::now();
            q2.pop();                    ss.pop_us  += (i < WARMUP ? 0 : lap(t));   // 取り出し待ち
            if (i == WARMUP) tstart = steady_clock::now();
            ext_ms(i % 10 == 0 ? 60 : 30);
                                         ss.work_us += (i < WARMUP ? 0 : lap(t));   // show
        }
    });
    rd.join(); in.join(); sh.join();

    // ---- 表示は全部終わってから1回だけ（作法①）----
    double sec = duration_cast<microseconds>(steady_clock::now() - tstart).count() / 1e6;
    int n = N - WARMUP;
    std::cout << std::fixed << std::setprecision(1)
              << "3段パイプライン（Read / Infer / Show を1人ずつ、容量 " << CAP << "）  実測 "
              << (n / sec) << " FPS\n\n"
              << "  取り出し待ち     正味     入れ待ち   段\n";
    auto line = [&](const char* nm, const Stat& s) {
        std::cout << std::setw(11) << (s.pop_us / 1000.0 / n) << "ms"
                  << std::setw(9)  << (s.work_us / 1000.0 / n) << "ms"
                  << std::setw(11) << (s.push_us / 1000.0 / n) << "ms   " << nm << "\n";
    };
    line("Read",  sr);
    line("Infer", si);
    line("Show",  ss);
    return 0;
}
