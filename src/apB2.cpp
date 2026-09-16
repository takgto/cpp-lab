#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
using namespace std::chrono;

// ---- 演習5のキューに「閉じる」を足したもの ----
template <typename T>
class ClosableQueue {
public:
    explicit ClosableQueue(std::size_t capacity) : capacity_(capacity) {}

    // 入れる。閉じられていたら何もせず false
    bool push(const T& v) {
        std::unique_lock<std::mutex> lk(mtx_);
        can_push_.wait(lk, [this] { return q_.size() < capacity_ || closed_; });
        if (closed_) return false;
        q_.push(v);
        lk.unlock(); can_pop_.notify_one();
        return true;
    }

    // 取り出す。取れたら true。閉じられていて、かつ空なら false
    bool pop(T& out) {
        std::unique_lock<std::mutex> lk(mtx_);
        can_pop_.wait(lk, [this] { return !q_.empty() || closed_; });
        if (q_.empty()) return false;              // 閉じていて、もう残っていない
        out = q_.front(); q_.pop();
        lk.unlock(); can_push_.notify_one();
        return true;
    }

    // もう入れない、と宣言する
    void close() {
        { std::lock_guard<std::mutex> g(mtx_); closed_ = true; }
        can_pop_.notify_all();                     // 待っている全員を起こす
        can_push_.notify_all();
    }

private:
    std::queue<T> q_;
    std::size_t capacity_;
    bool closed_ = false;
    mutable std::mutex mtx_;
    std::condition_variable can_pop_, can_push_;
};

void wait_ms(int ms) { std::this_thread::sleep_for(milliseconds(ms)); }

int main() {
    ClosableQueue<int> q1(4), q2(4);

    std::thread reader([&] {
        for (int i = 0; i < 10; i++) { wait_ms(50); q1.push(i); }
        std::cout << "Read  : 動画が終わった -> q1 を閉じる\n" << std::flush;
        q1.close();                                // ← ここが要
    });

    std::thread inferer([&] {
        int f;
        while (q1.pop(f)) {                        // false が返ったら「もう来ない」
            wait_ms(60);
            q2.push(f);
        }
        std::cout << "Infer : q1 が閉じた -> q2 を閉じる\n" << std::flush;
        q2.close();                                // ← 終了を下流へ伝える
    });

    std::thread shower([&] {
        int f;
        while (q2.pop(f)) { wait_ms(30); std::cout << "  表示した : " << f << "\n" << std::flush; }
        std::cout << "Show  : q2 が閉じた -> 終わる\n" << std::flush;
    });

    reader.join(); inferer.join(); shower.join();
    std::cout << "\nすべてのスレッドが join() できた\n";
    return 0;
}