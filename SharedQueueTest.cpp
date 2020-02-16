//
// Created by jin on 2/16/20.
//

#include "SharedQueue.h"
#define BOOST_TEST_MODULE SharedQueueTest
#include <boost/test//unit_test.hpp>
#include <valarray>
#include <iomanip>


BOOST_AUTO_TEST_SUITE(Funtionality_test);

BOOST_AUTO_TEST_CASE(init_test) {
    SharedQueue<int> q(1);
    BOOST_CHECK(q.count() == 0);
}

BOOST_AUTO_TEST_CASE(enqueue_test) {
    SharedQueue<int> q(1);
    int *tmp = new int {2};
    q.enqueue(tmp);
    BOOST_CHECK_EQUAL(q.count(), 1);
    delete tmp;
    tmp = nullptr;
}

BOOST_AUTO_TEST_CASE(dequeue_test) {
    SharedQueue<int> q(1);
    q.enqueue(new int{4});
    BOOST_REQUIRE(q.count() == 1);
    auto res = q.dequeue();
    delete res;
    BOOST_CHECK(q.count() == 0);
}

BOOST_AUTO_TEST_CASE(enqueue_timeout) {
    SharedQueue<int> q(0);
    BOOST_REQUIRE(q.count() == 0);
    auto res = new int {5};
    BOOST_CHECK(!q.enqueue(res, 1000));
    delete res;
    res = nullptr;
}

BOOST_AUTO_TEST_CASE(dequeue_timeout) {
    SharedQueue<int> q(0);
    auto res = q.dequeue(1000);
    BOOST_CHECK(res == nullptr);
}

BOOST_AUTO_TEST_CASE(enqueue_dequeue_multiply_values) {
    SharedQueue<int> q(100);
    int j = 0,i = 0;
    while (i < 100) {
        q.enqueue(new int{0});
        i++;
    }
    while (q.count() != 0) {
        auto ptr = q.dequeue();
        delete ptr;
        j++;
    }
    BOOST_REQUIRE_EQUAL(i,j);
}

bool message_check(const std::runtime_error &ec) {
    BOOST_CHECK_EQUAL(ec.what(), std::string("Nullptr given as argument."));
    return true;
}

BOOST_AUTO_TEST_CASE(enqueue_null_as_argument) {
        SharedQueue<int> q(0);
        BOOST_CHECK_EXCEPTION(q.enqueue(nullptr), std::runtime_error, message_check);
}

BOOST_AUTO_TEST_CASE(enqueue_timeout_null_as_argument) {
    SharedQueue<int> q(0);
    BOOST_CHECK_EXCEPTION(q.enqueue(nullptr, 1000), std::runtime_error, message_check);
}

constexpr int examples = 10000;
using time_array_t = std::array<std::chrono::steady_clock::time_point, examples>;

void producer(SharedQueue<int> &queue, time_array_t &time){
    for (int i = 0; i < examples; i++) {
        queue.enqueue(new int {i});
        time[i] = std::chrono::steady_clock::now();
    }
}

void consumer(SharedQueue<int> &queue, time_array_t &time){
    for(int i = 0; i < examples; i++) {
        auto ptr = queue.dequeue();
        time[i] = std::chrono::steady_clock::now();
        delete ptr;
    }
}


BOOST_AUTO_TEST_CASE(thread_test) {
    SharedQueue<int> q(examples);

    std::array<std::chrono::steady_clock::time_point, examples> start_arr;
    std::array<std::chrono::steady_clock::time_point, examples> end_arr;

    std::thread producer_thread(producer, std::ref(q), std::ref(start_arr));
    std::thread consumer_thread(consumer, std::ref(q), std::ref(end_arr));
    consumer_thread.join();
    producer_thread.join();


    std::valarray<double> durations(examples);
    for(int i = 0; i < examples; i++)
        durations[i] = std::chrono::duration_cast<std::chrono::duration<double>>(end_arr[i] - start_arr[i]).count();

    double min = durations.min();
    double  max = durations.max();
    double  avg = durations.sum() / durations.size();

    std::cout << "Min "  << std::fixed << std::setprecision(9) << min << " sec | Max " << max << " sec | Avg " << avg << " sec"<< std::endl;
}


BOOST_AUTO_TEST_SUITE_END();