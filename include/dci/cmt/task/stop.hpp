/* This file is part of the the dci project. Copyright (C) 2013-2021 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#pragma once

#include <dci/exception.hpp>

namespace dci::cmt::task
{
    class Stop
        : public dci::exception::Skeleton<Stop, dci::Exception>
    {
    public:
        using dci::exception::Skeleton<Stop, dci::Exception>::Skeleton;

    public:
        static constexpr Eid _eid {0x04,0xd0,0xc8,0x71,0x07,0x23,0x43,0x03,0x9d,0xdb,0x7e,0xfd,0x0e,0xa0,0xc5,0xce};
    };
}
