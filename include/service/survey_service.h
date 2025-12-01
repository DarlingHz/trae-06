#ifndef SURVEY_SERVICE_H
#define SURVEY_SERVICE_H

#include <string>
#include <vector>
#include <optional>
#include "model/survey.h"
#include "model/question.h"
#include "model/response.h"
#include "repository/survey_repository.h"

namespace service {

    class SurveyService {
    public:
        SurveyService() = default;
        ~SurveyService() = default;

        bool init(const std::string& db_path);
        void close();

        std::optional<std::string> createSurvey(const std::string& owner_id, const std::string& title, const std::string& description);
        bool publishSurvey(const std::string& survey_id);
        bool closeSurvey(const std::string& survey_id);
        std::optional<model::Survey> getSurveyById(const std::string& survey_id);
        std::vector<model::Survey> getSurveysByOwnerId(const std::string& owner_id, int page, int page_size, const std::string& status = "");

        bool addQuestionsToSurvey(const std::string& survey_id, const std::vector<model::Question>& questions);
        std::vector<model::Question> getQuestionsBySurveyId(const std::string& survey_id);

        std::optional<std::string> submitResponse(const std::string& survey_id, const std::string& respondent_id, const std::vector<model::Answer>& answers);
        std::vector<model::Response> getResponsesBySurveyId(const std::string& survey_id);

        struct OptionStats {
            int index;
            std::string text;
            int count;
        };

        struct QuestionStats {
            int index;
            std::string type;
            std::string title;
            std::vector<struct OptionStats> options;
            std::vector<std::string> latest_text_answers;
        };

        struct SurveyStats {
            std::string survey_id;
            int total_responses;
            std::vector<struct QuestionStats> questions;
        };

        std::optional<SurveyStats> getSurveyStats(const std::string& survey_id);

    private:
        repository::SurveyRepository survey_repository_;
    };

} // namespace service

#endif // SURVEY_SERVICE_H