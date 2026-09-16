#include <iostream>
#include <iomanip>
#include <thread>
#include <vector>
#include <map>
#include <chrono>
#include "cq.h"
using namespace std::chrono;


const int N = 30;
void wait_ms(int ms) { std::this_thread::sleep_for(milliseconds(ms)); }
int infer_ms(int i) { return 20 + (i % 3) * 40; }   // 20 / 60 / 100、平均 60

struct Frame { int id; steady_clock::time_point born; };

void run(bool reorder) {
    ConcurrentQueue<Frame> q1(4), q2(4);
    std::vector<int> shown;                  // 実際に表示した順
    long long lat = 0;
    std::size_t worst_pending = 0;

    auto t0 = steady_clock::now();

    std::thread reader([&] {
        for (int i = 0; i < N; i++) { wait_ms(10); q1.push({i, steady_clock::now()}); }
    });

    std::vector<std::thread> inferers;
    for (int k = 0; k < 2; k++)
        inferers.emplace_back([&, k] {
            for (int i = k; i < N; i += 2) { Frame f = q1.pop(); wait_ms(infer_ms(f.id)); q2.push(f); }
        });

    std::thread shower([&] {
        std::map<int, Frame> pending;        // まだ順番が来ていないフレームの置き場
        int next = 0;                        // 次に出すべきフレーム番号

        for (int i = 0; i < N; i++) {
            Frame f = q2.pop();

            if (!reorder) {                  // 並べ直さない：来た順にそのまま出す
                shown.push_back(f.id);
                wait_ms(30);
                lat += duration_cast<milliseconds>(steady_clock::now() - f.born).count();
                continue;
            }

            pending[f.id] = f;                                  // いったん預かる
            if (pending.size() > worst_pending) worst_pending = pending.size();

            // 先頭が「次に出すべき番号」になっているあいだ、出し続ける
            while (!pending.empty() && pending.begin()->first == next) {
                Frame g = pending.begin()->second;
                pending.erase(pending.begin());
                shown.push_back(g.id);
                wait_ms(30);
                lat += duration_cast<milliseconds>(steady_clock::now() - g.born).count();
                next++;
            }
        }
    });

    reader.join();
    for (auto& t : inferers) t.join();
    shower.join();
    int ms = duration_cast<milliseconds>(steady_clock::now() - t0).count();

    bool ok = true;
    for (int i = 0; i < N; i++) if (shown[i] != i) ok = false;

    std::cout << (reorder ? "【並べ直しあり】" : "【並べ直しなし】") << "\n";
    std::cout << "  表示した順  : ";
    for (int v : shown) std::cout << v << " ";
    std::cout << "\n";
    std::cout << "  順番は正しいか : " << (ok ? "OK" : "NG") << "\n";
    std::cout << "  所要時間 " << ms << " ms   " << std::fixed << std::setprecision(1)
              << (1000.0 * N / ms) << " FPS   レイテンシ "
              << std::setprecision(0) << ((double)lat / N) << " ms"
              << "   手元に預かった最大数 " << worst_pending << "\n\n";
}

int main() {
    run(false);
    run(true);
    return 0;
}