/* This file is part of the the dci project. Copyright (C) 2013-2023 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#include <dci/test.hpp>
#include <dci/cmt.hpp>

using namespace dci::cmt;
using namespace dci::cmt::task;

/////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
TEST(cmt, task_face)
{
    spawn() += [&]
    {
        bool workerStart = false;
        bool workerDone = false;
        bool workerInterrupted = false;

        Face f1;

        Notifier firstStarted;
        Notifier secondComplete;

        spawn() += [&]
        {
            firstStarted.raise();
            workerStart = true;

            f1 = currentTask();

            Event e;
            try
            {
                e.wait();//hold worker
            }
            catch(const Stop&)
            {
                //interrupted
                workerInterrupted = true;
            }

            workerDone = true;
        };

        spawn() += [&]
        {
            firstStarted.wait();
            f1.stop(false);
            secondComplete.raise();
        };

        secondComplete.wait();

        EXPECT_TRUE(workerStart);
        EXPECT_TRUE(workerInterrupted);
        EXPECT_TRUE(workerDone);

    };

    executeReadyFibers();
}

/////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
TEST(cmt, task_face_p)
{
    bool allDone = false;
    spawn() += [&]
    {
        Notifier started;
        Notifier interrupted;
        Notifier done;

        Promise<> prm;

        spawn() += [&]
        {
            started.raise();

            currentTask().stopOnResolvedCancel(prm);

            Event e;
            try
            {
                e.wait();//hold worker
            }
            catch(const Stop&)
            {
                //interrupted
                interrupted.raise();
            }

            done.raise();
        };

        started.wait();

        prm.resolveCancel();

        interrupted.wait();
        done.wait();
        allDone = true;
    };

    executeReadyFibers();
    EXPECT_TRUE(allDone);
}

/////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
TEST(cmt, task_face_f)
{
    bool allDone = false;

    spawn() += [&]
    {
        Notifier started;
        Notifier interrupted;
        Notifier done;

        Promise<> prm;
        Future<> fut = prm.future();

        spawn() += [&]
        {
            started.raise();

            currentTask().stopOnResolvedValue(fut);

            Event e;
            try
            {
                e.wait();//hold worker
            }
            catch(const Stop&)
            {
                //interrupted
                interrupted.raise();
            }

            done.raise();
        };

        started.wait();

        prm.resolveValue();

        interrupted.wait();
        done.wait();
        allDone = true;
    };

    executeReadyFibers();
    EXPECT_TRUE(allDone);
}
