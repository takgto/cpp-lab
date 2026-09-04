#include <iostream>
#include <thread>
#include <chrono>
#include "cq.h"                                  // ← これだけで使える
using namespace std::chrono;

ConcurrentQueue<int> q(3);                       // 容量3のキュー

int main() {
    auto t0 = steady_clock::now();

    // 作る側：10ms に1個つくって push する
    std::thread producer([&] {
        for (int i = 1; i <= 30; i++) {
            std::this_thread::sleep_for(milliseconds(10));
            q.push(i);                           // 満杯なら、勝手に待ってくれる
        }
    });

    // 使う側：1個を 50ms かけて処理する（作る側より5倍おそい）
    std::thread consumer([&] {
        for (int i = 0; i < 30; i++) {
            int v = q.pop();                     // 空なら、勝手に待ってくれる
            std::this_thread::sleep_for(milliseconds(50));
            (void)v;
        }
    });

    // 観測係：0.2秒ごとに「いま何個並んでいるか」を見に行くだけ
    std::thread watcher([&] {
        for (int i = 0; i < 8; i++) {
            std::this_thread::sleep_for(milliseconds(200));
            std::cout << "  いまキューに並んでいる数 = " << q.size() << "\n";
        }
    });

    producer.join(); consumer.join(); watcher.join();

    std::cout << "\n30個を処理するのにかかった時間 = "
              << duration_cast<milliseconds>(steady_clock::now() - t0).count() << " ms\n";
    std::cout << "キューに並んだ最大の個数 = " << q.peak() << " 個（容量は 3）\n";
    return 0;
}
