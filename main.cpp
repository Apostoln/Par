#include <iostream>
#include <vector>
#include <algorithm>
#include <thread>
#include <random>
#include <limits>
#include <functional>
#include <chrono>
#include <cmath>
#include <tuple>
#include <unordered_map>

using namespace std;

#define EXECUTION_MEASURE(...)                                                            \
    [&]{                                                                                  \
        using namespace std::chrono;                                                      \
        auto start = high_resolution_clock::now();                                        \
        __VA_ARGS__;                                                                      \
        return duration_cast<microseconds>(high_resolution_clock::now() - start).count(); \
    }()

static const int SIZE = 100000000;
static const size_t HARDWARE_THREADS = thread::hardware_concurrency();

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

template <class T>
class Test {
    private:
        Parallel parallel;
        vector<T> standartVector;
        static const long MULTIPLIER = 1000;


    public:
        Test(size_t size) {
            standartVector.resize(size);
            iota(standartVector.begin(), standartVector.end(), 0);
        }

        Test(size_t size, int i) {
            standartVector.resize(size);
            size_t coef = size / 10000; // coefficient for avoiding overflow
            random_device randomDevice;
            mt19937 generator(randomDevice());
            uniform_int_distribution<T> dist(numeric_limits<T>::min() / coef,
                                               numeric_limits<T>::max() / coef);
            generate(standartVector.begin(), standartVector.end(), [&]() {
                return dist(generator);
            });
        }

        void seqVsPar() {
            cout << "Analyse multithreading with different array size and number of threads:" << endl;

            auto simpleOperation = [&](int& value) {
                value *= MULTIPLIER;
            };

            for(int size = 10; size <= 100000; size *= 10) {
                for(int threads = 2; threads <= 10; threads++ ) {
                    vector<int> vect(standartVector.begin(), standartVector.begin() + size);
                    parallel.solve(vect, Method::PAR,  simpleOperation, threads, "Simple");
                    if(5 == threads) {
                        threads = 9;
                    }
                }
            }
        }

        void withDifferentSizeAndThreads() {
            auto simpleOperation = [&](int& value) {
                value *= MULTIPLIER;
            };

            cout << "Analyse multithreading with different array size and number of threads:" << endl;
            for(int size = 10; size <= 100000; size *= 10) {
                for(int threads = 2; threads <= 10; threads++ ) {
                    vector<int> vect(standartVector.begin(), standartVector.begin() + size);
                    parallel.solve(vect, Method::PAR,  simpleOperation, threads, "Simple");
                    if(5 == threads) {
                        threads = 9;
                    }
                }
            }
        }

        void withDifferentComplexity() {
            cout << "\nAnalyse dependency of complexity with performance:" << endl;

            int complexityCoefitient = 10;
            auto complexOperation = [&](int& value) {
                const double magicConst = 1.789;
                for (int i = 0; i < complexityCoefitient; i++) {
                    value += pow(value, magicConst);
                }
            };
            int size = 1000;
            for(; complexityCoefitient <= 100; complexityCoefitient += 10) {
                string complexString = "Complex, K="s + to_string(complexityCoefitient);

                vector<int> seqVec(standartVector.begin(), standartVector.begin() + size);
                parallel.solve(seqVec, Method::SEQ, complexOperation, 1, complexString);

                vector<int> parVec(standartVector.begin(), standartVector.begin() + size);
                parallel.solve(parVec, Method::PAR, complexOperation, HARDWARE_THREADS, complexString);
            }
        }

        void circleWithDifferentComplexity() {
            cout << "\nAnalyse dependency of complexity with performance in case of circle:" << endl;

            int complexityCoefitient = 10;
            auto complexOperation = [&](int& value) {
                const double magicConst = 1.789;
                for (int i = 0; i < complexityCoefitient; i++) {
                    value += pow(value, magicConst);
                }
            };
            int size = 1000;
            for(; complexityCoefitient <= 100; complexityCoefitient += 10) {
                string complexString = "Complex, K="s + to_string(complexityCoefitient);

                vector<int> seqVec(standartVector.begin(), standartVector.begin() + size);
                parallel.solve(seqVec, Method::SEQ, complexOperation, 1, complexString);

                vector<int> parVec(standartVector.begin(), standartVector.begin() + size);
                parallel.solve(parVec, Method::CIRCLE, complexOperation, HARDWARE_THREADS, complexString);
            }
        }
};

int main() {
    cout << "There are " << HARDWARE_THREADS << " hardware threads on this computer\n" << endl ;
    Test<int> test(SIZE);
    test.seqVsPar();
    test.withDifferentSizeAndThreads();
    test.withDifferentComplexity();
    test.circleWithDifferentComplexity();
}