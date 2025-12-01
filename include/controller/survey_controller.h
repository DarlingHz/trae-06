#ifndef SURVEY_CONTROLLER_H
#define SURVEY_CONTROLLER_H

#include <string>
#include <map>
#include <vector>
#include <nlohmann/json.hpp>
#include "service/survey_service.h"

namespace controller {

    class SurveyController {
    public:
        SurveyController() = default;
        ~SurveyController() = default;

        bool init(const std::string& db_path);
        void close();

        // HTTP request handlers
        std::string handleCreateSurvey(const std::string& body);
        std::string handlePublishSurvey(const std::string& survey_id);
        std::string handleCloseSurvey(const std::string& survey_id);
        std::string handleGetSurveyById(const std::string& survey_id);
        std::string handleGetSurveysByOwnerId(const std::map<std::string, std::string>& query_params);
        std::string handleAddQuestionsToSurvey(const std::string& survey_id, const std::string& body);
        std::string handleSubmitResponse(const std::string& survey_id, const std::string& body);
        std::string handleGetSurveyStats(const std::string& survey_id);

    private:
        service::SurveyService survey_service_;

        // Helper functions
        std::string createJsonResponse(int code, const std::string& message, const nlohmann::json& data = nlohmann::json::object());
    };

} // namespace controller

#endif // SURVEY_CONTROLLER_H
