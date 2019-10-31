#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>

/* Building a simple task system using a scheduler and task stealing
 * The mutex class is a synchronization primitive that can be used to protect shared data from being
 * simultaneously accessed by multiple threads.
 * The class unique_lock is a general-purpose mutex ownership wrapper allowing deferred locking, ...
 */
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
    // returns the number of concurrent threads supported by the implementation
    const unsigned _count{std::thread::hardware_concurrency()};
    // one notification queue for each thread...
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
            e.join(); // waits for a thread to finish its execution
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
