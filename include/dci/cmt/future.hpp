/* This file is part of the the dci project. Copyright (C) 2013-2023 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#pragma once

#include "waitable.hpp"
#include "details/transformHolder.hpp"
#include "details/future/statePtr.hpp"
#include "future/exception.hpp"
#include <exception>
#include <type_traits>

namespace dci::cmt
{
    template<class>
    class Promise;

    template<class T>
    struct FutureThen;

    template <class T, class R>
    struct FutureApply;

    template<class T = void>
    class Future
    {
        using StatePtr = details::future::StatePtr<T, true>;
        using State = details::future::State<T>;

        friend class cmt::Promise<T>;
        explicit Future(State* rawStatePtr);

    public:
        static constexpr bool _isVoid = State::_isVoid;
        using Value = typename State::Value;

        using Promise = cmt::Promise<T>;
        using Opposite = Promise;

    public:
        Future();
        Future(const Future& other);
        Future(Future&& other);
        ~Future();

        Future& operator=(const Future& other);
        Future& operator=(Future&& other);

        bool charged() const;
        void uncharge();

        //////////////////////////////////////////////////
        cmt::Waitable& waitable();
        void wait() const;
        bool waitValue() const;
        bool waitException() const;
        bool waitCancel() const;

        //////////////////////////////////////////////////
        bool resolveCancel();

        //////////////////////////////////////////////////
        bool resolved() const;
        bool resolvedValue() const;
        bool resolvedException() const;
        bool resolvedCancel() const;

        //////////////////////////////////////////////////
        decltype(auto) value() &&;
        decltype(auto) value() &;
        decltype(auto) value() const &;

        decltype(auto) detachValue();

        //////////////////////////////////////////////////
        std::exception_ptr exception() &&;
        std::exception_ptr& exception() &;
        const std::exception_ptr& exception() const &;

        std::exception_ptr&& detachException();

    public:

        /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
        template <class F>
        static constexpr bool thenValid = details::TransformHolder<F, cmt::Future<T>>::_valid;

        template <class F> void then(                   F&&) requires thenValid<F>;
        template <class F> void then(sbs::Owner& owner, F&&) requires thenValid<F>;
        template <class F> void then(sbs::Owner* owner, F&&) requires thenValid<F>;
        FutureThen<T> then();

        /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
        template <class R, class F>
        static constexpr bool applyValid = details::TransformHolder<F, cmt::Future<T>, cmt::Promise, R>::_valid;

        template <class R = details::transformHolder::AutoTag, class F> auto apply(                   F&&) requires applyValid<R, F>;
        template <class R = details::transformHolder::AutoTag, class F> auto apply(sbs::Owner& owner, F&&) requires applyValid<R, F>;
        template <class R = details::transformHolder::AutoTag, class F> auto apply(sbs::Owner* owner, F&&) requires applyValid<R, F>;
        template <class R = details::transformHolder::AutoTag> FutureApply<T, R> apply();

    private:
        void condition(auto&& f) const;

    private:
        StatePtr _statePtr;
    };

    template<class T>
    struct FutureThen
    {
        Future<T>& _future;
    };

    template <class T, class R>
    struct FutureApply
    {
        Future<T>& _future;
    };

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T = void>
    Future<std::decay_t<T>> readyFuture(T&& v);

    template<class T = void>
    Future<void> readyFuture() requires(std::is_void_v<T>);

    template<class T = void>
    Future<std::decay_t<T>> readyFuture(std::exception_ptr&& exc);

    template<class T = void>
    Future<std::decay_t<T>> readyFuture(const std::exception_ptr& exc);

    template<class T = void>
    Future<std::decay_t<T>> cancelledFuture();


    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    Future<T>::Future(State* rawStatePtr)
        : _statePtr(rawStatePtr)
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    Future<T>::Future()
        : _statePtr()
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    Future<T>::Future(const Future& other)
        : _statePtr(other._statePtr)
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    Future<T>::Future(Future&& other)
        : _statePtr(std::move(other._statePtr))
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    Future<T>::~Future()
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    Future<T>& Future<T>::operator=(const Future& other)
    {
        _statePtr = other._statePtr;
        return *this;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    Future<T>& Future<T>::operator=(Future&& other)
    {
        _statePtr = std::move(other._statePtr);
        return *this;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    bool Future<T>::charged() const
    {
        return _statePtr.charged();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    void Future<T>::uncharge()
    {
        _statePtr.uncharge();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    cmt::Waitable& Future<T>::waitable()
    {
        condition([&]{return _statePtr.charged();});
        return _statePtr->waitable();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    void Future<T>::wait() const
    {
        condition([&]{return _statePtr.charged();});
        if(!_statePtr->resolved())
        {
            _statePtr->wait();
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    bool Future<T>::waitValue() const
    {
        wait();
        return resolvedValue();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    bool Future<T>::waitException() const
    {
        wait();
        return resolvedException();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    bool Future<T>::waitCancel() const
    {
        wait();
        return resolvedCancel();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    bool Future<T>::resolveCancel()
    {
        condition([&]{return _statePtr.charged();});
        return _statePtr->resolveCancel();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    bool Future<T>::resolved() const
    {
        condition([&]{return _statePtr.charged();});
        return _statePtr->resolved();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    bool Future<T>::resolvedValue() const
    {
        condition([&]{return _statePtr.charged();});
        return _statePtr->resolvedValue();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    bool Future<T>::resolvedException() const
    {
        condition([&]{return _statePtr.charged();});
        return _statePtr->resolvedException();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    bool Future<T>::resolvedCancel() const
    {
        condition([&]{return _statePtr.charged();});
        return _statePtr->resolvedCancel();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    decltype(auto) Future<T>::value() &&
    {
        condition([&]{return _statePtr.charged();});
        wait();
        if constexpr(_isVoid)
        {
            _statePtr->value();
            return;
        }
        else
        {
            return std::move(_statePtr->value());
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    decltype(auto) Future<T>::value() &
    {
        condition([&]{return _statePtr.charged();});
        wait();
        if constexpr(_isVoid)
        {
            _statePtr->value();
            return;
        }
        else
        {
            return static_cast<Value&>(_statePtr->value());
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    decltype(auto) Future<T>::value() const &
    {
        condition([&]{return _statePtr.charged();});
        wait();
        if constexpr(_isVoid)
        {
            _statePtr->value();
            return;
        }
        else
        {
            return static_cast<const Value&>(_statePtr->value());
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    decltype(auto) Future<T>::detachValue()
    {
        condition([&]{return _statePtr.charged();});
        wait();
        if constexpr(_isVoid)
        {
            _statePtr->value();
            return;
        }
        else
        {
            return std::move(_statePtr->value());
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    std::exception_ptr Future<T>::exception() &&
    {
        condition([&]{return _statePtr.charged();});
        wait();
        return _statePtr->exception();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    std::exception_ptr& Future<T>::exception() &
    {
        condition([&]{return _statePtr.charged();});
        wait();
        return _statePtr->exception();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    const std::exception_ptr& Future<T>::exception() const &
    {
        condition([&]{return _statePtr.charged();});
        wait();
        return _statePtr->exception();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    std::exception_ptr&& Future<T>::detachException()
    {
        condition([&]{return _statePtr.charged();});
        wait();
        return std::move(_statePtr->exception());
    }
}

#include "promise.hpp"

namespace dci::cmt
{

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    template <class F>
    void Future<T>::then(F&& f) requires thenValid<F>
    {
        return then(static_cast<sbs::Owner*>(nullptr), std::forward<F>(f));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    template <class F>
    void Future<T>::then(sbs::Owner& owner, F&& f) requires thenValid<F>
    {
        return then(&owner, std::forward<F>(f));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    template <class F>
    void Future<T>::then(sbs::Owner* owner, F&& f) requires thenValid<F>
    {
        condition([&]{return _statePtr.charged();});

        using TransformHolder = details::TransformHolder<F, cmt::Future<T>>;

        static_assert(std::is_same<typename TransformHolder::TransformResult, void>::value, "future continuation expected to be void");

        class Continuation
            : public sbs::Subscription
            , public mm::heap::Allocable<Continuation>
        {
            State* _rawState;
            TransformHolder _transformHolder;

        public:
            Continuation(sbs::Owner* owner, State* rawState, F&& f)
                : sbs::Subscription(&Continuation::activator, owner)
                , _rawState(rawState)
                , _transformHolder(std::forward<F>(f))
            {
            }
            ~Continuation()
            {
                sbs::Subscription::removeSelf();
                _rawState->continuationRemoved();
            }

            static void activator(Subscription* cp, void* /*sp*/, std::uint_fast8_t flags)
            {
                dbgAssert(cp);
                Continuation* self = static_cast<Continuation*>(cp);
                //State* rawState = static_cast<State*>(sp);

                if(flags & sbs::Subscription::act)
                {
                    try
                    {
                        Future source(self->_rawState);
                        self->_transformHolder.invoke(&source);
                    }
                    catch(...)
                    {
                        dbgWarn("exception from future-continuation, but no results are expected");
                        abort();
                    }
                }

                if(flags & sbs::Subscription::del)
                {
                    delete self;
                }
            }

        };

        _statePtr->pushContinuation(new Continuation{owner, _statePtr.raw(), std::forward<F>(f)});
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    FutureThen<T> Future<T>::then()
    {
        return {*this};
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    template <class R, class F>
    auto Future<T>::apply(F&& f) requires applyValid<R, F>
    {
        return apply<R>(static_cast<sbs::Owner*>(nullptr), std::forward<F>(f));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    template <class R, class F>
    auto Future<T>::apply(sbs::Owner& owner, F&& f) requires applyValid<R, F>
    {
        return apply<R>(&owner, std::forward<F>(f));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    template <class R, class F>
    auto Future<T>::apply(sbs::Owner* owner, F&& f) requires applyValid<R, F>
    {
        condition([&]{return _statePtr.charged();});

        using TransformHolder = details::TransformHolder<F, cmt::Future<T>, cmt::Promise, R>;

        class Continuation
            : public sbs::Subscription
            , public mm::heap::Allocable<Continuation>
        {
            State* _rawState;
            TransformHolder _transformHolder;
            typename TransformHolder::DestinationStream _destinationPromise;

        public:

            typename TransformHolder::TransformResult future()
            {
                return _destinationPromise.future();
            }

            Continuation(sbs::Owner* owner, State* rawState, F&& f)
                : sbs::Subscription(&Continuation::activator, owner)
                , _rawState(rawState)
                , _transformHolder(std::forward<F>(f))
            {
                _destinationPromise.canceled() += [this](){delete this;};
            }

            ~Continuation()
            {
                sbs::Subscription::removeSelf();
                _rawState->continuationRemoved();
            }

            static void activator(Subscription* cp, void* /*sp*/, std::uint_fast8_t flags)
            {
                dbgAssert(cp);
                Continuation* self = static_cast<Continuation*>(cp);
                //State* rawState = static_cast<State*>(sp);

                if(flags & sbs::Subscription::act)
                {
                    try
                    {
                        Future source(self->_rawState);

                        if constexpr (
                            std::is_same<
                                decltype(self->_transformHolder.invoke(&source, &self->_destinationPromise)),
                                void
                            >::value &&
                            std::is_same<
                                cmt::Promise<void>,
                                typename TransformHolder::DestinationStream
                            >::value)
                        {
                            self->_transformHolder.invoke(&source, &self->_destinationPromise);

                            if(self->_destinationPromise && !self->_destinationPromise.resolved())
                            {
                                self->_destinationPromise.resolveValue();
                            }
                        }
                        else if constexpr (
                            std::is_same<
                                cmt::Promise<decltype(_transformHolder.invoke(&source, &_destinationPromise))>,
                                typename TransformHolder::DestinationStream
                            >::value)
                        {
                            auto ret = self->_transformHolder.invoke(&source, &self->_destinationPromise);

                            dbgAssert(self->_destinationPromise);//wasnt moved

                            if(self->_destinationPromise && !self->_destinationPromise.resolved())
                            {
                                self->_destinationPromise.resolveValue(ret);
                            }
                        }
                        else
                        {
                            self->_transformHolder.invoke(&source, &self->_destinationPromise);
                        }
                    }
                    catch(...)
                    {
                        if(!self->_destinationPromise)
                        {
                            dbgWarn("exception from future-continuation, but promise was moved out");
                            abort();
                        }

                        if(self->_destinationPromise.resolved())
                        {
                            dbgWarn("exception from future-continuation, but promise already resolved");
                            abort();
                        }
                        self->_destinationPromise.resolveException(std::current_exception());
                    }
                }

                if(flags & sbs::Subscription::del)
                {
                    delete self;
                }
            }

        }* continuation = new Continuation{owner, _statePtr.raw(), std::forward<F>(f)};

        auto res = continuation->future();

        _statePtr->pushContinuation(continuation);

        return res;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    template <class R>
    FutureApply<T, R> Future<T>::apply()
    {
        return {*this};
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    void Future<T>::condition(auto&& f) const
    {
        dbgAssert(f());
        if(!f())
        {
            abort();
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    Future<std::decay_t<T>> readyFuture(T&& v)
    {
        Promise<std::decay_t<T>> promise;
        promise.resolveValue(std::forward<T>(v));
        return promise.future();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    Future<void> readyFuture() requires(std::is_void_v<T>)
    {
        Promise<void> promise;
        promise.resolveValue();
        return promise.future();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    Future<std::decay_t<T>> readyFuture(std::exception_ptr&& exc)
    {
        Promise<std::decay_t<T>> promise;
        promise.resolveException(std::move(exc));
        return promise.future();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    Future<std::decay_t<T>> readyFuture(const std::exception_ptr& exc)
    {
        Promise<std::decay_t<T>> promise;
        promise.resolveException(exc);
        return promise.future();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T>
    Future<std::decay_t<T>> cancelledFuture()
    {
        Promise<std::decay_t<T>> promise;
        promise.resolveCancel();
        return promise.future();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T, class F>
    void operator+=(FutureThen<T>&& then, F&& f) requires Future<T>::template thenValid<F>
    {
        return then._future.then(std::forward<F>(f));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T, class F>
    void operator+=(FutureThen<T>& then, F&& f) requires Future<T>::template thenValid<F>
    {
        return then._future.then(std::forward<F>(f));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T, class F>
    void operator+=(FutureThen<T>&& then, sbs::OwnedFunctor<F>&& of) requires Future<T>::template thenValid<F>
    {
        return then._future.then(of._owner, std::forward<F>(of._f));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T, class F>
    void operator+=(FutureThen<T>& then, sbs::OwnedFunctor<F>&& of) requires Future<T>::template thenValid<F>
    {
        return then._future.then(of._owner, std::forward<F>(of._f));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T, class R, class F>
    decltype(auto) operator+=(FutureApply<T, R>&& apply, F&& f) requires Future<T>::template applyValid<R, F>
    {
        return apply._future.template apply<R>(std::forward<F>(f));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T, class R, class F>
    decltype(auto) operator+=(FutureApply<T, R>& apply, F&& f) requires Future<T>::template applyValid<R, F>
    {
        return apply._future.template apply<R>(std::forward<F>(f));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T, class R, class F>
    decltype(auto) operator+=(FutureApply<T, R>&& apply, sbs::OwnedFunctor<F>&& of) requires Future<T>::template applyValid<R, F>
    {
        return apply._future.template apply<R>(of._owner, std::forward<F>(of._f));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class T, class R, class F>
    decltype(auto) operator+=(FutureApply<T, R>& apply, sbs::OwnedFunctor<F>&& of) requires Future<T>::template applyValid<R, F>
    {
        return apply._future.template apply<R>(of._owner, std::forward<F>(of._f));
    }
}
