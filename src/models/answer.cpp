#include "models/answer.h"

Answer::Answer(int id, const std::string& content, int author_id, int question_id, 
               const std::string& created_at, bool is_accepted) 
    : id(id), content(content), author_id(author_id), question_id(question_id), 
      created_at(created_at), is_accepted(is_accepted) {}

Answer::Answer(const std::string& content, int author_id, int question_id) 
    : content(content), author_id(author_id), question_id(question_id), 
      is_accepted(false) {}

int Answer::getId() const { return id; }

std::string Answer::getContent() const { return content; }

int Answer::getAuthorId() const { return author_id; }

int Answer::getQuestionId() const { return question_id; }

std::string Answer::getCreatedAt() const { return created_at; }

bool Answer::isAccepted() const { return is_accepted; }

void Answer::setContent(const std::string& content) { this->content = content; }

void Answer::setIsAccepted(bool accepted) { is_accepted = accepted; }
