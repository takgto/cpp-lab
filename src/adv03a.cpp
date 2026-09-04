// 問3-1  直列 と 3段パイプラインを比べる
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <atomic>
#include "cq.h"
using namespace std::chrono;

// ---- 各段の仕事。「CPU」＝コアを占有する、「外部」＝コアを手放す ----
//                        CPU    外部     合計
const int READ_CPU =  2, READ_WAIT =  8;     //  10ms  （外部＝ディスク）
const int INF_CPU  = 35;                     //  外部は 0/25/50 → 平均 60ms（アクセラレータ）
const int SHOW_CPU =  3, SHOW_WAIT = 27;     //  30ms  （外部＝画面）

const int N = 60;                  // 流すフレーム数（少なすぎると立ち上がりの影響でぶれる）
const std::size_t CAP = 4;         // キューの容量（2本とも）

// ---- 1ms ぶん「本当に計算する」道具（sleep ではコアを手放してしまう）----
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
void stage(int cpu_ms, int ext_ms) {
    if (cpu_ms) sink.fetch_add(burn(calib * cpu_ms), std::memory_order_relaxed);  // CPU：コアを占有
    if (ext_ms) std::this_thread::sleep_for(milliseconds(ext_ms));                // 外部：コアを手放す
}
void do_read()       { stage(READ_CPU, READ_WAIT); }
void do_infer(int i) { stage(INF_CPU, (i % 3) * 25); }
void do_show()       { stage(SHOW_CPU, SHOW_WAIT); }

// Infer 1個ぶんの内訳を足し込む場所（Infer スレッド1本しか触らないので鍵は要らない）
long long us_pop = 0, us_work = 0, us_push = 0;

// 演習4-2 の作法④：最初の数個はキャッシュが冷えているので捨てる
void warmup() { for (int i = 0; i < 3; i++) { do_read(); do_infer(i); do_show(); } }

// ================= (1) 直列 ―― 1本の for ループ =================
double serial() {
    auto t0 = steady_clock::now();
    for (int i = 0; i < N; i++) { do_read(); do_infer(i); do_show(); }
    return 1000.0 * N / duration_cast<milliseconds>(steady_clock::now() - t0).count();
}

// ================= (2) 3段 ―― Read / Infer / Show を1本ずつ =================
double pipeline3() {
    ConcurrentQueue<int> q1(CAP), q2(CAP);
    auto t0 = steady_clock::now();
    std::thread rd  ([&]{ for (int i = 0; i < N; i++) { do_read();        q1.push(i);  } });
    std::thread inf ([&]{
        us_pop = us_work = us_push = 0;
        for (int i = 0; i < N; i++) {
            auto a = steady_clock::now(); int x = q1.pop();   // 取り出し待ち
            auto b = steady_clock::now(); do_infer(x);        // 処理（設計では平均 60ms）
            auto c = steady_clock::now(); q2.push(x);         // 入れ待ち
            auto d = steady_clock::now();
            us_pop  += duration_cast<microseconds>(b - a).count();
            us_work += duration_cast<microseconds>(c - b).count();
            us_push += duration_cast<microseconds>(d - c).count();
        }
    });
    std::thread show([&]{ for (int i = 0; i < N; i++) { int x = q2.pop();  (void)x; do_show(); } });
    rd.join(); inf.join(); show.join();
    return 1000.0 * N / duration_cast<milliseconds>(steady_clock::now() - t0).count();
}

int main() {
    calibrate();
    warmup();
    std::cout << std::fixed << std::setprecision(1)
      << "各段の時間（計算＋待ち）: Read 10ms / Infer 60ms(平均) / Show 30ms\n"
      << "1個を直列で通す時間     : 10 + 60 + 30 = 100ms\n"
      << "キューの容量            : 2本とも " << CAP << "\n\n"
      << "上限A = 1000 / 一番遅い段の時間\n\n"
      << "  上限A  実測FPS   構成\n";

    double s = serial();
    std::cout << std::setw(7) << "-" << std::setw(9) << s
              << "   直列（1本の for ループ）\n";
    double p = pipeline3();
    std::cout << std::setw(7) << (1000.0 / 60) << std::setw(9) << p
              << "   3段（Read / Infer / Show を1本ずつ）\n\n";
    std::cout << "3段にして " << (p / s) << " 倍\n\n";

    // ---- 3段のとき Infer の1個あたりが何に使われていたか ----
    double pop = us_pop / 1000.0 / N, wrk = us_work / 1000.0 / N, psh = us_push / 1000.0 / N;
    double tot = pop + wrk + psh;
    std::cout << "3段のときの Infer 1個あたりの内訳\n"
              << "    取り出し待ち " << std::setw(6) << pop << "ms\n"
              << "    処理         " << std::setw(6) << wrk << "ms   <- 60ms のはず\n"
              << "    入れ待ち     " << std::setw(6) << psh << "ms\n"
              << "    合計         " << std::setw(6) << tot << "ms"
              << "   -> 上限A(実測) = " << (1000.0 / tot) << " FPS\n\n"
              << "「処理」が 60ms より大きく伸びていたら、CPU が足りていません（問3-2 へ）。\n"
              << "そのときは 上限A(設計) ではなく 上限A(実測) と実測FPS を見比べてください。\n";
    return 0;
}
