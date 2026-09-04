// 付録  速度倍率の測り方 ―― 同じ計算を 1本 / 2本 / 3本 / 4本 でやって、時間を比べる
#include <iostream>
#include <iomanip>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
using namespace std::chrono;

std::atomic<long> sink{0};        // 結果は捨てるが、最適化で消されないように受ける

// 中身のない足し算ループ。sleep ではなく、本当に CPU を使うのが要点
long burn(long n) { long s = 0; for (long i = 0; i < n; i++) s += (i * 2654435761u) % 7; return s; }

// n 本のスレッドで、それぞれ U 回ぶんの計算をさせて、全部終わるまでの時間(us)を返す
double run_n(long U, int n) {
    std::vector<std::thread> th;
    auto t0 = steady_clock::now();
    for (int k = 0; k < n; k++)
        th.emplace_back([&]{ sink.fetch_add(burn(U), std::memory_order_relaxed); });
    for (auto& t : th) t.join();
    return duration_cast<microseconds>(steady_clock::now() - t0).count();
}

int main() {
    // 1ms ぶんの繰り返し回数を測る
    long n = 100000, calib = 0;
    for (;;) {
        auto t0 = steady_clock::now();
        sink.fetch_add(burn(n), std::memory_order_relaxed);
        auto us = duration_cast<microseconds>(steady_clock::now() - t0).count();
        if (us > 30000) { calib = n * 1000 / us; break; }
        n *= 2;
    }

    long U = calib * 200;         // 1本あたり 200ms 分の計算（短いとぶれる）
    run_n(U, 1);                  // ウォームアップ（1回目は捨てる）
    double one = run_n(U, 1);     // これが基準

    std::cout << "hardware_concurrency() = " << std::thread::hardware_concurrency() << "\n\n"
              << std::fixed << " 本数   かかった時間   速度倍率\n";
    for (int k = 1; k <= 4; k++) {
        double t = run_n(U, k);
        std::cout << std::setw(4) << k
                  << std::setw(12) << std::setprecision(0) << t / 1000 << "ms"
                  << std::setw(11) << std::setprecision(2) << (k * one / t) << "\n";
    }
    return 0;
}
