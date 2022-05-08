#include <iostream>
#include <memory>
#include <coroutine>

class lazyInt
{
public:
    int value() const{
        if(auto handle = shared_state_->handle)
        {
            handle.resume();
        }
        return shared_state_->value;}
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;
private:
    lazyInt(handle_type handle): shared_state_{std::make_shared<state>()}
    {
        shared_state_->handle = handle;
    }

    struct state
            {
        int value =0;
        handle_type handle;
        ~state()
        {
            if(handle)
            {
                handle.destroy();
            }
        }
            };
    std::shared_ptr<state> shared_state_;
};

struct lazyInt::promise_type
{
    std::weak_ptr<state> shared_state_;
    auto get_return_object()
    {
        std::cout << "promise_type::get_return_object" << std::endl;
        auto handle = handle_type::from_promise(*this);
        auto return_object = lazyInt{handle};
        shared_state_ = return_object.shared_state_;
        return return_object;
    }
    auto initial_suspend()
    {
        std::cout << "promise_type::initial_suspend()\n";
        return std::suspend_always{};
    }
    auto final_suspend() noexcept
    {
        std::cout << "promise_type::final_suspend()\n";
        if(auto state = shared_state_.lock())
        {
            state->handle = nullptr;
        }
        return std::suspend_never{};
    }
    auto return_value(int value)
    {
        std::cout << "promise_type::return_void()\n";
        if(auto state = shared_state_.lock())
        {
            state->value = value;
        }
    }
    auto unhandled_exception()
    {
        std::cout << "promise_type::unhandled_exception()\n";
        std::terminate();
    }
};

lazyInt my_coroutine()
{
    std::cout << "The Coroutine has resumed\n";
    co_return 66;
}

// this is how the developer should used this library
lazyInt MyCoroutine()
{
    int value =0;
    std::cout << "please enter a value : \n";
    std::cin >> value;
    co_return value;
}


int main()
{
    std::cout << "Calling the coroutine...\n";
    auto result = my_coroutine();
    std::cout << "Coroutine has returned : " << result.value() << std::endl;
    return 0;
}
