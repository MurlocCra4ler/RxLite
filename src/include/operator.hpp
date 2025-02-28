#pragma once

#include <array>
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
 * @brief Combines multiple observables and emits tuples containing the latest values.
 * 
 * The `combineLatest` operator takes a source observable emitting values of type `T` and 
 * one or more additional observables emitting values of types `Us...`. It produces an 
 * observable that emits a tuple containing the latest values from all observables, including 
 * the source.
 * 
 * This operator waits until all observables, including the source, have emitted at least 
 * one value before emitting its first tuple. Thereafter, it emits a new tuple whenever 
 * any of the observables (including the source) emits a new value.
 * 
 * The resulting observable completes when **all** observables complete. Errors from 
 * any observable are immediately forwarded to the resulting observable.
 * 
 * @tparam T The type of values emitted by the source observable.
 * @tparam Us The types of values emitted by the additional observables.
 * @param latestObservables One or more observables whose latest values will be combined with the source.
 * @return Operator<T, std::tuple<T, Us...>> A function that applies the combination logic to an observable.
 */
template <typename T, typename... Us>
Operator<T, std::tuple<T, Us...>> combineLatest(Observable<Us>... latestObservables) {
    return [latestObservables...](const Observable<T>& sourceObservable) {
        return impl::ObservableFactory<std::tuple<T, Us...>>(
            [sourceObservable, latestObservables...](const Subscriber<std::tuple<T, Us...>>& subscriber) {
                Subscription subscriptions;
                auto latestValues = std::make_shared<std::tuple<std::optional<T>, std::optional<Us>...>>();
                auto completedFlags = std::make_shared<std::array<bool, sizeof...(Us) + 1>>();

                auto emitIfReady = [subscriber, latestValues]() {
                    if (std::apply([](auto&... values) { return (... && values.has_value()); }, *latestValues)) {
                        subscriber.next(std::apply([](auto&... values) {
                            return std::make_tuple(values.value()...);
                        }, *latestValues));
                    }
                };

                auto completeIfReady = [subscriber, completedFlags]() {
                    if (std::apply([](auto... flags) { return (... && flags); }, *completedFlags)) {
                        subscriber.complete();
                    }
                };

                Observer<T> sourceObserver(
                    [latestValues, emitIfReady](const T& t) {
                        std::get<0>(*latestValues) = t;
                        emitIfReady();
                    },
                    [subscriber](const std::exception_ptr& err) { subscriber.error(err); },
                    [completedFlags, completeIfReady]() { 
                        completedFlags->at(0) = true;
                        completeIfReady(); 
                    }
                );
                subscriptions.add(sourceObservable.subscribe(sourceObserver));

                auto subscribeLatest = [&]<std::size_t... Is>(std::index_sequence<Is...>) {
                    (void)std::initializer_list<int>{
                        (subscriptions.add(
                            latestObservables.subscribe(
                                Observer<Us>(
                                    [latestValues, emitIfReady](const Us& value) {
                                        std::get<Is + 1>(*latestValues) = value;
                                        emitIfReady();
                                    },
                                    [subscriber](const std::exception_ptr& err) { subscriber.error(err); },
                                    [completedFlags, completeIfReady]() { 
                                        completedFlags->at(Is + 1) = true;
                                        completeIfReady(); 
                                    }
                                )
                            )
                        ), 0)...
                    };
                };

                subscribeLatest(std::index_sequence_for<Us...>{});
                return [subscriptions]() mutable {
                    subscriptions.unsubscribe();
                };
            }
        );
    };
}

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
template <typename T, typename Func, typename U = std::invoke_result_t<Func, const T&>>
Operator<T, U> map(Func&& mapFunc) {
    return [mapFunc = std::forward<Func>(mapFunc)](const Observable<T>& sourceObservable) {
        return impl::ObservableFactory<U>([mapFunc, sourceObservable](const Subscriber<U>& subscriber) {
            Observer<T> intermediateObserver(
                [mapFunc, subscriber](const T& t) {
                    subscriber.next(std::invoke(mapFunc, t));
                },
                [subscriber](const std::exception_ptr& err) { subscriber.error(err); },
                [subscriber]() { subscriber.complete(); }
            );

            return [subscription = sourceObservable.subscribe(intermediateObserver)]() mutable {
                subscription.unsubscribe();
            };
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
            [sourceObservable, latestObservables...](const Subscriber<std::tuple<T, Us...>>& subscriber) {
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
                                    [subscriber](const std::exception_ptr& err) { subscriber.error(err); }
                                )
                            )
                        ), 0)... // Using comma operator to expand the parameter pack
                    };
                };

                // Create observers dynamically for each latest observable
                subscribeLatest(std::index_sequence_for<Us...>{});

                // Subscribe to the source observable
                Observer<T> combinedObserver(
                    [subscriber, latestValues, allSet](const T& t) {
                        if (allSet()) {
                            subscriber.next(
                                std::tuple_cat(std::make_tuple(t), 
                                    std::apply([](auto&... values) { return std::make_tuple(values.value()...); }, *latestValues)
                                )
                            );
                        }
                    },
                    [subscriber](const std::exception_ptr& err) { subscriber.error(err); },
                    [subscriber]() { subscriber.complete(); }
                );

                subscriptions.add(sourceObservable.subscribe(combinedObserver));
                return [subscriptions]() mutable {
                    subscriptions.unsubscribe();
                };
            }
        );
    };
}

} // namespace RxLite
