/* This file is part of the the dci project. Copyright (C) 2013-2023 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#pragma once

#include <dci/cmt/details/wwLink.hpp>
#include <dci/primitives.hpp>
#include "../ctx/fiber.hpp"

namespace dci::cmt::impl::details
{
    using namespace dci::cmt::details;

    class Waitable;

    class Waiter final
    {
        using ExprEvaluator = bool(*)(void* eeData);

    public:
        Waiter(WWLink* links, std::size_t amount);
        Waiter(WWLink* links, std::size_t amount, void(*cb)(void* cbData), void* cbData);
        ~Waiter();

        void any(std::size_t* acquiredIndex);
        void all();
        void expr(ExprEvaluator ee, void* eeData, std::byte* bits);

        void reset();

    public:
        ctx::Fiber* fiber();
        bool readyOffer(WWLink* link);
        void waitableDead(WWLink* link);

    private:
        struct ModeSync
        {
            ctx::Fiber* _fiber{};
        };

        struct ModeAsync
        {
            void(*_cb)(void* cbData){};
            void* _cbData{};
        };

    private:
        struct StateNull
        {
        };

        struct StateAll
        {
        };

        struct StateAny
        {
            std::size_t* _acquiredIndex{};
        };

        struct StateExpr
        {
            ExprEvaluator   _evaluator{};
            void*           _evaluatorData{};
            std::byte*      _bits{};
        };

    private:
        template <class State> void exec(auto&&... args);

        bool ready(StateNull& state, WWLink* offeredFrom = {});
        bool ready(StateAll& state, WWLink* offeredFrom = {});
        bool ready(StateAny& state, WWLink* offeredFrom = {});
        bool ready(StateExpr& state, WWLink* offeredFrom = {});

        void commit(StateNull& state, WWLink* offeredFrom = {});
        void commit(StateAll& state, WWLink* offeredFrom = {});
        void commit(StateAny& state, WWLink* offeredFrom = {});
        void commit(StateExpr& state, WWLink* offeredFrom = {});

        void beginAcquire();
        void endAcquire();

    private:
        WWLink *    _links{};
        std::size_t _linksAmount{};

        Variant<ModeSync, ModeAsync> _mode;
        Variant<StateNull, StateAll, StateAny, StateExpr> _state{};
    };

}
