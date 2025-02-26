#include <gtest/gtest.h>

#include "RxLite.hpp"

TEST(OperatorTestsuite, CombineLatest) {
    RxLite::Subject<int> sourceSubject;
    RxLite::Subject<int> latestSubject;

    // Using withLatestFrom to combine source and latest values
    RxLite::Observable<std::tuple<int, int>> combinedObservable = sourceSubject.pipe(
        RxLite::combineLatest<int>(latestSubject)
    );

    int result = 0;
    bool hasCompleted = false;
    RxLite::Observer<std::tuple<int, int>> combinedObserver(
        [&result](std::tuple<int, int> values) {
            result += std::get<0>(values) * std::get<1>(values);
        },
        [](const std::exception_ptr&) {},
        [&hasCompleted]() { hasCompleted = true; }
    );

    RxLite::Subscription subscription = combinedObservable.subscribe(combinedObserver);

    // Sending data to both subjects
    sourceSubject.next(1);  // Source emits 1  -> gets dropped
    latestSubject.next(10); // Latest emits 10 -> <1, 10>
    sourceSubject.next(2);  // Source emits 2  -> <2, 10>
    latestSubject.next(20); // Latest emits 20 -> <2, 20>
    sourceSubject.next(3);  // Source emits 3  -> <3, 20>
    latestSubject.next(30); // Latest emits 30 -> <3, 30>

    // After two emissions, the result should be (1 * 10) + (2 * 10) + (2 * 20) + (3 * 20) + (3 * 30)
    int expectedResult = (1 * 10) + (2 * 10) + (2 * 20) + (3 * 20) + (3 * 30);
    ASSERT_EQ(result, expectedResult);

    // Test completion
    ASSERT_EQ(hasCompleted, false);
    latestSubject.complete();
    ASSERT_EQ(hasCompleted, false);
    sourceSubject.complete();
    ASSERT_EQ(hasCompleted, true);
}

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
    bool hasCompleted = false;
    RxLite::Observer<std::tuple<int, int>> combinedObserver(
        [&result](std::tuple<int, int> values) {
            result += std::get<0>(values) * std::get<1>(values);
        },
        [](const std::exception_ptr&) {},
        [&hasCompleted]() { hasCompleted = true; }
    );

    RxLite::Subscription subscription = combinedObservable.subscribe(combinedObserver);

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

    // Test completion
    ASSERT_EQ(hasCompleted, false);
    latestSubject.complete();
    ASSERT_EQ(hasCompleted, false);
    sourceSubject.complete();
    ASSERT_EQ(hasCompleted, true);
}