#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <random>
#include <future>

#include "affinity.hpp"

int main() {
    affinity::pool<1,1> pool(3);
    std::vector<std::future<size_t>> results(100);

    for(size_t i = 0; i < 12; ++i) {
        auto promise = std::promise<size_t>();
        results[i] = promise.get_future();
        auto func = [](size_t x) -> size_t {
            std::cout << "Running task " << x << std::endl;
            return x;
        };
        auto task = affinity::wrap_task(std::move(promise), std::ref(func), i);
        pool.submit_task(std::move(task));
    }

    for(int i=0; i<12; i++){
        std::cout << results[i].get() << " Completed!" << std::endl;
    }
}
