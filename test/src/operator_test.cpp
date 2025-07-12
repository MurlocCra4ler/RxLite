#include <gtest/gtest.h>
#include <cmath>
#include <numeric>

#include "RxLite.hpp"

TEST(OperatorTestsuite, CombineLatestTest) {
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

TEST(OperatorTestsuite, DistinctTest) {
    RxLite::Subject<int> sourceSubject;

    // Applying distinctUntilChanged operator
    RxLite::Observable<int> filteredObservable = sourceSubject.pipe(
        RxLite::distinct<int>()
    );

    std::vector<int> results;
    bool hasCompleted = false;

    RxLite::Observer<int> observer(
        [&results](int value) { results.push_back(value); },
        [](const std::exception_ptr&) {},
        [&hasCompleted]() { hasCompleted = true; }
    );

    RxLite::Subscription subscription = filteredObservable.subscribe(observer);

    // Emitting values
    sourceSubject.next(1);  // Emits 1
    sourceSubject.next(2);  // Emits 2
    sourceSubject.next(2);  // Ignored (duplicate)
    sourceSubject.next(3);  // Emits 3
    sourceSubject.next(4);  // Emits 4
    sourceSubject.next(5);  // Emits 5
    sourceSubject.next(2);  // Ignored (duplicate)
    sourceSubject.next(3);  // Ignored (duplicate)
    sourceSubject.next(4);  // Ignored (duplicate)
    sourceSubject.next(5);  // Ignored (duplicate)

    // Expected result: {1, 2, 3, 4, 5}
    std::vector<int> expectedResults = {1, 2, 3, 4, 5};
    ASSERT_EQ(results, expectedResults);

    // Test completion
    ASSERT_EQ(hasCompleted, false);
    sourceSubject.complete();
    ASSERT_EQ(hasCompleted, true);
}

TEST(OperatorTestsuite, DistinctUntilChangedTest) {
    RxLite::Subject<int> sourceSubject;

    // Applying distinctUntilChanged operator
    RxLite::Observable<int> filteredObservable = sourceSubject.pipe(
        RxLite::distinctUntilChanged<int>()
    );

    std::vector<int> results;
    bool hasCompleted = false;

    RxLite::Observer<int> observer(
        [&results](int value) { results.push_back(value); },
        [](const std::exception_ptr&) {},
        [&hasCompleted]() { hasCompleted = true; }
    );

    RxLite::Subscription subscription = filteredObservable.subscribe(observer);

    // Emitting values
    sourceSubject.next(1);  // Emits 1
    sourceSubject.next(1);  // Ignored (duplicate)
    sourceSubject.next(2);  // Emits 2
    sourceSubject.next(2);  // Ignored (duplicate)
    sourceSubject.next(3);  // Emits 3
    sourceSubject.next(3);  // Ignored (duplicate)
    sourceSubject.next(3);  // Ignored (duplicate)
    sourceSubject.next(4);  // Emits 4
    sourceSubject.next(4);  // Ignored (duplicate)
    sourceSubject.next(5);  // Emits 5

    // Expected result: {1, 2, 3, 4, 5}
    std::vector<int> expectedResults = {1, 2, 3, 4, 5};
    ASSERT_EQ(results, expectedResults);

    // Test completion
    ASSERT_EQ(hasCompleted, false);
    sourceSubject.complete();
    ASSERT_EQ(hasCompleted, true);
}

TEST(OperatorTestsuite, MapTest) {
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

TEST(OperatorTestsuite, MergeTest) {
    RxLite::Observable<int> even = RxLite::Observable<int>::from({ 0, 2, 4, 6, 8 });
    RxLite::Observable<int> odd = RxLite::Observable<int>::from({ 1, 3, 5, 7, 9 });

    RxLite::Observable<int> combined = even.pipe(RxLite::merge<int>(odd));

    std::vector<int> results;
    bool hasCompleted = false;

    RxLite::Observer<int> observer(
        [&results](int value) { results.push_back(value); },
        [](const std::exception_ptr&) {},
        [&hasCompleted]() { hasCompleted = true; }
    );

    RxLite::Subscription subscription = combined.subscribe(observer);

    std::sort(results.begin(), results.end());
    std::vector<int> expectedResults(10);
    std::iota(expectedResults.begin(), expectedResults.end(), 0);
    ASSERT_EQ(results, expectedResults);
    ASSERT_EQ(hasCompleted, hasCompleted);
}

TEST(OperatorTestsuite, WithLatestFromTest) {
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

TEST(OperatorTestsuite, CombinedTest) {
    std::vector<size_t> input = { 1, 2, 3, 4, 5 };
    RxLite::Observable<size_t> source = RxLite::Observable<size_t>::from(input);
    RxLite::Observable<size_t> inter1 = source.pipe(
        RxLite::withLatestFrom<size_t>(source, source),
        RxLite::map<std::tuple<size_t, size_t, size_t>>([](const auto& nested) {
            auto [x, y, z] = nested;
            return x * y * z;
        }),
        RxLite::map<size_t>([](size_t x) {
            return x / 25;
        }));
    RxLite::Observable<size_t> inter2 = inter1.pipe(
        RxLite::withLatestFrom<size_t>(inter1, inter1),
        RxLite::map<std::tuple<size_t, size_t, size_t>>([](const auto& nested) {
            auto [x, y, z] = nested;
            return x * y * z;
        }),
        RxLite::map<size_t>([](size_t x) {
            return x / 25;
        }));
    RxLite::Observable<size_t> inter3 = inter2.pipe(
        RxLite::withLatestFrom<size_t>(inter2, inter2),
        RxLite::map<std::tuple<size_t, size_t, size_t>>([](const auto& nested) {
            auto [x, y, z] = nested;
            return x * y * z;
        }),
        RxLite::map<size_t>([](size_t x) {
            return x / 25;
        }));
    RxLite::Observable<size_t> inter4 = inter3.pipe(
        RxLite::withLatestFrom<size_t>(inter3, inter3),
        RxLite::map<std::tuple<size_t, size_t, size_t>>([](const auto& nested) {
            auto [x, y, z] = nested;
            return x * y * z;
        }),
        RxLite::map<size_t>([](size_t x) {
            return x / 25;
        }));
    RxLite::Observable<size_t> inter5 = inter4.pipe(
        RxLite::withLatestFrom<size_t>(inter4, inter4),
        RxLite::map<std::tuple<size_t, size_t, size_t>>([](const auto& nested) {
            auto [x, y, z] = nested;
            return x * y * z;
        }),
        RxLite::map<size_t>([](size_t x) {
            return x / 25;
        }));
    RxLite::Observable<size_t> inter6 = inter5.pipe(
        RxLite::withLatestFrom<size_t>(inter5, inter5),
        RxLite::map<std::tuple<size_t, size_t, size_t>>([](const auto& nested) {
            auto [x, y, z] = nested;
            return x * y * z;
        }),
        RxLite::map<size_t>([](size_t x) {
            return x / 25;
        }));
    RxLite::Observable<size_t> inter7 = inter6.pipe(
        RxLite::withLatestFrom<size_t>(inter6, inter6),
        RxLite::map<std::tuple<size_t, size_t, size_t>>([](const auto& nested) {
            auto [x, y, z] = nested;
            return x * y * z;
        }),
        RxLite::map<size_t>([](size_t x) {
            return x / 25;
        }));
    RxLite::Observable<size_t> inter8 = inter7.pipe(
        RxLite::withLatestFrom<size_t>(inter7, inter7),
        RxLite::map<std::tuple<size_t, size_t, size_t>>([](const auto& nested) {
            auto [x, y, z] = nested;
            return x * y * z;
        }),
        RxLite::map<size_t>([](size_t x) {
            return x / 25;
        }));
    

    std::vector<size_t> output;
    inter8.subscribe([&output](double x) { output.push_back(x); });
    ASSERT_EQ(input, output);
}