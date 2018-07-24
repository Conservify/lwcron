#include <gtest/gtest.h>
#include <vector>

#include <lwcron/lwcron.h>

using namespace lwcron;

static DateTime JacobsBirth{ 1982, 4, 23, 7, 30, 00 };

class CronSuite : public ::testing::Test {
protected:

};

TEST_F(CronSuite, DateTime) {
    ASSERT_EQ(JacobsBirth.unix(), 388395000);
    ASSERT_EQ(DateTime{ 388395000 }, JacobsBirth);
}

TEST_F(CronSuite, Empty) {
    Scheduler scheduler;

    scheduler.begin(JacobsBirth);

    ASSERT_FALSE(scheduler.nextTask(JacobsBirth));
}

TEST_F(CronSuite, Interval10s) {
    Periodic task{ 10 };
    Task *tasks[1] = { &task };
    Scheduler scheduler{ tasks };

    scheduler.begin(JacobsBirth);

    auto n1 = scheduler.nextTask(JacobsBirth);
    ASSERT_EQ(n1.time, JacobsBirth.unix());

    auto n2 = scheduler.nextTask(JacobsBirth + 1);
    ASSERT_EQ(n2.time, JacobsBirth.unix() + 10);

    auto n3 = scheduler.nextTask(JacobsBirth + 5);
    ASSERT_EQ(n3.time, JacobsBirth.unix() + 10);

    auto n4 = scheduler.nextTask(JacobsBirth + 15);
    ASSERT_EQ(n4.time, JacobsBirth.unix() + 20);
}

TEST_F(CronSuite, Interval60s) {
    Periodic task{ 60 };
    Task *tasks[1] = { &task };
    Scheduler scheduler{ tasks };

    scheduler.begin(JacobsBirth);

    auto n1 = scheduler.nextTask(JacobsBirth + 1);
    ASSERT_EQ(n1.time, JacobsBirth.unix() + 60);

    auto n2 = scheduler.nextTask(JacobsBirth + 30);
    ASSERT_EQ(n2.time, JacobsBirth.unix() + 60);

    auto n3 = scheduler.nextTask(JacobsBirth + 90);
    ASSERT_EQ(n3.time, JacobsBirth.unix() + 120);
}

TEST_F(CronSuite, Interval10m) {
    Periodic task{ 60 * 10 };
    Task *tasks[1] = { &task };
    Scheduler scheduler{ tasks };

    scheduler.begin(JacobsBirth);

    auto n1 = scheduler.nextTask(JacobsBirth);
    ASSERT_EQ(n1.time, JacobsBirth.unix());

    auto n2 = scheduler.nextTask(JacobsBirth + 30);
    ASSERT_EQ(n2.time, JacobsBirth.unix() + (60 * 10));
}

TEST_F(CronSuite, MultipleIntervals) {
    Periodic task1{ 60 * 2 };
    Periodic task2{ 60 * 5 };
    Task *tasks[2] = { &task1, &task2 };
    Scheduler scheduler{ tasks };

    scheduler.begin(JacobsBirth);

    auto n1 = scheduler.nextTask(JacobsBirth);
    ASSERT_EQ(n1.time, JacobsBirth.unix());
    ASSERT_EQ(n1.task, &task1);

    auto n2 = scheduler.nextTask(JacobsBirth + 5);
    ASSERT_EQ(n2.time, JacobsBirth.unix() + 60 * 2);
    ASSERT_EQ(n2.task, &task1);

    auto n3 = scheduler.nextTask(JacobsBirth + 60 * 3);
    ASSERT_EQ(n3.time, JacobsBirth.unix() + 60 * 4);
    ASSERT_EQ(n3.task, &task1);

    auto n4 = scheduler.nextTask(JacobsBirth + 60 * 4);
    ASSERT_EQ(n4.time, JacobsBirth.unix() + 60 * 4);
    ASSERT_EQ(n4.task, &task1);

    auto n5 = scheduler.nextTask(JacobsBirth + 60 * 4 + 5);
    ASSERT_EQ(n5.time, JacobsBirth.unix() + 60 * 5);
    ASSERT_EQ(n5.task, &task2);
}

TEST_F(CronSuite, RunningTasksMultipleIntervals) {
    Periodic task1{ 60 * 2 };
    Periodic task2{ 60 * 5 };
    Task *tasks[2] = { &task1, &task2 };
    Scheduler scheduler{ tasks };

    auto now = JacobsBirth;
    scheduler.begin(now + 5);

    ASSERT_FALSE(scheduler.check(now + 5));
    auto n1 = scheduler.nextTask();
    ASSERT_EQ(n1.task, &task1);
    ASSERT_EQ(n1.time, now.unix() + 60 * 2);

    auto to2 = scheduler.check(now + 60 * 2);
    ASSERT_EQ(to2.task, &task1);
    ASSERT_FALSE(scheduler.check(now + 60 * 2));
    auto n2 = scheduler.nextTask();
    ASSERT_EQ(n2.task, &task1);
    ASSERT_EQ(n2.time, now.unix() + 60 * 4);

    auto to3 = scheduler.check(now + 60 * 4);
    ASSERT_EQ(to3.task, &task1);
    ASSERT_FALSE(scheduler.check(now + 60 * 4));
    auto n3 = scheduler.nextTask();
    ASSERT_EQ(n3.task, &task2);
}

TEST_F(CronSuite, RunningTasksMultipleIntervalsDoesntMissTask1) {
    Periodic task1{ 60 * 2 };
    Periodic task2{ 60 * 2 };
    Task *tasks[2] = { &task1, &task2 };
    Scheduler scheduler{ tasks };

    auto now = JacobsBirth;
    scheduler.begin(now + 5);

    ASSERT_FALSE(scheduler.check(now + 5));
    auto n1 = scheduler.nextTask();
    ASSERT_EQ(n1.task, &task1);
    ASSERT_EQ(n1.time, now.unix() + 60 * 2);

    auto o1 = scheduler.check(now + 60 * 2);
    ASSERT_EQ(o1.task, &task1);
    auto o2 = scheduler.check(now + 60 * 2);
    ASSERT_EQ(o2.task, &task2);
    ASSERT_FALSE(scheduler.check(now + 60 * 2));
}

TEST_F(CronSuite, RunningTasksMultipleIntervalsDoesntMissTask2) {
    Periodic task1{ 60 * 2 };
    Periodic task2{ 60 * 2 };
    Task *tasks[2] = { &task1, &task2 };
    Scheduler scheduler{ tasks };

    auto now = JacobsBirth;
    scheduler.begin(now + 5);

    ASSERT_FALSE(scheduler.check(now + 5));
    auto n1 = scheduler.nextTask();
    ASSERT_EQ(n1.task, &task1);
    ASSERT_EQ(n1.time, now.unix() + 60 * 2);

    auto o1 = scheduler.check(now + 60 * 2);
    ASSERT_EQ(o1.task, &task1);

    auto n2 = scheduler.nextTask();
    ASSERT_EQ(n2.task, &task2);
    ASSERT_EQ(n2.time, now.unix() + 60 * 2);

    auto o2 = scheduler.check(now + 60 * 3);
    ASSERT_EQ(o2.task, &task2);
    ASSERT_FALSE(scheduler.check(now + 60 * 3));
}
