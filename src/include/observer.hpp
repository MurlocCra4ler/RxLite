#pragma once

#include <exception>
#include <functional>
#include <memory>

namespace RxLite {

/**
 * @brief A concrete Observer class for a specific type.
 * 
 * The Observer class represents an observer that handles a specific type
 * of data (e.g., int, float). Observers are simply a set of callbacks,
 * one for each type of notification delivered by the Observable:
 * - `next` for handling emitted values
 * - `error` for handling errors
 * - `complete` for handling completion signals.
 *
 * The observer pattern is used to allow an object to react to changes in
 * another object (the observable) without tightly coupling the two.
 * 
 * @tparam T The type of data this observer handles.
 */
template <typename T>
class Observer;

/**
 * @brief Contains implementation details.
 * 
 * Users of RxLite should not need to interact with this directly.
 */
namespace impl {

class ObserverBase;
using SharedObserver = std::shared_ptr<ObserverBase>;

class ObserverBase {
public:
    virtual ~ObserverBase() = default;

    void error(const std::exception_ptr& err) const {
        onError(err);
    }

    void complete() const {
        onComplete();
    }

    template <typename Derived>
    Derived& as() {
        return dynamic_cast<Derived&>(*this);
    }

protected:
    ObserverBase(std::function<void(const std::exception_ptr&)> onError, std::function<void()> onComplete)
        : onError(std::move(onError)), onComplete(std::move(onComplete)) {}

private:
    const std::function<void(const std::exception_ptr&)> onError;
    const std::function<void()> onComplete;
};
    
} // namespace impl

template <typename T>
class Observer : public impl::ObserverBase {
public:
    /**
     * @brief Constructs an Observer with provided callbacks.
     * 
     * This constructor sets up the `next`, `error`, and `complete` callbacks 
     * for the observer.
     * 
     * @param onNext The callback for handling the next value.
     * @param onError (Optional) The callback for handling errors.
     * @param onComplete (Optional) The callback for handling completion.
     */
    template<typename OnNext>
    requires std::is_invocable_v<OnNext, const T&>
    Observer(OnNext&& onNext,
             std::function<void(const std::exception_ptr&)> onError = [](const std::exception_ptr&) {},
             std::function<void()> onComplete = []() {})
        : ObserverBase(std::move(onError), std::move(onComplete)), onNext(std::forward<OnNext>(onNext)) {}

    void next(T t) const {
        onNext(t);
    }

private:
    const std::function<void(T)> onNext;
};

} // RxLite
