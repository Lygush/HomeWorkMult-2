#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <memory>

class Data {
    std::mutex m;
    std::string word{};
public:
    Data(std::string word) : word(word) {};

    std::mutex& get_mutex() {
        return this->m;
    }

    std::string get_value() {
        return word;
    }

    Data&& operator=(Data & other) {
        this->word = other.word;
        return std::move(*this);
    }

    Data(Data&& other) {
        this->word = other.word ;
    }
};

void print(Data& d1, Data& d2) {
    std::cout << d1.get_value() << " " << d2.get_value() << "\n";
}

void swap(Data& first, Data& second) {
    first.get_mutex().lock();
    second.get_mutex().lock();
    Data temp = std::move(first);
    first = std::move(second);
    second = std::move(temp);
    print(first, second);
    first.get_mutex().unlock();
    second.get_mutex().unlock();
}

void swap_ul(Data& first, Data& second) {
    std::unique_lock<std::mutex> lk1(first.get_mutex());
    std::unique_lock<std::mutex> lk2(second.get_mutex());
    Data temp = std::move(first);
    first = std::move(second);
    second = std::move(temp);
    print(first, second);
}

void swap_sl(Data& first, Data& second) {
    std::scoped_lock<std::mutex> sl1(first.get_mutex());
    std::scoped_lock<std::mutex> sl2(second.get_mutex());
    Data temp = std::move(first);
    first = std::move(second);
    second = std::move(temp);
    print(first, second);
}


int main() {
    Data d1("world");
    Data d2("Hellow");
    std::vector<std::thread> th;
    th.push_back(std::thread(swap, std::ref(d1), std::ref(d2)));
    th.push_back(std::thread(swap_ul, std::ref(d1), std::ref(d2)));
    th.push_back(std::thread(swap_sl, std::ref(d1), std::ref(d2)));
    for (auto& i: th) {
        i.join();
    }
}