#ifndef RESPONSE_H
#define RESPONSE_H

#include <string>
#include <vector>
#include <ctime>

namespace model {

    class Answer {
    public:
        Answer() = default;
        Answer(int question_index, const std::vector<int>& choice_indices, const std::string& text)
            : question_index_(question_index), choice_indices_(choice_indices), text_(text) {}

        int getQuestionIndex() const { return question_index_; }
        const std::vector<int>& getChoiceIndices() const { return choice_indices_; }
        const std::string& getText() const { return text_; }

        void setQuestionIndex(int question_index) { question_index_ = question_index; }
        void setChoiceIndices(const std::vector<int>& choice_indices) { choice_indices_ = choice_indices; }
        void setText(const std::string& text) { text_ = text; }

    private:
        int question_index_;
        std::vector<int> choice_indices_;
        std::string text_;
    };

    class Response {
    public:
        Response() = default;
        Response(const std::string& id, const std::string& survey_id, const std::string& respondent_id,
                 const std::vector<Answer>& answers, const std::time_t& submitted_at)
            : id_(id), survey_id_(survey_id), respondent_id_(respondent_id), answers_(answers), submitted_at_(submitted_at) {}

        const std::string& getId() const { return id_; }
        const std::string& getSurveyId() const { return survey_id_; }
        const std::string& getRespondentId() const { return respondent_id_; }
        const std::vector<Answer>& getAnswers() const { return answers_; }
        const std::time_t& getSubmittedAt() const { return submitted_at_; }

        void setId(const std::string& id) { id_ = id; }
        void setSurveyId(const std::string& survey_id) { survey_id_ = survey_id; }
        void setRespondentId(const std::string& respondent_id) { respondent_id_ = respondent_id; }
        void setAnswers(const std::vector<Answer>& answers) { answers_ = answers; }
        void setSubmittedAt(const std::time_t& submitted_at) { submitted_at_ = submitted_at; }
        void addAnswer(const Answer& answer) { answers_.push_back(answer); }

    private:
        std::string id_;
        std::string survey_id_;
        std::string respondent_id_;
        std::vector<Answer> answers_;
        std::time_t submitted_at_;
    };

} // namespace model

#endif // RESPONSE_H
