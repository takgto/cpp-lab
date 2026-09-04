#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
using namespace std;

enum Mode { TWO_CV, ONE_CV_ALL, ONE_CV_ONE };

template <typename T>
class Q {
public:
    Q(size_t cap, Mode m) : capacity_(cap), mode_(m) {}

    void push(const T& v) {
        unique_lock<mutex> lk(mtx_);
        cv_push().wait(lk, [this] { woke_++; return q_.size() < capacity_; });
        q_.push(v);
        lk.unlock();
        wake(cv_pop());
    }
    T pop() {
        unique_lock<mutex> lk(mtx_);
        cv_pop().wait(lk, [this] { woke_++; return !q_.empty(); });
        T v = q_.front(); q_.pop();
        lk.unlock();
        wake(cv_push());
        return v;
    }
    long woke() const { return woke_; }

private:
    // 2本モードでは別々の条件変数、1本モードでは同じものを返す
    condition_variable& cv_pop()  { return a_; }
    condition_variable& cv_push() { return (mode_ == TWO_CV) ? b_ : a_; }
    void wake(condition_variable& cv) {
        if (mode_ == ONE_CV_ALL) cv.notify_all(); else cv.notify_one();
    }
    queue<T> q_;
    size_t capacity_;
    Mode mode_;
    long woke_ = 0;
    mutex mtx_;
    condition_variable a_, b_;
};

void run(Mode m, const char* label) {
    Q<int> q(1, m);                       // 容量1、作る係2人、受け取る係1人
    cout << label << flush;
    vector<thread> ts;
    ts.emplace_back([&] { for (int i = 0; i < 10; i++) q.push(i); });
    ts.emplace_back([&] { for (int i = 0; i < 10; i++) q.push(i); });
    ts.emplace_back([&] { for (int i = 0; i < 20; i++) q.pop(); });
    for (auto& t : ts) t.join();
    cout << " 20個すべて処理できた。述語を評価した回数 = " << q.woke() << "\n" << flush;
}

int main() {
    run(TWO_CV,     "【条件変数2本 + notify_one（正しい形）】");
    run(ONE_CV_ALL, "【条件変数1本 + notify_all      】");
    run(ONE_CV_ONE, "【条件変数1本 + notify_one      】");
    cout << "ここには到達しない\n";
    return 0;
}