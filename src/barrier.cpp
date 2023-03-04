/* This file is part of the the dci project. Copyright (C) 2013-2023 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#include <dci/cmt/barrier.hpp>
#include "impl/barrier.hpp"

namespace dci::cmt
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Barrier::Barrier(std::size_t depth)
        : FaceLayout(depth)
    {

    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Barrier::~Barrier()
    {

    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Barrier::canStride() const
    {
        return impl().canStride();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Barrier::tryStride()
    {
        return impl().tryStride();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Barrier::stride()
    {
        return impl().stride();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Barrier::wait()
    {
        return impl().wait();
    }
}
