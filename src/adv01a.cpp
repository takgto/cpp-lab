#include <iostream>
#include <thread>
#include <chrono>
using namespace std::chrono;

void job(const char* name) {
    std::this_thread::sleep_for(milliseconds(300));
    std::cout << name << " が終わった\n";
}

int main() {
    std::cout << "スレッドを2本立てる\n" << std::flush;
    std::thread t1(job, "仕事A");
    std::thread t2(job, "仕事B");
    // t1.join();  t2.join();      ← わざと消してある
    std::cout << "main が先に終わろうとしている\n" << std::flush;
    return 0;
}
