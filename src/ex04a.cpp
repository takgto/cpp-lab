#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>
using namespace std::chrono;

volatile long long sink = 0;

template <class F>
double ns_per_call(int n, F f) {          // 1回あたりのナノ秒
    auto t0 = steady_clock::now();
    for (int i = 0; i < n; i++) f(i);
    return duration_cast<nanoseconds>(steady_clock::now() - t0).count() / (double)n;
}

int main() {
    long long acc = 0;

    // (1) 時計を1回読む
    double c_clock = ns_per_call(1000000, [&](int) {
        sink += steady_clock::now().time_since_epoch().count() & 1;
    });

    // (2) 測って足し込む（時計2回 + 引き算 + 足し算）＝ 計測1回ぶん
    double c_lap = ns_per_call(1000000, [&](int) {
        auto a = steady_clock::now();
        auto b = steady_clock::now();
        acc += duration_cast<nanoseconds>(b - a).count();
    });

    // (3) 1行ぶんの文字列を作るだけ（画面には出さない）
    double c_str = ns_per_call(100000, [&](int i) {
        std::ostringstream os;
        os << "\nrunYOLO preprocessing time= " << i << " [mS]\n";
        sink += os.str().size();
    });

    // (4) cout に1行流す（flush なし）
    double c_out = ns_per_call(20000, [&](int i) {
        std::cout << "\nrunYOLO preprocessing time= " << i << " [mS]\n";
    });

    // (5) cout に1行流して flush する
    double c_flush = ns_per_call(20000, [&](int i) {
        std::cout << "\nrunYOLO preprocessing time= " << i << " [mS]\n" << std::flush;
    });

    std::cerr << std::fixed << std::setprecision(0);
    std::cerr << "\n1回あたりの値段（このマシン、この出力先で）\n\n";
    std::cerr << std::setw(10) << c_clock << " ns   steady_clock::now() を1回読む\n";
    std::cerr << std::setw(10) << c_lap   << " ns   時計2回 + 引き算 + 足し込み（計測1回ぶん）\n";
    std::cerr << std::setw(10) << c_str   << " ns   1行ぶんの文字列を作る（表示はしない）\n";
    std::cerr << std::setw(10) << c_out   << " ns   cout に1行流す（flush なし）\n";
    std::cerr << std::setw(10) << c_flush << " ns   cout に1行流して flush する\n";
    std::cerr << "\n(" << acc << " " << sink << ")\n";
    return 0;
}