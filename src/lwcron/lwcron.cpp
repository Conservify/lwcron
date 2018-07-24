#include "lwcron.h"

#include <iostream>

namespace lwcron {

constexpr uint8_t DaysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

constexpr bool is_leap_year(uint16_t year) {
    return year % 4 == 0;
}

DateTime::DateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) :
    year_(year), month_(month - 1), day_(day), hour_(hour), minute_(minute), second_(second) {
}

DateTime::DateTime(uint32_t unix) {
    auto t = unix;

    second_ = t % 60;
    t /= 60;
    minute_ = t % 60;
    t /= 60;
    hour_ = t % 24;
    t /= 24;

    auto year = 70;
    auto days = 0;
    while ((uint16_t)(days += (is_leap_year(year) ? 366 : 365)) <= t) {
        year++;
    }

    year_ = year + 1900;
    days -= is_leap_year(year) ? 366 : 365;
    t -= days;

    auto month = 0;
    auto length = 0;
    for (month = 0; month < 12; month++) {
        if (month == 1) { // february
            if (is_leap_year(year)) {
                length = 29;
            } else {
                length = 28;
            }
        } else {
            length = DaysInMonth[month];
        }

        if (t >= length) {
            t -= length;
        }
        else {
            break;
        }
    }

    month_ = month;
    day_ = t + 1;
}

uint32_t DateTime::unix() {
    constexpr uint32_t SecondsPerYear = (60 * 60 * 24L * 365);
    constexpr uint32_t SecondsPerDay = 60 * 60 * 24L;
    constexpr uint32_t SecondsPerHour = 3600L;

    auto year = year_;
    auto seconds = (year - 1970) * SecondsPerYear;
    for (auto i = 1970; i < year; i++) {
        if (is_leap_year(i)) {
            seconds += SecondsPerDay;
        }
    }
    for (auto i = 0; i < month_; i++) {
        if (i == 1 && is_leap_year(year)) {
            seconds += SecondsPerDay * 29;
        }
        else {
            seconds += SecondsPerDay * DaysInMonth[i];
        }
    }

    seconds += (day_ - 1) * SecondsPerDay;
    seconds += hour_ * SecondsPerHour;
    seconds += minute_ * 60L;
    seconds += second_;
    return seconds;
}

void Periodic::run() {
}

bool Periodic::valid() {
    return interval_ > 0;
}

uint32_t Periodic::getNextTime(DateTime after) {
    auto seconds = after.unix();
    auto r = seconds % interval_;
    if (r == 0) {
        return seconds;
    }
    return seconds + (interval_ - r);
}

void Cron::run() {
}

bool Cron::valid() {
    return spec_.valid();
}

uint32_t Cron::getNextTime(DateTime after) {
    return spec_.getNextTime(after);
}

void Scheduler::begin(DateTime now) {
    for (auto i = (size_t)0; i < size_; i++) {
        auto task = tasks_[i];
        if (task->valid()) {
            task->scheduled_ = task->getNextTime(now);
        }
    }
}

Scheduler::TaskAndTime Scheduler::check(DateTime now) {
    auto now_unix = now.unix();
    for (auto i = (size_t)0; i < size_; i++) {
        auto task = tasks_[i];
        if (task->valid()) {
            if (task->scheduled_ <= now_unix) {
                auto scheduled = task->scheduled_;
                task->scheduled_ = task->getNextTime(now + 1);
                return TaskAndTime { scheduled, task };
            }
        }
    }

    return { };
}

Scheduler::TaskAndTime Scheduler::nextTask(DateTime now) {
    TaskAndTime found;
    for (auto i = (size_t)0; i < size_; i++) {
        auto task = tasks_[i];
        if (task->valid()) {
            auto time = task->getNextTime(now);
            if (!found || found.time > time) {
                found = TaskAndTime { time, task };
            }
        }
    }
    return found;
}

Scheduler::TaskAndTime Scheduler::nextTask() {
    TaskAndTime found;
    for (auto i = (size_t)0; i < size_; i++) {
        auto task = tasks_[i];
        if (task->valid()) {
            auto time = task->scheduled_;
            if (!found || found.time > time) {
                found = TaskAndTime { time, task };
            }
        }
    }
    return found;
}

}
