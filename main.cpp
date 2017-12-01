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
static const int SIZE = 100000000;
static long MULTIPLIER = 100;

#define EXECUTION_MEASURE(...)                                                            \
    [&]{                                                                                  \
        using namespace std::chrono;                                                      \
        auto start = high_resolution_clock::now();                                        \
        __VA_ARGS__;                                                                      \
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
        thread tempThread([=](auto it) {
            auto limit = (it + sliceSize);
            limit = limit < end? limit : end;
            for_each(it, limit, functor);
        }, iter);
        threads.push_back(std::move(tempThread));
    }
    for (auto& val: threads) {
        val.join();
    }
}

int main() {
    random_device randomDevice;
    mt19937 generator(randomDevice());
    uniform_int_distribution<int> dist(numeric_limits<int>::min() / COEF,
                                       numeric_limits<int>::max() / COEF);

    vector<int> vectorSeq(SIZE);
    generate(begin(vectorSeq), end(vectorSeq), [&]() {
        return dist(generator);
    });
    auto vectorPar = vectorSeq;

    auto stuff = [&](int& value){
        value *= MULTIPLIER;
    };

    cout << "Sequential algorithm " << "(Size=" << SIZE << ", Multiplier=" << MULTIPLIER << "): "
         << EXECUTION_MEASURE(mutateSequentual(vectorSeq, stuff)) << " microseconds. " << endl;
    cout << "Parallel algorithm " << "(Size=" << SIZE << ", Multiplier=" << MULTIPLIER << "): "
         << EXECUTION_MEASURE(mutateParallel(vectorPar, stuff)) << " microseconds. " << endl;

    return 0;
}