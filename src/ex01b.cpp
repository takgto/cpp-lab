#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
using namespace std::chrono;

// 待つ仕事：300ms 待つだけ（ファイル読み込みや外部ハードウェアの応答待ちのモデル）
void wait_job(int) { std::this_thread::sleep_for(milliseconds(300)); }

// 計算する仕事：CPU で計算し続ける（およそ300ms分）
long calc_job(int seed) {
    long s = 0;
    for (int i = 0; i < 100000000; i++) s += (seed + i) % 7;
    return s;
}

template <class Job>
void run(const char* kind, Job job, int nthread, int njob) {
    auto t0 = steady_clock::now();
    std::vector<std::thread> ts;
    for (int k = 0; k < nthread; k++)
        ts.emplace_back([=]() {          // k 番目のスレッドは k, k+n, k+2n ... 件目を担当
            for (int j = k; j < njob; j += nthread) job(j);
        });
    for (auto& t : ts) t.join();
    std::cout << kind << " " << nthread << " スレッド : "
              << duration_cast<milliseconds>(steady_clock::now() - t0).count() << " ms\n";
}

int main() {
    std::cout << "hardware_concurrency() = "
              << std::thread::hardware_concurrency() << "\n\n";
    for (int n : {1, 2, 4}) run("待つ仕事    ", wait_job, n, 4);
    std::cout << "\n";
    for (int n : {1, 2, 4}) run("計算する仕事", calc_job, n, 4);
    return 0;
}