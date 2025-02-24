#include <gtest/gtest.h>

#include "RxLite.hpp"

TEST(OperatorTestsuite, OperatorTestsuite) {
    RxLite::Subject<int> subject;
    RxLite::Observable<int> observable = subject.pipe(
        RxLite::map<int, int>([](int i) {
            return i*2;
        })
    );

    
    int sum = 0;
    RxLite::Subscription subscription = observable.subscribe([&sum](int i) {
        sum += i;
    });

    int expectedSum = 0;
    for (int i = 1; i <= 100; i++) {
        subject.next(i);
        expectedSum += i * 2;
    }

    ASSERT_EQ(sum, expectedSum);
}