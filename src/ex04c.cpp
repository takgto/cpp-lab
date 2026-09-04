#include <iostream>
#include <iomanip>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
using namespace std::chrono;

using TP = steady_clock::time_point;
static inline TP now_() { return steady_clock::now(); }

// ============ 計測の道具 ============ 実行中は足し込むだけ。表示は最後に1回だけ
class Profiler {
public:
    int stage(const char* name, bool ext) {         // ext = 外部（コアを手放している）段
        st_.push_back({name, ext, 0, 0});
        return (int)st_.size() - 1;
    }
    void lap(int id, TP& t) {                       // t から今までを段 id に足し、t を進める
        TP n = now_();
        long long us = duration_cast<microseconds>(n - t).count();
        t = n;
        if (warm_) return;                          // 立ち上がりは捨てる
        st_[id].sum += us;
        if (us > st_[id].max) st_[id].max = us;
        frame_ += us;                               // 検算用の合計
    }
    void frame_end(TP fstart) {
        long long total = duration_cast<microseconds>(now_() - fstart).count();
        if (warm_) { if (++skipped_ >= WARMUP) { warm_ = false; t0_ = now_(); } return; }
        other_ += (total > frame_ ? total - frame_ : 0);   // 測り漏らし ＝ 計測外
        frame_ = 0;
        frames_++;
    }
    void report() const {
        double sec = duration_cast<microseconds>(now_() - t0_).count() / 1e6;
        std::cout << std::fixed << std::setprecision(1)
                  << frames_ << " 個ぶん（先頭 " << WARMUP << " 個は捨てた）   "
                  << (frames_ / sec) << " FPS\n\n"
                  << "    平均     最大   種類  段\n";
        for (const auto& s : st_)
            std::cout << std::setw(8) << (s.sum / 1000.0 / frames_) << "ms"
                      << std::setw(7) << (s.max / 1000.0) << "ms"
                      << (s.ext ? "   外部  " : "   CPU   ") << s.name << "\n";
        std::cout << std::setw(8) << (other_ / 1000.0 / frames_) << "ms"
                  << std::setw(9) << "-" << "         計測外\n";
    }
private:
    struct S { const char* name; bool ext; long long sum, max; };
    static const int WARMUP = 3;
    std::vector<S> st_;
    long long frame_ = 0, other_ = 0;
    int frames_ = 0, skipped_ = 0;
    bool warm_ = true;
    TP t0_ = now_();
};

// ============ 仕事の中身（推論の軽いモデルを使った画像処理を模したもの） ============
long calib = 0;
std::atomic<long> sink{0};        // 最適化で消されないための置き場（発展課題4-1 では複数スレッドから足す）
long burn(long n) { long s = 0; for (long i = 0; i < n; i++) s += (i * 2654435761u) % 7; return s; }
void cpu_ms(int ms)  { sink.fetch_add(burn(calib * ms), std::memory_order_relaxed); }   // CPU が計算する
void ext_ms(int ms)  { std::this_thread::sleep_for(milliseconds(ms)); }                      // 外部にまかせている

void calibrate() {
    long n = 100000;
    for (;;) {
        TP t = now_(); sink.fetch_add(burn(n), std::memory_order_relaxed);
        auto us = duration_cast<microseconds>(now_() - t).count();
        if (us > 30000) { calib = n * 1000 / us; break; }
        n *= 2;
    }
}

int main() {
    calibrate();
    Profiler p;
    int READ = p.stage("read (デコード)", false);
    int PRE  = p.stage("pre  (前処理)",   false);
    int DPU  = p.stage("dpu  (推論)",     true);      // 投げたあとはアクセラレータの仕事
    int POST = p.stage("post (後処理)",   false);
    int SHOW = p.stage("show (表示)",     true);

    for (int i = 0; i < 33; i++) {
        TP t = now_(), fstart = t;
        cpu_ms(13);                       p.lap(READ, t);
        cpu_ms(9);                        p.lap(PRE,  t);
        ext_ms(2);                        p.lap(DPU,  t);
        cpu_ms(2);                        p.lap(POST, t);
        ext_ms(i % 10 == 0 ? 60 : 30);    p.lap(SHOW, t);
        p.frame_end(fstart);
    }
    p.report();
    return 0;
}
