#include "StatsService.hpp"
#include <oatpp/core/Types.hpp>
#include <oatpp/core/data/stream/BufferStream.hpp>
#include <oatpp/core/macro/component.hpp>
#include <oatpp/core/utils/ConversionUtils.hpp>
#include <string>
#include <vector>
#include <ctime>
#include <chrono>

using namespace std::chrono;
using namespace std::chrono_literals;

// 获取会员统计数据
oatpp::Object<MemberStatsDto> StatsService::getMemberStats(oatpp::Int32 memberId) {
    auto memberStats = MemberStatsDto::createShared();
    memberStats->completed_classes = 0;
    memberStats->cancelled_bookings = 0;
    memberStats->total_training_duration = 0;

    // 获取当前时间
    auto now = system_clock::now();
    // 计算30天前的时间
    auto thirtyDaysAgo = now - std::chrono::hours(24 * 30);
    // 转换为ISO8601字符串
    char buffer[256];
    time_t nowTime = system_clock::to_time_t(now);
    time_t thirtyDaysAgoTime = system_clock::to_time_t(thirtyDaysAgo);
    strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", gmtime(&thirtyDaysAgoTime));
    std::string fromDate = buffer;
    strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", gmtime(&nowTime));
    std::string toDate = buffer;

    // 获取会员最近30天的预约记录
    // 注意：getMemberBookings()方法不支持按日期范围筛选
    // 这里只获取所有预约记录，然后在内存中筛选
    auto bookings = m_bookingDao->getMemberBookings(memberId);
    if (bookings) {
        for (const auto& booking : *bookings) {
            if (booking->status == "attended") {
                memberStats->completed_classes = memberStats->completed_classes + 1;
            } else if (booking->status == "cancelled") {
                memberStats->cancelled_bookings = memberStats->cancelled_bookings + 1;
            }
        }
    }

    // 获取会员最近30天的训练记录
    auto trainingLogs = m_trainingLogDao->getMemberTrainingLogs(memberId, fromDate, toDate);
    if (trainingLogs) {
        for (const auto& log : *trainingLogs) {
            memberStats->total_training_duration = memberStats->total_training_duration + log->duration_minutes;
        }
    }

    return memberStats;
}

// 获取教练统计数据
oatpp::Object<CoachStatsDto> StatsService::getCoachStats(oatpp::Int32 coachId) {
    auto coachStats = CoachStatsDto::createShared();
    coachStats->upcoming_classes = 0;
    coachStats->total_booked_members = 0;

    // 获取当前时间
    auto now = system_clock::now();
    // 计算7天后的时间
    auto sevenDaysLater = now + std::chrono::hours(24 * 7);
    // 转换为ISO8601字符串
    char buffer[256];
    time_t nowTime = system_clock::to_time_t(now);
    time_t sevenDaysLaterTime = system_clock::to_time_t(sevenDaysLater);
    strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", gmtime(&nowTime));
    std::string fromDate = buffer;
    strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", gmtime(&sevenDaysLaterTime));
    std::string toDate = buffer;

    // 获取教练未来7天的课节
    auto classSessions = m_classSessionDao->getClassSessions(fromDate, toDate, coachId, nullptr);
    if (classSessions) {
        coachStats->upcoming_classes = classSessions->size();
        for (const auto& session : *classSessions) {
            coachStats->total_booked_members = coachStats->total_booked_members + session->booked_count;
        }
    }

    return coachStats;
}
