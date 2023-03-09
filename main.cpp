#include <iostream>
#include <thread>
#include <chrono>
#include <queue>
#include <mutex>
#include <condition_variable>

using namespace std;

void computeSquare(int num, int thread_id) {
    cout << "Working from thead " << thread_id << " ..." << endl;
    this_thread::sleep_for(2000ms);
    int square = num * num;
    cout << "Square of " << num << " is " << square << " (thread_id: " << thread_id << ")" << endl;
}

class ThreadPool {
public:
    ThreadPool(int num_threads) : stop(false) {
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([this, i]() {
                while (true) {
                    int num;
                    int thread_id = i + 1;
                    { // the local scope ensure that the mutex will be released
                      // even if the computeSquare method throw

                        // acquire lock to be sure to modify
                        // the queue safely.
                        unique_lock<mutex> lock(m);

                        // Wait to be awaken
                        cv.wait(lock, [this]() { return !q.empty() || stop; });
                        
                        // return if the queue is empty or
                        // stop is true
                        if (stop && q.empty()) return;

                        // get the first element in the queue
                        // and use it
                        num = q.front();
                        q.pop();
                    }
                    computeSquare(num, thread_id);
                }
            });
        }
    }
    
    ~ThreadPool() {
        {
            // Acquire a lock then we are that
            // stop won't be modified by another thread.
            unique_lock<mutex> lock(m);
            stop = true;
        }
        
        // Will force all thread to leave their loop
        cv.notify_all();

        // Ensure that all worker threads have completed their processing  
        for (auto& thread : threads) {
            thread.join();
        }
    }
    
    void enqueue(int num) {
        // Acquire a lock then we are sure that
        // the queue cannot be modified by another thread.
        unique_lock<mutex> lock(m);

        // Wait until there are space on the queue. 
        cv.wait(lock, [this]() { return q.size() < 4; });

        // Add the new integer to the queue.
        q.push(num);

        // Awake a thread 
        cv.notify_one();
    }
    
private:
    queue<int> q;
    vector<thread> threads;
    mutex m;
    condition_variable cv;
    bool stop;
};

int main() {
    ThreadPool pool(4);
    cout << "Type a number and enter:" << endl;
    cout << "(0 to exit)" << endl;

    while (true) {
        int num;
        cin >> num;
        if (num == 0) {
          cout << "Exit..." << endl;
          break;
        }
        pool.enqueue(num);
    }

    return 0;
}

