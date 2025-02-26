#include <gtest/gtest.h>

#include "RxLite.hpp"

TEST(OperatorTestsuite, OperatorTestsuite) {
    RxLite::Subject<int> subject;
    RxLite::Observable<int> observable = subject.pipe(
        RxLite::map<int>([](int i) {
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

TEST(OperatorTestsuite, WithLatestFrom) {
    RxLite::Subject<int> sourceSubject;
    RxLite::Subject<int> latestSubject;

    // Using withLatestFrom to combine source and latest values
    RxLite::Observable<std::tuple<int, int>> combinedObservable = sourceSubject.pipe(
        RxLite::withLatestFrom<int>(latestSubject)
    );

    int result = 0;
    RxLite::Subscription subscription = combinedObservable.subscribe([&result](std::tuple<int, int> values) {
        result += std::get<0>(values) * std::get<1>(values);
    });

    // Sending data to both subjects
    sourceSubject.next(1);  // Source emits 1 -> gets dropped
    latestSubject.next(10); // Latest emits 10
    sourceSubject.next(2);  // Source emits 2 -> <2, 10>
    latestSubject.next(20); // Latest emits 20
    sourceSubject.next(3);  // Source emits 3 -> <3, 20>
    latestSubject.next(30); // Latest emits 30

    // After two emissions, the result should be (2 * 10) + (3 * 20)
    int expectedResult = (2 * 10) + (3 * 20);

    ASSERT_EQ(result, expectedResult);
}