#include <iostream>
#include <iomanip>
#include <thread>
#include <atomic>
#include <chrono>
#include "cq.h"
using namespace std::chrono;

const int N = 40;
const std::size_t CAP = 8;
void wait_ms(int ms) { std::this_thread::sleep_for(milliseconds(ms)); }

// 3段パイプラインを走らせ、そのあいだ q1 と q2 の長さを 10ms ごとに覗く
void run(const char* label, int read_ms, int infer_ms, int show_ms) {
    ConcurrentQueue<int> q1(CAP), q2(CAP);
    std::atomic<bool> running{true};
    double s1 = 0, s2 = 0; long n = 0;

    std::thread watcher([&] {
        while (running) { s1 += q1.size(); s2 += q2.size(); n++; wait_ms(10); }
    });
    auto t0 = steady_clock::now();
    std::thread rd([&] { for (int i = 0; i < N; i++) { wait_ms(read_ms);  q1.push(i); } });
    std::thread in([&] { for (int i = 0; i < N; i++) { int v = q1.pop(); wait_ms(infer_ms); q2.push(v); } });
    std::thread sh([&] { for (int i = 0; i < N; i++) { q2.pop(); wait_ms(show_ms); } });
    rd.join(); in.join(); sh.join();
    int ms = duration_cast<milliseconds>(steady_clock::now() - t0).count();
    running = false; watcher.join();

    std::cout << std::fixed << std::setprecision(1)
              << std::setw(9) << (s1 / n) << std::setw(9) << (s2 / n)
              << std::setw(10) << (1000.0 * N / ms) << "   " << label << "\n";
}

int main() {
    std::cout << "容量はどちらも " << CAP << "。q1 = Read→Infer、q2 = Infer→Show の待ち行列\n\n";
    std::cout << " q1の平均  q2の平均   実測FPS   どの段が遅いか\n";
    run("Infer が遅い（10 / 60 / 20 ms）", 10, 60, 20);
    run("Show  が遅い（10 / 20 / 60 ms）", 10, 20, 60);
    run("Read  が遅い（60 / 20 / 10 ms）", 60, 20, 10);
    return 0;
}
