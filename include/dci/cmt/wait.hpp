/* This file is part of the the dci project. Copyright (C) 2013-2023 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#pragma once

#include "details/waiterCaller.hpp"
#include <cstdint>

namespace dci::cmt
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template <details::CWaitableOrContainer... Waitables> std::size_t waitAny (Waitables&... waitables);
    template <details::CWaitableOrContainer... Waitables> void        waitAll (Waitables&... waitables);

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template <details::CExpr Expr> std::bitset<details::expr::countWaitables<Expr>()> wait(Expr&& expr);
    template <details::CVSrc VSrc> std::bitset<1                                    > wait(VSrc&& vSrc);
}

namespace dci::cmt
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template <details::CWaitableOrContainer... Waitables> std::size_t waitAny(Waitables&... waitables)
    {
        return details::waiterCaller<details::Kind::any>(waitables...);
    }

    template <details::CWaitableOrContainer... Waitables> void waitAll(Waitables&... waitables)
    {
        return details::waiterCaller<details::Kind::all>(waitables...);
    }

    template <details::CExpr Expr> std::bitset<details::expr::countWaitables<Expr>()> wait(Expr&& expr)
    {
        return details::waiterCaller<details::Kind::expr>(expr);
    }

    template <details::CVSrc VSrc> std::bitset<1> wait(VSrc&& vSrc)
    {
        return wait(details::Val{&vSrc});
    }
}
