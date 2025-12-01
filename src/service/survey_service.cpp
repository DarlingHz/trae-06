#include "service/survey_service.h"
#include <iostream>
#include <algorithm>
#include <ctime>

namespace service {

    bool SurveyService::init(const std::string& db_path) {
        return survey_repository_.init(db_path);
    }

    void SurveyService::close() {
        survey_repository_.close();
    }

    std::optional<std::string> SurveyService::createSurvey(const std::string& owner_id, const std::string& title, const std::string& description) {
        if (owner_id.empty() || title.empty()) {
            return std::nullopt;
        }

        std::time_t now = std::time(nullptr);
        model::Survey survey("", owner_id, title, description, model::SurveyStatus::DRAFT, now);

        return survey_repository_.createSurvey(survey);
    }

    bool SurveyService::publishSurvey(const std::string& survey_id) {
        std::optional<model::Survey> survey_opt = survey_repository_.getSurveyById(survey_id);
        if (!survey_opt) {
            return false;
        }

        model::Survey survey = survey_opt.value();
        if (survey.getStatus() != model::SurveyStatus::DRAFT) {
            return false;
        }

        survey.setStatus(model::SurveyStatus::ACTIVE);
        return survey_repository_.updateSurvey(survey);
    }

    bool SurveyService::closeSurvey(const std::string& survey_id) {
        std::optional<model::Survey> survey_opt = survey_repository_.getSurveyById(survey_id);
        if (!survey_opt) {
            return false;
        }

        model::Survey survey = survey_opt.value();
        if (survey.getStatus() != model::SurveyStatus::ACTIVE) {
            return false;
        }

        survey.setStatus(model::SurveyStatus::CLOSED);
        return survey_repository_.updateSurvey(survey);
    }

    std::optional<model::Survey> SurveyService::getSurveyById(const std::string& survey_id) {
        return survey_repository_.getSurveyById(survey_id);
    }

    std::vector<model::Survey> SurveyService::getSurveysByOwnerId(const std::string& owner_id, int page, int page_size, const std::string& status) {
        if (owner_id.empty() || page <= 0 || page_size <= 0) {
            return {};
        }

        return survey_repository_.getSurveysByOwnerId(owner_id, page, page_size, status);
    }

    bool SurveyService::addQuestionsToSurvey(const std::string& survey_id, const std::vector<model::Question>& questions) {
        std::optional<model::Survey> survey_opt = survey_repository_.getSurveyById(survey_id);
        if (!survey_opt) {
            return false;
        }

        model::Survey survey = survey_opt.value();
        if (survey.getStatus() != model::SurveyStatus::DRAFT) {
            return false;
        }

        // Validate questions
        std::vector<int> indices;
        for (const auto& question : questions) {
            if (question.getIndex() <= 0) {
                return false;
            }

            if (std::find(indices.begin(), indices.end(), question.getIndex()) != indices.end()) {
                return false;
            }
            indices.push_back(question.getIndex());

            if (question.getType() == model::QuestionType::SINGLE || question.getType() == model::QuestionType::MULTIPLE) {
                if (question.getOptions().empty()) {
                    return false;
                }
            }
        }

        return survey_repository_.addQuestionsToSurvey(survey_id, questions);
    }

    std::vector<model::Question> SurveyService::getQuestionsBySurveyId(const std::string& survey_id) {
        return survey_repository_.getQuestionsBySurveyId(survey_id);
    }

    std::optional<std::string> SurveyService::submitResponse(const std::string& survey_id, const std::string& respondent_id, const std::vector<model::Answer>& answers) {
        std::optional<model::Survey> survey_opt = survey_repository_.getSurveyById(survey_id);
        if (!survey_opt) {
            return std::nullopt;
        }

        model::Survey survey = survey_opt.value();
        if (survey.getStatus() != model::SurveyStatus::ACTIVE) {
            return std::nullopt;
        }

        // Validate answers
        std::vector<int> question_indices;
        for (const auto& question : survey.getQuestions()) {
            question_indices.push_back(question.getIndex());
        }

        for (const auto& answer : answers) {
            if (std::find(question_indices.begin(), question_indices.end(), answer.getQuestionIndex()) == question_indices.end()) {
                return std::nullopt;
            }

            // Find the question
            const model::Question* question = nullptr;
            for (const auto& q : survey.getQuestions()) {
                if (q.getIndex() == answer.getQuestionIndex()) {
                    question = &q;
                    break;
                }
            }

            if (!question) {
                return std::nullopt;
            }

            if (question->getType() == model::QuestionType::SINGLE) {
                if (answer.getChoiceIndices().size() != 1) {
                    return std::nullopt;
                }

                int choice_index = answer.getChoiceIndices()[0];
                if (choice_index < 1 || choice_index > static_cast<int>(question->getOptions().size())) {
                    return std::nullopt;
                }
            } else if (question->getType() == model::QuestionType::MULTIPLE) {
                if (answer.getChoiceIndices().empty()) {
                    return std::nullopt;
                }

                for (int choice_index : answer.getChoiceIndices()) {
                    if (choice_index < 1 || choice_index > static_cast<int>(question->getOptions().size())) {
                        return std::nullopt;
                    }
                }
            } else if (question->getType() == model::QuestionType::TEXT) {
                if (!answer.getChoiceIndices().empty()) {
                    return std::nullopt;
                }
            }
        }

        std::time_t now = std::time(nullptr);
        model::Response response("", survey_id, respondent_id, answers, now);

        return survey_repository_.createResponse(response);
    }

    std::vector<model::Response> SurveyService::getResponsesBySurveyId(const std::string& survey_id) {
        return survey_repository_.getResponsesBySurveyId(survey_id);
    }

    std::optional<SurveyService::SurveyStats> SurveyService::getSurveyStats(const std::string& survey_id) {
        std::optional<model::Survey> survey_opt = survey_repository_.getSurveyById(survey_id);
        if (!survey_opt) {
            return std::nullopt;
        }

        model::Survey survey = survey_opt.value();
        std::vector<model::Response> responses = survey_repository_.getResponsesBySurveyId(survey_id);

        SurveyStats stats;
        stats.survey_id = survey_id;
        stats.total_responses = static_cast<int>(responses.size());

        // Initialize question stats
        for (const auto& question : survey.getQuestions()) {
            QuestionStats question_stats;
            question_stats.index = question.getIndex();
            question_stats.title = question.getTitle();

            if (question.getType() == model::QuestionType::SINGLE) {
                question_stats.type = "single";
            } else if (question.getType() == model::QuestionType::MULTIPLE) {
                question_stats.type = "multiple";
            } else if (question.getType() == model::QuestionType::TEXT) {
                question_stats.type = "text";
            }

            // Initialize option stats for choice questions
            if (question.getType() == model::QuestionType::SINGLE || question.getType() == model::QuestionType::MULTIPLE) {
                for (size_t i = 0; i < question.getOptions().size(); ++i) {
                    OptionStats option_stats;
                    option_stats.index = static_cast<int>(i + 1);
                    option_stats.text = question.getOptions()[i];
                    option_stats.count = 0;

                    question_stats.options.push_back(option_stats);
                }
            }

            stats.questions.push_back(question_stats);
        }

        // Calculate stats from responses
        for (const auto& response : responses) {
            for (const auto& answer : response.getAnswers()) {
                // Find the question stats
                for (auto& question_stats : stats.questions) {
                    if (question_stats.index == answer.getQuestionIndex()) {
                        if (question_stats.type == "single" || question_stats.type == "multiple") {
                            // Increment option counts
                            for (int choice_index : answer.getChoiceIndices()) {
                                for (auto& option_stats : question_stats.options) {
                                    if (option_stats.index == choice_index) {
                                        option_stats.count++;
                                        break;
                                    }
                                }
                            }
                        } else if (question_stats.type == "text") {
                            // Add text answer to latest_text_answers
                            if (!answer.getText().empty()) {
                                question_stats.latest_text_answers.push_back(answer.getText());

                                // Keep only the latest 10 text answers
                                if (question_stats.latest_text_answers.size() > 10) {
                                    question_stats.latest_text_answers.erase(question_stats.latest_text_answers.begin());
                                }
                            }
                        }

                        break;
                    }
                }
            }
        }

        return stats;
    }

} // namespace service
