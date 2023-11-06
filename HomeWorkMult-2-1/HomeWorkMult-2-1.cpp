#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <Windows.h>
#include <atomic>

std::mutex m;

void queue(std::atomic<int>& counter) {
    const int max_queue{5};
    while (counter != max_queue) {
        m.lock();
        counter++;
        HANDLE  hConsole {GetStdHandle(STD_OUTPUT_HANDLE)};
        SetConsoleTextAttribute(hConsole, 11);
        std::cout << counter << " ";
        SetConsoleTextAttribute(hConsole, 15);
        m.unlock();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void window(std::atomic<int>& counter) {
    while (counter != 0) {
        m.lock();
        counter--;
        HANDLE  hConsole {GetStdHandle(STD_OUTPUT_HANDLE)};
        SetConsoleTextAttribute(hConsole, 12);
        std::cout << counter << " ";
        SetConsoleTextAttribute(hConsole, 15);
        m.unlock();
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

int main() {
    std::atomic<int> counter{0};
    std::thread t1(queue, std::ref(counter));
    std::thread t2(window, std::ref(counter));
    t1.join();
    t2.join();
}