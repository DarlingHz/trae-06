#include "repository/survey_repository.h"
#include <iostream>
#include <sstream>
#include <random>
#include <algorithm>

namespace repository {

    bool SurveyRepository::init(const std::string& db_path) {
        if (!db_.open(db_path)) {
            return false;
        }

        // Create surveys table
        std::string create_surveys_table = R"(
            CREATE TABLE IF NOT EXISTS surveys (
                id TEXT PRIMARY KEY,
                owner_id TEXT NOT NULL,
                title TEXT NOT NULL,
                description TEXT,
                status INTEGER NOT NULL DEFAULT 0,
                created_at INTEGER NOT NULL
            );
        )";

        if (!db_.execute(create_surveys_table)) {
            return false;
        }

        // Create questions table
        std::string create_questions_table = R"(
            CREATE TABLE IF NOT EXISTS questions (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                survey_id TEXT NOT NULL,
                index INTEGER NOT NULL,
                type INTEGER NOT NULL,
                title TEXT NOT NULL,
                options TEXT,
                FOREIGN KEY (survey_id) REFERENCES surveys (id) ON DELETE CASCADE
            );
        )";

        if (!db_.execute(create_questions_table)) {
            return false;
        }

        // Create responses table
        std::string create_responses_table = R"(
            CREATE TABLE IF NOT EXISTS responses (
                id TEXT PRIMARY KEY,
                survey_id TEXT NOT NULL,
                respondent_id TEXT,
                submitted_at INTEGER NOT NULL,
                FOREIGN KEY (survey_id) REFERENCES surveys (id) ON DELETE CASCADE
            );
        )";

        if (!db_.execute(create_responses_table)) {
            return false;
        }

        // Create answers table
        std::string create_answers_table = R"(
            CREATE TABLE IF NOT EXISTS answers (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                response_id TEXT NOT NULL,
                question_index INTEGER NOT NULL,
                choice_indices TEXT,
                text TEXT,
                FOREIGN KEY (response_id) REFERENCES responses (id) ON DELETE CASCADE
            );
        )";

        if (!db_.execute(create_answers_table)) {
            return false;
        }

        return true;
    }

    void SurveyRepository::close() {
        db_.close();
    }

    std::optional<std::string> SurveyRepository::createSurvey(const model::Survey& survey) {
        // Generate random survey ID
        std::stringstream ss;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 15);
        const char* hex_chars = "0123456789abcdef";

        for (int i = 0; i < 32; ++i) {
            ss << hex_chars[dis(gen)];
        }

        std::string survey_id = ss.str();

        // Insert survey into database
        std::stringstream insert_sql;
        insert_sql << "INSERT INTO surveys (id, owner_id, title, description, status, created_at) VALUES (";
        insert_sql << "'" << survey_id << "',";
        insert_sql << "'" << survey.getOwnerId() << "',";
        insert_sql << "'" << survey.getTitle() << "',";
        insert_sql << "'" << survey.getDescription() << "',";
        insert_sql << static_cast<int>(survey.getStatus()) << ",";
        insert_sql << static_cast<long long>(survey.getCreatedAt()) << ")";

        if (!db_.execute(insert_sql.str())) {
            return std::nullopt;
        }

        return survey_id;
    }

    bool SurveyRepository::updateSurvey(const model::Survey& survey) {
        std::stringstream update_sql;
        update_sql << "UPDATE surveys SET owner_id = '" << survey.getOwnerId() << "',";
        update_sql << "title = '" << survey.getTitle() << "',";
        update_sql << "description = '" << survey.getDescription() << "',";
        update_sql << "status = " << static_cast<int>(survey.getStatus()) << ",";
        update_sql << "created_at = " << static_cast<long long>(survey.getCreatedAt()) << "";
        update_sql << " WHERE id = '" << survey.getId() << "'";

        return db_.execute(update_sql.str());
    }

    std::optional<model::Survey> SurveyRepository::getSurveyById(const std::string& id) {
        std::stringstream select_sql;
        select_sql << "SELECT * FROM surveys WHERE id = '" << id << "'";

        std::vector<std::map<std::string, std::string>> results;
        if (!db_.query(select_sql.str(), results)) {
            return std::nullopt;
        }

        if (results.empty()) {
            return std::nullopt;
        }

        const auto& row = results[0];
        model::Survey survey(
                row.at("id"),
                row.at("owner_id"),
                row.at("title"),
                row.at("description"),
                stringToSurveyStatus(row.at("status")),
                static_cast<std::time_t>(std::stoll(row.at("created_at")))
            );

        // Get questions for this survey
        std::vector<model::Question> questions = getQuestionsBySurveyId(id);
        survey.setQuestions(questions);

        return survey;
    }

    std::vector<model::Survey> SurveyRepository::getSurveysByOwnerId(const std::string& owner_id, int page, int page_size, const std::string& status) {
        std::vector<model::Survey> surveys;

        std::stringstream select_sql;
        select_sql << "SELECT * FROM surveys WHERE owner_id = '" << owner_id << "'";

        if (!status.empty()) {
            select_sql << " AND status = " << static_cast<int>(stringToSurveyStatus(status));
        }

        select_sql << " ORDER BY created_at DESC";
        select_sql << " LIMIT " << page_size << " OFFSET " << (page - 1) * page_size;

        std::vector<std::map<std::string, std::string>> results;
        if (!db_.query(select_sql.str(), results)) {
            return surveys;
        }

        for (const auto& row : results) {
            model::Survey survey(
                row.at("id"),
                row.at("owner_id"),
                row.at("title"),
                row.at("description"),
                stringToSurveyStatus(row.at("status")),
                static_cast<std::time_t>(std::stoll(row.at("created_at")))
            );

            surveys.push_back(survey);
        }

        return surveys;
    }

    bool SurveyRepository::addQuestionsToSurvey(const std::string& survey_id, const std::vector<model::Question>& questions) {
        // Start transaction
        if (!db_.execute("BEGIN TRANSACTION")) {
            return false;
        }

        for (const auto& question : questions) {
            std::stringstream insert_sql;
            insert_sql << "INSERT INTO questions (survey_id, index, type, title, options) VALUES (";
            insert_sql << "'" << survey_id << "',";
            insert_sql << question.getIndex() << ",";
            insert_sql << static_cast<int>(question.getType()) << ",";
            insert_sql << "'" << question.getTitle() << "',";

            if (!question.getOptions().empty()) {
                insert_sql << "'";
                for (size_t i = 0; i < question.getOptions().size(); ++i) {
                    if (i > 0) {
                        insert_sql << ";";
                    }
                    insert_sql << question.getOptions()[i];
                }
                insert_sql << "'";
            } else {
                insert_sql << "NULL";
            }

            insert_sql << ")";

            if (!db_.execute(insert_sql.str())) {
                db_.execute("ROLLBACK TRANSACTION");
                return false;
            }
        }

        // Commit transaction
        if (!db_.execute("COMMIT TRANSACTION")) {
            db_.execute("ROLLBACK TRANSACTION");
            return false;
        }

        return true;
    }

    std::vector<model::Question> SurveyRepository::getQuestionsBySurveyId(const std::string& survey_id) {
        std::vector<model::Question> questions;

        std::stringstream select_sql;
        select_sql << "SELECT * FROM questions WHERE survey_id = '" << survey_id << "' ORDER BY index";

        std::vector<std::map<std::string, std::string>> results;
        if (!db_.query(select_sql.str(), results)) {
            return questions;
        }

        for (const auto& row : results) {
            // Parse options
            std::vector<std::string> options;
            if (!row.at("options").empty()) {
                std::stringstream ss(row.at("options"));
                std::string option;

                while (std::getline(ss, option, ';')) {
                    options.push_back(option);
                }
            }

            model::Question question(
                std::stoi(row.at("index")),
                stringToQuestionType(row.at("type")),
                row.at("title"),
                options
            );

            questions.push_back(question);
        }

        return questions;
    }

    std::optional<std::string> SurveyRepository::createResponse(const model::Response& response) {
        // Generate random response ID
        std::stringstream ss;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 15);
        const char* hex_chars = "0123456789abcdef";

        for (int i = 0; i < 32; ++i) {
            ss << hex_chars[dis(gen)];
        }

        std::string response_id = ss.str();

        // Start transaction
        if (!db_.execute("BEGIN TRANSACTION")) {
            return std::nullopt;
        }

        // Insert response into database
        std::stringstream insert_response_sql;
        insert_response_sql << "INSERT INTO responses (id, survey_id, respondent_id, submitted_at) VALUES (";
        insert_response_sql << "'" << response_id << "',";
        insert_response_sql << "'" << response.getSurveyId() << "',";

        if (!response.getRespondentId().empty()) {
            insert_response_sql << "'" << response.getRespondentId() << "'";
        } else {
            insert_response_sql << "NULL";
        }

        insert_response_sql << "," << static_cast<long long>(response.getSubmittedAt()) << ")";

        if (!db_.execute(insert_response_sql.str())) {
            db_.execute("ROLLBACK TRANSACTION");
            return std::nullopt;
        }

        // Insert answers into database
        for (const auto& answer : response.getAnswers()) {
            std::stringstream insert_answer_sql;
            insert_answer_sql << "INSERT INTO answers (response_id, question_index, choice_indices, text) VALUES (";
            insert_answer_sql << "'" << response_id << "',";
            insert_answer_sql << answer.getQuestionIndex() << ",";

            if (!answer.getChoiceIndices().empty()) {
                insert_answer_sql << "'";
                for (size_t i = 0; i < answer.getChoiceIndices().size(); ++i) {
                    if (i > 0) {
                        insert_answer_sql << ";";
                    }
                    insert_answer_sql << answer.getChoiceIndices()[i];
                }
                insert_answer_sql << "'";
            } else {
                insert_answer_sql << "NULL";
            }

            insert_answer_sql << ",";

            if (!answer.getText().empty()) {
                insert_answer_sql << "'" << answer.getText() << "'";
            } else {
                insert_answer_sql << "NULL";
            }

            insert_answer_sql << ")";

            if (!db_.execute(insert_answer_sql.str())) {
                db_.execute("ROLLBACK TRANSACTION");
                return std::nullopt;
            }
        }

        // Commit transaction
        if (!db_.execute("COMMIT TRANSACTION")) {
            db_.execute("ROLLBACK TRANSACTION");
            return std::nullopt;
        }

        return response_id;
    }

    std::vector<model::Response> SurveyRepository::getResponsesBySurveyId(const std::string& survey_id) {
        std::vector<model::Response> responses;

        std::stringstream select_sql;
        select_sql << "SELECT * FROM responses WHERE survey_id = '" << survey_id << "' ORDER BY submitted_at DESC";

        std::vector<std::map<std::string, std::string>> results;
        if (!db_.query(select_sql.str(), results)) {
            return responses;
        }

        for (const auto& row : results) {
            model::Response response(
                row.at("id"),
                row.at("survey_id"),
                row.at("respondent_id"),
                {},
                static_cast<std::time_t>(std::stoll(row.at("submitted_at")))
            );

            // Get answers for this response
            std::stringstream select_answers_sql;
            select_answers_sql << "SELECT * FROM answers WHERE response_id = '" << row.at("id") << "'";

            std::vector<std::map<std::string, std::string>> answers_results;
            if (db_.query(select_answers_sql.str(), answers_results)) {
                for (const auto& answer_row : answers_results) {
                    // Parse choice indices
                    std::vector<int> choice_indices;
                    if (!answer_row.at("choice_indices").empty()) {
                        std::stringstream ss(answer_row.at("choice_indices"));
                        std::string choice_index;

                        while (std::getline(ss, choice_index, ';')) {
                            choice_indices.push_back(std::stoi(choice_index));
                        }
                    }

                    model::Answer answer(
                        std::stoi(answer_row.at("question_index")),
                        choice_indices,
                        answer_row.at("text")
                    );

                    response.addAnswer(answer);
                }
            }

            responses.push_back(response);
        }

        return responses;
    }

    model::SurveyStatus SurveyRepository::stringToSurveyStatus(const std::string& status) {
        if (status == "0" || status == "draft") {
            return model::SurveyStatus::DRAFT;
        } else if (status == "1" || status == "active") {
            return model::SurveyStatus::ACTIVE;
        } else if (status == "2" || status == "closed") {
            return model::SurveyStatus::CLOSED;
        } else {
            return model::SurveyStatus::DRAFT;
        }
    }

    std::string SurveyRepository::surveyStatusToString(model::SurveyStatus status) {
        switch (status) {
            case model::SurveyStatus::DRAFT:
                return "draft";
            case model::SurveyStatus::ACTIVE:
                return "active";
            case model::SurveyStatus::CLOSED:
                return "closed";
            default:
                return "draft";
        }
    }

    model::QuestionType SurveyRepository::stringToQuestionType(const std::string& type) {
        if (type == "0" || type == "single") {
            return model::QuestionType::SINGLE;
        } else if (type == "1" || type == "multiple") {
            return model::QuestionType::MULTIPLE;
        } else if (type == "2" || type == "text") {
            return model::QuestionType::TEXT;
        } else {
            return model::QuestionType::TEXT;
        }
    }

    std::string SurveyRepository::questionTypeToString(model::QuestionType type) {
        switch (type) {
            case model::QuestionType::SINGLE:
                return "single";
            case model::QuestionType::MULTIPLE:
                return "multiple";
            case model::QuestionType::TEXT:
                return "text";
            default:
                return "text";
        }
    }

} // namespace repository
