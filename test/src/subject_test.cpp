#include <gtest/gtest.h>

#include "RxLite.hpp"

TEST(SubjectTestsuite, SubjectTest) {
    RxLite::Subject<int> subject;
    std::vector<RxLite::Subscription> subscriptions;

    int sum = 0;
    RxLite::Observer<int> subscriber([&sum](int i) {
        sum += i;
    });

    int expectedSum = 0;
    for (int i = 1; i <= 10; i++) {
        subject.next(i);
        expectedSum += i * subscriptions.size(); 

        subscriptions.push_back(subject.subscribe(subscriber));
    }

    for (int i = 10; i > 0; i--) {
        subscriptions.resize(subscriptions.size() - 1);

        subject.next(i);
        expectedSum += i * subscriptions.size(); 
    }

    subject.next(INT32_MAX);
    ASSERT_EQ(sum, expectedSum);
}

TEST(SubjectTestsuite, BehaviorSubjectTest) {
    RxLite::BehaviorSubject<int> subject(0);
    std::vector<RxLite::Subscription> subscriptions;

    int sum = 0;
    RxLite::Observer<int> subscriber([&sum](int i) {
        sum += i;
    });

    int expectedSum = 0;
    for (int i = 1; i <= 10; i++) {
        subject.next(i);
        
        subscriptions.push_back(subject.subscribe(subscriber));
        expectedSum += i * subscriptions.size(); 
    }

    for (int i = 10; i > 0; i--) {
        subscriptions.resize(subscriptions.size() - 1);

        subject.next(i);
        expectedSum += i * subscriptions.size(); 
    }

    subject.next(INT32_MAX);    
    ASSERT_EQ(sum, expectedSum);
}

TEST(SubjectTestsuite, ReplaySubjectTest) {
    RxLite::ReplaySubject<int> replaySubject;

    std::vector<int> results1;
    std::vector<int> results2;
    bool hasCompleted1 = false;
    bool hasCompleted2 = false;

    RxLite::Observer<int> observer1(
        [&results1](int value) { results1.push_back(value); },
        [](const std::exception_ptr&) {},
        [&hasCompleted1]() { hasCompleted1 = true; }
    );
    RxLite::Subscription subscription1 = replaySubject.subscribe(observer1);

    replaySubject.next(1);
    replaySubject.next(2);
    replaySubject.next(3);

    RxLite::Observer<int> observer2(
        [&results2](int value) { results2.push_back(value); },
        [](const std::exception_ptr&) {},
        [&hasCompleted2]() { hasCompleted2 = true; }
    );
    RxLite::Subscription subscription2 = replaySubject.subscribe(observer2);

    replaySubject.next(4);
    replaySubject.next(5);

    replaySubject.complete();

    ASSERT_EQ(results1, (std::vector<int>{1, 2, 3, 4, 5}));
    ASSERT_EQ(results2, (std::vector<int>{1, 2, 3, 4, 5}));

    ASSERT_EQ(hasCompleted1, true);
    ASSERT_EQ(hasCompleted2, true);
}
