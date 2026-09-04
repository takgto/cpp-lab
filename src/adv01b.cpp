#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <mutex>
using namespace std::chrono;

std::atomic<long> sink{0};   // 複数スレッドから足すので atomic
std::mutex out_mtx;          // 表示が混ざらないように
steady_clock::time_point t0;
int ms_now() { return (int)duration_cast<milliseconds>(steady_clock::now() - t0).count(); }

long calib = 0;
long burn(long n) { long s = 0; for (long i = 0; i < n; i++) s += (i * 2654435761u) % 7; return s; }
void calibrate() {
    long n = 100000;
    for (;;) {
        auto a = steady_clock::now(); sink.fetch_add(burn(n), std::memory_order_relaxed);
        auto us = duration_cast<microseconds>(steady_clock::now() - a).count();
        if (us > 30000) { calib = n * 1000 / us; break; }
        n *= 2;
    }
}

// 3本のスレッドを立てて、それぞれが終わった時刻を出す
void run(const char* kind, bool cpu) {
    std::cout << "\n【" << kind << " を 3本】\n";
    t0 = steady_clock::now();
    std::vector<std::thread> ts;
    for (int k = 0; k < 3; k++)
        ts.emplace_back([=] {
            if (cpu) sink.fetch_add(burn(calib * 300), std::memory_order_relaxed);  // 300ms 分の計算
            else     std::this_thread::sleep_for(milliseconds(300));        // 300ms 待つだけ
            std::lock_guard<std::mutex> g(out_mtx);   // 表示だけ鍵の中（計算は外）
            std::cout << "  スレッド" << k << " が終わった : " << ms_now() << " ms\n";
        });
    for (auto& t : ts) t.join();
    std::cout << "  全部終わるまで : " << ms_now() << " ms\n";
}

int main() {
    calibrate();
    std::cout << "hardware_concurrency() = " << std::thread::hardware_concurrency() << "\n";
    run("計算しかしない仕事（1本あたり300ms分）", true);
    run("ひたすら待つだけの仕事（300ms）",        false);
    return 0;
}
