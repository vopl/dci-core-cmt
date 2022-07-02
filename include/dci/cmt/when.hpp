/* This file is part of the the dci project. Copyright (C) 2013-2022 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#pragma once

#include "details/waiterCaller.hpp"
#include "future.hpp"
#include <cstdint>

namespace dci::cmt
{
    template <class... Waitables> Future<std::size_t>    whenAny         (Waitables&...);
    template <class... Waitables> Future<void>           whenAll         (Waitables&...);
    template <class... Waitables> Future<void>           whenAllAtOnce   (Waitables&...);



    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template <class... Waitables> Future<std::size_t> whenAny(Waitables&... waitables)
    {
        return details::waiterCaller<details::WaitKind::any, false>(waitables...);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template <class... Waitables> Future<void> whenAll(Waitables&... waitables)
    {
        return details::waiterCaller<details::WaitKind::all, false>(waitables...);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template <class... Waitables> Future<void> whenAllAtOnce(Waitables&... waitables)
    {
        return details::waiterCaller<details::WaitKind::allAtOnce, false>(waitables...);
    }
}
