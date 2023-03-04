/* This file is part of the the dci project. Copyright (C) 2013-2023 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#pragma once

#include "task/body.hpp"
#include "ctx/root.hpp"
#include "ctx/fiber.hpp"
#include "scheduler/effortContainer.hpp"
//#include <memory>

namespace dci::cmt::task
{
    class Face;
}

namespace dci::cmt::impl
{
    class Scheduler
    {
    public:
        Scheduler();
        ~Scheduler();

    public:
        static Scheduler& instance();

    public:
        void spawnTask(task::Body* task);
        bool yield();
        void hold();
        void ready(ctx::Fiber* fiber);
        void ready4Stop(ctx::Fiber* fiber);
        bool executeReadyFibers();

        using FiberEnumerationCallback = void(*)(cmt::task::State, void*);
        void enumerateFibers(FiberEnumerationCallback, void*);

    public:
        ctx::Fiber* currentFiber();
        task::Body* currentTask();

        void fiberStarted();
        void taskCompleted();

    private:
        ctx::Fiber* dequeueReadyFiber();
        void f2f(ctx::Fiber* from, ctx::Fiber* to);
        void f2r(ctx::Fiber* from);
        void r2f(ctx::Fiber* to);

    private:
        void handleRootAwake();
        void handleFiberAwake();

    private:
        static Scheduler    _instance;
        ctx::Root           _rootContext;
        ctx::Fiber*         _currentFiber;

        scheduler::EffortContainer<ctx::Fiber>  _hold;
        scheduler::EffortContainer<ctx::Fiber>  _empty;
        scheduler::EffortContainer<ctx::Fiber>  _ready;

    private:
        ctx::Fiber*                 _enumerateInitiator{};
        ctx::Fiber*                 _enumerateFiber{};
        FiberEnumerationCallback    _fiberEnumerationCallback {};
        void*                       _fiberEnumerationCallbackData {};
    };
}
