/* This file is part of the the dci project. Copyright (C) 2013-2021 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#pragma once

#include "waitable.hpp"

namespace dci::cmt
{
    class API_DCI_CMT Lockable
        : public himpl::FaceLayout<Lockable, impl::Lockable, Waitable>
    {
        Lockable(const Lockable&) = delete;
        void operator=(const Lockable&) = delete;

    protected:
        DCI_INTEGRATION_APIDECL_LOCAL Lockable(himpl::FakeConstructionArg);
        Lockable() = delete;
        DCI_INTEGRATION_APIDECL_LOCAL ~Lockable();

    public:
        bool canLock() const;
        bool tryLock();
        void lock();
        void unlock();
    };
}
