/* This file is part of the the dci project. Copyright (C) 2013-2021 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#include "event.hpp"
#include "details/waiter.hpp"
#include "scheduler.hpp"
#include <dci/cmt/task/stop.hpp>

namespace dci::cmt::impl
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Event::Event()
        : Waitable(
            [](const Waitable* w){ return static_cast<const Event*>(w)->canAcquire(); },
            [](Waitable* w){ return static_cast<Event*>(w)->tryAcquire(); })
        , Raisable([](Raisable* r){ return static_cast<Event*>(r)->raise(); })
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Event::~Event()
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Event::tryDestruction(Event* e)
    {
        e->~Event();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Event::isRaised() const
    {
        return _raised;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Event::reset()
    {
        _raised = false;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Event::wait()
    {
        throwTaskStopIfNeed();

        if(_raised)
        {
            return;
        }

        WWLink l;
        l._waitable = this;
        details::Waiter(&l, 1).any();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Event::raise()
    {
        if(!_raised)
        {
            _raised = true;

            std::pair<WWLink*, WWLink*> range = _links.range();
            for(WWLink* link = range.first; link!=range.second;)
            {
                WWLink* next = link->_next;
                link->_waiter->readyOffer(link);
                link = next;
            }
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Event::canAcquire() const
    {
        return _raised;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Event::tryAcquire()
    {
        throwTaskStopIfNeed();

        return _raised;
    }
}
