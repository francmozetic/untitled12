#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>

// Building a task system
class notification_queue {
    std::deque<std::function<void()>> _q;
    bool _done{false};
    std::mutex _mutex;
    std::condition_variable _ready;

public:
    void done() {
        {
            std::unique_lock<std::mutex> lock{_mutex};
            _done = true;
        }
        _ready.notify_all();
    }

    void pop(std::function<void()>& x) {
        std::unique_lock<std::mutex> lock{_mutex};
        while (_q.empty() && !_done) _ready.wait(lock);
        x = std::move(_q.front());
        _q.pop_front();
    }

    template<typename F>
    void push(F&& f) {
        {
            std::unique_lock<std::mutex> lock{_mutex};
            _q.emplace_back(std::forward<F>(f));
        }
        _ready.notify_one();
    }
};

class task_system {
    const unsigned _count{std::thread::hardware_concurrency()};
    std::vector<std::thread> _threads;
    std::vector<notification_queue> _q{_count};
    std::atomic<unsigned> _index{0};

    void run(unsigned i) {
        while (true) {
            std::function<void()> f;
            _q[i].pop(f);
            f();
        }
    }

public:
    task_system() {
        for (unsigned n=0; n!=_count; ++n) {
            _threads.emplace_back([&, n](){ run(n); });
        }
    }

    ~task_system() {
        for (auto& e : _threads) {
            e.join();
        }
        for (auto& e : _q) {
            e.done();
        }
    }

    template<typename F>
    void async(F&& f) {
        auto i = _index++;
        _q[i % _count].push(std::forward<F>(f));
    }



};
