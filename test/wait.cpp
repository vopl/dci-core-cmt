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
TEST(cmt, wait)
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
            auto res3 = spawnv() += [&]{
                waitAllAtOnce(e);
            };

            yield();

            EXPECT_FALSE(res1.resolved());
            EXPECT_FALSE(res2.resolved());
            EXPECT_FALSE(res3.resolved());

            e.raise();

            EXPECT_TRUE(res1.waitValue());
            EXPECT_TRUE(res2.waitValue());
            EXPECT_TRUE(res3.waitValue());

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
            auto res3 = spawnv() += [&]{
                waitAllAtOnce(e0, e1);
            };

            yield();

            EXPECT_FALSE(res1.resolved());
            EXPECT_FALSE(res2.resolved());
            EXPECT_FALSE(res3.resolved());

            e1.raise();

            EXPECT_TRUE(res1.waitValue());
            EXPECT_EQ(res1.value(), 1u);
            EXPECT_FALSE(res2.resolved());
            EXPECT_FALSE(res3.resolved());

            e0.raise();

            EXPECT_TRUE(res1.waitValue());
            EXPECT_TRUE(res2.waitValue());
            EXPECT_TRUE(res3.waitValue());
        }
    };

    executeReadyFibers();
}
