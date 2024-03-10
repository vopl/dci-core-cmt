/* This file is part of the the dci project. Copyright (C) 2013-2023 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#pragma once

#include "details/waiterCaller.hpp"

namespace dci::cmt
{
    inline namespace operators
    {
        template <details::CExpr T> constexpr auto operator!(T   v) { return details::Not{v}; }
        template <details::CVSrc T> constexpr auto operator!(T&& v) { return !details::Val{&v}; }

        template <details::CExpr L, details::CExpr R> constexpr auto operator ||(L   l, R   r) { return details::Or{l, r}; }
        template <details::CVSrc L, details::CExpr R> constexpr auto operator ||(L&& l, R   r) { return details::Val{&l} ||               r ; }
        template <details::CExpr L, details::CVSrc R> constexpr auto operator ||(L   l, R&& r) { return l                || details::Val{&r}; }
        template <details::CVSrc L, details::CVSrc R> constexpr auto operator ||(L&& l, R&& r) { return details::Val{&l} || details::Val{&r}; }

        template <details::CExpr L, details::CExpr R> constexpr auto operator &&(L   l, R   r) { return details::And{l, r}; }
        template <details::CVSrc L, details::CExpr R> constexpr auto operator &&(L&& l, R   r) { return details::Val{&l} &&               r ; }
        template <details::CExpr L, details::CVSrc R> constexpr auto operator &&(L   l, R&& r) { return l                && details::Val{&r}; }
        template <details::CVSrc L, details::CVSrc R> constexpr auto operator &&(L&& l, R&& r) { return details::Val{&l} && details::Val{&r}; }
    }
}
