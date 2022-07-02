/* This file is part of the the dci project. Copyright (C) 2013-2022 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#pragma once

namespace dci::cmt::impl::scheduler
{
    template <class T>
    class EffortContainer
    {

    public:
        EffortContainer();
        ~EffortContainer();

        bool empty();
        void enqueue(T* v);
        T* dequeue();

        T* first() const;

    private:
        T* _first;
        T* _last;
    };



    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template <class T>
    EffortContainer<T>::EffortContainer()
        : _first(nullptr)
        , _last(nullptr)
    {

    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template <class T>
    EffortContainer<T>::~EffortContainer()
    {
        dbgAssert(!_first);
        dbgAssert(!_last);
    }


    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template <class T>
    bool EffortContainer<T>::empty()
    {
        dbgAssert(!_first == !_last);

        return !_first;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template <class T>
    void EffortContainer<T>::enqueue(T* v)
    {
        dbgAssert(!_first == !_last);

        dbgAssert(v != _first);
        dbgAssert(v != _last);
        dbgAssert(!v->_nextInEffortContainer);

        if(_last)
        {
            dbgAssert(!_last->_nextInEffortContainer);
            _last->_nextInEffortContainer = v;
            _last = v;
        }
        else
        {
            _first = _last = v;
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template <class T>
    T* EffortContainer<T>::dequeue()
    {
        dbgAssert(!_first == !_last);

        if(_first)
        {
            T* v = _first;
            _first = _first->_nextInEffortContainer;
            if(!_first)
            {
                dbgAssert(v == _last);
                dbgAssert(!v->_nextInEffortContainer);
                _last = nullptr;
            }
            else
            {
                dbgAssert(v != _last);
                dbgAssert(v->_nextInEffortContainer);
                v->_nextInEffortContainer = nullptr;
            }
            return v;
        }

        return nullptr;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template <class T>
    T* EffortContainer<T>::first() const
    {
        return _first;
    }
}
