#ifndef LWCRON_H_INCLUDED
#define LWCRON_H_INCLUDED

#include <cinttypes>
#include <cstring>

namespace lwcron {

struct TimeOfDay {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint32_t remainder;

    TimeOfDay(uint32_t t) {
        second = t % 60;
        t /= 60;
        minute = t % 60;
        t /= 60;
        hour = t % 24;
        t /= 24;
        remainder = t;
    }
};

class DateTime {
private:
    uint16_t year_{ 0 };
    uint8_t month_{ 0 }, day_{ 0 }, hour_{ 0 }, minute_{ 0 }, second_{ 0 };

public:
    DateTime() {
    }

    DateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);
    DateTime(uint32_t time);

public:
    uint16_t year() const {
        return year_;
    }
    uint8_t month() const {
        return month_;
    }
    uint8_t day() const {
        return day_;
    }
    uint8_t hour() const {
        return hour_;
    }
    uint8_t minute() const {
        return minute_;
    }
    uint8_t second() const {
        return second_;
    }

public:
    uint32_t unix_time();

public:
    DateTime operator+(const uint32_t seconds) {
        return DateTime(unix_time() + seconds);
    }

    DateTime operator-(const uint32_t seconds) {
        return DateTime(unix_time() - seconds);
    }

    DateTime& operator+=(const uint32_t rhs){
        *this = DateTime(unix_time() + rhs);
        return *this;
    }

    DateTime& operator-=(const uint32_t rhs){
        *this = DateTime(unix_time() - rhs);
        return *this;
    }

    bool operator ==(const DateTime &b) const {
        return year_ == b.year_ && month_ == b.month_ &&
               day_ == b.day_ && hour_ == b.hour_ &&
               minute_ == b.minute_ && second_ == b.second_;
    }

    bool operator !=(const DateTime &b) const {
        return !(*this == b);
    }

};

class Scheduler;

class Task {
private:
    uint32_t scheduled_{ 0 };
    bool pending_{ false };

public:
    virtual void run() = 0;
    virtual bool valid() = 0;
    virtual uint32_t getNextTime(DateTime after) = 0;
    virtual const char *toString() const {
        return "Task<>";
    }

    friend class Scheduler;

};

class PeriodicTask : public Task {
private:
    uint32_t interval_{ 0 };

public:
    PeriodicTask() {
    }

    PeriodicTask(uint32_t interval) : interval_(interval) {
    }

public:
    void run() override;
    bool valid() override;
    uint32_t getNextTime(DateTime after) override;

};

template<size_t N>
bool bitarray_any(uint8_t (&bytes)[N]) {
    for (auto i = (size_t)0; i < N; ++i) {
        if (bytes[i] > 0) {
            return true;
        }
    }
    return false;
}

template<size_t N>
static inline bool bitarray_test(uint8_t (&p)[N], uint32_t n) {
    return p[n / 8] & (0x1 << (n % 8));
}

template<size_t N>
uint32_t bitarray_nset(uint8_t (&bytes)[N]) {
    auto c = 0;
    for (auto i = (size_t)0; i < N * 8; ++i) {
        if (bitarray_test(bytes, i)) {
            c++;
        }
    }
    return c;
}

template<size_t N>
static inline void bitarray_set(uint8_t (&p)[N], uint32_t n) {
    p[n / 8] |= (0x1 << (n % 8));
}

template<size_t N>
static inline void bitarray_clear(uint8_t (&p)[N], uint32_t n) {
    p[n / 8] &= ~(0x1 << (n % 8));
}

struct CronSpec {
public:
    uint8_t seconds[8] = { 0 };
    uint8_t minutes[8] = { 0 };
    uint8_t hours[3] = { 0 };

public:
    CronSpec() {
    }

    CronSpec(DateTime when) {
        bitarray_set(seconds, when.second());
        bitarray_set(minutes, when.minute());
        bitarray_set(hours, when.hour());
    }

public:
    bool valid();

    void set(TimeOfDay tod);

    uint32_t getNextTime(DateTime after);

    static CronSpec interval(uint32_t seconds);

    static CronSpec specific(uint8_t second, uint8_t minute = 0xff, uint8_t hour = 0xff);

private:
    bool matches(CronSpec cs) {
        return matches(hours, cs.hours, sizeof(hours)) &&
            matches(minutes, cs.minutes, sizeof(minutes)) &&
            matches(seconds, cs.seconds, sizeof(seconds));
    }

    static bool matches(uint8_t *a, uint8_t *b, size_t size) {
        for (auto i = (size_t)0; i < size; ++i) {
            if ((a[i] & b[i]) > 0) {
                return true;
            }
        }
        return false;
    }
};

class CronTask : public Task {
private:
    CronSpec spec_;

public:
    CronTask() {
    }

    CronTask(CronSpec spec) : spec_(spec) {
    }

public:
    void run() override;
    bool valid() override;
    uint32_t getNextTime(DateTime after) override;

};

class Scheduler {
private:
    Task **tasks_{ nullptr };
    size_t size_{ 0 };

public:
    Scheduler() {
    }

    template<size_t N>
    Scheduler(Task* (&tasks)[N]) : tasks_(&tasks[0]), size_(N) {
    }

public:
    struct TaskAndTime {
        uint32_t time{ 0 };
        Task *task{ nullptr };

        TaskAndTime() {
        }

        TaskAndTime(uint32_t time, Task *task) : time(time), task(task) {
        }

        operator bool() {
            return task != nullptr;
        }
    };

    void begin(DateTime now);

    TaskAndTime nextTask(DateTime now);

    TaskAndTime nextTask();

    TaskAndTime check(DateTime now);

};

}

#endif
