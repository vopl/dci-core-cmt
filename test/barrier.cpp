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
TEST(cmt, barrier)
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    //construction/destruction
    {
        int progress = 0;
        Barrier b(0);

        EXPECT_TRUE(b.canStride());

        spawn() += [&](){progress=1;b.stride();progress=2;};
        executeReadyFibers();
        EXPECT_EQ(progress, 2);
    }
    {
        int progress = 0;
        Barrier b(1);

        EXPECT_TRUE(b.canStride());

        spawn() += [&](){progress=1;b.stride();progress=2;};
        executeReadyFibers();
        EXPECT_EQ(progress, 2);
    }

    {
        int progress = 0;
        {
            Barrier b(2);

            EXPECT_FALSE(b.canStride());

            spawn() += [&](){progress=1;b.stride();progress=2;};
            executeReadyFibers();
            EXPECT_EQ(progress, 1);
        }//b destruction
        executeReadyFibers();
        EXPECT_EQ(progress, 2);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    //wait and stride are equal
    {
        int progress = 0;
        Barrier b(2);

        spawn() += [&](){b.wait();progress++;};
        spawn() += [&](){b.stride();progress++;};
        executeReadyFibers();
        EXPECT_EQ(progress, 2);
    }
    {
        int progress = 0;
        Barrier b(3);

        spawn() += [&](){b.stride();progress++;};
        spawn() += [&](){b.wait();progress++;};
        spawn() += [&](){b.stride();progress++;};
        executeReadyFibers();
        EXPECT_EQ(progress, 3);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    //non wake
    {
        int progress = 0;
        Barrier b(2);

        spawn() += [&](){b.stride();progress++;};

        executeReadyFibers();
        EXPECT_EQ(progress, 0);

        spawn() += [&](){b.stride();progress++;};
        executeReadyFibers();
        EXPECT_EQ(progress, 2);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    //canStride
    {
        Barrier b(3);

        EXPECT_FALSE(b.canStride());

        spawn() += [&](){b.stride();};
        executeReadyFibers();
        EXPECT_FALSE(b.canStride());

        spawn() += [&](){b.stride();};
        executeReadyFibers();
        EXPECT_TRUE(b.canStride());

        spawn() += [&](){b.stride();};
        executeReadyFibers();
        EXPECT_FALSE(b.canStride());
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    //canStride / tryStride
    {
        int progress = 0;
        Barrier b(2);

        spawn() += [&](){EXPECT_FALSE(b.canStride()); EXPECT_FALSE(b.tryStride());b.stride();progress++;};
        spawn() += [&](){EXPECT_TRUE(b.canStride());EXPECT_TRUE (b.tryStride());progress++;};
        executeReadyFibers();
        EXPECT_EQ(progress, 2);
    }
}
