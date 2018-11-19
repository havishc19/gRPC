#pragma once

#ifndef CONCURRENT_THREADPOOL_H
#define CONCURRENT_THREADPOOL_H

#include <atomic>
#include <thread>
#include <mutex>
#include <vector>
#include <list>
#include <iostream>
#include <functional>
#include <condition_variable>

using namespace std;

class threadpool{
    public:
        int numThreads = thread::hardware_concurrency();
        vector<thread> pool;
        list<function<void(void)>> queue;
        mutex queue_mutex;
        condition_variable jobCondition;

        void setThreadPool(int numThreads1){
            if(numThreads1 <= numThreads)
                numThreads = numThreads1;
            for(int i = 0; i < numThreads; i++)
            {  
                pool.push_back(thread([this,i]{ this->threadWaitFunc(); }));
            }
        }

        ~threadpool(){
            for(int i=0; i < numThreads; i++){
                if(pool[i].joinable()){
                    pool[i].join();
                }
            }
        }

        void threadWaitFunc()
        {
            function<void(void)> job;
            while(true)
            {
                {
                    unique_lock<mutex> lock(queue_mutex);
                    jobCondition.wait(lock, [this]{return !queue.empty();});
                    job = queue.front();
                    queue.pop_front();
                }
                job();
            }
        }

        void addJob(function<void(void)> newJob)
        {
            {
                unique_lock<mutex> lock(queue_mutex);
                queue.push_back(newJob);
            }
            jobCondition.notify_one();
        }
};


#endif //CONCURRENT_THREADPOOL_H
