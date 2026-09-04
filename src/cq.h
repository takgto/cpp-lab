// ============================================================
//  cq.h ―― この演習で使うスレッドセーフなキュー
//  中身は読まなくてよい。使うのは push / pop / size の3つだけ。
// ============================================================
#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class ConcurrentQueue {
public:
    explicit ConcurrentQueue(std::size_t capacity) : capacity_(capacity) {}

    // 入れる。満杯なら空くまで待つ
    void push(const T& v) {
        std::unique_lock<std::mutex> lk(mtx_);
        can_push_.wait(lk, [this] { return q_.size() < capacity_; });
        q_.push(v);
        if (q_.size() > peak_) peak_ = q_.size();
        lk.unlock();
        can_pop_.notify_one();
    }

    // 取り出す。空なら来るまで待つ（失敗しない）
    T pop() {
        std::unique_lock<std::mutex> lk(mtx_);
        can_pop_.wait(lk, [this] { return !q_.empty(); });
        T v = q_.front(); q_.pop();
        lk.unlock();
        can_push_.notify_one();
        return v;
    }

    std::size_t size() const { std::lock_guard<std::mutex> g(mtx_); return q_.size(); }
    std::size_t peak() const { std::lock_guard<std::mutex> g(mtx_); return peak_; }   // 観測用：並んだ最大数

private:
    std::queue<T> q_;
    std::size_t capacity_;
    std::size_t peak_ = 0;
    mutable std::mutex mtx_;
    std::condition_variable can_pop_;    // 「取り出せるようになった」
    std::condition_variable can_push_;   // 「入れられるようになった」
};
