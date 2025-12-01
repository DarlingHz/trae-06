#ifndef SURVEY_H
#define SURVEY_H

#include <string>
#include <vector>
#include <ctime>
#include "question.h"

namespace model {

    enum class SurveyStatus {
        DRAFT = 0,
        ACTIVE = 1,
        CLOSED = 2
    };

    class Survey {
    public:
        Survey() = default;
        Survey(const std::string& id, const std::string& owner_id, const std::string& title,
               const std::string& description, SurveyStatus status, const std::time_t& created_at)
            : id_(id), owner_id_(owner_id), title_(title), description_(description), status_(status), created_at_(created_at) {}

        const std::string& getId() const { return id_; }
        const std::string& getOwnerId() const { return owner_id_; }
        const std::string& getTitle() const { return title_; }
        const std::string& getDescription() const { return description_; }
        SurveyStatus getStatus() const { return status_; }
        const std::time_t& getCreatedAt() const { return created_at_; }
        const std::vector<Question>& getQuestions() const { return questions_; }

        void setId(const std::string& id) { id_ = id; }
        void setOwnerId(const std::string& owner_id) { owner_id_ = owner_id; }
        void setTitle(const std::string& title) { title_ = title; }
        void setDescription(const std::string& description) { description_ = description; }
        void setStatus(SurveyStatus status) { status_ = status; }
        void setCreatedAt(const std::time_t& created_at) { created_at_ = created_at; }
        void setQuestions(const std::vector<Question>& questions) { questions_ = questions; }
        void addQuestion(const Question& question) { questions_.push_back(question); }

    private:
        std::string id_;
        std::string owner_id_;
        std::string title_;
        std::string description_;
        SurveyStatus status_ = SurveyStatus::DRAFT;
        std::time_t created_at_;
        std::vector<Question> questions_;
    };

} // namespace model

#endif // SURVEY_H
