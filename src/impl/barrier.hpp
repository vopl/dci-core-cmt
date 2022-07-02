/* This file is part of the the dci project. Copyright (C) 2013-2022 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#pragma once

#include "waitable.hpp"

namespace dci::cmt::impl
{
    class Barrier
        : public Waitable
    {
        Barrier(const Barrier&) = delete;
        void operator=(const Barrier&) = delete;

    public:
        Barrier(std::size_t depth);
        ~Barrier();
        static void tryDestruction(Barrier* b);

        bool canStride() const;
        bool tryStride();
        void stride();

    public:
        void wait();

    private:
        std::size_t _depth = 0;
    };
}
