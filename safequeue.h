#ifndef SAFEQUEUE_H
#define SAFEQUEUE_H


#include <queue>
#include <mutex>
#include <thread>
#include <iostream>
#include <condition_variable>

// A threadsafe-queue.
template <class T>
class SafeQueue
{
public:
  SafeQueue(void)
    : q()
    , m()
    , c()
  {}

  ~SafeQueue(void)
  {}

  // Add an element to the queue.
  void enqueue(T t)
  {
    std::lock_guard<std::mutex> lock(m);
    q.push(t);
    c.notify_one();
  }
// prueba de funcion tamaño
/*
  T sizequeue(std::queue<T> p){
    std::lock_guard<std::mutex> lock(m);
    T val = p.size();
    return val;
  }
*/
  // Get the "front"-element.
  // If the queue is empty, wait till a element is avaiable.
  T dequeue(void)
  {
    std::unique_lock<std::mutex> lock(m);
    while(q.empty())
    {
      // release lock as long as the wait and reaquire it afterwards.
      c.wait(lock);
    }
    T val = q.front();
    q.pop();
    return val;
  }

  bool qempty(void){
    if(!q.empty()){
      return false;
    }
    return true;
  }

private:
  std::queue<T> q;
  mutable std::mutex m;
  std::condition_variable c;
};
#endif
