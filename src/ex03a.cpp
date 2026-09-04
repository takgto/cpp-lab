#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include "cq.h"
using namespace std::chrono;

const int N = 60;                                // 流すデータの個数
void wait_ms(int ms) { std::this_thread::sleep_for(milliseconds(ms)); }

// 作る側は make_time(i) ms で1個つくり、使う側は use_ms ms で1個つかう。それを N 個くり返す。
// 「N個を流し終えるまでの時間」を測り、FPS（= N ÷ 秒）に直して表示する。
void run(std::size_t cap, int (*make_time)(int), int use_ms) {
    ConcurrentQueue<steady_clock::time_point> q(cap);
    long long idle_us = 0;                       // 使う側が、次のデータを待って手待ちしていた時間
    long long stay_us = 0;                       // データがキューに並んでいた時間

    auto t0 = steady_clock::now();
    std::thread producer([&] {
        for (int i = 0; i < N; i++) { wait_ms(make_time(i)); q.push(steady_clock::now()); }
    });
    std::thread consumer([&] {
        for (int i = 0; i < N; i++) {
            auto w0 = steady_clock::now();
            auto born = q.pop();
            auto now = steady_clock::now();
            idle_us += duration_cast<microseconds>(now - w0).count();   // pop で待った時間
            stay_us += duration_cast<microseconds>(now - born).count(); // 並んでいた時間
            wait_ms(use_ms);
        }
    });
    producer.join(); consumer.join();
    double sec = duration_cast<microseconds>(steady_clock::now() - t0).count() / 1e6;

    std::cout << std::right << std::fixed
              << std::setw(9) << cap
              << std::setw(11) << std::setprecision(1) << (N / sec)
              << std::setw(10) << q.peak()
              << std::setw(11) << (idle_us / 1000)
              << std::setw(11) << (stay_us / N / 1000) << "\n";
}

int even(int)    { return 10; }                        // いつも 10ms
int bumpy(int i) { return (i % 5 == 4) ? 90 : 10; }     // ふだん10ms、5個に1回だけ90ms（平均26ms）

int main() {
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "【A】作る側 いつも 10ms/個、使う側 50ms/個"
              << " ―― 使う側が上限なので、理想は 1000/50 = " << (1000.0 / 50) << " FPS\n";
    std::cout << " capacity        FPS  peak(個) idle(ms) delay(ms)\n";
    for (std::size_t c : {1, 4, 32, 1000}) run(c, even, 50);

    std::cout << "\n【B】作る側 ふだん 10ms、5個に1回 90ms（平均26ms）、使う側 30ms/個"
              << " ―― 理想は 1000/30 = " << (1000.0 / 30) << " FPS\n";
    std::cout << " capacity        FPS  peak(個) idle(ms) delay(ms)\n";
    for (std::size_t c : {1, 2, 4, 8, 32}) run(c, bumpy, 30);
    return 0;
}
