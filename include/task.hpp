#include <functional>
#include <atomic>
#include <type_traits>
#include <future>

#pragma once

namespace affinity {

using task = std::function<void()>;

// Essentially a custom packaged task for use with the pool.
template<typename F, typename... Args>
requires std::is_invocable_v<F, Args...>
task wrap_task(std::promise<std::invoke_result_t<F, Args...>>&& p, F&& f, Args&&... args) {
    using result_type = std::invoke_result_t<F, Args...>;

    auto shared_promise = std::make_shared<std::promise<result_type>>(std::move(p));
    auto arg_tuple = std::make_tuple(std::forward<Args>(args)...);

    auto func = [f = std::forward<F>(f),
            args = std::move(arg_tuple),
            shared_promise]() mutable {
        try {
            if constexpr (std::is_void_v<result_type>) {
                std::apply(f, args);
                shared_promise->set_value();
            } else {
                shared_promise->set_value(std::apply(f, args));
            }
        } catch(...) {
            shared_promise->set_exception(std::current_exception());
        }
    };

    return task(std::move(func));
}

}
