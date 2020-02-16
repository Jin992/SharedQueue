//
// Created by jin on 2/14/20.
//

#ifndef SHAREDQUEUE_SHAREDQUEUE_H
#define SHAREDQUEUE_SHAREDQUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <iostream>
#include <chrono>
#include <thread>

#define DURATION(start, end) std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()

using std::chrono::milliseconds;
using std::chrono::steady_clock;

template <typename  T> class SharedQueue {
public:
    explicit SharedQueue(size_t size);

    int count() const;
    void enqueue(T* item);
    bool enqueue(T* item, int millisecondsTimeout);
    T* dequeue();
    T* dequeue(int millisecondsTimeout);

private:
    // push item to queue
    void _put_to_collection(T* item);
    T* _get_from_collection();

private:
    std::queue<T*>              _collection;
    std::mutex                  _mutex;
    std::condition_variable     _cond;
    std::atomic<int>            _cnt;
    const size_t                _capacity;
};


template <typename T>
SharedQueue<T>::SharedQueue(size_t size)
: _cnt(0), _capacity(size)
{}

template <typename T>
int SharedQueue<T>::count() const {
    return _cnt.load();
}

template <typename T>
void SharedQueue<T>::enqueue(T* item) {
    if (item == nullptr)
        throw std::runtime_error("Nullptr given as argument.");
    // if cnt bigger or equal to capacity yield the thread
    while(_cnt >= _capacity)
        std::this_thread::yield();
    // local scope for lock_guard
    _put_to_collection(item);
}

template <typename T>
bool SharedQueue<T>::enqueue(T* item, int millisecondsTimeout) {
    if (item == nullptr)
        throw std::runtime_error("Nullptr given as argument.");
    // get current timestamp
    auto start = steady_clock::now();
    // if cnt bigger or equal to capacity yield the thread
    while(_cnt >= _capacity) {
        // if timeout expired return false
        if (DURATION(start, steady_clock::now()) >= millisecondsTimeout)
            return false;
        std::this_thread::yield();
    }
    // put item to queue
    _put_to_collection(item);
    return true;
}

template <typename T>
T* SharedQueue<T>::dequeue(){
    std::unique_lock<std::mutex> guard(_mutex);
    _cond.wait(guard, [this]()->bool{ return _cnt > 0;});
    auto res = _get_from_collection();
    return  res;
}

template <typename T>
T* SharedQueue<T>::dequeue(int millisecondsTimeout) {
    std::unique_lock<std::mutex> guard(_mutex);
    if (_cond.wait_for(guard, milliseconds(millisecondsTimeout), [this]()->bool{ return _cnt.load() > 0;})) {
        auto res = _get_from_collection();
        return res;
    }
    return nullptr;
}

template <typename T>
void SharedQueue<T>::_put_to_collection(T* item) {
    std::lock_guard<std::mutex> guard(_mutex);
    try {
        _collection.push(item);
        _cnt.fetch_add(1);
        _cond.notify_all();
    } catch (std::exception const &ec) {
        std::cerr << ec.what() << std::endl;
    }
}

template <typename T>
T* SharedQueue<T>::_get_from_collection() {
    T* value = nullptr;
    try {
        value = _collection.front();
        _collection.pop();
        _cnt.fetch_sub(1);

    } catch (std::exception const &ec) {
        std::cerr << ec.what() << std::endl;
    }
    return value;
}

#endif //SHAREDQUEUE_SHAREDQUEUE_H
