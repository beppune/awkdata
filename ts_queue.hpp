#ifndef TS_QUEUE_HPP
#define TS_QUEUE_HPP

#include <queue>
#include <mutex>
#include <optional>

template<typename T>
struct ts_queue{
    using lg = std::lock_guard<std::mutex>;

    void push(T t) {
        lg{m};

        q.push(t);
    }

    std::optional<T> pop() {
        lg{m};

        if( q.empty() ) {
            return {};
        }

        auto t = q.front();
        q.pop();
        return t;
    }


    private:
        std::queue<T> q;
        mutable std::mutex  m;
};

#endif // TS_QUEUE_HPP
