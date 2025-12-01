#ifndef QUESTION_H
#define QUESTION_H

#include <string>
#include <vector>

namespace model {

    enum class QuestionType {
        SINGLE = 0,
        MULTIPLE = 1,
        TEXT = 2
    };

    class Question {
    public:
        Question() = default;
        Question(int index, QuestionType type, const std::string& title, const std::vector<std::string>& options)
            : index_(index), type_(type), title_(title), options_(options) {}

        int getIndex() const { return index_; }
        QuestionType getType() const { return type_; }
        const std::string& getTitle() const { return title_; }
        const std::vector<std::string>& getOptions() const { return options_; }

        void setIndex(int index) { index_ = index; }
        void setType(QuestionType type) { type_ = type; }
        void setTitle(const std::string& title) { title_ = title; }
        void setOptions(const std::vector<std::string>& options) { options_ = options; }

    private:
        int index_;
        QuestionType type_;
        std::string title_;
        std::vector<std::string> options_;
    };

} // namespace model

#endif // QUESTION_H
