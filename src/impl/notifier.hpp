/* This file is part of the the dci project. Copyright (C) 2013-2022 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#pragma once

#include "waitable.hpp"
#include "raisable.hpp"
#include <dci/cmt/wakeMode.hpp>

namespace dci::cmt::impl
{
    class Notifier
        : public Waitable
        , public Raisable
    {
        Notifier(const Notifier&) = delete;
        void operator=(const Notifier&) = delete;

    public:
        Notifier(WakeMode wakeMode);
        ~Notifier();
        static void tryDestruction(Notifier* n);

        bool isRaised() const;
        void reset();

    public:
        void wait();

    public:
        void raise();

    private:
        bool canAcquire() const;
        bool tryAcquire();

    private:
        WakeMode    _wakeMode   = WakeMode::all;
        bool        _raised     = false;
    };
}
