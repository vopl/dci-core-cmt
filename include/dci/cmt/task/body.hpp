/* This file is part of the the dci project. Copyright (C) 2013-2022 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#pragma once
#include <dci/cmt/api.hpp>
#include <dci/himpl.hpp>
#include <dci/cmt/implMetaInfo.hpp>

namespace dci::cmt::task
{
    class Owner;

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    class API_DCI_CMT Body
        : public himpl::FaceLayout<Body, impl::task::Body>
    {
        Body() = delete;
        Body(const Body&) = delete;
        void operator=(const Body&) = delete;

    protected:
        using CallAndDestroyExecutor = void (*)(Body* task, bool call, bool destroy) noexcept(true);

    protected:
        Body(Owner* owner, CallAndDestroyExecutor callAndDestroyExecutor);
        ~Body();
    };
}
