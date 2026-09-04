#include <iostream>
#include <string>
#include <algorithm>
#include <thread>
#include <chrono>
using namespace std;
using namespace std::chrono;

const int SCALE = 20;                    // 図の1文字あたりの時間(ms)
steady_clock::time_point t0;
int t_begin[2], t_end[2];                // 仕事A(=0) と 仕事B(=1) の開始/終了時刻
const char MARK[2] = {'A', 'B'};

int now_ms() { return (int)duration_cast<milliseconds>(steady_clock::now() - t0).count(); }

// 300ms かかる仕事を1つこなす（id=0 が仕事A、id=1 が仕事B）
void job(int id) {
    t_begin[id] = now_ms();
    this_thread::sleep_for(milliseconds(300));
    t_end[id] = now_ms();
}
// ※ 実行中は何も表示しない。開始・終了時刻だけを記録し、最後に draw() がまとめて図を描く。
//    （複数のスレッドが同時に画面へ書き込むと表示が乱れることがあるため。詳しくは演習2で）

void draw(bool oneThread) {
    int last = max(t_end[0], t_end[1]);
    int w = last / SCALE;

    if (oneThread) {
        // スレッドは1本。その1本が仕事A→仕事Bの順にこなす
        string s(w, '.');
        for (int id = 0; id < 2; id++)
            for (int c = t_begin[id] / SCALE; c < t_end[id] / SCALE && c < w; c++) s[c] = MARK[id];
        cout << "T1   " << s << "\n";
        cout << "Total " << last << " ms\n";
    } else {
        for (int id = 0; id < 2; id++) {
            string s(w, '.');
            for (int c = t_begin[id] / SCALE; c < t_end[id] / SCALE && c < w; c++) s[c] = MARK[id];
            cout << "T" << (id + 1) << "   " << s << "\n";
        }
        int ov = min(t_end[0], t_end[1]) - max(t_begin[0], t_begin[1]);
        if (ov < 0) ov = 0;
        cout << "Total " << last << " ms   （T1 と T2 が同時に働いた時間 = " << ov << " ms）\n";
    }
}

int main() {
    cout << "図の見かた： 1文字 = 20ms（帯の長さがそのまま所要時間）\n"
         << "  T1 / T2 = スレッド（行が1本 = スレッド1本）\n"
         << "  A = 仕事A（300ms）、B = 仕事B（300ms）、'.' = 何もしていない\n";

    cout << "\n【1スレッドで順番に】   スレッドは1本だけ\n";
    t0 = steady_clock::now();
    job(0);
    job(1);
    draw(true);

    cout << "\n【2スレッドで】   スレッドは2本\n";
    t0 = steady_clock::now();
    thread t1(job, 0);
    thread t2(job, 1);
    t1.join();
    t2.join();
    draw(false);
    return 0;
}