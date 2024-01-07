#include "ThreadPool.h"
#include "iostream"

ThreadPool::ThreadPool(int nrThreads) : end(false) {

    threads.reserve(nrThreads);
    for (int i = 0; i < nrThreads; i++) {
        threads.emplace_back([this]() {this->run();});
    }
}

void ThreadPool::enqueue(function<void()> func) {

    unique_lock<mutex> lock(mtx);
    que.push(std::move(func));
    cond_var.notify_one();
}

void ThreadPool::close() {

    unique_lock<mutex> lock(mtx);
    end = true;
    cond_var.notify_all();
}

ThreadPool::~ThreadPool() {

    close();
    for(thread& t : threads) {
        t.join();
    }
}

void ThreadPool::run() {

    while(true) {
        function<void()> execute;

        unique_lock<mutex> lock(mtx);
        while(!que.empty() && !end) {
            cond_var.wait(lock);
        }

        if (que.empty()) {
            return;
        }
        execute = std::move(que.front());
        que.pop();
        execute();
    }
}
