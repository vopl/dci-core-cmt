/* This file is part of the the dci project. Copyright (C) 2013-2023 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#include <dci/test.hpp>
#include <dci/cmt.hpp>

using namespace dci::cmt;

/////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
TEST(cmt, stopHalf)
{
    spawn() += []
    {
        Event e1, e2;
        Event done1, done2;
        task::Owner to;

        e1.raise();

        spawn() += to * [&]
        {
            try
            {
                waitAll(e1, e2);
            }
            catch(...)
            {
            }

            done1.raise();
        };

        spawn() += [&]
        {
            to.stop();
            done2.raise();
        };

        done1.wait();
        done2.wait();
    };

    executeReadyFibers();
}
