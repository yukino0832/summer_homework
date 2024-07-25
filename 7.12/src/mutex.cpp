#include <iostream>
#include <vector>

#include <thread>

#include <mutex> // 互斥锁

int main(int argc, char **argv)
{
    long long a = 0;
    int tread_count = std::stoi(argv[1]);
    std::vector<std::thread> threads;
    std::mutex mtx;

    for (int i = 0; i < tread_count; i++) {
        threads.push_back(std::thread([&a, tread_count, &mtx]() {
            mtx.lock();
            for (int i = 0; i < 1e8 / tread_count; i++) {
                a++;
            }
            mtx.unlock();
            std::cout << "thread exit!" << std::endl;
        }));
    }
    for (auto &thread : threads) {
        thread.join();
    }
    std::cout << a << std::endl;
}