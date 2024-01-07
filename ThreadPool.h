#ifndef PDP_LAB3_THREADPOOL_H
#define PDP_LAB3_THREADPOOL_H

#include "mutex"
#include "condition_variable"
#include "functional"
#include "queue"
#include "vector"
#include "thread"

using namespace std;

class ThreadPool {
public:
    explicit ThreadPool(int nrThreads);
    void enqueue(function<void()> func);
    void close();
    ~ThreadPool();

private:
    mutex mtx;
    condition_variable cond_var;
    queue<function<void()>> que;
    bool end;
    vector<thread> threads;

    void run();
};


#endif
