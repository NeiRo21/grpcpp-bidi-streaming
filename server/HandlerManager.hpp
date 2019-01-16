#pragma once

#include "HandlerInterface.h"

#include <memory>
#include <mutex>
#include <forward_list>
#include <utility>


namespace FileExchange {

class HandlerManager final
{
public:
    HandlerManager()  = default;
    ~HandlerManager() = default;

    HandlerManager(const HandlerManager&)            = delete;
    HandlerManager& operator=(const HandlerManager&) = delete;
    HandlerManager(HandlerManager&&)                 = delete;
    HandlerManager& operator=(HandlerManager&&)      = delete;

    HandlerPtr& getHandler(HandlerTag tag) const
    {
        //TODO assert
        //TODO comment
        return *tag;
    }

    template<class HANDLER, class... ARGS>
    HandlerTag addHandler(ARGS... args)
    {
        HandlerPtr* handlerPtr = nullptr;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            handlers_.push_front(HandlerPtr());
            handlerPtr = &handlers_.front();
        }

        auto tag = handlerPtr;
        *handlerPtr = std::make_unique<HANDLER>(tag, std::forward<ARGS>(args)...);
        return tag;
    }

    //TODO
    //void removeHandler(HandlerTag tag);

private:
    std::forward_list<HandlerPtr> handlers_;
    std::mutex                    mutex_;
};

}

