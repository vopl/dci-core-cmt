/* This file is part of the the dci project. Copyright (C) 2013-2021 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#include "fiber.hpp"
#include <dci/mm/stack.hpp>
#include "../scheduler.hpp"
#include <dci/logger.hpp>

namespace dci::cmt::impl::ctx
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Fiber* Fiber::alloc(Scheduler* scheduler)
    {
        mm::Stack stack;
        stack.initialize();
        if(!stack.initialized())
        {
            dbgWarn("internal error");
            return nullptr;
        }

        Fiber* fiber;
        if(stack.growsDown())
        {
            fiber = reinterpret_cast<Fiber*>(stack.end() - sizeof(Fiber));
        }
        else
        {
            fiber = reinterpret_cast<Fiber*>(stack.begin());
        }

        new(fiber) Fiber(scheduler, std::move(stack));

        return fiber;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Fiber::Fiber(Scheduler* scheduler, mm::Stack&& stack)
        : _scheduler(scheduler)
        , _stack(std::move(stack))
    {
        if(_stack.growsDown())
        {
            char* end = reinterpret_cast<char *>(this);
            constructFiber(
                        _stack.growsDown(),
                        _stack.begin(),
                        static_cast<std::size_t>(end - _stack.begin()));
        }
        else
        {
            char* begin = reinterpret_cast<char *>(this) + sizeof(Fiber);
            constructFiber(
                        _stack.growsDown(),
                        begin,
                        static_cast<std::size_t>(_stack.end() - begin));
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Fiber::free()
    {
        mm::Stack stack(std::move(_stack));
        this->~Fiber();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Fiber::~Fiber()
    {
        dbgAssert(!_nextInEffortContainer);
        dbgAssert(!_task);
        destructFiber();
        //_stack = NULL;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Fiber::setTask(task::Body* task)
    {
        dbgAssert(!_task);
        _task = task;
        dbgAssert(_task);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    task::Body* Fiber::task()
    {
        return _task;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Fiber::contextProc()
    {
        dbgAssert(this == _scheduler->currentFiber());

        for(;;)
        {
            dbgAssert(this == _scheduler->currentFiber());
            dbgAssert(_task);

            _task->callAndDestroy();
            dbgAssert(_task);
            _task = nullptr;

            dbgAssert(this == _scheduler->currentFiber());
            _scheduler->taskCompleted();
        }
    }
}
