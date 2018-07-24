#ifndef LWCRON_H_INCLUDED
#define LWCRON_H_INCLUDED

#include <cinttypes>
#include <cstring>

namespace lwcron {

class DateTime {
private:
    uint16_t year_{ 0 };
    uint8_t month_{ 0 }, day_{ 0 }, hour_{ 0 }, minute_{ 0 }, second_{ 0 };

public:
    DateTime() {
    }

    DateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);
    DateTime(uint32_t unix);

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
    uint32_t unix();

public:
    DateTime operator+(const uint32_t seconds) {
        return DateTime(unix() + seconds);
    }

    DateTime operator-(const uint32_t seconds) {
        return DateTime(unix() - seconds);
    }

    DateTime& operator+=(const uint32_t rhs){
        *this = DateTime(unix() + rhs);
        return *this;
    }

    DateTime& operator-=(const uint32_t rhs){
        *this = DateTime(unix() - rhs);
        return *this;
    }

    bool operator !=(const DateTime &b) const {
        return !(*this == b);
    }

    bool operator ==(const DateTime &b) const {
        return year_ == b.year_ && month_ == b.month_ &&
               day_ == b.day_ && hour_ == b.hour_ &&
               minute_ == b.minute_ && second_ == b.second_;
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

    friend class Scheduler;

};

class Periodic : public Task {
private:
    uint32_t interval_{ 0 };

public:
    Periodic() {
    }

    Periodic(uint32_t interval) : interval_(interval) {
    }

public:
    void run() override;
    bool valid() override;
    uint32_t getNextTime(DateTime after) override;

};

class Cron : public Task {
private:
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
