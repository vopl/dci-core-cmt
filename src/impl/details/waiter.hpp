/* This file is part of the the dci project. Copyright (C) 2013-2022 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#pragma once

#include <dci/cmt/details/wwLink.hpp>
#include "../ctx/fiber.hpp"
#include <cstdint>

namespace dci::cmt::impl::details
{
    using namespace dci::cmt::details;

    class Waitable;

    class Waiter final
    {
    public:
        Waiter(WWLink* links, std::size_t amount);
        ~Waiter();

        std::size_t any();
        void all();
        void allAtOnce();

        void any(void(*cb)(void* cbData, std::size_t index), void* cbData);
        void all(void(*cb)(void* cbData), void* cbData);
        void allAtOnce(void(*cb)(void* cbData), void* cbData);

        void reset();

    private:
        bool anyImpl();
        bool allImpl();
        bool allAtOnceImpl();

    public:
        ctx::Fiber* fiber();
        bool readyOffer(WWLink* link);
        void waitableDead(WWLink* link);

    private:
        void throwTaskStopIfNeed();

    private:
        WWLink *    _links;
        std::size_t _linksAmount;

        enum class Mode
        {
            null,

            any,
            anyAsync,

            all,
            allAsync,

            allAtOnce,
            allAtOnceAsync,
        } _mode;

        union
        {
            std::size_t _acquiredAmount;
            std::size_t _acquiredIndex;
        } _perModeState;

        union
        {
            void(*_cb)(void* cbData);
            void(*_cbIndex)(void* cbData, std::size_t index);
            ctx::Fiber* _fiber;
        } _asyncData;

        void* _asyncCallbackData;
    };

}
