/* This file is part of the the dci project. Copyright (C) 2013-2021 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#include "scheduler.hpp"
#include "ctx/fiber.hpp"
#include <dci/cmt/task/stop.hpp>
#include <dci/cmt/task/face.hpp>
#include "task/face.hpp"

#include <algorithm>
#include <utility>

namespace dci::cmt::impl
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Scheduler::Scheduler()
        : _currentFiber(nullptr)
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Scheduler::~Scheduler()
    {
        {
            dbgAssert(_ready.empty());

            ctx::Fiber* fiber = _ready.dequeue();
            while(fiber)
            {
                task::Body* task = fiber->task();
                dbgAssert(task);
                if(task)
                {
                    task->destroy();
                }

                dbgAssert(!fiber->task());

                fiber->free();
                fiber = _ready.dequeue();
            }
        }

        {
            ctx::Fiber* fiber = _empty.dequeue();
            while(fiber)
            {
                dbgAssert(!fiber->task());
                fiber->free();
                fiber = _empty.dequeue();
            }
        }

        {
            dbgAssert(_hold.empty());

            ctx::Fiber* fiber = _hold.dequeue();
            while(fiber)
            {
                task::Body* task = fiber->task();
                dbgAssert(task);
                if(task)
                {
                    task->destroy();
                }

                dbgAssert(!fiber->task());

                fiber->free();
                fiber = _hold.dequeue();
            }
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Scheduler& Scheduler::instance()
    {
        return _instance;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Scheduler::spawnTask(task::Body* task)
    {
        dbgAssert(task);
        dbgAssert(cmt::task::State::null == task->state());

        ctx::Fiber* fiber = _empty.dequeue();
        if(!fiber)
        {
            fiber = ctx::Fiber::alloc(this);
            if(!fiber)
            {
                dbgWarn("unable to allocate new fiber");
                std::abort();
            }
        }

        dbgAssert(!fiber->task());
        fiber->setTask(task);
        task->setFiber(fiber);
        task->setState(cmt::task::State::ready);

        _ready.enqueue(fiber);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Scheduler::yield()
    {
        dbgAssert(_currentFiber);

        task::Body* task = _currentFiber->task();
        dbgAssert(task);
        dbgAssert(cmt::task::State::work == task->state());

        if(_ready.empty())
        {
            return false;
        }

        ctx::Fiber* current = _currentFiber;

        ctx::Fiber* next = dequeueReadyFiber();
        dbgAssert(next != current);

        task->setState(cmt::task::State::ready);
        _ready.enqueue(current);

        f2f(current, next);

        dbgAssert(current == _currentFiber);
        dbgAssert(current->task() == task);
        dbgAssert(cmt::task::State::work == task->state());

        return true;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Scheduler::hold()
    {
        dbgAssert(_currentFiber);

        task::Body* task = _currentFiber->task();
        (void)task;

        dbgAssert(task);
        dbgAssert(!task->stopRequested());
        dbgAssert(cmt::task::State::work == task->state());

        ctx::Fiber* current = _currentFiber;

        if(ctx::Fiber* next = dequeueReadyFiber())
        {
            f2f(current, next);
        }
        else
        {
            f2r(current);
        }

        dbgAssert(current == _currentFiber);
        dbgAssert(current->task() == task);
        dbgAssert(cmt::task::State::work == task->state());
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Scheduler::ready(ctx::Fiber* fiber)
    {
        task::Body* task = fiber->task();
        dbgAssert(task);

        if(cmt::task::State::hold == task->state())
        {
            task->setState(cmt::task::State::ready);
            _ready.enqueue(fiber);
        }
        else
        {
            dbgAssert(cmt::task::State::ready == task->state());
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Scheduler::ready4Stop(ctx::Fiber* fiber)
    {
        dbgAssert(fiber != _currentFiber);

        task::Body* task = fiber->task();
        dbgAssert(task);
        dbgAssert(task->stopRequested());

        if(cmt::task::State::hold == task->state())
        {
            task->setState(cmt::task::State::ready);
            _ready.enqueue(fiber);
        }
        else
        {
            dbgAssert(cmt::task::State::ready == task->state());
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Scheduler::executeReadyFibers()
    {
        dbgAssert(!_currentFiber);
        if(_currentFiber)
        {
            dbgWarn("executeReadyFibers available only from root context");
            std::abort();
        }

        bool res = false;

        while(ctx::Fiber* next = dequeueReadyFiber())
        {
            res = true;
            r2f(next);

            dbgAssert(!_currentFiber);
        }

        dbgAssert(_ready.empty());

        return res;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Scheduler::enumerateFibers(FiberEnumerationCallback cb, void* data)
    {
        dbgAssert(!_fiberEnumerationCallback);
        _fiberEnumerationCallback = cb;
        _fiberEnumerationCallbackData = data;

        _enumerateInitiator = std::exchange(_currentFiber, {});
        if(_enumerateInitiator)
        {
            //enumerate initiator
            _fiberEnumerationCallback(_enumerateInitiator->task()->state(), _fiberEnumerationCallbackData);

            //switch to root
            _enumerateInitiator->switchTo(&_rootContext, false);
        }
        else
        {
            //already in root, enumerate
            _fiberEnumerationCallback(cmt::task::State::null, _fiberEnumerationCallbackData);
        }

        //ready fibers
        _enumerateFiber = _ready.first();
        if(_enumerateFiber)
        {
            if(_enumerateInitiator)
            {
                _enumerateInitiator->switchTo(_enumerateFiber, false);
            }
            else
            {
                _rootContext.switchTo(_enumerateFiber);
            }
            dbgAssert(!_enumerateFiber);
        }

        //fibers on hold
        _enumerateFiber = _hold.first();
        if(_enumerateFiber)
        {
            if(_enumerateInitiator)
            {
                _enumerateInitiator->switchTo(_enumerateFiber, false);
            }
            else
            {
                _rootContext.switchTo(_enumerateFiber);
            }
            dbgAssert(!_enumerateFiber);
        }

        //already switched back to original fiber or root
        _currentFiber = std::exchange(_enumerateInitiator, {});

        dbgAssert(cb == _fiberEnumerationCallback);
        _fiberEnumerationCallback = {};
        _fiberEnumerationCallbackData = {};
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    ctx::Fiber* Scheduler::currentFiber()
    {
        return _currentFiber;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    task::Body* Scheduler::currentTask()
    {
        if(!_currentFiber)
        {
            return nullptr;
        }

        return _currentFiber->task();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Scheduler::fiberStarted()
    {
        handleFiberAwake();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Scheduler::taskCompleted()
    {
        dbgAssert(_currentFiber);
        ctx::Fiber* current = _currentFiber;
        dbgAssert(!current->task());
        _empty.enqueue(current);

        if(ctx::Fiber* next = dequeueReadyFiber())
        {
            if(next == current)
            {
                dbgAssert(current->task());
                dbgAssert(cmt::task::State::ready == current->task()->state());
                current->task()->setState(cmt::task::State::work);
            }
            else
            {
                f2f(current, next);
            }
        }
        else
        {
            f2r(current);
        }

        dbgAssert(current == _currentFiber);
        dbgAssert(current->task());
        dbgAssert(cmt::task::State::work == current->task()->state());
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    ctx::Fiber* Scheduler::dequeueReadyFiber()
    {
        ctx::Fiber* fiber = _ready.dequeue();
        if(fiber)
        {
            dbgAssert(fiber->task());
            dbgAssert(cmt::task::State::ready == fiber->task()->state());
            dbgAssert(fiber == fiber->task()->fiber());
            return fiber;
        }

        return nullptr;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Scheduler::f2f(ctx::Fiber* from, ctx::Fiber* to)
    {
        dbgAssert(from);
        dbgAssert(from == _currentFiber);
        if(from->task())
        {
            if(cmt::task::State::work == from->task()->state())
            {
                from->task()->setState(cmt::task::State::hold);
            }
            else
            {
                dbgAssert(cmt::task::State::ready == from->task()->state());
            }
        }

        dbgAssert(to);
        dbgAssert(to != _currentFiber);
        dbgAssert(to->task());
        dbgAssert(cmt::task::State::ready == to->task()->state());
        to->task()->setState(cmt::task::State::work);

        _currentFiber = to;
        from->switchTo(to);
        handleFiberAwake();

        dbgAssert(from == _currentFiber);
        dbgAssert(from->task());
        dbgAssert(cmt::task::State::work == from->task()->state());
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Scheduler::f2r(ctx::Fiber* from)
    {
        dbgAssert(from);
        dbgAssert(from == _currentFiber);
        if(from->task())
        {
            dbgAssert(cmt::task::State::work == from->task()->state());
            dbgAssert(!from->task()->stopRequested());
            from->task()->setState(cmt::task::State::hold);
        }

        _currentFiber = nullptr;
        from->switchTo(&_rootContext);
        handleFiberAwake();

        dbgAssert(from == _currentFiber);
        dbgAssert(from->task());
        dbgAssert(cmt::task::State::work == from->task()->state());
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Scheduler::r2f(ctx::Fiber* to)
    {
        dbgAssert(!_currentFiber);

        dbgAssert(to);
        dbgAssert(to != _currentFiber);
        dbgAssert(to->task());
        dbgAssert(cmt::task::State::ready == to->task()->state());
        dbgAssert(!to->task()->stopRequested());
        to->task()->setState(cmt::task::State::work);

        _currentFiber = to;
        _rootContext.switchTo(to);
        handleRootAwake();

        dbgAssert(!_currentFiber);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Scheduler::handleRootAwake()
    {
        dbgAssert(!_currentFiber);
        dbgAssert(!_enumerateFiber);
        while(_enumerateInitiator)
        {
            //enumerate root
            _fiberEnumerationCallback(cmt::task::State::null, _fiberEnumerationCallbackData);

            _rootContext.switchTo(_enumerateInitiator);
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Scheduler::handleFiberAwake()
    {
        while(_enumerateInitiator)
        {
            //enumerate
            dbgAssert(_enumerateFiber);
            dbgAssert(_fiberEnumerationCallback);
            _fiberEnumerationCallback(_enumerateFiber->task()->state(), _fiberEnumerationCallbackData);

            ctx::Fiber *prev = _enumerateFiber;
            _enumerateFiber = _enumerateFiber->_nextInEffortContainer;
            if(_enumerateFiber == _enumerateInitiator)
            {
                _enumerateFiber = _enumerateFiber->_nextInEffortContainer;
            }

            if(_enumerateFiber)
            {
                prev->switchTo(_enumerateFiber, false);
            }
            else
            {
                prev->switchTo(_enumerateInitiator, false);
            }
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Scheduler Scheduler::_instance{};
}
