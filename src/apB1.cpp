#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include "cq.h"
using namespace std::chrono;


void wait_ms(int ms) { std::this_thread::sleep_for(milliseconds(ms)); }

int main() {
    ConcurrentQueue<int> q(4);

    // 読む側：10フレームで動画が終わる（何枚あるかは、読み終わるまで分からない）
    std::thread reader([&] {
        for (int i = 0; i < 10; i++) { wait_ms(50); q.push(i); }
        std::cout << "読む側 : 動画が終わったので、読むのをやめる\n" << std::flush;
    });

    // 使う側：何枚来るか知らないので、来るかぎり処理し続ける
    std::thread worker([&] {
        while (true) {
            int f = q.pop();                       // ← 11枚目を永久に待つことになる
            std::cout << "  処理した : " << f << "\n" << std::flush;
        }
    });

    reader.join();
    std::cout << "読む側の join() は返った\n" << std::flush;
    worker.join();                                 // ← ここから先へ進めない
    std::cout << "ここには到達しない\n";
    return 0;
}