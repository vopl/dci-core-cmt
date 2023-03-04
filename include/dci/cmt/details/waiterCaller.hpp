/* This file is part of the the dci project. Copyright (C) 2013-2023 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#pragma once

#include "waiter.hpp"
#include "../waitable.hpp"
#include "../future.hpp"

#include <type_traits>

namespace dci::cmt::details
{
    namespace waiterCallerSpares
    {
        /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
        using WaitablePayload = dci::cmt::impl::Waitable*;


        /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
        //template <class T>
        //inline void extractOneWaitablePayload(const T&) {}

        template <class T>
        requires std::is_convertible_v<T*, dci::cmt::Waitable*>
        WaitablePayload extractOneWaitablePayload(T* v)
        {
            return himpl::face2Impl(static_cast<dci::cmt::Waitable*>(v));
        }

        template <class T>
        requires std::is_convertible_v<T*, dci::cmt::Waitable*>
        WaitablePayload extractOneWaitablePayload(T& v)
        {
            return extractOneWaitablePayload(&v);
        }

        template <class WaitableWrapper, class=decltype(&WaitableWrapper::waitable)>
        WaitablePayload extractOneWaitablePayload(WaitableWrapper& v)
        {
            return extractOneWaitablePayload(v.waitable());
        }

        template <class WaitableWrapper, class=decltype(&WaitableWrapper::waitable)>
        WaitablePayload extractOneWaitablePayload(WaitableWrapper* v)
        {
            return extractOneWaitablePayload(v->waitable());
        }

        template <class T>
        concept OneWaitable = std::is_same_v<WaitablePayload, decltype(extractOneWaitablePayload(std::declval<T&>()))>;

        template<typename Cnt>
        concept WaitablesContainer = requires(Cnt cnt)
        {
//            { cnt.size()    } -> std::size_t;
//            { cnt.begin()   } -> typename Cnt::iterator;
//            { cnt.end()     } -> typename Cnt::iterator;
//            { *cnt.begin()  } -> OneWaitable&;

            {   std::size_t{std::size(cnt)}         };
            {   std::begin(cnt)                     };
            {   std::end(cnt)                       };
            {   std::begin(cnt) != std::end(cnt)    };
            {   ++std::begin(cnt)                   };
            {   std::begin(cnt)++                   };
            {   *std::begin(cnt)                    } -> OneWaitable;
        };

        /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
        constexpr std::size_t waitablesAmount()
        {
            return 0;
        }

        template <class... Tail>
        constexpr std::size_t waitablesAmount(WaitablesContainer auto& cnt, Tail&... tail)
        {
            return cnt.size() + waitablesAmount(tail...);
        }

        template <class... Tail>
        constexpr std::size_t waitablesAmount(OneWaitable auto&, Tail&... tail)
        {
            return 1 + waitablesAmount(tail...);
        }

        /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
        inline void collectWaitables(WWLink *)
        {
        }

        template <class... Tail>
        void collectWaitables(WWLink* links, WaitablesContainer auto& cnt, Tail&... tail)
        {
            for(OneWaitable auto& w : cnt)
            {
                links->_waitable = extractOneWaitablePayload(w);
                links++;
            }

            return collectWaitables(links, tail...);
        }

        template <class... Tail>
        void collectWaitables(WWLink* links, OneWaitable auto& w, Tail&... tail)
        {
            links->_waitable = extractOneWaitablePayload(w);
            collectWaitables(links+1, tail...);
        }

        /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
        template <bool ctSize>
        struct TLinksDefiner;

        template <>
        struct TLinksDefiner<true>
        {
            template <class... Waitables>
            static auto exec(Waitables&... waitables)
            {
                using Res = WWLink[sizeof...(waitables)];
                using ResPtr = Res*;
                return ResPtr{};
            }
        };

        template <>
        struct TLinksDefiner<false>
        {
            template <class... Waitables>
            static auto exec(Waitables&...)
            {
                using Res = WWLink *;
                using ResPtr = Res *;
                return ResPtr{};
            }
        };
    }

    enum class WaitKind
    {
        any,
        all,
        allAtOnce
    };

    template <WaitKind kind, bool sync=true, class... Waitables>
    auto waiterCaller(Waitables&... waitables)
    {
        using namespace waiterCallerSpares;

        static constexpr bool ctSize = (OneWaitable<Waitables>&& ...);

        using TLinksPtr = decltype(TLinksDefiner<ctSize>::template exec(waitables...));
        using TLinks = std::remove_pointer_t<TLinksPtr>;

        if constexpr(sync)
        {
            const std::size_t linksAmount = waitablesAmount(waitables...);
            TLinks links;

            if constexpr(!ctSize)
            {
                links = new(alloca(sizeof(WWLink) * linksAmount)) WWLink[linksAmount];
            }

            collectWaitables(links, waitables...);
            Waiter waiter(links, linksAmount);

            if constexpr(kind == WaitKind::any)
            {
                return waiter.any();
            }

            if constexpr(kind == WaitKind::all)
            {
                return waiter.all();
            }

            if constexpr(kind == WaitKind::allAtOnce)
            {
                return waiter.allAtOnce();
            }
        }
        else //cmt
        {
            struct DynamicState
                : mm::heap::Allocable<DynamicState>
            {
                TLinks      _links;
                Waiter      _waiter;
                sbs::Owner  _sbsOwner;

                std::conditional_t
                <
                    kind == WaitKind::any,
                    cmt::Promise<std::size_t>,
                    cmt::Promise<void>
                >           _promise;

                static TLinks& linksInitializer(TLinks& links, Waitables&... waitables)
                {
                    if constexpr(!ctSize)
                    {
                        links = new (mm::heap::alloc(sizeof(WWLink) * waitablesAmount(waitables...))) WWLink[waitablesAmount(waitables...)];
                    }

                    return links;
                }

                DynamicState(Waitables&... waitables)
                    : _waiter(linksInitializer(_links, waitables...), waitablesAmount(waitables...))
                {
                    collectWaitables(_links, waitables...);

                    _promise.canceled() += _sbsOwner * [this]
                    {
                        delete this;
                    };
                }

                ~DynamicState()
                {
                    _sbsOwner.flush();
                    _waiter.reset();
                    if constexpr(!ctSize)
                    {
                        mm::heap::free(_links);
                    }
                }

                static void callback(void* cbData, std::size_t index)
                    requires (kind == WaitKind::any)
                {
                    DynamicState* self = static_cast<DynamicState *>(cbData);

                    if constexpr(kind == WaitKind::any)
                    {
                        self->_promise.resolveValue(index);
                    }

                    (void)index;

                    delete self;
                }

                static void callback(void* cbData)
                    requires (kind != WaitKind::any)
                {
                    DynamicState* self = static_cast<DynamicState *>(cbData);

                    if constexpr(kind != WaitKind::any)
                    {
                        self->_promise.resolveValue();
                    }

                    delete self;
                }
            };

            DynamicState* dynamicState = new DynamicState(waitables...);

            if constexpr(kind == WaitKind::any)
            {
                auto res = dynamicState->_promise.future();
                dynamicState->_waiter.any(&DynamicState::callback, dynamicState);
                return res;
            }

            if constexpr(kind == WaitKind::all)
            {
                auto res = dynamicState->_promise.future();
                dynamicState->_waiter.all(&DynamicState::callback, dynamicState);
                return res;
            }

            if constexpr(kind == WaitKind::allAtOnce)
            {
                auto res = dynamicState->_promise.future();
                dynamicState->_waiter.allAtOnce(&DynamicState::callback, dynamicState);
                return res;
            }
        }
    }
}
