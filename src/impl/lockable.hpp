/* This file is part of the the dci project. Copyright (C) 2013-2021 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#pragma once

#include "waitable.hpp"
#include <dci/cmt/recursionMode.hpp>

namespace dci::cmt::impl
{
    class Lockable
        : public Waitable
    {
        Lockable(const Lockable&) = delete;
        void operator=(const Lockable&) = delete;

    public:
        Lockable(
                bool (* canLock)(const Waitable* lockableBase),
                bool (* tryLock)(Waitable* lockableBase),
                void (* unlock)(Lockable* lockable));
        ~Lockable();
        static void tryDestruction(Lockable*);

        bool canLock() const;
        bool tryLock();
        void lock();
        void unlock();

    private:
        void (* _unlock)(Lockable* self) = nullptr;
    };
}
