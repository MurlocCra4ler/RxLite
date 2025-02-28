#pragma once

#include <list>
#include <mutex>
#include <shared_mutex>

#include "observable.hpp"


namespace RxLite {

/**
 * @brief A concrete Subject class for a specific type.
 * 
 * A Subject is a special type of Observable that allows values to be multicasted to many Observers.
 * 
 * @tparam T The type of data this subject handles.
 */
template <typename T>
class Subject;

/**
 * @brief Contains implementation details.
 * 
 * Users of RxLite should not need to interact with this directly.
 */
namespace impl {

template <typename T>
class SubscriberManager {
public:
    void add(const Subscriber<T>& subscriber) {
        std::unique_lock lock(mutex);
        subscribers.push_back(subscriber);
    }

    void removeInactive() {
        std::unique_lock lock(mutex, std::try_to_lock);

        if (!lock.owns_lock()) {
            return;
        }

        subscribers.remove_if([](const Subscriber<T>& subscriber) {
            return subscriber.isInactive();
        });
    }

    void clear() {
        std::unique_lock lock(mutex);
        subscribers.clear();
    }

    template <typename Func>
    requires std::invocable<Func, const std::list<Subscriber<T>>&>
    void read(Func&& func) const {
        std::shared_lock lock(mutex);
        func(subscribers);
    }

private:
    std::list<Subscriber<T>> subscribers;
    mutable std::shared_mutex mutex;
};

template <typename T>
class SubjectBase {
protected:
    const std::shared_ptr<SubscriberManager<T>> sharedManager;

    SubjectBase() : sharedManager(std::make_shared<SubscriberManager<T>>()) {}

    void broadcastValue(const T& value) const {
        sharedManager->removeInactive();
        sharedManager->read([&value](const std::list<Subscriber<T>>& subscribers) {
            for (const auto& subscriber : subscribers) {
                subscriber.next(value);
            }
        });
    }

    void broadcastError(const std::exception_ptr& err) {
        sharedManager->removeInactive();
        for (const auto& subscriber : *this->subscribers) {
            subscriber->error(err);
        }
    }

    void broadcastCompletion() const {
        sharedManager->removeInactive();
        sharedManager->read([](const std::list<Subscriber<T>>& subscribers) {
            for (const auto& subscriber : subscribers) {
                subscriber.complete();
            }
        });

        sharedManager->clear();
    }
};

} // namespace impl

template <typename T>
class Subject : public impl::SubjectBase<T>, public Observable<T> {
public:
    Subject() : impl::SubjectBase<T>(), Observable<T>(createOnSubscribe()) {}
    
    /**
     * @brief Emit a new value to all subscribers.
     * 
     * This method is called to notify all subscribers of the latest value.
     * 
     * @param value The new value to broadcast to subscribers.
     */
    void next(T value) const {
        this->broadcastValue(value);
    }

    /**
     * @brief Emits an error to all subscribers.
     * 
     * This method notifies all subscribers that an error has occurred, thereby terminating the observable sequence.
     * After calling `error()`, any subsequent calls to `next()`, `complete()`, or further invocations of `error()`
     * will have no effect.
     * 
     * @param err The exception pointer representing the error to be broadcast to subscribers.
     */
    void error(const std::exception_ptr& err) const {
        this->broadcastError(err);
    }

    /**
     * @brief Completes the observable sequence.
     * 
     * This method notifies all subscribers that no more values will be emitted.
     * After calling `complete()`, any further calls to `next()` or `error()` will have no effect.
     */
    void complete() const {
        this->broadcastCompletion();
    }

private:
    std::function<void(const Subscriber<T>&)> createOnSubscribe() {
        return [sharedManager = this->sharedManager](const Subscriber<T>& subscriber) {
            sharedManager->add(subscriber);
        };
    }
};

} // namespace RxLite
