#include <iostream>
#include <iomanip>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include "cq.h"
using namespace std::chrono;

// ---- 仕事の中身（演習4-2 と同じ） ----
long calib = 0;
std::atomic<long> sink{0};        // 複数スレッドから足すので atomic（volatile では守れない）
long burn(long n) { long s = 0; for (long i = 0; i < n; i++) s += (i * 2654435761u) % 7; return s; }
void cpu_ms(int ms)  { sink.fetch_add(burn(calib * ms), std::memory_order_relaxed); }   // CPU が計算する区間
void ext_ms(int ms)  { std::this_thread::sleep_for(milliseconds(ms)); }   // 外部（DPU・画面）にまかせている区間
void calibrate() {
    long n = 100000;
    for (;;) {
        auto a = steady_clock::now(); sink.fetch_add(burn(n), std::memory_order_relaxed);
        auto us = duration_cast<microseconds>(steady_clock::now() - a).count();
        if (us > 30000) { calib = n * 1000 / us; break; }
        n *= 2;
    }
}

const int N = 33, WARMUP = 3;
const std::size_t CAP = 4;

// 段ごとの記録（各スレッドが自分専用に持つので、鍵は要らない）―― 演習4-3 と同じ
struct Stat { long long pop_us = 0, work_us = 0, push_us = 0; };

void run(int show_ms) {
    ConcurrentQueue<int> q1(CAP), q2(CAP);
    Stat sr, si, ss;
    auto t0 = steady_clock::now();
    steady_clock::time_point tstart;

    auto lap = [](steady_clock::time_point& t) {
        auto n = steady_clock::now();
        long long us = duration_cast<microseconds>(n - t).count();
        t = n; return us;
    };

    std::thread rd([&] {
        for (int i = 0; i < N; i++) {
            auto t = steady_clock::now();
            cpu_ms(13);                     sr.work_us += (i < WARMUP ? 0 : lap(t));   // read
            q1.push(i);                     sr.push_us += (i < WARMUP ? 0 : lap(t));   // 入れ待ち
        }
    });
    std::thread in([&] {
        for (int i = 0; i < N; i++) {
            auto t = steady_clock::now();
            int v = q1.pop();               si.pop_us  += (i < WARMUP ? 0 : lap(t));   // 取り出し待ち
            cpu_ms(9); ext_ms(2); cpu_ms(2);
                                            si.work_us += (i < WARMUP ? 0 : lap(t));   // pre+dpu+post
            q2.push(v);                     si.push_us += (i < WARMUP ? 0 : lap(t));   // 入れ待ち
        }
    });
    std::thread sh([&] {
        for (int i = 0; i < N; i++) {
            auto t = steady_clock::now();
            q2.pop();                       ss.pop_us  += (i < WARMUP ? 0 : lap(t));   // 取り出し待ち
            if (i == WARMUP) tstart = steady_clock::now();
            ext_ms(i % 10 == 0 ? show_ms * 2 : show_ms);
                                            ss.work_us += (i < WARMUP ? 0 : lap(t));   // show
        }
    });
    rd.join(); in.join(); sh.join();

    double sec = duration_cast<microseconds>(steady_clock::now() - tstart).count() / 1e6;
    int n = N - WARMUP;
    std::cout << "\n【Show = " << show_ms << "ms のとき】  実測 "
              << std::fixed << std::setprecision(1) << ((N - WARMUP) / sec) << " FPS\n";
    std::cout << "  取り出し待ち     正味     入れ待ち   段\n";
    auto line = [&](const char* nm, const Stat& s) {
        std::cout << std::setw(11) << std::setprecision(1) << (s.pop_us / 1000.0 / n) << "ms"
                  << std::setw(9)  << (s.work_us / 1000.0 / n) << "ms"
                  << std::setw(11) << (s.push_us / 1000.0 / n) << "ms   " << nm << "\n";
    };
    line("Read",  sr);
    line("Infer", si);
    line("Show",  ss);
}

int main() {
    calibrate();
    std::cout << "3段パイプライン（Read / Infer / Show を1人ずつ、容量" << CAP << "）\n"
              << "「待っていない段」がボトルネックです。\n";
    run(30);        // 演習4-3 と同じ条件
    run(10);        // Show を 3倍速くしてみる
    return 0;
}
