//
// Created by firewolf on 09.02.23.
//
#include <coroutine>
#include <thread>
#include <iostream>

/*static auto switch_to_new_thread(jthread& out) // async func
{
    struct awaitable
    {
        jthread* p_out;
        bool await_ready() { return false; }
        void await_suspend(coroutine_handle<> h) // changer
        {
            jthread& out = *p_out;
            if (out.joinable())
                throw runtime_error("Output jthread parameter not empty");
            out = jthread([h] { h.resume(); }); // lambda resume
            std::cout << "New thread: " << out.get_id() << '\n'; // this is OK
        }
        void await_resume() {}
    };
    return awaitable{&out}; // send new awaible with new thread
}

struct task // tasker coroutine
{
    struct promise_type
    {
        task get_return_object() { return {}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
    struct awaitable
    {
        jthread* p_out;
        bool await_ready() { return false; }
        void await_suspend(coroutine_handle<> h) // changer
        {
            jthread& out = *p_out;
            if (out.joinable())
                throw runtime_error("Output jthread parameter not empty");
            out = jthread([h] { h.resume(); }); // lambda resume
            std::cout << "New thread: " << out.get_id() << '\n'; // this is OK
        }
        void await_resume() {}
    };

};*/
struct task // tasker coroutine
{
    struct promise_type
    {
        task get_return_object() { return {}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
    struct awaitable
    {
        std::jthread* p_out;
        bool await_ready() { return false; }
        void await_suspend(std::coroutine_handle<> h) // changer
        {
            std::jthread& out = *p_out;
            if (out.joinable())
                throw std::runtime_error("Output jthread parameter not empty");
            out = std::jthread([h] { h.resume(); }); // lambda resume
            std::cout << "New thread: " << out.get_id() << '\n'; // this is OK
        }
        void await_resume() {}
    };

};
template <typename T>
struct _gentask
{
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type // required
    {
        T value_;
        std::exception_ptr exception_;

        _gentask Generator(std::coroutine_handle<promise_type> handle) {
            return _gentask(std::__n4861::coroutine_handle());
        }

        _gentask get_return_object()
        {
            return Generator(handle_type::from_promise(*this));
        }
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void unhandled_exception() { exception_ = std::current_exception(); } // saving
        // exception

        template <std::convertible_to<T> From> // C++20 concept
        std::suspend_always yield_value(From&& from)
        {
            value_ = std::forward<From>(from); // caching the result in promise
            return {};
        }
        void return_void() { }
    };

    handle_type h_;

    _gentask(handle_type h)
            : h_(h)
    {
    }
    ~_gentask() { h_.destroy(); }
    explicit operator bool()
    {
        fill();
        return !h_.done();
    }
    T operator()()
    {
        fill();
        full_ = false;
        return std::move(h_.promise().value_);
    }

private:
    bool full_ = false;

    void fill()
    {
        if (!full_)
        {
            h_();
            if (h_.promise().exception_)
                std::rethrow_exception(h_.promise().exception_);
            // propagate coroutine exception in called context

            full_ = true;
        }
    }
};
struct awaitable
{
    std::jthread* p_out;
    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<> h) // changer
    {
        std::jthread& out = *p_out;
        if (out.joinable())
            throw std::runtime_error("Output jthread parameter not empty");
        out = std::jthread([h] { h.resume(); }); // lambda resume
        //std::cout << "New thread: " << out.get_id() << '\n'; // this is OK
    }
    void await_resume() {}
};
static auto switch_to_new_thread(std::jthread& out) // async func
{
    return awaitable{&out}; // send new awaible with new thread
}




/*task test(std::jthread& out) // test func
{
    std::cout << "BEFORE - " << std::this_thread::get_id() << '\n';
    co_await switch_to_new_thread(out); // change thread
    // awaiter destroyed here
    std::cout << "AFTER - " << std::this_thread::get_id() << '\n';
    for(int i = 0; ; i++)
    {
        cout << "123" << endl;
    }
    out.detach;
}*/