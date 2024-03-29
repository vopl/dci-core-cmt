/* This file is part of the the dci project. Copyright (C) 2013-2023 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#pragma once

#include <dci/utils/intrusiveDlist.hpp>
#include <cstdint>

namespace dci::cmt::impl
{
    class Waitable;
}

namespace dci::cmt::impl::details
{
    class Waiter;
}

namespace dci::cmt::details
{

    struct WWLink
        : utils::IntrusiveDlistElement<WWLink>
    {
        impl::details::Waiter * _waiter {};
        impl::Waitable *        _waitable {};
        enum class State
        {
            regular,
            regularConnected,
            repeat,
        }                       _state{State::regular};
    };
}
