#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include "cq.h"
using namespace std::chrono;


const int NWORKER = 2;              // Infer は2人
const int END = -1;                 // これが番兵（終わりの目印）
void wait_ms(int ms) { std::this_thread::sleep_for(milliseconds(ms)); }

int main() {
    ConcurrentQueue<int> q1(4), q2(4);     // 演習5のキューのまま。close() は無い

    std::thread reader([&] {
        for (int i = 0; i < 10; i++) { wait_ms(50); q1.push(i); }
        std::cout << "Read  : 動画が終わった -> 番兵を " << NWORKER << " 個流す\n" << std::flush;
        for (int k = 0; k < NWORKER; k++) q1.push(END);   // ← 人数分いる
    });

    std::vector<std::thread> inferers;
    for (int k = 0; k < NWORKER; k++)
        inferers.emplace_back([&, k] {
            while (true) {
                int f = q1.pop();
                if (f == END) break;                     // 番兵を受け取ったら抜ける
                wait_ms(60);
                q2.push(f);
            }
            std::cout << "Infer" << k << ": 番兵を受け取った -> 番兵を1個流して終わる\n" << std::flush;
            q2.push(END);                                // 受け取った1個を下流へ渡す
        });

    std::thread shower([&] {
        int seen = 0;
        while (seen < NWORKER) {                         // 番兵を人数分そろえる
            int f = q2.pop();
            if (f == END) { seen++; continue; }
            wait_ms(30);
            std::cout << "  表示した : " << f << "\n" << std::flush;
        }
        std::cout << "Show  : 番兵が " << NWORKER << " 個そろった -> 終わる\n" << std::flush;
    });

    reader.join();
    for (auto& t : inferers) t.join();
    shower.join();
    std::cout << "\nすべてのスレッドが join() できた\n";
    return 0;
}