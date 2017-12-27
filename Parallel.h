#ifndef PARALLELISM_PARALLEL_HPP
#define PARALLELISM_PARALLEL_HPP

#include <iostream>
#include <vector>
#include <functional>
#include <unordered_map>

using namespace std;

#define EXECUTION_MEASURE(...)                                                            \
    [&]{                                                                                  \
        using namespace std::chrono;                                                      \
        auto start = high_resolution_clock::now();                                        \
        __VA_ARGS__;                                                                      \
        return duration_cast<microseconds>(high_resolution_clock::now() - start).count(); \
    }()

enum class Method { SEQ, PAR, CIRCLE };

class Parallel {
protected:
    unordered_map<Method, string> METHODS = {
            {Method::SEQ, "Sequential"},
            {Method::PAR, "Parallel"},
            {Method::CIRCLE, "Circle"}
    };

public:
    template <class T, class F>
    void mutateSequentual(vector<T> &v, F functor) {
        std::for_each(v.begin(), v.end(), functor);
    }

    template <class T, class F = function<void(T)> >
    void mutateParallel(vector<T> &values, F functor, size_t threadNumber) {
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
        for (auto& thr: threads) {
            thr.join();
        }
    }

    template <class T, class F>
    void mutateCircle(vector<T> &values, F functor, size_t threadNumber) {
        vector<vector<reference_wrapper<T>>> subVectors(threadNumber);
        for(int valueIndex = 0, currentSubVectorIndex = 0; valueIndex  < values.size(); ++valueIndex) {
            subVectors[currentSubVectorIndex].push_back(ref(values[valueIndex]));
            ++currentSubVectorIndex;
            if (currentSubVectorIndex == threadNumber ) {
                currentSubVectorIndex = 0;
            }
        }

        vector<thread> threads;
        for(auto& subVector: subVectors) {
            thread tempThread([&](){
                for_each(subVector.begin(), subVector.end(), functor);
            });
            threads.push_back(std::move(tempThread));
        }
        for(auto& thr: threads) {
            thr.join();
        }
    }

    template<class T, class F>
    void solve(vector<T> vect, Method method, F stuff, size_t threadsNumber, string operation) {
        string methodName = METHODS[method];
        long result = 0;
        switch (method) {
            case Method::SEQ:
                result = EXECUTION_MEASURE(mutateSequentual(vect, stuff));
                break;
            case Method::PAR:
                result = EXECUTION_MEASURE(mutateParallel(vect, stuff, threadsNumber));
                break;
            case Method::CIRCLE:
                result = EXECUTION_MEASURE(mutateCircle(vect, stuff, threadsNumber));
                break;
        }
        cout << methodName << " algorithm " << "(Size=" << vect.size() << ", Threads=" << threadsNumber
             << ", Operation=" << operation << "): " << result << " microseconds. " << endl;
    }

};

#endif //PARALLELISM_PARALLEL_HPP
