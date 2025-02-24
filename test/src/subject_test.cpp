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