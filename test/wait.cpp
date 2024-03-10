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
TEST(cmt, wait1)
{
    spawn() += []
    {
        {
            Event e;

            auto res1 = spawnv() += [&]{
                return waitAny(e);
            };
            auto res2 = spawnv() += [&]{
                waitAll(e);
            };

            yield();

            EXPECT_FALSE(res1.resolved());
            EXPECT_FALSE(res2.resolved());

            e.raise();

            EXPECT_TRUE(res1.waitValue());
            EXPECT_TRUE(res2.waitValue());

            EXPECT_EQ(res1.value(), 0u);
        }
    };

    executeReadyFibers();
}

/////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
TEST(cmt, wait2)
{
    spawn() += []
    {
        {
            Event e;

            auto res1 = spawnv() += [&]{
                return waitAny(e);
            };
            auto res2 = spawnv() += [&]{
                waitAll(e);
            };

            yield();

            EXPECT_FALSE(res1.resolved());
            EXPECT_FALSE(res2.resolved());

            e.raise();

            EXPECT_TRUE(res1.waitValue());
            EXPECT_TRUE(res2.waitValue());

            EXPECT_EQ(res1.value(), 0u);
        }

        {
            Event e0, e1;

            auto res1 = spawnv() += [&]{
                return waitAny(e0, e1);
            };
            auto res2 = spawnv() += [&]{
                waitAll(e0, e1);
            };

            yield();

            EXPECT_FALSE(res1.resolved());
            EXPECT_FALSE(res2.resolved());

            e1.raise();

            EXPECT_TRUE(res1.waitValue());
            EXPECT_EQ(res1.value(), 1u);
            EXPECT_FALSE(res2.resolved());

            e0.raise();

            EXPECT_TRUE(res1.waitValue());
            EXPECT_TRUE(res2.waitValue());
        }
    };

    executeReadyFibers();
}

/////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
TEST(cmt, waitExpr)
{
    spawn() += []
    {
        {
            Event e0, e1, e2;

            auto res1 = spawnv() += [&]{
                return wait(e0 || e1);
            };
            auto res2 = spawnv() += [&]{
                return wait(e0 && e1);
            };
            auto res3 = spawnv() += [&]{
                return wait((e0 || !e1) && e2);
            };

            yield();

            EXPECT_FALSE(res1.resolved());
            EXPECT_FALSE(res2.resolved());
            EXPECT_FALSE(res3.resolved());

            e0.raise();

            EXPECT_TRUE(res1.waitValue());
            EXPECT_EQ(res1.value(), (std::bitset<2>{0b01}));
            EXPECT_FALSE(res2.resolved());
            EXPECT_FALSE(res3.resolved());

            e1.raise();

            EXPECT_TRUE(res2.waitValue());
            EXPECT_EQ(res2.value(), (std::bitset<2>{0b11}));
            EXPECT_FALSE(res3.waitValue());

            e1.reset();
            e2.raise();

            EXPECT_TRUE(res3.waitValue());
            EXPECT_EQ(res3.value(), (std::bitset<3>{0b101}));
        }
    };

    executeReadyFibers();
}

/////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
TEST(cmt, waitAnyWithRepeats)
{
    spawn() += []
    {
        Event e1,e2;

        auto res = spawnv() += [&]{
            return waitAny(e1,e1,e1,e2,e1,e2);
        };

        yield();

        EXPECT_FALSE(res.resolved());

        e1.raise();

        EXPECT_TRUE(res.waitValue());
        EXPECT_EQ(res.value(), 0u);
    };

    executeReadyFibers();
}

/////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
TEST(cmt, waitAllWithRepeats)
{
    spawn() += []
    {
        Event e1,e2;

        auto res = spawnv() += [&]{
            return waitAll(e1,e1,e1,e2,e1,e2);
        };

        yield();

        EXPECT_FALSE(res.resolved());

        e1.raise();
        e2.raise();

        EXPECT_TRUE(res.waitValue());
    };

    executeReadyFibers();
}

/////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
TEST(cmt, waitExprWithRepeats)
{
    spawn() += []
    {
        Event e1,e2;

        auto res = spawnv() += [&]{
            return wait(e1||e1||e1||e2||e1||e2);
        };

        yield();

        EXPECT_FALSE(res.resolved());

        e1.raise();

        EXPECT_TRUE(res.waitValue());
        EXPECT_EQ(res.value(), (std::bitset<6>{0b010111}));
    };

    executeReadyFibers();
}
