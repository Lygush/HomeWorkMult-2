#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <random>
#include <exception>
#include <queue>
#include <Windows.h>
#include <list>

std::mutex m;
const int elem{ 100000 };
const int cores{ 6 };
const int section{ 20 };

std::queue<size_t> update_buffer;

class Thread_info {
    int number{};
    size_t id{};
    std::list<char> progress_bar{};
    size_t time{};
    bool live{};
public:
    Thread_info(int number, size_t id, bool live) : number(number), id(id), live(live) {
        for (int i{}; i < section; ++i) {
            progress_bar.push_back(32);
        }
    }

    void add_progress() {
        progress_bar.push_front(char(219));
        progress_bar.pop_back();
    }

    int get_number() {
        return this->number;
    }

    size_t get_id() {
        return this->id;
    }

    std::list<char> get_progress() {
        return this->progress_bar;
    }

    size_t get_time() {
        return this->time;
    }

    bool get_live() {
        return this->live;
    }

    void change_live() {
        this->live = false;
    }

    void change_time(size_t now) {
        time = now;
    }
};

std::vector<Thread_info> memory;

void calculation(std::vector<int>& vec, int index) {
    m.lock();
    int count{};
    for (int i{}; i < memory.size(); ++i) {
        count++;
    }
    size_t id{ std::hash<std::thread::id>{}(std::this_thread::get_id()) };
    Thread_info temp(count, id, true);
    memory.push_back(temp);
    m.unlock();
    int counter{};
    std::mt19937 gen;
    std::uniform_int_distribution<int> dist(0, 32767);
    srand( dist(gen));
    while (index < elem) {
        int temp{ dist(gen) };
        while (vec.at(index) != temp) {
        temp = dist(gen);
        }
        if (counter == (elem/cores)/section) {
            m.lock();
            update_buffer.push(id);
            m.unlock();
            counter = 0;
        }
        else {
            counter++;
        }
        index += cores;
    }
    memory.at(count).change_live();
}

void setcur(int x, int y)
{
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
};

void print() {
    setcur(0, 0);
    std::cout << "#\t" << "id\t" << "Progress Bar        " << "time\n";
    for (auto& p : memory) {
        std::cout << p.get_number() << "  " << p.get_id() << "\t";
        auto list = p.get_progress();
        for (auto& l : list) {
            std::cout << char(l);
        }
        double time{ static_cast<double>(p.get_time()) };
        std::cout << time / 1000 << " s ";
        std::cout << "\n";
    }
}

void watch() {
    auto start = std::chrono::steady_clock::now();
    print();
    while (true) {
        if (!update_buffer.empty()) {
            for (auto& i : memory) {
                if (update_buffer.front() == i.get_id()) {
                    i.add_progress();
                    m.lock();
                    update_buffer.pop();
                    m.unlock();
                    break;
                }
            }
        }
        int dead_threads{};
        for (auto& i : memory) {
            if (i.get_live()) {
                auto end = std::chrono::steady_clock::now();
                auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                i.change_time(static_cast<size_t>(diff.count()));
            }
            else {
                dead_threads++;
            }
        }
        if (dead_threads == cores) {
            print();
            break;
        }
        print();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
     }

int main() {
    system("cls");
    srand(time(NULL));
    std::vector<int> vec;
    for (int i{}; i < elem; ++i) {
        vec.push_back(rand());
    }
    std::vector<std::thread> threads;
    for (int i{}; i < cores; ++i) {
        threads.push_back(std::thread(calculation, std::ref(vec), i));
    }
    std::thread print_th(watch);
    for (auto& i : threads) {
        i.join();
    }
    print_th.join();
}