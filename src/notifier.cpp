/* This file is part of the the dci project. Copyright (C) 2013-2021 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#include <dci/cmt/notifier.hpp>
#include "impl/notifier.hpp"

namespace dci::cmt
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Notifier::Notifier(WakeMode wakeMode)
        : FaceLayout(wakeMode)
    {

    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Notifier::~Notifier()
    {

    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Notifier::isRaised() const
    {
        return impl().isRaised();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Notifier::reset()
    {
        return impl().reset();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Notifier::wait()
    {
        return impl().wait();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Notifier::raise()
    {
        return impl().raise();
    }
}
