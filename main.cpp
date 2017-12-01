#include <iostream>
#include <vector>
#include <algorithm>
#include <thread>
#include <random>
#include <limits>
#include <functional>
#include <chrono>
#include <cmath>

using namespace std;

static const short COEF = 1000; // coefficient for avoiding overflow
static const int SIZE = 1000000;
static long MULTIPLIER = 100;

#define EXECUTION_MEASURE(...)                                                           \
    [&]{                                                                                 \
        using namespace std::chrono;                                                     \
        auto start = high_resolution_clock::now();                                       \
        __VA_ARGS__;                                                                     \
        return duration_cast<microseconds>(high_resolution_clock::now() - start).count(); \
    }()

template <class T, class F>
void mutateSequentual(vector<T> &v, F functor) {
    std::for_each(begin(v), end(v), functor);
}

template <class T, class F = function<void(T)> >
void mutateParallel(vector<T> &values, F functor, size_t threadNumber = std::thread::hardware_concurrency()) {
    size_t sliceSize = values.size() / threadNumber;
    vector<thread> threads;
    for (auto iter = values.begin(), end = values.end(); iter < end; iter += sliceSize) {
        threads.push_back(thread([=](auto it) {
            auto leftLimit = it;
            auto rightLimit = (it + sliceSize) < end? it + sliceSize : end ;
            for_each(leftLimit, rightLimit, functor);
        }, iter));
    }
    for (auto& val: threads) {
        val.join();
    }
}

int main() {
    random_device randomDevice;
    mt19937 generator(randomDevice());
    uniform_int_distribution<int> dist(numeric_limits<int>::min()/COEF,
                                       numeric_limits<int>::max()/COEF);

    vector<int> vectorSeq(SIZE);
    generate(begin(vectorSeq), end(vectorSeq), [&]() {
        return dist(generator);
    });
    auto vectorPar = vectorSeq;

    auto stuff = [&](int& value){
        value *= MULTIPLIER;
    };

    cout << EXECUTION_MEASURE(mutateSequentual(vectorSeq, stuff)) << endl;
    cout << EXECUTION_MEASURE(mutateParallel(vectorPar, stuff)) << endl;

    return 0;
}