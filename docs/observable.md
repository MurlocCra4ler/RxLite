# Observable (under construction)

Observables are lazy Push collections of multiple values. They fill the missing spot in the following table:

|          | SINGLE   | MULTIPLE   |
|----------|----------|------------|
| **Pull** | Function | Iterator   |
| **Push** | Promise  | Observable |

**Example.** The following is an Observable that pushes the values 1, 2, 3 immediately (synchronously) when subscribed, and the value 4 after one second has passed since the subscribe call, then completes:

```cpp
#include <RxLite.hpp>

int main() {
    RxLite::Observable<int> observable([](const RxLite::Observer<int>& subscriber) {
        subscriber.next(1);
        subscriber.next(2);
        subscriber.next(3);
        std::thread([&subscriber]() {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            subscriber.next(4);
            subscriber.complete();
        }).detach();
    });
}
```

To invoke the Observable and see these values, we need to *subscribe* to it:

```cpp
#include <RxLite.hpp>

int main() {
    RxLite::Observable<int> observable([](const RxLite::Observer<int>& subscriber) {
        subscriber.next(1);
        subscriber.next(2);
        subscriber.next(3);
        std::thread([&subscriber]() {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            subscriber.next(4);
            subscriber.complete();
        }).detach();
    });
       
    std::cout << "just before subscribe" << std::endl;
    observable.subscribe(RxLite::Observer<int>({
        [](int i) {
            std::cout << "got value " << i << std::endl;
        },
        [](const std::exception_ptr& err) {
            std::cout << "something wrong occurred!" << std::endl;
            std::rethrow_exception(err);
        },
        []() {
            std::cout << "done" << std::endl;
        }
    }));
    std::cout << "just after subscribe" << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(2));
}
```

Which executes as such on the console:

```
just before subscribe
got value 1
got value 2
got value 3
just after subscribe
got value 4
done
```

## Pull versus Push

*Pull* and *Push* are two different protocols that describe how a data *Producer* can communicate with a data *Consumer*.

### What is Pull?

In Pull systems, the Consumer determines when it receives data from the data Producer. The Producer itself is unaware of when the data will be delivered to the Consumer.

Every C++ Function is a Pull system. The function is a Producer of data, and the code that calls the function is consuming it by "pulling" out a *single* return value from its call.

C++ provides pull-based iteration through input streams (eg. **std::fstream**). In this model, the consumer explicitly requests data from a passive producer, repeatedly "pulling" *multiple* values as needed. This approach is commonly used when reading lines from a file, where each pull retrieves the next piece of data.

|          | PRODUCER                                   | CONSUMER                                    |
|----------|--------------------------------------------|---------------------------------------------|
| **Pull** | **Passive:** produces data when requested. | **Active:** decides when data is requested. |
| **Push** | **Active:** produces data at its own pace. | **Passive:** reacts to received data.       |

### What is Push?

In Push systems, the Producer determines when to send data to the Consumer. The Consumer does not control when it will receive that data.

In C++, `std::future` represents a common Push system. A `std::promise` (the Producer) sets a value asynchronously, and the associated `std::future` (the Consumer) retrieves it once it becomes available. Unlike regular function calls, where the caller dictates execution, the Producer determines exactly when the value is "pushed" to the callbacks.

RxLite introduces Observables, a new Push system for C++. An Observable is a Producer of multiple values, "pushing" them to Observers (Consumers).

 - A **Function** is a lazily evaluated computation that synchronously returns a single value on invocation.
 - An **Input Stream** is a lazily evaluated computation that synchronously returns zero to (potentially) infinite values on read.
 - A **Future** is a computation that may (or may not) eventually return a single value.
 - An **Observable** is a lazily evaluated computation that can synchronously or asynchronously return zero to (potentially) infinite values from the time it's invoked onwards.

## Observables as Generalizations of Functions

Contrary to common misconceptions, Observables are not just `std::future` extended to multiple values.

> Observables are like functions with zero arguments, but generalize those to allow multiple values.

Consider the following:

```cpp
auto foo = []() {
    std::cout << "Hello" << std::endl;
    return 42;
};

int x = foo();
std::cout << x << std::endl;
int y = foo();
std::cout << y << std::endl;
```

We expect to see as output:

```cpp
"Hello"
42
"Hello"
42
```

You can write the same behavior above, but with Observables:

```cpp
RxLite::Observable<int> foo([](const RxLite::Observer<int>& subscriber) {
    std::cout << "Hello" << std::endl;
    subscriber.next(42);
});
    
foo.subscribe([](int x) {
    std::cout << x << std::endl;
});
foo.subscribe([](int y) {
    std::cout << y << std::endl;
});
```

And the output is the same:

```cpp
"Hello"
42
"Hello"
42
```

This happens because both functions and Observables are lazy computations. If you don't call the function, the `std::cout << "Hello" << std::endl` won't happen. Also with Observables, if you don't "call" it (with `subscribe`), the `std::cout << "Hello" << std::endl` won't happen. Plus, "calling" or "subscribing" is an isolated operation: two function calls trigger two separate side effects, and two Observable subscribes trigger two separate side effects.

> Subscribing to an Observable is analogous to calling a Function.

Some people claim that Observables are asynchronous. That is not true. If you surround a function call with logs, like this:

```cpp
std::cout << "before" << std::endl;
std::cout << foo() << std::endl;
std::cout << "after" << std::endl
```

You will see the output:

```cpp
"before"
"Hello"
42
"after"
```

And this is the same behavior with Observables:

```cpp
std::cout << "before" << std::endl;
foo.subscribe([](int x) {
    std::cout << x << std::endl;
});
std::cout << "after" << std::endl
```

And the output is:

```cpp
"before"
"Hello"
42
"after"
```

Which proves the subscription of `foo` was entirely synchronous, just like a function.

> Observables are able to deliver values either synchronously or asynchronously.

What is the difference between an Observable and a function? **Observables can "return" multiple values over time**, something which functions cannot. You can't do this:

```cpp
int foo() {
    std::cout << "Hello" << std::endl;
    return 42;
    return 100; // dead code. will never happen
}
```

Functions can only return one value. Observables, however, can do this:

```cpp
RxLite::Observable<int> foo([](const RxLite::Observer<int>& subscriber) {
    std::cout << "Hello" << std::endl;
    subscriber.next(42);
    subscriber.next(100); // "return" another value
    subscriber.next(200); // "return" yet another
});

std::cout << "before" << std::endl;
foo.subscribe([](int x) {
    std::cout << x << std::endl;
});
std::cout << "after" << std::endl
```

With synchronous output:

```cpp
"before"
"Hello"
42
100
200
"after"
```

But you can also "return" values asynchronously:

```cpp
RxLite::Observable<int> foo([](const RxLite::Observer<int>& subscriber) {
    std::cout << "Hello" << std::endl;
    subscriber.next(42);
    subscriber.next(100);
    subscriber.next(200);
    std::thread([subscriber]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        subscriber.next(300);
    }).detach();
});

std::cout << "before" << std::endl;
foo.subscribe([](int x) {
    std::cout << x << std::endl;
});
std::cout << "after" << std::endl
```

With output:

```cpp
"before"
"Hello"
42
100
200
"after"
300
```

Conclusion:

 - `func.call()` means "give me one value synchronously"
 - `observable.subscribe()` means "give me any amount of values, either synchronously or asynchronously"

## Anatomy of an Observable

Observables are **created** using the `Observable constructor` or one of its `static member functions` (eg. `Observable<T>::of` or `Observable<T>::from`), are **subscribed** to with an Observer, execute to deliver `next` / `error` / `complete` notifications to the Observer, and their execution may be **disposed**. These four aspects are all encoded in an Observable instance, but some of these aspects are related to other types, like Observer and Subscription.

Core Observable concerns:
  -  **Creating** Observables
  -  **Subscribing** to Observables
  -  **Executing** the Observable
  -  **Disposing** Observables

### Creating Observables

The `Observable` constructor takes one argument: the `onSubscribe` function.

<br><br><br>
> <small> This documentation is based on the [RxJS documentation](https://rxjs.dev/), licensed under [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/). Changes may have been made. </small>