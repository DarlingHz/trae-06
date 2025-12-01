#include "controller/survey_controller.h"
#include <iostream>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace controller {

    bool SurveyController::init(const std::string& db_path) {
        return survey_service_.init(db_path);
    }

    void SurveyController::close() {
        survey_service_.close();
    }

    std::string SurveyController::handleCreateSurvey(const std::string& body) {
        try {
            json request = json::parse(body);

            // Validate request parameters
            if (!request.contains("owner_id") || !request.contains("title")) {
                return createJsonResponse(400, "Missing required parameters");
            }

            std::string owner_id = request["owner_id"];
            std::string title = request["title"];
            std::string description = request.contains("description") ? request["description"] : "";

            // Call service to create survey
            std::optional<std::string> survey_id_opt = survey_service_.createSurvey(owner_id, title, description);
            if (!survey_id_opt) {
                return createJsonResponse(500, "Failed to create survey");
            }

            // Create response
            json data;
            data["survey_id"] = survey_id_opt.value();

            return createJsonResponse(0, "ok", data);
        } catch (const json::parse_error& e) {
            return createJsonResponse(400, "Invalid JSON format");
        } catch (const std::exception& e) {
            return createJsonResponse(500, "Internal server error");
        }
    }

    std::string SurveyController::handlePublishSurvey(const std::string& survey_id) {
        try {
            if (survey_id.empty()) {
                return createJsonResponse(400, "Missing survey ID");
            }

            // Call service to publish survey
            bool success = survey_service_.publishSurvey(survey_id);
            if (!success) {
                return createJsonResponse(400, "Failed to publish survey");
            }

            return createJsonResponse(0, "ok");
        } catch (const std::exception& e) {
            return createJsonResponse(500, "Internal server error");
        }
    }

    std::string SurveyController::handleCloseSurvey(const std::string& survey_id) {
        try {
            if (survey_id.empty()) {
                return createJsonResponse(400, "Missing survey ID");
            }

            // Call service to close survey
            bool success = survey_service_.closeSurvey(survey_id);
            if (!success) {
                return createJsonResponse(400, "Failed to close survey");
            }

            return createJsonResponse(0, "ok");
        } catch (const std::exception& e) {
            return createJsonResponse(500, "Internal server error");
        }
    }

    std::string SurveyController::handleGetSurveyById(const std::string& survey_id) {
        try {
            if (survey_id.empty()) {
                return createJsonResponse(400, "Missing survey ID");
            }

            // Call service to get survey by ID
            std::optional<model::Survey> survey_opt = survey_service_.getSurveyById(survey_id);
            if (!survey_opt) {
                return createJsonResponse(404, "Survey not found");
            }

            model::Survey survey = survey_opt.value();

            // Create response
            json data;
            data["id"] = survey.getId();
            data["owner_id"] = survey.getOwnerId();
            data["title"] = survey.getTitle();
            data["description"] = survey.getDescription();
            data["status"] = survey.getStatus() == model::SurveyStatus::DRAFT ? "draft" : (survey.getStatus() == model::SurveyStatus::ACTIVE ? "active" : "closed");
            data["created_at"] = static_cast<long long>(survey.getCreatedAt());

            // Add questions
            json questions = json::array();
            for (const auto& question : survey.getQuestions()) {
                json q;
                q["index"] = question.getIndex();
                q["type"] = question.getType() == model::QuestionType::SINGLE ? "single" : (question.getType() == model::QuestionType::MULTIPLE ? "multiple" : "text");
                q["title"] = question.getTitle();

                if (!question.getOptions().empty()) {
                    q["options"] = question.getOptions();
                }

                questions.push_back(q);
            }

            data["questions"] = questions;

            return createJsonResponse(0, "ok", data);
        } catch (const std::exception& e) {
            return createJsonResponse(500, "Internal server error");
        }
    }

    std::string SurveyController::handleGetSurveysByOwnerId(const std::map<std::string, std::string>& query_params) {
        try {
            // Get query parameters
            auto owner_id_it = query_params.find("owner_id");
            if (owner_id_it == query_params.end()) {
                return createJsonResponse(400, "Missing owner ID");
            }

            std::string owner_id = owner_id_it->second;
            int page = 1;
            int page_size = 10;
            std::string status = "";

            auto page_it = query_params.find("page");
            if (page_it != query_params.end()) {
                page = std::stoi(page_it->second);
            }

            auto page_size_it = query_params.find("page_size");
            if (page_size_it != query_params.end()) {
                page_size = std::stoi(page_size_it->second);
            }

            auto status_it = query_params.find("status");
            if (status_it != query_params.end()) {
                status = status_it->second;
            }

            // Call service to get surveys by owner ID
            std::vector<model::Survey> surveys = survey_service_.getSurveysByOwnerId(owner_id, page, page_size, status);

            // Create response
            json data;
            json survey_list = json::array();

            for (const auto& survey : surveys) {
                json s;
                s["id"] = survey.getId();
                s["owner_id"] = survey.getOwnerId();
                s["title"] = survey.getTitle();
                s["description"] = survey.getDescription();
                s["status"] = survey.getStatus() == model::SurveyStatus::DRAFT ? "draft" : (survey.getStatus() == model::SurveyStatus::ACTIVE ? "active" : "closed");
                s["created_at"] = static_cast<long long>(survey.getCreatedAt());

                survey_list.push_back(s);
            }

            data["surveys"] = survey_list;
            data["page"] = page;
            data["page_size"] = page_size;
            data["total"] = static_cast<int>(surveys.size());

            return createJsonResponse(0, "ok", data);
        } catch (const std::exception& e) {
            return createJsonResponse(500, "Internal server error");
        }
    }

    std::string SurveyController::handleAddQuestionsToSurvey(const std::string& survey_id, const std::string& body) {
        try {
            if (survey_id.empty()) {
                return createJsonResponse(400, "Missing survey ID");
            }

            json request = json::parse(body);

            // Validate request parameters
            if (!request.contains("questions") || !request["questions"].is_array()) {
                return createJsonResponse(400, "Missing or invalid questions parameter");
            }

            // Parse questions
            std::vector<model::Question> questions;
            for (const auto& question_json : request["questions"]) {
                // Validate question parameters
                if (!question_json.contains("index") || !question_json.contains("type") || !question_json.contains("title")) {
                    return createJsonResponse(400, "Missing required parameters for question");
                }

                int index = question_json["index"];
                std::string type_str = question_json["type"];
                std::string title = question_json["title"];

                // Parse question type
                model::QuestionType type;
                if (type_str == "single") {
                    type = model::QuestionType::SINGLE;
                } else if (type_str == "multiple") {
                    type = model::QuestionType::MULTIPLE;
                } else if (type_str == "text") {
                    type = model::QuestionType::TEXT;
                } else {
                    return createJsonResponse(400, "Invalid question type");
                }

                // Parse options for choice questions
                std::vector<std::string> options;
                if (type == model::QuestionType::SINGLE || type == model::QuestionType::MULTIPLE) {
                    if (!question_json.contains("options") || !question_json["options"].is_array()) {
                        return createJsonResponse(400, "Missing or invalid options for choice question");
                    }

                    for (const auto& option : question_json["options"]) {
                        options.push_back(option);
                    }
                }

                // Create question object
                model::Question question(index, type, title, options);
                questions.push_back(question);
            }

            // Call service to add questions to survey
            bool success = survey_service_.addQuestionsToSurvey(survey_id, questions);
            if (!success) {
                return createJsonResponse(400, "Failed to add questions to survey");
            }

            return createJsonResponse(0, "ok");
        } catch (const json::parse_error& e) {
            return createJsonResponse(400, "Invalid JSON format");
        } catch (const std::exception& e) {
            return createJsonResponse(500, "Internal server error");
        }
    }

    std::string SurveyController::handleSubmitResponse(const std::string& survey_id, const std::string& body) {
        try {
            if (survey_id.empty()) {
                return createJsonResponse(400, "Missing survey ID");
            }

            json request = json::parse(body);

            // Validate request parameters
            if (!request.contains("answers") || !request["answers"].is_array()) {
                return createJsonResponse(400, "Missing or invalid answers parameter");
            }

            // Parse respondent ID
            std::string respondent_id = request.contains("respondent_id") ? request["respondent_id"] : "";

            // Parse answers
            std::vector<model::Answer> answers;
            for (const auto& answer_json : request["answers"]) {
                // Validate answer parameters
                if (!answer_json.contains("question_index")) {
                    return createJsonResponse(400, "Missing question_index parameter for answer");
                }

                int question_index = answer_json["question_index"];

                // Parse choice indices or text
                std::vector<int> choice_indices;
                std::string text;

                if (answer_json.contains("choice_indices") && answer_json["choice_indices"].is_array()) {
                    for (const auto& choice_index : answer_json["choice_indices"]) {
                        choice_indices.push_back(choice_index);
                    }
                } else if (answer_json.contains("text")) {
                    text = answer_json["text"];
                } else {
                    return createJsonResponse(400, "Missing choice_indices or text parameter for answer");
                }

                // Create answer object
                model::Answer answer(question_index, choice_indices, text);
                answers.push_back(answer);
            }

            // Call service to submit response
            std::optional<std::string> response_id_opt = survey_service_.submitResponse(survey_id, respondent_id, answers);
            if (!response_id_opt) {
                return createJsonResponse(400, "Failed to submit response");
            }

            // Create response
            json data;
            data["response_id"] = response_id_opt.value();

            return createJsonResponse(0, "ok", data);
        } catch (const json::parse_error& e) {
            return createJsonResponse(400, "Invalid JSON format");
        } catch (const std::exception& e) {
            return createJsonResponse(500, "Internal server error");
        }
    }

    std::string SurveyController::handleGetSurveyStats(const std::string& survey_id) {
        try {
            if (survey_id.empty()) {
                return createJsonResponse(400, "Missing survey ID");
            }

            // Call service to get survey stats
            std::optional<service::SurveyService::SurveyStats> stats_opt = survey_service_.getSurveyStats(survey_id);
            if (!stats_opt) {
                return createJsonResponse(404, "Survey not found");
            }

            service::SurveyService::SurveyStats stats = stats_opt.value();

            // Create response
            json data;
            data["survey_id"] = stats.survey_id;
            data["total_responses"] = stats.total_responses;

            // Add questions
            json questions = json::array();
            for (const auto& question_stats : stats.questions) {
                json q;
                q["index"] = question_stats.index;
                q["type"] = question_stats.type;
                q["title"] = question_stats.title;

                // Add options for choice questions
                if (!question_stats.options.empty()) {
                    json options = json::array();
                    for (const auto& option_stats : question_stats.options) {
                        json o;
                        o["index"] = option_stats.index;
                        o["text"] = option_stats.text;
                        o["count"] = option_stats.count;

                        options.push_back(o);
                    }

                    q["options"] = options;
                }

                // Add latest text answers for text questions
                if (!question_stats.latest_text_answers.empty()) {
                    q["latest_text_answers"] = question_stats.latest_text_answers;
                }

                questions.push_back(q);
            }

            data["questions"] = questions;

            return createJsonResponse(0, "ok", data);
        } catch (const std::exception& e) {
            return createJsonResponse(500, "Internal server error");
        }
    }

    std::string SurveyController::createJsonResponse(int code, const std::string& message, const json& data) {
        json response;
        response["code"] = code;
        response["message"] = message;
        response["data"] = data;

        return response.dump();
    }

} // namespace controller
