/* This file is part of the the dci project. Copyright (C) 2013-2023 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#include <dci/cmt/task/owner.hpp>
#include "impl/task/owner.hpp"

namespace dci::cmt::task
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Owner::Owner()
        : himpl::FaceLayout<Owner, impl::task::Owner>()
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Owner::~Owner()
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Owner::stopRequested() const
    {
        return impl().stopRequested();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Owner::empty()
    {
        return impl().empty();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Owner::flush(bool andWait)
    {
        return impl().flush(andWait);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Owner::stop(bool andWait)
    {
        return impl().stop(andWait);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Owner::wait()
    {
        return impl().wait();
    }

}
