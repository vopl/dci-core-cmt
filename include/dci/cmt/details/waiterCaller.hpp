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
#include <dci/utils/dbg.hpp>
#include <dci/utils/staticSort.hpp>

#include <type_traits>
#include <concepts>
#include <bitset>

namespace dci::cmt::details
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    using WaitableImpl = dci::cmt::impl::Waitable*;

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template <class T> void extractWaitableImpl(T*) {}
    template <class T> auto extractWaitableImpl(T&& /*rv*/);

    template <class T> requires requires(T* pv) { {pv} -> std::convertible_to<dci::cmt::Waitable*>; }
    WaitableImpl extractWaitableImpl(T* pv)
    {
        return himpl::face2Impl(static_cast<dci::cmt::Waitable*>(pv));
    }

    template <class Wrapper> requires requires(Wrapper* pw) { pw->waitable(); }
    auto extractWaitableImpl(Wrapper* pw)
    {
        return extractWaitableImpl(pw->waitable());
    }

    template <class T>
    auto extractWaitableImpl(T&& rv)
    {
        return extractWaitableImpl(&rv);
    }

    template <class T>
    concept CWaitable = requires (T&& v) { {extractWaitableImpl(std::move(v))} -> std::same_as<WaitableImpl>; };

    template<typename T>
    concept CContainer = requires(T cnt)
    {
        {   std::size(cnt)                      } -> std::convertible_to<std::size_t>;
        {   std::begin(cnt)                     };
        {   std::end(cnt)                       };
        {   std::begin(cnt) != std::end(cnt)    } -> std::convertible_to<bool>;
        {   ++std::begin(cnt)                   };
        {   std::begin(cnt)++                   };
        {   *std::begin(cnt)                    } -> CWaitable;
    };

    template <class T>
    concept CWaitableOrContainer = CWaitable<T> || CContainer<T>;


    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template <class T> struct Val{ T v; };
    template <class T> struct Not{ T v; };
    template <class L, class R> struct And{ L l; R r; };
    template <class L, class R> struct Or{ L l; R r; };

    template <class T> struct IsExpr { static constexpr bool value = false; };
    template <class T> struct IsExpr<Val<T>> { static constexpr bool value = true; };
    template <class T> struct IsExpr<Not<T>> { static constexpr bool value = true; };
    template <class L, class R> struct IsExpr<And<L, R>> { static constexpr bool value = true; };
    template <class L, class R> struct IsExpr<Or<L, R>> { static constexpr bool value = true; };

    template <class T> concept CExpr = IsExpr<T>::value;

    template <class T>
    concept CVSrc = CWaitable<T>;

    namespace links
    {
        ///////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
        constexpr std::size_t countWaitables();

        template <class... Tail>
        constexpr std::size_t countWaitables(CContainer auto& cnt, Tail&... tail);

        template <class... Tail>
        constexpr std::size_t countWaitables(CWaitable auto&, Tail&... tail);

        ///////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
        constexpr std::size_t countWaitables()
        {
            return 0;
        }

        template <class... Tail>
        constexpr std::size_t countWaitables(CContainer auto& cnt, Tail&... tail)
        {
            return cnt.size() + countWaitables(tail...);
        }

        template <class... Tail>
        constexpr std::size_t countWaitables(CWaitable auto&, Tail&... tail)
        {
            return 1 + countWaitables(tail...);
        }

        /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
        inline void collectWaitables(WWLink *);

        template <class... Tail>
        void collectWaitables(WWLink* links, CContainer auto& cnt, Tail&... tail);

        template <class... Tail>
        void collectWaitables(WWLink* links, CWaitable auto& w, Tail&... tail);

        inline void collectWaitables(WWLink *)
        {
        }

        template <class... Tail>
        void collectWaitables(WWLink* links, CContainer auto& cnt, Tail&... tail)
        {
            for(CWaitable auto& w : cnt)
            {
                links->_waitable = extractWaitableImpl(w);
                links++;
            }

            return collectWaitables(links, tail...);
        }

        template <class... Tail>
        void collectWaitables(WWLink* links, CWaitable auto& w, Tail&... tail)
        {
            links->_waitable = extractWaitableImpl(w);
            collectWaitables(links+1, tail...);
        }
    }

    namespace links
    {
        template <class T>
        void collectWaitables(WWLink* links, Val<T>& v, std::size_t& valIdx);

        template <class T>
        void collectWaitables(WWLink* links, Not<T>& e, std::size_t& valIdx);

        template <class L, class R>
        void collectWaitables(WWLink* links, Or<L,R>& e, std::size_t& valIdx);

        template <class L, class R>
        void collectWaitables(WWLink* links, And<L,R>& e, std::size_t& valIdx);

        template <class T>
        void collectWaitables(WWLink* links, Val<T>& v, std::size_t& valIdx)
        {
            links[valIdx++]._waitable = extractWaitableImpl(v.v);
        }

        template <class T>
        void collectWaitables(WWLink* links, Not<T>& e, std::size_t& valIdx)
        {
            collectWaitables(links, e.v, valIdx);
        }

        template <class L, class R>
        void collectWaitables(WWLink* links, Or<L,R>& e, std::size_t& valIdx)
        {
            collectWaitables(links, e.l, valIdx);
            collectWaitables(links, e.r, valIdx);
        }

        template <class L, class R>
        void collectWaitables(WWLink* links, And<L,R>& e, std::size_t& valIdx)
        {
            collectWaitables(links, e.l, valIdx);
            collectWaitables(links, e.r, valIdx);
        }
    }

    namespace links
    {
        void markLinksRepeats(WWLink* links, auto linksAmount)
        {
            WWLink** ordered = static_cast<WWLink**>(alloca(sizeof(WWLink*) * linksAmount));
            for(std::size_t linkIndex{}; linkIndex < linksAmount; ++linkIndex)
                ordered[linkIndex] = links + linkIndex;

            auto cmp = [](WWLink* a, WWLink* b)
            {
                return a->_waitable < b->_waitable;
            };

            if constexpr(requires { utils::staticSort<linksAmount>(ordered, cmp); })
                utils::staticSort<linksAmount>(ordered, cmp);
            else
                std::sort(ordered, ordered+linksAmount, cmp);

            for(std::size_t linkIndex{1}; linkIndex < linksAmount; ++linkIndex)
                if(ordered[linkIndex-1]->_waitable == ordered[linkIndex]->_waitable)
                    ordered[linkIndex]->_state = WWLink::State::repeat;
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template <bool sizeIsConst, bool useExtMem, class... Waitables>
    struct Links
    {
        static constexpr std::size_t _size = sizeof...(Waitables);
        WWLink _data[_size];

        Links(Waitables&... waitables)
        {
            links::collectWaitables(_data, waitables...);
            links::markLinksRepeats(_data, std::integral_constant<std::size_t, _size>{});
        }
    };

    template <bool useExtMem, class... Waitables>
    struct Links<false, useExtMem, Waitables...>
    {
        std::size_t _size;
        WWLink*     _data;

        static WWLink* init(void* mem, std::size_t size)
        {
            static_assert(std::is_trivially_copyable_v<WWLink>);
            WWLink* data = static_cast<WWLink*>(mem);
            for(std::size_t i{}; i<size; ++i)
                data[i] = WWLink{};
            return data;
        }

        Links(std::size_t size, void* extMem, Waitables&... waitables) requires (useExtMem)
            : _size{size}
            , _data{init(extMem, _size)}
        {
            links::collectWaitables(_data, waitables...);
            links::markLinksRepeats(_data, _size);
        }

        Links(Waitables&... waitables) requires (!useExtMem)
            : _size{links::countWaitables(waitables...)}
            , _data{init(mm::heap::alloc(sizeof(WWLink) * _size), _size)}
        {
            links::collectWaitables(_data, waitables...);
            links::markLinksRepeats(_data, _size);
        }

        ~Links()
        {
            if constexpr(!useExtMem)
                mm::heap::free(_data);
        }
    };

    namespace expr
    {
        template <CVSrc VSrc>
        constexpr std::size_t countWaitables()
        {
            return 1;
        }

        template <CExpr Expr>
        constexpr std::size_t countWaitables()
        {
            if constexpr(requires(Expr e){ e.v;})
                return countWaitables<decltype(Expr::v)>();

            if constexpr(requires(Expr e){ e.l; e.r;})
                return countWaitables<decltype(Expr::l)>() + countWaitables<decltype(Expr::r)>();

            return 0;
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template <CExpr Expr>
    struct ExprLinks
    {
        static constexpr std::size_t _size = expr::countWaitables<Expr>();
        WWLink _data[_size];

        ExprLinks(Expr& expr)
        {
            std::size_t valIdx{};
            links::collectWaitables(_data, expr, valIdx);
            dbgAssert(valIdx == _size);

            links::markLinksRepeats(_data, std::integral_constant<std::size_t, _size>{});
        }
    };

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template <CExpr Expr>
    struct ExprEngine
    {
        static constexpr std::size_t _bitsAmount = expr::countWaitables<Expr>();
        using BitsContainer = std::array<std::byte, (_bitsAmount+CHAR_BIT-1) / CHAR_BIT>;
        BitsContainer _bitsContainer{};

        template <class T> class Tag {};

        template <class T>
        bool evalStep(Tag<Val<T>>, std::size_t& valIdx)
        {
            return bit(valIdx++);
        }

        template <class T>
        bool evalStep(Tag<Not<T>>, std::size_t& valIdx)
        {
            bool v = evalStep(Tag<T>{}, valIdx);
            return !v;
        }

        template <class L, class R>
        bool evalStep(Tag<Or<L,R>>, std::size_t& valIdx)
        {
            bool l = evalStep(Tag<L>{}, valIdx);
            bool r = evalStep(Tag<R>{}, valIdx);
            return l || r;
        }

        template <class L, class R>
        bool evalStep(Tag<And<L,R>>, std::size_t& valIdx)
        {
            bool l = evalStep(Tag<L>{}, valIdx);
            bool r = evalStep(Tag<R>{}, valIdx);
            return l && r;
        }

        static bool eval(void* eeData)
        {
            ExprEngine* self = static_cast<ExprEngine*>(eeData);

            std::size_t valIdx{};
            bool res = self->evalStep(Tag<Expr>{}, valIdx);
            dbgAssert(_bitsAmount == valIdx);
            return res;
        }

        bool bit(std::size_t idx)
        {
            dbgAssert(idx < _bitsAmount);
            return static_cast<unsigned char>(_bitsContainer[idx / CHAR_BIT] >> (idx % CHAR_BIT)) & 1;
        }

        std::byte* bits4Waiter()
        {
            return _bitsContainer.data();
        }

        std::bitset<_bitsAmount> bits4Result()
        {
            std::bitset<_bitsAmount> res;
            for(std::size_t i{}; i<res.size(); ++i)
                res[i] = bit(i);
            return res;
        };
    };

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    enum class Kind
    {
        any,
        all,
        expr
    };

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    struct DynamicState_any
    {
        std::size_t _acquiredIndex{};
    };

    struct DynamicState_all
    {
    };

    template <class... Waitables>
    struct DynamicState_expr : ExprEngine<Waitables...>
    {
    };

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template <Kind kind, bool sync=true, class... Waitables>
    auto waiterCaller(Waitables&... waitables)
    {
        static constexpr bool linksSizeIsConst = (CWaitable<Waitables>&& ...) || Kind::expr == kind;

        if constexpr(sync)
        {
            auto exec = [](auto&& links)
            {
                if constexpr(Kind::any == kind)
                {
                    std::size_t acquiredIndex{};
                    Waiter{links._data, links._size}.any(&acquiredIndex);
                    return acquiredIndex;
                }

                if constexpr(Kind::all == kind)
                {
                    Waiter{links._data, links._size}.all();
                    return;
                }

                if constexpr(Kind::expr == kind)
                {
                    ExprEngine<Waitables...> ee{};
                    Waiter{links._data, links._size}.expr(&ExprEngine<Waitables...>::eval, &ee, ee.bits4Waiter());
                    return ee.bits4Result();
                }
            };

            if constexpr(linksSizeIsConst && Kind::expr != kind)
                return exec(Links<linksSizeIsConst, false, Waitables...>{waitables...});
            else if constexpr(linksSizeIsConst && Kind::expr == kind)
                return exec(ExprLinks<Waitables...>{waitables...});
            else
            {
                std::size_t linksAmount = links::countWaitables(waitables...);
                return exec(Links<linksSizeIsConst, true, Waitables...>{linksAmount, alloca(sizeof(WWLink) * linksAmount), waitables...});
            }
        }
        else //cmt
        {
            using DynamicStateBase = decltype([]
            {
                if constexpr(Kind::any == kind)
                    return DynamicState_any{};
                if constexpr(Kind::all == kind)
                    return DynamicState_all{};
                if constexpr(Kind::expr == kind)
                    return DynamicState_expr<Waitables...>{};
            }());

            struct DynamicState
                : mm::heap::Allocable<DynamicState>
                , DynamicStateBase
            {
                using Promise = cmt::Promise<
                    decltype([]
                    {
                        if constexpr(Kind::any == kind)
                            return std::size_t{};
                        if constexpr(Kind::all == kind)
                            return;
                        if constexpr(Kind::expr == kind)
                            return std::bitset<expr::countWaitables<Waitables...>()>{};
                    }())>;

                Links<linksSizeIsConst, false, Waitables...>    _links;
                Waiter                                          _waiter;
                sbs::Owner                                      _sbsOwner;
                Promise                                         _promise;

                DynamicState(Waitables&... waitables)
                    : _links{waitables...}
                    , _waiter{_links._data, _links._size, &DynamicState::callback, this}
                {
                    _promise.canceled() += _sbsOwner * [this]
                    {
                        delete this;
                    };
                }

                ~DynamicState()
                {
                    _sbsOwner.flush();
                    _waiter.reset();
                }

                auto exec()
                {
                    auto res = _promise.future();

                    if constexpr(Kind::any == kind)
                        _waiter.any(&this->_acquiredIndex);

                    if constexpr(Kind::all == kind)
                        _waiter.all();

                    if constexpr(Kind::expr == kind)
                    {
                        _waiter.expr(
                                    &ExprEngine<Waitables...>::eval, static_cast<ExprEngine<Waitables...>*>(this),
                                    this->bits4Waiter(),
                                    &DynamicState::callback, this);
                    }

                    return res;
                }

                static void callback(void* cbData)
                {
                    DynamicState* self = static_cast<DynamicState *>(cbData);

                    if constexpr(Kind::any == kind)
                        self->_promise.resolveValue(self->_acquiredIndex);
                    if constexpr(Kind::all == kind)
                        self->_promise.resolveValue();
                    if constexpr(Kind::expr == kind)
                        self->_promise.resolveValue(self->bits4Result());

                    delete self;
                }
            };

            return (new DynamicState(waitables...))->exec();
        }
    }
}
