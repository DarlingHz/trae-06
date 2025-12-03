#pragma once

#include <string>
#include <ctime>
#include <stdexcept>

class DateUtils {
public:
    static time_t parseDate(const std::string& dateStr, const std::string& format = "%Y-%m-%d") {
        struct tm tm = {0};
        char* result = strptime(dateStr.c_str(), format.c_str(), &tm);
        if (!result || *result != '\0') {
            throw std::runtime_error("Invalid date format");
        }
        return mktime(&tm);
    }

    static std::string formatDate(time_t timestamp, const std::string& format = "%Y-%m-%d") {
        char buf[100];
        struct tm tm;
        localtime_r(&timestamp, &tm);
        strftime(buf, sizeof(buf), format.c_str(), &tm);
        return buf;
    }

    static bool isBefore(time_t t1, time_t t2) {
        return difftime(t1, t2) < 0;
    }

    static bool isAfter(time_t t1, time_t t2) {
        return difftime(t1, t2) > 0;
    }

    static bool isSameDay(time_t t1, time_t t2) {
        struct tm tm1, tm2;
        localtime_r(&t1, &tm1);
        localtime_r(&t2, &tm2);
        return tm1.tm_year == tm2.tm_year && tm1.tm_mon == tm2.tm_mon && tm1.tm_mday == tm2.tm_mday;
    }

    static time_t getToday() {
        time_t now = time(nullptr);
        struct tm tm;
        localtime_r(&now, &tm);
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;
        return mktime(&tm);
    }

    static time_t addDays(time_t timestamp, int days) {
        return timestamp + (days * 24 * 60 * 60);
    }
};
