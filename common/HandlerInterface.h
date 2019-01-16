#pragma once

#include <memory>


namespace FileExchange {

class HandlerInterface
{
public:
    virtual ~HandlerInterface() = default;

    virtual bool onNext(bool ok) = 0;

    virtual void cancel() = 0;
};

using HandlerPtr = std::unique_ptr<HandlerInterface>;
using HandlerTag = HandlerPtr*;

}

