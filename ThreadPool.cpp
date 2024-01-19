#include "ThreadPool.h"
#include "iostream"

ThreadPool::ThreadPool(int nrThreads) {
    end = false;
    for (int i = 0; i < nrThreads; i++) {
        threads.emplace_back(&ThreadPool::run, this);
    }
}

ThreadPool::~ThreadPool() {
    close();
}

void ThreadPool::enqueue(const function<void()>& func) {
    {
        std::lock_guard<mutex> lock{mtx};
        que.push(func);
    }
    cond_var.notify_one();
}

void ThreadPool::close() {
    if (end) {
        return;
    }

    end = true;
    cond_var.notify_all();
    while (!threads.empty()) {
        auto& thread = threads.back();
        thread.join();
        threads.pop_back();
    }
}

void ThreadPool::run() {
    while (true) {
        unique_lock<mutex> lock{mtx};
        while (que.empty() && !end) {
            cond_var.wait(lock);
        }

        if (end && que.empty()) {
            return;
        }

        function<void()> execute = que.front();
        que.pop();

        lock.unlock();

        execute();
    }
}
