/* This file is part of the the dci project. Copyright (C) 2013-2021 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#pragma once

#include <dci/utils/intrusiveDlist.hpp>
#include <dci/cmt/task/key.hpp>
#include <dci/cmt/task/state.hpp>
#include <dci/sbs/owner.hpp>

namespace dci::cmt::task
{
    class Body;
    using CallAndDestroyExecutor = void (*)(Body* task, bool call, bool destroy) noexcept(true);
}

namespace dci::cmt::impl::scheduler
{
    template <class T>
    class EffortContainer;
}

namespace dci::cmt::impl::ctx
{
    class Fiber;
}

namespace dci::cmt::impl::task
{
    class Owner;
    class Face;

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    class Body final
        : public utils::IntrusiveDlistElement<Body, Owner>
    {
    public:
        Body(Owner* owner, cmt::task::CallAndDestroyExecutor callAndDestroyExecutor);
        ~Body();

    public:
        void setFiber(ctx::Fiber* fiber);
        ctx::Fiber* fiber();
        void callAndDestroy() noexcept(true);
        void destroy() noexcept(true);

    public:
        void subscribe(Face* face);
        void unsubscribe(Face* face);

    public:
        void setState(cmt::task::State state);

    public:
        cmt::task::Key key() const;
        bool isCurrent() const;
        cmt::task::State state() const;

        void stop(bool throwSelf = true);
        bool stopRequested() const;

        void ownTo(Owner* owner);
        Owner* owner();
        void detachOwner();

        sbs::Owner* sbsOwner();
    private:
        ctx::Fiber* _fiber {nullptr};

        sbs::Owner _sbsOwner;

        Owner*  _owner;
        cmt::task::CallAndDestroyExecutor _callAndDestroyExecutor;

        friend class dci::cmt::impl::scheduler::EffortContainer<Body>;
        Body*   _nextInEffortContainer = nullptr;

    private:
        cmt::task::State    _state {cmt::task::State::null};
        bool                _stopRequested {false};

    private:
        utils::IntrusiveDlist<Face, Body> _faces;
    };

}
