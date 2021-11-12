/* This file is part of the the dci project. Copyright (C) 2013-2021 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#include <dci/test.hpp>
#include <dci/cmt.hpp>

using namespace dci::cmt;

/////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
TEST(cmt, waiter_fromContainer)
{
    bool allDone = false;
    spawn() += [&]
    {
        Event e1, e2;
        std::vector<Event*> es;
        es.emplace_back(&e2);

        auto res1 = whenAny(e1, es);
        auto res2 = whenAll(e1, es);
        auto res3 = whenAllAtOnce(e1, es);

        yield();

        EXPECT_FALSE(res1.resolved());
        EXPECT_FALSE(res2.resolved());
        EXPECT_FALSE(res3.resolved());

        e1.raise();

        EXPECT_TRUE(res1.waitValue());
        EXPECT_EQ(res1.value(), 0u);

        EXPECT_TRUE(res1.resolved());
        EXPECT_FALSE(res2.resolved());
        EXPECT_FALSE(res3.resolved());

        e2.raise();

        EXPECT_TRUE(res2.waitValue());
        EXPECT_TRUE(res3.waitValue());

        EXPECT_TRUE(res1.resolved());
        EXPECT_TRUE(res2.resolved());
        EXPECT_TRUE(res3.resolved());

        allDone = true;
    };

    executeReadyFibers();

    EXPECT_TRUE(allDone);
}
