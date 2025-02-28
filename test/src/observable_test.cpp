#include <iostream>
#include <thread>
#include <chrono>

#include <gtest/gtest.h>

#include "RxLite.hpp"

TEST(ObservableTestsuite, Example1) {
    RxLite::Observable<int> observable([](const RxLite::Subscriber<int>& subscriber) {
        subscriber.next(1);
        subscriber.next(2);
        subscriber.next(3);
        std::thread([subscriber]() {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            subscriber.next(4);
            subscriber.complete();
        }).detach();
    });
       
    std::cout << "just before subscribe" << std::endl;
    RxLite::Subscription subscription = observable.subscribe(RxLite::Observer<int>({
        [](int i) {
            std::cout << "got value " << i << std::endl;
        },
        [](const std::exception_ptr& err) {
            std::cout << "something wrong occurred!" << std::endl;
            std::rethrow_exception(err);
        },
        []() {
            std::cout << "done" << std::endl;
        }
    }));
    std::cout << "just after subscribe" << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(2));
}

TEST(ObservableTestsuite, Example2) {
    RxLite::Observable<int> foo([](const RxLite::Subscriber<int>& subscriber) {
        std::cout << "Hello" << std::endl;
        subscriber.next(42);
    });
       
    foo.subscribe([](int x) {
        std::cout << x << std::endl;
    });
    foo.subscribe([](int y) {
        std::cout << y << std::endl;
    });
}