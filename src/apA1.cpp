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

// 1フレーム分のデータ。id は何番目のフレームか、box は推論結果のつもり
struct Frame { int id; int box; };

int main() {
    ConcurrentQueue<Frame> q1(4), q2(4);

    std::thread reader([&] {
        for (int i = 0; i < N; i++) { wait_ms(10); q1.push({i, -1}); }
    });

    std::vector<std::thread> inferers;                 // Infer は2人
    for (int k = 0; k < 2; k++)
        inferers.emplace_back([&, k] {
            for (int i = k; i < N; i += 2) {
                Frame f = q1.pop();
                wait_ms(infer_ms(f.id));
                f.box = f.id * 100;                    // 「そのフレームの検出結果」
                q2.push(f);
            }
        });

    std::vector<int> arrived;
    int mismatch = 0;
    std::thread shower([&] {
        // Show 側は「来た順 ＝ フレーム番号」と思い込んでいる
        for (int expected = 0; expected < N; expected++) {
            Frame f = q2.pop();
            arrived.push_back(f.id);
            if (f.id != expected) mismatch++;          // 思い込みが外れた回数
            wait_ms(30);
        }
    });

    reader.join();
    for (auto& t : inferers) t.join();
    shower.join();

    std::cout << "Show に届いた順番 :\n  ";
    for (int v : arrived) std::cout << v << " ";
    std::cout << "\n\n";

    int worst = 0;
    for (int i = 0; i < N; i++) { int d = arrived[i] - i; if (d < 0) d = -d; if (d > worst) worst = d; }
    std::cout << "順番どおりでなかったフレーム = " << mismatch << " / " << N << "\n";
    std::cout << "本来の位置からのずれ（最大） = " << worst << " 個分\n\n";

    std::cout << "Show 側が思い込みで処理すると、こうなる（最初の8フレーム）:\n";
    for (int i = 0; i < 8; i++)
        std::cout << "  " << i << " 番のフレームに、box=" << arrived[i] * 100
                  << "（" << arrived[i] << " 番の結果）を描いてしまう"
                  << (arrived[i] == i ? "" : "   <-- ずれている") << "\n";
    return 0;
}