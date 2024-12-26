#include <atomic>
#include <array>
#include <memory>
#include "task.hpp"
#include "dispatcher.hpp"
#pragma once    

#define MAX_THREADS_PER_CORE 16

namespace affinity {

// The pool is an over-arching supervisor which distributes batched tasks to numa node-specific dispatchers.
// Thus, numa nodes act as if they each have their own thread-pool (embodied by `dispatcher`).
// Parameterized by number of cores (`Nc`) and number of threads (`Nt`).
template <size_t Nc, size_t Nt>
class pool {
public:
    pool(size_t batch_size) : batch_size(batch_size) {}
    
    ~pool() {
        submit_batched_tasks();
        // thread termination handled automatically by dispatcher's destructor
    }

    void submit_task(task&& t) {
        std::lock_guard<std::mutex> lock(tasks_mutex);
        tasks.push_back(std::move(t));
        if(tasks.size() >= batch_size) {
            submit_batched_tasks();
        }
    }

private:
    size_t batch_size;
    std::array<dispatcher<Nt>, Nc> dispatchers;  
    std::vector<task> tasks;
    std::mutex tasks_mutex;

    void submit_batched_tasks() {
        if(tasks.empty()) return;

        size_t best_core = 0;
        size_t best_count = dispatchers[0].count();
        for(size_t i = 1; i < Nc; ++i) {
            size_t count = dispatchers[i].count();
            if(count < best_count) {
                best_core = i;
                best_count = count;
            }
        }
        dispatchers[best_core].add_tasks(tasks);
        tasks.clear();
    }
};

}
