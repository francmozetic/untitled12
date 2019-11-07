#ifndef TASK_H
#define TASK_H

#include <functional>
#include <future>
#include <memory>

template<class T>
using decay_t = typename std::decay<T>::type;
template<class T>
using result_of_t = typename std::result_of<T>::type;

template<typename F, typename A>
std::future<result_of_t<decay_t<F>(decay_t<A>)>> spawn_task(F&& f, A&& a) {
    //std::cout << "in spawn_task()" << "\n";
    using result_t = result_of_t<decay_t<F>(decay_t<A>)>;
    std::packaged_task<result_t(decay_t<A>)> task(std::forward<F>(f));
    auto res = task.get_future();
    std::thread t(std::move(task), std::forward<A>(a));
    t.detach();
    return res;
}

template <class Func, class... Args>
std::future <std::result_of_t<std::decay_t<Func>(std::decay_t <Args>...)>> async_task(Func&& f, Args&&... args) {
    using return_type = std::result_of_t<std::decay_t<Func>(std::decay_t<Args>...)>;

    // std::packaged_task<> is movable but not copy-able, ...
    std::packaged_task<return_type(std::decay_t<Func> f, std::decay_t<Args>... args)> task(
        [](std::decay_t<Func> f, std::decay_t<Args>... args){ return f(args...); }
    );

    // ..., therefore we fetch the std::future<> object from it before moving it to thread.
    auto task_future = task.get_future();
    std::thread t(std::move(task), f, std::forward<Args>(args)...);
    t.detach ();
    return task_future;
}

#endif // TASK_H
