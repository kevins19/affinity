#include <thread>
#include <vector>
#include <queue>
#include "task.hpp"

#pragma once

namespace affinity {

// Dispatcher is a core-specific scheduler, which receives batched tasks from the pool and distributes them to the threads.
// Ensure to allocate dispatchers on its own numa node, for demonstration purposes.
template<size_t Nt>
class dispatcher {
public:
    dispatcher() {
        for(size_t i = 0; i < Nt; ++i) {
            workers[i] = std::thread([this]() {
                while(true) {
                    std::unique_lock lock(queue_mtx);
                    queue_cv.wait(lock, [this]() { return stop || !queue.empty(); });
                    if(stop && queue.empty()) {
                        break;
                    }
                    task t = std::move(queue.front());
                    queue.pop();
                    lock.unlock();
                    t();
                }   
            });
        }
    }
    ~dispatcher() {
        {
            std::unique_lock lock(queue_mtx);
            stop = true;
        }
        queue_cv.notify_all();
        for(auto& worker : workers) {
            worker.join();
        }
    }

    void add_tasks(std::vector<task>& tasks) {
        std::unique_lock lock(queue_mtx);
        for(auto& t : tasks) {
            queue.push(std::move(t));
        }
        lock.unlock();
        queue_cv.notify_all();
    }

    size_t count() {
        std::unique_lock lock(queue_mtx);
        return queue.size();
    }
    
private:
    bool stop = false;
    std::queue<task> queue;
    std::array<std::thread, Nt> workers;
    std::mutex queue_mtx;
    std::condition_variable queue_cv;

};

}