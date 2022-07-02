/* This file is part of the the dci project. Copyright (C) 2013-2022 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#include <dci/cmt/pulser.hpp>
#include "impl/pulser.hpp"

namespace dci::cmt
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Pulser::Pulser(WakeMode wakeMode)
        : FaceLayout(wakeMode)
    {

    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Pulser::~Pulser()
    {

    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Pulser::wait()
    {
        return impl().wait();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Pulser::raise()
    {
        return impl().raise();
    }
}

