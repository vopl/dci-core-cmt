/* This file is part of the the dci project. Copyright (C) 2013-2021 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#pragma once

#include "config.hpp"

#if defined(CMTSCHEDULER_CONTEXTENGINE_WINFIBER) && CMTSCHEDULER_CONTEXTENGINE_WINFIBER
#   include "engine_winfiber.hpp"
#elif defined(CMTSCHEDULER_CONTEXTENGINE_UCONTEXT) && CMTSCHEDULER_CONTEXTENGINE_UCONTEXT
#   include "engine_ucontext.hpp"
#elif defined(CMTSCHEDULER_CONTEXTENGINE_BOOSTCONTEXT) && CMTSCHEDULER_CONTEXTENGINE_BOOSTCONTEXT
#   include "engine_boostcontext.hpp"
#else
#   error "unknown context engine"
#endif

#include <cstdint>

namespace dci::cmt::impl::ctx
{
    class Root
        : public Engine<Root>
    {
        Root& operator=(const Root&) = delete;

    public:
        Root();
        ~Root();

        template <class D2>
        void switchTo(Engine<D2>* to)
        {
            Engine::switchTo(to);
        }
    };
}
