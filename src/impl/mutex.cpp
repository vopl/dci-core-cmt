/* This file is part of the the dci project. Copyright (C) 2013-2022 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#include "mutex.hpp"
#include "details/waiter.hpp"
#include "scheduler.hpp"
#include <dci/cmt/task/stop.hpp>

namespace dci::cmt::impl
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Mutex::Mutex(RecursionMode recursionMode)
        : Lockable(
              [](const Waitable* w){ return static_cast<const Mutex*>(w)->canLock(); },
              [](Waitable* w){ return static_cast<Mutex*>(w)->tryLock(); },
              [](Lockable* l){ return static_cast<Mutex*>(l)->unlock(); })
        , _recursionMode(recursionMode)
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Mutex::~Mutex()
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Mutex::tryDestruction(Mutex* m)
    {
        m->~Mutex();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Mutex::canLock() const
    {
        return
                (RecursionMode::nonRecursive == _recursionMode && ( !_counter                                                 )) ||
                (RecursionMode::recursive    == _recursionMode && ( !_owner || Scheduler::instance().currentFiber() == _owner ));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Mutex::tryLock()
    {
        throwTaskStopIfNeed();

        switch(_recursionMode)
        {
        case RecursionMode::nonRecursive:
            {
                if(!_counter)
                {
                    _counter = 1;
                    return true;
                }
            }
            break;

        case RecursionMode::recursive:
            {
                dbgAssert(Scheduler::instance().currentFiber());

                if(!_owner)
                {
                    dbgAssert(!_counter);
                    _owner = Scheduler::instance().currentFiber();
                    _counter++;
                    return true;
                }
                else if(Scheduler::instance().currentFiber() == _owner)
                {
                    _counter++;
                    return true;
                }
            }
            break;
        }

        return false;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Mutex::lock()
    {
        if(tryLock())
        {
            return;
        }

        WWLink l;
        l._waitable = this;
        details::Waiter(&l, 1).any();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Mutex::unlock()
    {
        switch(_recursionMode)
        {
        case RecursionMode::nonRecursive:
            {
                if(!_counter)
                {
                    dbgAssert(_links.empty());
                    return;
                }

                _counter = 1;

                std::pair<WWLink*, WWLink*> range = _links.range();
                for(WWLink* link = range.first; link!=range.second;)
                {
                    WWLink* next = link->_next;
                    if(link->_waiter->readyOffer(link))
                    {
                        dbgAssert(1 == _counter);
                        return;
                    }
                    link = next;
                }

                _counter = 0;
            }
            break;

        case RecursionMode::recursive:
            {
                dbgAssert(!!_owner == !!_counter);
                if(!_owner || !_counter)
                {
                    dbgAssert(_links.empty());
                    return;
                }

                _counter--;

                if(!_counter)
                {
                    _counter = 1;

                    std::pair<WWLink*, WWLink*> range = _links.range();
                    for(WWLink* link = range.first; link!=range.second;)
                    {
                        WWLink* next = link->_next;

                        _owner = link->_waiter->fiber();
                        //_counter = 1;

                        if(link->_waiter->readyOffer(link))
                        {
                            //link may be destroyed already
                            //dbgAssert(link->_waiter->fiber() == _owner);
                            dbgAssert(1 == _counter);
                            return;
                        }
                        link = next;
                    }

                    _owner = nullptr;
                    _counter = 0;
                }
            }
            break;
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Mutex::wait()
    {
        lock();
    }
}
