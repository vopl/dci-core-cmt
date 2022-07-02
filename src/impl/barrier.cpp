/* This file is part of the the dci project. Copyright (C) 2013-2022 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#include "barrier.hpp"
#include "details/waiter.hpp"
#include "scheduler.hpp"
#include <dci/cmt/task/stop.hpp>

namespace dci::cmt::impl
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Barrier::Barrier(std::size_t depth)
        : Waitable(
            [](const Waitable* w){ return static_cast<const Barrier*>(w)->canStride();},
            [](Waitable* w){ return static_cast<Barrier*>(w)->tryStride();})
        , _depth(std::max(depth, std::size_t(1)))
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Barrier::~Barrier()
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Barrier::tryDestruction(Barrier* b)
    {
        b->~Barrier();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Barrier::canStride() const
    {
        return _linksAmount + 1 >= _depth;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Barrier::tryStride()
    {
        throwTaskStopIfNeed();

        if(canStride())
        {
            std::pair<WWLink*, WWLink*> range = _links.range();
            for(WWLink* link = range.first; link!=range.second;)
            {
                WWLink* next = link->_next;
                link->_waiter->readyOffer(link);
                link = next;
            }

            return true;
        }

        return false;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Barrier::stride()
    {
        if(tryStride())
        {
            return;
        }

        WWLink l;
        l._waitable = this;
        details::Waiter(&l, 1).any();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Barrier::wait()
    {
        stride();
    }
}
