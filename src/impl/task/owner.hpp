/* This file is part of the the dci project. Copyright (C) 2013-2023 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#pragma once

#include <cstdint>
#include "body.hpp"
#include <dci/utils/intrusiveDlist.hpp>

namespace dci::cmt::impl::task
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    class Owner final
    {
        Owner(const Owner&) = delete;
        Owner(Owner&&) = delete;
        void operator=(const Owner&) = delete;
        void operator=(Owner&&) = delete;

    public:
        Owner();
        ~Owner();

    public:
        void subscribe(Body* task);
        void unsubscribe(Body* task);

        bool stopRequested() const;
        bool empty();

        void flush(bool andWait);
        void stop(bool andWait);
        void wait();

    private:
        void stopImpl(bool once, bool andWait);

    private:
        utils::IntrusiveDlist<Body, Owner>  _tasks;
        bool                                _stopRequested{false};
    };
}
