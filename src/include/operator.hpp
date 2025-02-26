#pragma once

#include <optional>

#include "observable.hpp"


namespace RxLite {

/**
 * @brief Defines a transformation function that maps one observable type to another.
 * 
 * This alias represents an operator that takes an `Observable<T>` as input and produces an `Observable<U>`.
 * 
 * @tparam T The input observable type.
 * @tparam U The output observable type.
 */
template <typename T, typename U>
using Operator = std::function<Observable<U>(Observable<T>&)>;

/**
 * @brief Transforms values emitted by an observable using a mapping function.
 * 
 * The `map` function applies `mapFunc` to each value emitted by the source observable,
 * producing a new observable of type `U`. It supports any callable, including 
 * function pointers, lambdas, and functors, without unnecessary heap allocations.
 * 
 * Errors and completion signals from the source observable are properly propagated 
 * to the resulting observable.
 * 
 * @tparam T The input value type.
 * @param mapFunc A callable that transforms values of type `T` to an output type `U` (deduced automatically).
 * @return Operator<T, U> A function that applies the transformation to an observable.
 */
template <typename T, typename F, typename U = std::invoke_result_t<F, const T&>>
Operator<T, U> map(F&& mapFunc) {
    return [mapFunc = std::forward<F>(mapFunc)](const Observable<T>& sourceObservable) {
        return impl::ObservableFactory<U>([mapFunc, sourceObservable](const Observer<U>& observer) {
            Observer<T> intermediateObserver(
                [mapFunc, observer](const T& t) {
                    observer.next(std::invoke(mapFunc, t));
                },
                [observer](const std::exception_ptr& err) { observer.error(err); },
                [observer]() { observer.complete(); }
            );

            return sourceObservable.subscribe(intermediateObserver);
        });
    };
}

/**
 * @brief Combines the source observable with the latest values from one or more other observables.
 * 
 * The `withLatestFrom` operator takes a source observable emitting values of type `T` and 
 * combines each of its emissions with the latest values from additional observables of types `Us...`. 
 * The resulting observable emits a tuple containing the source value and the most recent values 
 * from all provided observables.
 * 
 * The additional observables are **only sampled when the source emits**. If a sampled 
 * observable has not emitted any values yet, the source emission is ignored until at least one value 
 * is available for each observable.
 * 
 * The resulting observable **only completes when the source observable completes**, not when 
 * any of the latest observables complete. The latest observables can complete without affecting 
 * the completion behavior of the resulting observable.
 * 
 * Errors from either the source or the latest observables will be forwarded to the resulting observable.
 * 
 * @tparam T The type of values emitted by the source observable.
 * @param latestObservables One or more observables whose latest values will be combined with the source.
 * @return Operator<T, std::tuple<T, Us...>> A function that applies the combination logic to an observable.
 */
template <typename T, typename... Us>
Operator<T, std::tuple<T, Us...>> withLatestFrom(Observable<Us>... latestObservables) {
    return [latestObservables...](const Observable<T>& sourceObservable) {
        return impl::ObservableFactory<std::tuple<T, Us...>>(
            [sourceObservable, latestObservables...](const Observer<std::tuple<T, Us...>>& observer) {
                auto latestValues = std::make_shared<std::tuple<std::optional<Us>...>>();

                // Function to check if all values in the tuple are set
                auto allSet = [latestValues]() {
                    return std::apply([](auto&... values) {
                        return (... && values.has_value());
                    }, *latestValues);
                };

                Subscription subscriptions;

                // Subscribe to each latestObservable and update its corresponding value in latestValues
                auto subscribeLatest = [&]<std::size_t... Is>(std::index_sequence<Is...>) {
                    (void)std::initializer_list<int>{
                        (subscriptions.add(
                            latestObservables.subscribe(
                                Observer<Us>(
                                    [latestValues](const Us& value) {
                                        std::get<Is>(*latestValues) = value;
                                    },
                                    [observer](const std::exception_ptr& err) { observer.error(err); }
                                )
                            )
                        ), 0)... // Using comma operator to expand the parameter pack
                    };
                };

                // Create observers dynamically for each latest observable
                subscribeLatest(std::index_sequence_for<Us...>{});

                // Subscribe to the source observable
                Observer<T> combinedObserver(
                    [observer, latestValues, allSet](const T& t) {
                        if (allSet()) {
                            observer.next(
                                std::tuple_cat(std::make_tuple(t), 
                                    std::apply([](auto&... values) { return std::make_tuple(values.value()...); }, *latestValues)
                                )
                            );
                        }
                    },
                    [observer](const std::exception_ptr& err) { observer.error(err); },
                    [observer]() { observer.complete(); }
                );

                subscriptions.add(sourceObservable.subscribe(combinedObserver));
                return subscriptions;
            }
        );
    };
}

/*
template <typename T, typename U>
Operator<T, std::pair<T, U>> withLatestFrom(Observable<U> latestObservable) {
    return [latestObservable = std::move(latestObservable)](const Observable<T>& sourceObservable) {
        return impl::ObservableFactory<U>([sourceObservable, latestObservable]
            (const Observer<std::pair<T, U>>& observer) {
            std::shared_ptr<std::optional<U>> latestValue = std::make_shared<std::optional<U>>();
            Observer<U> latestObserver(
                [latestValue](const U& u) {
                    *latestValue = u;
                },
                [observer](const std::exception_ptr& err) { observer.error(err); }
            );

            Observer<T> combinedObserver(
                [observer, latestValue](const T& t) {
                    if (latestValue->has_value()) {
                        observer.next({ t, latestValue->value() }); 
                    }
                },
                [observer](const std::exception_ptr& err) { observer.error(err); },
                [observer]() { observer.complete(); }
            );

            Subscription subscription;
            subscription.add(latestObservable.subscribe(latestObserver));
            subscription.add(sourceObservable.subscribe(combinedObserver));
            return subscription;
        });
    };
} */

} // namespace RxLite
