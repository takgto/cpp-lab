#include <iostream>
#include <thread>
#include <vector>

const int NMAX = 200000;

// ---- ① 共有カウンタを、守らずに増やす ----
long counter = 0;
void add_many() { for (int i = 0; i < 1000000; i++) counter++; }      // 守っていない

// ---- ② 共有の「次に取る番号」を、守らずに2人で進める ----
int idx = 0;                          // 次に取り出す番号（共有）
std::vector<int> got(NMAX, 0);        // 何番を何回取ったか
void worker() {
    while (true) {
        if (idx >= NMAX) return;      // ← まだ残っているか確かめて…
        int i = idx;                  // ← 自分の分として取って…
        idx++;                        // ← 番号を1つ進める
        got[i]++;                     //    この3行の「すきま」が危ない
    }
}

int main() {
    std::thread a1(add_many), a2(add_many);
    a1.join(); a2.join();
    std::cout << "① counter = " << counter << "   （期待値 2000000）\n" << std::flush;

    std::thread w1(worker), w2(worker);
    w1.join(); w2.join();
    int dup = 0, lost = 0;
    for (int v : got) { if (v > 1) dup++; if (v == 0) lost++; }
    std::cout << "② 2人が同じ番号を取ってしまった回数 = " << dup << "\n"
              << "   誰も取らなかった番号の数         = " << lost << "\n";
    return 0;
}
