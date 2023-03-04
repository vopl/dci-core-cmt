/* This file is part of the the dci project. Copyright (C) 2013-2023 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#pragma once

#include "root.hpp"
#include "../task/body.hpp"
#include <dci/mm/stack.hpp>

namespace dci::cmt::impl
{
    class Scheduler;

    namespace scheduler
    {
        template <class T>
        class EffortContainer;
    }
}

namespace dci::cmt::impl::ctx
{
    namespace fiber
    {
        template <bool needStack>
        class StackOrNone
        {
        protected:
            mm::Stack _stack;

            void assignStackIfNeed(mm::Stack&& stack)
            {
                _stack = std::move(stack);
            }

            void doStackCompactIfNeed()
            {
                _stack.compact();
            }
        };

        template <>
        class StackOrNone<false>
        {
        protected:
            void assignStackIfNeed(mm::Stack&&)
            {
            }

            void doStackCompactIfNeed()
            {
            }
        };
    }

    class Fiber
        : public Engine<Fiber>
        , private fiber::StackOrNone<Engine<Fiber>::_needStack>
    {
        Fiber& operator=(const Fiber&) = delete;

        Fiber(Scheduler* scheduler);
        ~Fiber();

    public:
        static Fiber* alloc(Scheduler* scheduler);
        void free();

        void setTask(task::Body* task);
        task::Body* task();

    public:
        template <class D2>
        void switchTo(Engine<D2>* to, bool doCompact = true)
        {
            if constexpr(Engine<Fiber>::_needStack)
            {
                if(doCompact)
                {
                    doStackCompactIfNeed();
                }
            }

            Engine::switchTo(to);
        }

    public:
        [[noreturn]] void contextProc();

    private:
        Scheduler*  _scheduler {};
        task::Body* _task {};

    private:
        friend class dci::cmt::impl::scheduler::EffortContainer<Fiber>;
        friend class dci::cmt::impl::Scheduler;
        Fiber*      _nextInEffortContainer {};
    };
}
