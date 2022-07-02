/* This file is part of the the dci project. Copyright (C) 2013-2022 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#include <dci/cmt/details/waiter.hpp>
#include "impl/details/waiter.hpp"

namespace dci::cmt::details
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Waiter::Waiter(WWLink* links, std::size_t amount)
        : himpl::FaceLayout<Waiter, impl::details::Waiter>(links, amount)
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Waiter::~Waiter()
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::size_t Waiter::any()
    {
        return impl().any();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Waiter::all()
    {
        return impl().all();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Waiter::allAtOnce()
    {
        return impl().allAtOnce();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Waiter::any(void(*cb)(void* cbData, std::size_t index), void* cbData)
    {
        return impl().any(cb, cbData);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Waiter::all(void(*cb)(void* cbData), void* cbData)
    {
        return impl().all(cb, cbData);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Waiter::allAtOnce(void(*cb)(void* cbData), void* cbData)
    {
        return impl().allAtOnce(cb, cbData);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Waiter::reset()
    {
        return impl().reset();
    }

}
