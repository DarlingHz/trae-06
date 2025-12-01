#ifndef SURVEY_REPOSITORY_H
#define SURVEY_REPOSITORY_H

#include <string>
#include <vector>
#include <optional>
#include "model/survey.h"
#include "model/question.h"
#include "model/response.h"
#include "db/database.h"

namespace repository {

    class SurveyRepository {
    public:
        SurveyRepository() = default;
        ~SurveyRepository() = default;

        bool init(const std::string& db_path);
        void close();

        std::optional<std::string> createSurvey(const model::Survey& survey);
        bool updateSurvey(const model::Survey& survey);
        std::optional<model::Survey> getSurveyById(const std::string& id);
        std::vector<model::Survey> getSurveysByOwnerId(const std::string& owner_id, int page, int page_size, const std::string& status = "");

        bool addQuestionsToSurvey(const std::string& survey_id, const std::vector<model::Question>& questions);
        std::vector<model::Question> getQuestionsBySurveyId(const std::string& survey_id);

        std::optional<std::string> createResponse(const model::Response& response);
        std::vector<model::Response> getResponsesBySurveyId(const std::string& survey_id);

    private:
        db::Database db_;

        model::SurveyStatus stringToSurveyStatus(const std::string& status);
        std::string surveyStatusToString(model::SurveyStatus status);

        model::QuestionType stringToQuestionType(const std::string& type);
        std::string questionTypeToString(model::QuestionType type);
    };

} // namespace repository

#endif // SURVEY_REPOSITORY_H
