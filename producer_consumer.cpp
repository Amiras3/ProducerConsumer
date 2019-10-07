#include <iostream>
#include <condition_variable>
#include <thread>
#include <mutex>
#include <queue>
#include <type_traits>

using namespace std;
 
template<typename T>
class blockingQueue
{
    const int capacity;
    queue<T> q;
    mutex m;
    condition_variable cv_producer, cv_consumer;
 
public:
    blockingQueue(): capacity{1} {}
    blockingQueue(int capacity): capacity{capacity} {}
 
    template<typename U>
    void push(U&& u) {
        {
            unique_lock<mutex> lock(m);
            cv_producer.wait(lock, [this]() { return q.size() < capacity; });
            q.push(forward<U>(u));
        }
        cv_consumer.notify_one();      
    }
 
    T pop() {
        T t{};
        {
            unique_lock<mutex> lock(m);
            cv_consumer.wait(lock, [this]() { return q.size() > 0; });
            t = q.front();
            q.pop();
        }
        cv_producer.notify_one();
        return t;  
    }
};

int main() {
    blockingQueue<int> q(10);
    thread t([&]() {for(int i = 0; i < 10; ++i) { q.push(i); }});
    thread t1([&]() {for(int i = 0; i < 10; ++i) { cout << q.pop() << endl; }});
    thread t2([&]() {for(int i = 0; i < 10; ++i) { cout << q.pop() << endl; }});
    thread t3([&]() {for(int i = 0; i < 10; ++i) { q.push(i); }});
    t.join();
    t1.join();
    t2.join();
    t3.join();
    return 0;
}
