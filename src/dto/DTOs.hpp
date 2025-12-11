#ifndef DTOs_hpp
#define DTOs_hpp

#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/core/Types.hpp>
#include <nlohmann/json.hpp>

#include OATPP_CODEGEN_BEGIN(DTO)

// 会员DTO
class MemberDto : public oatpp::DTO {
  DTO_INIT(MemberDto, DTO)
  
  DTO_FIELD(Int32, id);
  DTO_FIELD(String, name);
  DTO_FIELD(String, phone);
  DTO_FIELD(String, level) = "normal";
  DTO_FIELD(String, created_at);
};

// 教练DTO
class CoachDto : public oatpp::DTO {
  DTO_INIT(CoachDto, DTO)
  
  DTO_FIELD(Int32, id);
  DTO_FIELD(String, name);
  DTO_FIELD(String, speciality);
};

// 课程模板DTO
class ClassTemplateDto : public oatpp::DTO {
  DTO_INIT(ClassTemplateDto, DTO)
  
  DTO_FIELD(Int32, id);
  DTO_FIELD(String, title);
  DTO_FIELD(String, level_required) = "normal";
  DTO_FIELD(Int32, capacity);
  DTO_FIELD(Int32, duration_minutes);
  DTO_FIELD(Int32, coach_id);
};

// 课节DTO
class ClassSessionDto : public oatpp::DTO {
  DTO_INIT(ClassSessionDto, DTO)
  
  DTO_FIELD(Int32, id);
  DTO_FIELD(Int32, template_id);
  DTO_FIELD(String, start_time);
  DTO_FIELD(String, status) = "scheduled";
  DTO_FIELD(Int32, capacity);
  DTO_FIELD(Int32, booked_count) = 0;
  DTO_FIELD(String, template_title);
  DTO_FIELD(Int32, template_duration);
  DTO_FIELD(Int32, coach_id);
  DTO_FIELD(String, coach_name);
};

// 预约DTO
class BookingDto : public oatpp::DTO {
  DTO_INIT(BookingDto, DTO)
  
  DTO_FIELD(Int32, id);
  DTO_FIELD(Int32, member_id);
  DTO_FIELD(Int32, session_id);
  DTO_FIELD(String, status) = "booked";
  DTO_FIELD(String, created_at);
};

// 训练记录DTO
class TrainingLogDto : public oatpp::DTO {
  DTO_INIT(TrainingLogDto, DTO)
  
  DTO_FIELD(Int32, id);
  DTO_FIELD(Int32, member_id);
  DTO_FIELD(Int32, session_id);
  DTO_FIELD(String, notes);
  DTO_FIELD(Int32, duration_minutes);
  DTO_FIELD(Int32, calories);
  DTO_FIELD(String, created_at);
};

// 会员统计DTO
class MemberStatsDto : public oatpp::DTO {
  DTO_INIT(MemberStatsDto, DTO)
  
  DTO_FIELD(Int32, completed_classes);
  DTO_FIELD(Int32, cancelled_bookings);
  DTO_FIELD(Int32, total_training_duration);
};

// 教练统计DTO
class CoachStatsDto : public oatpp::DTO {
  DTO_INIT(CoachStatsDto, DTO)
  
  DTO_FIELD(Int32, upcoming_classes);
  DTO_FIELD(Int32, total_booked_members);
};

// 创建会员请求DTO
class CreateMemberRequestDto : public oatpp::DTO {
  DTO_INIT(CreateMemberRequestDto, DTO)
  
  DTO_FIELD(String, name);
  DTO_FIELD(String, phone);
  DTO_FIELD(String, level);
};

// 创建教练请求DTO
class CreateCoachRequestDto : public oatpp::DTO {
  DTO_INIT(CreateCoachRequestDto, DTO)
  
  DTO_FIELD(String, name);
  DTO_FIELD(String, speciality);
};

// 创建课程模板请求DTO
class CreateClassTemplateRequestDto : public oatpp::DTO {
  DTO_INIT(CreateClassTemplateRequestDto, DTO)
  
  DTO_FIELD(String, title);
  DTO_FIELD(String, level_required);
  DTO_FIELD(Int32, capacity);
  DTO_FIELD(Int32, duration_minutes);
  DTO_FIELD(Int32, coach_id);
};

// 更新课程模板请求DTO
class UpdateClassTemplateRequestDto : public oatpp::DTO {
  DTO_INIT(UpdateClassTemplateRequestDto, DTO)
  
  DTO_FIELD(String, title);
  DTO_FIELD(String, level_required);
  DTO_FIELD(Int32, capacity);
  DTO_FIELD(Int32, duration_minutes);
  DTO_FIELD(Int32, coach_id);
};

// 创建课节请求DTO
class CreateClassSessionRequestDto : public oatpp::DTO {
  DTO_INIT(CreateClassSessionRequestDto, DTO)
  
  DTO_FIELD(Int32, template_id);
  DTO_FIELD(String, start_time);
};

// 更新课节请求DTO
class UpdateClassSessionRequestDto : public oatpp::DTO {
  DTO_INIT(UpdateClassSessionRequestDto, DTO)
  
  DTO_FIELD(Int32, template_id);
  DTO_FIELD(String, start_time);
  DTO_FIELD(String, status);
  DTO_FIELD(Int32, capacity);
};

// 创建预约请求DTO
class CreateBookingRequestDto : public oatpp::DTO {
  DTO_INIT(CreateBookingRequestDto, DTO)
  
  DTO_FIELD(Int32, member_id);
  DTO_FIELD(Int32, session_id);
};

// 创建训练记录请求DTO
class CreateTrainingLogRequestDto : public oatpp::DTO {
  DTO_INIT(CreateTrainingLogRequestDto, DTO)
  
  DTO_FIELD(Int32, member_id);
  DTO_FIELD(Int32, session_id);
  DTO_FIELD(String, notes);
  DTO_FIELD(Int32, duration_minutes);
  DTO_FIELD(Int32, calories);
};

// 错误响应DTO
class ErrorDto : public oatpp::DTO {
  DTO_INIT(ErrorDto, DTO)
  
  DTO_FIELD(String, error_code);
  DTO_FIELD(String, message);
};

#include OATPP_CODEGEN_END(DTO)

#endif /* DTOs_hpp */
