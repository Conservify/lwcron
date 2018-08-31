#include "lwcron.h"

namespace lwcron {

constexpr uint32_t SecondsPerYear = (60 * 60 * 24L * 365);
constexpr uint32_t SecondsPerDay = 60 * 60 * 24L;
constexpr uint32_t SecondsPerHour = 3600L;

constexpr uint8_t DaysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

constexpr bool is_leap_year(uint16_t year) {
    return year % 4 == 0;
}

DateTime::DateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) :
    year_(year), month_(month - 1), day_(day), hour_(hour), minute_(minute), second_(second) {
}

DateTime::DateTime(uint32_t unix_time) {
    auto t = unix_time;

    TimeOfDay tod{ t };
    second_ = tod.second;
    minute_ = tod.minute;
    hour_ = tod.hour;
    t = tod.remainder;

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

uint32_t DateTime::unix_time() {
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

void PeriodicTask::run() {
}

bool PeriodicTask::valid() const {
    return interval_ > 0;
}

uint32_t PeriodicTask::getNextTime(DateTime after) {
    auto seconds = after.unix_time();
    auto r = seconds % interval_;
    if (r == 0) {
        return seconds;
    }
    return seconds + (interval_ - r);
}

CronSpec CronSpec::interval(uint32_t seconds) {
    CronSpec cs;

    for (uint32_t s = 0; s <= SecondsPerDay ; s += seconds) {
        TimeOfDay tod{ s };
        cs.set(tod);
    }

    return cs;
}

CronSpec CronSpec::specific(uint8_t second, uint8_t minute, uint8_t hour) {
    CronSpec cs;
    bitarray_set(cs.seconds, second);
    if (minute == 0xff) {
        memset(cs.minutes, 0xff, sizeof(cs.minutes));
    }
    else {
        bitarray_set(cs.minutes, minute);
    }
    if (hour == 0xff) {
        memset(cs.hours, 0xff, sizeof(cs.hours));
    }
    else {
        bitarray_set(cs.hours, hour);
    }
    return cs;
}

void CronSpec::set(TimeOfDay tod) {
    bitarray_set(hours, tod.hour);
    bitarray_set(minutes, tod.minute);
    bitarray_set(seconds, tod.second);
}

bool CronSpec::valid() const {
    return bitarray_any(seconds) && bitarray_any(minutes) && bitarray_any(hours);
}

// NOTE: This could be so much better.
uint32_t CronSpec::getNextTime(DateTime after) {
    auto unix_time = after.unix_time();
    for (auto i = 0; i < 3600 * 24; ++i) {
        CronSpec cs{ unix_time + i };
        if (matches(cs)) {
            return unix_time + i;
        }
    }
    return 0;
}

void CronTask::run() {
}

bool CronTask::valid() const {
    return spec_.valid();
}

uint32_t CronTask::getNextTime(DateTime after) {
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
    auto now_unix = now.unix_time();
    for (auto i = (size_t)0; i < size_; i++) {
        auto task = tasks_[i];
        if (task->valid()) {
            if (task->scheduled_ <= now_unix) {
                auto scheduled = task->scheduled_;
                task->scheduled_ = task->getNextTime(now + 1);
                task->run();
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
