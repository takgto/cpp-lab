// 問3-2  一番遅い段の人を増やしていく（上限A を上げると、どこで止まるか）
#include <iostream>
#include <iomanip>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>
#include <atomic>
#include <ctime>
#include "cq.h"
using namespace std::chrono;

const int READ_CPU =  2, READ_WAIT =  8;     //  10ms  （外部＝ディスク）
const int INF_CPU  = 35;                     //  外部は 0/25/50 → 平均 60ms（アクセラレータ）
const int SHOW_CPU =  3, SHOW_WAIT = 27;     //  30ms  （外部＝画面）

const int N = 60;                  // 少なすぎると立ち上がりの影響でぶれる
const std::size_t CAP = 4;

// このスレッドがこれまでに「実際に CPU を使った」時間(ms)。待っている間は増えない。
double thread_cpu_ms() {
    timespec ts; clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}
std::atomic<double> cpu_total{0};                       // 3本ぶんの合計
void add_cpu(double from) {
    double v = thread_cpu_ms() - from, old = cpu_total.load();
    while (!cpu_total.compare_exchange_weak(old, old + v)) {}
}

long calib = 0;
std::atomic<long> sink{0};        // 複数スレッドから足すので atomic（volatile では守れない）
long burn(long n) { long s = 0; for (long i = 0; i < n; i++) s += (i * 2654435761u) % 7; return s; }
void calibrate() {
    long n = 100000;
    for (;;) {
        auto t0 = steady_clock::now(); sink.fetch_add(burn(n), std::memory_order_relaxed);
        auto us = duration_cast<microseconds>(steady_clock::now() - t0).count();
        if (us > 30000) { calib = n * 1000 / us; return; }
        n *= 2;
    }
}
void stage(int cpu_ms, int wait_ms) {
    if (cpu_ms)  sink.fetch_add(burn(calib * cpu_ms), std::memory_order_relaxed);
    if (wait_ms) std::this_thread::sleep_for(milliseconds(wait_ms));
}
void do_read()       { stage(READ_CPU, READ_WAIT); }
void do_infer(int i) { stage(INF_CPU, (i % 3) * 25); }
void do_show()       { stage(SHOW_CPU, SHOW_WAIT); }

// 演習4-2 の作法④：最初の数個はキャッシュが冷えているので捨てる
void warmup() { for (int i = 0; i < 3; i++) { do_read(); do_infer(i); do_show(); } }

// hardware_concurrency() を信じず、「1本のときの何倍の速さで計算が進むか」を実測する。
// コアの数（整数）ではなく倍率（実数）が返る。
double measure_speedup(unsigned hw) {
    long U = calib * 200;              // 短すぎるとぶれるので 200ms 分
    auto t0 = steady_clock::now();
    sink.fetch_add(burn(U), std::memory_order_relaxed);
    double one = duration_cast<microseconds>(steady_clock::now() - t0).count();

    std::vector<std::thread> th;
    t0 = steady_clock::now();
    for (unsigned k = 0; k < hw; k++) th.emplace_back([&]{ sink.fetch_add(burn(U), std::memory_order_relaxed); });
    for (auto& t : th) t.join();
    double many = duration_cast<microseconds>(steady_clock::now() - t0).count();
    return hw * one / many;
}

// Infer を ninfer 人、Show を nshow 人にした3段パイプライン。
// 人を増やしても、2人とも同じキューから取り出し、同じキューに入れる。
struct Result { double fps; double cpu; int inv; };
Result pipeline(int ninfer, int nshow) {
    ConcurrentQueue<int> q1(CAP), q2(CAP);
    int inv = 0, last = -1;
    std::mutex m;
    cpu_total = 0;

    auto t0 = steady_clock::now();
    std::thread rd([&]{ double c = thread_cpu_ms();
        for (int i = 0; i < N; i++) { do_read(); q1.push(i); } add_cpu(c); });

    std::vector<std::thread> inf, sh;
    for (int k = 0; k < ninfer; k++)
        inf.emplace_back([&, k]{                       // ← k は値で捕まえる
            double c = thread_cpu_ms();
            for (int i = k; i < N; i += ninfer) { int x = q1.pop(); do_infer(x); q2.push(x); }
            add_cpu(c);
        });
    for (int k = 0; k < nshow; k++)
        sh.emplace_back([&, k]{
            double c = thread_cpu_ms();
            for (int i = k; i < N; i += nshow) {
                int x = q2.pop();
                do_show();
                std::lock_guard<std::mutex> g(m);
                if (x < last) inv++;                   // 前より小さい番号が来た＝追い越された
                last = x;
            }
            add_cpu(c);
        });

    rd.join(); for (auto& t : inf) t.join(); for (auto& t : sh) t.join();
    double ms = duration_cast<milliseconds>(steady_clock::now() - t0).count();
    return { 1000.0 * N / ms, cpu_total.load() / N, inv };   // CPU 時間は1個あたりに直す
}

double sp = 0, limitB = 0;        // 速度倍率と上限B（どちらも実測から）
void line(const char* label, double slowest, Result r) {
    std::cout << std::setw(7) << (1000.0 / slowest)
              << std::setw(7) << limitB
              << std::setw(9) << r.fps
              << std::setw(8) << r.cpu << "ms"
              << std::setw(6) << r.inv << "   " << label << "\n";
}

int main() {
    unsigned hw = std::thread::hardware_concurrency();
    calibrate();
    warmup();
    sp = measure_speedup(hw);

    std::cout << std::fixed << std::setprecision(1)
      << "各段の時間: Read 10ms / Infer 60ms(平均) / Show 30ms   キューの容量: 2本とも " << CAP << "\n\n";

    std::cout << std::setprecision(2)
      << "hardware_concurrency() = " << hw << " ですが、" << hw
      << "本で同時に計算させて速度を実測すると " << sp << " 倍でした\n"
      << "  （これはコアの数ではなく倍率です。整数になるとは限りません）\n\n";

    // 上限B の分母は、いちばん取り合いの少ない構成②で測った CPU/個 を使う
    Result r2 = pipeline(1, 1);
    limitB = 1000.0 * sp / r2.cpu;

    std::cout << std::setprecision(1)
      << "上限A = 1000 / 一番遅い段の時間                     ... 人数を変えると動く\n"
      << "上限B = 1000 x " << std::setprecision(2) << sp << std::setprecision(1)
      << " / " << r2.cpu << "ms = " << limitB << " FPS （目安）   ... 動かない\n\n";

    std::cout << "  上限A  上限B  実測FPS   CPU/個  乱れ   構成\n";
    line("3段（Infer 1人 / Show 1人）", 60, r2);
    line("Infer 2人",                   30, pipeline(2, 1));
    return 0;
}
