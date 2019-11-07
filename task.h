#ifndef TASK_H
#define TASK_H

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

template<class F>
void call_async(F&& fun) {
    auto ptr = std::make_shared<std::future<void>>();
    *ptr = std::async(std::launch::async, fun);
}

#endif // TASK_H
