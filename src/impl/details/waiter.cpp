/* This file is part of the the dci project. Copyright (C) 2013-2021 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#include "waiter.hpp"
#include "../waitable.hpp"
#include "../scheduler.hpp"
#include <dci/cmt/task/stop.hpp>

namespace dci::cmt::impl::details
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Waiter::Waiter(WWLink* links, std::size_t amount)
        : _links(links)
        , _linksAmount(amount)
        , _mode(Mode::null)
        , _perModeState()
        , _asyncData()
        , _asyncCallbackData()
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Waiter::~Waiter()
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::size_t Waiter::any()
    {
        dbgAssert(Mode::null == _mode);
        dbgAssert(!_asyncData._fiber);
        dbgAssert(!_perModeState._acquiredIndex);

        throwTaskStopIfNeed();

        _mode = Mode::any;

        if(!anyImpl())
        {
            dbgAssert(Scheduler::instance().currentFiber());
            _asyncData._fiber = Scheduler::instance().currentFiber();
            Scheduler::instance().hold();

            reset();

            task::Body* task = _asyncData._fiber->task();
            if(task->stopRequested())
            {
                throw cmt::task::Stop{};
            }
        }

        return _perModeState._acquiredIndex;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Waiter::all()
    {
        dbgAssert(Mode::null == _mode);
        dbgAssert(!_asyncData._fiber);
        dbgAssert(!_perModeState._acquiredAmount);

        throwTaskStopIfNeed();

        _mode = Mode::all;

        if(!allImpl())
        {
            dbgAssert(Scheduler::instance().currentFiber());
            _asyncData._fiber = Scheduler::instance().currentFiber();
            Scheduler::instance().hold();

            dbgAssert(_asyncData._fiber->task()->stopRequested() || _perModeState._acquiredAmount == _linksAmount);

            reset();

            task::Body* task = _asyncData._fiber->task();
            if(task->stopRequested())
            {
                throw cmt::task::Stop{};
            }
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Waiter::allAtOnce()
    {
        dbgAssert(Mode::null == _mode);
        dbgAssert(!_asyncData._fiber);

        throwTaskStopIfNeed();

        _mode = Mode::allAtOnce;

        if(!allAtOnceImpl())
        {
            dbgAssert(Scheduler::instance().currentFiber());
            _asyncData._fiber = Scheduler::instance().currentFiber();
            Scheduler::instance().hold();

            reset();

            task::Body* task = _asyncData._fiber->task();
            if(task->stopRequested())
            {
                throw cmt::task::Stop{};
            }
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Waiter::any(void(*cb)(void* cbData, std::size_t index), void* cbData)
    {
        dbgAssert(Mode::null == _mode);
        dbgAssert(!_asyncData._cbIndex);
        dbgAssert(!_perModeState._acquiredIndex);

        _asyncData._cbIndex = cb;
        _asyncCallbackData = cbData;
        _mode = Mode::anyAsync;

        if(anyImpl())
        {
            _asyncData._cbIndex(_asyncCallbackData, _perModeState._acquiredIndex);
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Waiter::all(void(*cb)(void* cbData), void* cbData)
    {
        dbgAssert(Mode::null == _mode);
        dbgAssert(!_asyncData._cb);
        dbgAssert(!_perModeState._acquiredIndex);

        _asyncData._cb = cb;
        _asyncCallbackData = cbData;
        _mode = Mode::allAsync;

        if(allImpl())
        {
            _asyncData._cb(_asyncCallbackData);
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Waiter::allAtOnce(void(*cb)(void* cbData), void* cbData)
    {
        dbgAssert(Mode::null == _mode);
        dbgAssert(!_asyncData._cb);
        dbgAssert(!_perModeState._acquiredIndex);

        _asyncData._cb = cb;
        _asyncCallbackData = cbData;
        _mode = Mode::allAtOnceAsync;

        if(allAtOnceImpl())
        {
            _asyncData._cb(_asyncCallbackData);
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Waiter::reset()
    {
        switch(_mode)
        {
        case Mode::null:
            break;

        case Mode::any:
        case Mode::all:
        case Mode::allAtOnce:

        case Mode::anyAsync:
        case Mode::allAsync:
        case Mode::allAtOnceAsync:
            {
                _mode = Mode::null;

                WWLink* linksEnd = _links + _linksAmount;
                for(WWLink* iter = _links; iter != linksEnd; ++iter)
                {
                    if(iter->_connected)
                    {
                        dbgAssert(iter->_waitable);
                        dbgAssert(this == iter->_waiter);
                        iter->_waitable->endAcquire(iter);
                        iter->_connected = false;
                    }
                }
            }
            break;
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Waiter::anyImpl()
    {
        dbgAssert(Mode::any == _mode || Mode::anyAsync == _mode);
        dbgAssert(!_asyncData._fiber || Mode::anyAsync == _mode);
        dbgAssert(!_perModeState._acquiredIndex);

        WWLink* linksEnd = _links + _linksAmount;
        for(WWLink* link = _links; link != linksEnd; ++link)
        {
            if(link->_waitable->tryAcquire())
            {
                for(WWLink* link2 = _links; link2 != link; ++link2)
                {
                    dbgAssert(link2->_connected);
                    dbgAssert(link2->_waitable);
                    dbgAssert(this == link2->_waiter);
                    link2->_waitable->endAcquire(link2);
                    link2->_connected = false;
                }

                _perModeState._acquiredIndex = static_cast<std::size_t>(link - _links);
                _mode = Mode::null;
                return true;
            }
            else
            {
                dbgAssert(!link->_connected);
                dbgAssert(!link->_waiter);
                dbgAssert(link->_waitable);
                link->_waiter = this;
                link->_waitable->beginAcquire(link);
                link->_connected = true;
            }
        }

        return false;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Waiter::allImpl()
    {
        dbgAssert(Mode::all == _mode || Mode::allAsync == _mode);
        dbgAssert(!_asyncData._fiber || Mode::allAsync == _mode);
        dbgAssert(!_perModeState._acquiredAmount);

        WWLink* linksEnd = _links + _linksAmount;
        for(WWLink* link = _links; link != linksEnd; ++link)
        {
            if(link->_waitable->tryAcquire())
            {
                _perModeState._acquiredAmount++;
            }
            else
            {
                dbgAssert(!link->_connected);
                dbgAssert(!link->_waiter);
                dbgAssert(link->_waitable);
                link->_waiter = this;
                link->_waitable->beginAcquire(link);
                link->_connected = true;
            }
        }

        if(_perModeState._acquiredAmount >= _linksAmount)
        {
            dbgAssert(_perModeState._acquiredAmount == _linksAmount);
            _mode = Mode::null;
            return true;
        }

        return false;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Waiter::allAtOnceImpl()
    {
        dbgAssert(Mode::allAtOnce == _mode || Mode::allAtOnceAsync == _mode);
        dbgAssert(!_asyncData._fiber || Mode::allAtOnceAsync == _mode);

        bool allReady = true;

        WWLink* linksEnd = _links + _linksAmount;
        for(WWLink* link = _links; link != linksEnd; ++link)
        {
            if(!link->_waitable->canAcquire())
            {
                allReady = false;
                break;
            }
        }

        if(allReady)
        {
            for(WWLink* link = _links; link != linksEnd; ++link)
            {
                bool b = link->_waitable->tryAcquire();
                dbgAssert(b);
                (void)b;
            }

            _mode = Mode::null;
            return true;
        }

        for(WWLink* link = _links; link != linksEnd; ++link)
        {
            dbgAssert(!link->_connected);
            dbgAssert(!link->_waiter);
            dbgAssert(link->_waitable);
            link->_waiter = this;
            link->_waitable->beginAcquire(link);
            link->_connected = true;
        }

        return false;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    ctx::Fiber* Waiter::fiber()
    {
        switch(_mode)
        {
        case Mode::null:
            dbgWarn("never here");
            return nullptr;

        case Mode::any:
        case Mode::all:
        case Mode::allAtOnce:
            dbgAssert(_asyncData._fiber);
            return _asyncData._fiber;

        case Mode::anyAsync:
        case Mode::allAsync:
        case Mode::allAtOnceAsync:
            return Scheduler::instance().currentFiber();
        }

        dbgWarn("never here");
        return nullptr;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Waiter::readyOffer(WWLink* link)
    {
        dbgAssert(link >= _links && link < _links+_linksAmount);
        dbgAssert(this == link->_waiter);

        switch(_mode)
        {
        case Mode::null:
            return true;

        case Mode::any:
        case Mode::anyAsync:
            {
                WWLink* linksEnd = _links + _linksAmount;
                for(WWLink* iter = _links; iter != linksEnd; ++iter)
                {
                    if(iter->_connected)
                    {
                        dbgAssert(iter->_waitable);
                        dbgAssert(this == iter->_waiter);
                        iter->_waitable->endAcquire(iter);
                        iter->_connected = false;
                    }
                }

                _perModeState._acquiredIndex = static_cast<std::size_t>(link - _links);

                if(Mode::anyAsync == _mode)
                {
                    _mode = Mode::null;
                    _asyncData._cbIndex(_asyncCallbackData, _perModeState._acquiredIndex);
                }
                else
                {
                    _mode = Mode::null;
                    Scheduler::instance().ready(_asyncData._fiber);
                }
                return true;
            }
            break;

        case Mode::all:
        case Mode::allAsync:
            {
                if(link->_connected)
                {
                    dbgAssert(link->_waitable);
                    dbgAssert(this == link->_waiter);
                    link->_waitable->endAcquire(link);
                    link->_connected = false;
                }

                _perModeState._acquiredAmount++;
                dbgAssert(_perModeState._acquiredAmount <= _linksAmount);

                if(_perModeState._acquiredAmount == _linksAmount)
                {
                    if(Mode::allAsync == _mode)
                    {
                        _mode = Mode::null;
                        _asyncData._cb(_asyncCallbackData);
                    }
                    else
                    {
                        _mode = Mode::null;
                        Scheduler::instance().ready(_asyncData._fiber);
                    }
                }

                return true;
            }
            break;

        case Mode::allAtOnce:
        case Mode::allAtOnceAsync:
            {
                WWLink* linksEnd = _links + _linksAmount;
                for(WWLink* iter = _links; iter != linksEnd; ++iter)
                {
                    if(iter == link)
                    {
                        continue;
                    }

                    if(iter->_waitable)
                    {
                        if(!iter->_waitable->canAcquire())
                        {
                            return false;
                        }
                    }
                }
                for(WWLink* iter = _links; iter != linksEnd; ++iter)
                {
                    if(iter->_connected)
                    {
                        dbgAssert(iter->_waitable);
                        dbgAssert(this == iter->_waiter);
                        iter->_waitable->endAcquire(iter);
                        iter->_connected = false;
                    }

                    if(link == iter)
                    {
                        continue;
                    }

                    if(iter->_waitable)
                    {
                        bool b = iter->_waitable->tryAcquire();
                        dbgAssert(b);
                        (void)b;
                    }
                }

                if(Mode::allAtOnceAsync == _mode)
                {
                    _mode = Mode::null;
                    _asyncData._cb(_asyncCallbackData);
                }
                else
                {
                    _mode = Mode::null;
                    Scheduler::instance().ready(_asyncData._fiber);
                }
                return true;
            }
            break;
        }

        dbgWarn("never here");
        abort();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Waiter::waitableDead(WWLink* link)
    {
        dbgAssert(link >= _links && link < _links+_linksAmount);
        dbgAssert(this == link->_waiter);

        if(link->_connected)
        {
            dbgAssert(link->_waitable);
            dbgAssert(link->_waiter);
            link->_waitable->endAcquire(link);
            link->_connected = false;
        }

        link->_waitable = nullptr;
        readyOffer(link);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Waiter::throwTaskStopIfNeed()
    {
        ctx::Fiber* currentFiber = Scheduler::instance().currentFiber();

        if(currentFiber)
        {
            dbgAssert(currentFiber->task());
            if(currentFiber->task()->stopRequested())
            {
                throw cmt::task::Stop{};
            }
        }
    }

}
