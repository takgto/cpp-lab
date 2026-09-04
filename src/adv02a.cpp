#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <atomic>
#include <vector>
using namespace std;

// push も size も、それぞれは鍵で正しく守られている
class SafeQueue {
public:
    void push(int v) {
        lock_guard<mutex> g(mtx_);
        q_.push(v);
        if (q_.size() > peak_) peak_ = q_.size();
    }
    bool pop(int& out) {
        lock_guard<mutex> g(mtx_);
        if (q_.empty()) return false;
        out = q_.front(); q_.pop();
        return true;
    }
    size_t size() const { lock_guard<mutex> g(mtx_); return q_.size(); }
    size_t peak() const { lock_guard<mutex> g(mtx_); return peak_; }
private:
    queue<int> q_;
    size_t peak_ = 0;
    mutable mutex mtx_;
};

SafeQueue q;
atomic<bool> stop{false};

// 「3個未満なら入れる」を、使う側で書いた場合
void producer() {
    for (int i = 0; i < 200000; i++) {
        if (q.size() < 3) q.push(i);        // ← 確かめてから入れる
    }
}

void consumer() {
    int v;
    while (!stop) q.pop(v);
}

int main() {
    thread c(consumer);
    vector<thread> ps;
    for (int k = 0; k < 4; k++) ps.emplace_back(producer);   // 作る係4人
    for (auto& t : ps) t.join();
    stop = true; c.join();
    cout << "「3個未満なら入れる」と書いたのに、実際に並んだ最大の個数 = "
         << q.peak() << " 個\n";
    return 0;
}