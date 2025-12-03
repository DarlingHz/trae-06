#include "models/question.h"

Question::Question(int id, const std::string& title, const std::string& content, 
                   int author_id, const std::string& created_at, int view_count, 
                   int like_count, int answer_count) 
    : id(id), title(title), content(content), author_id(author_id), 
      created_at(created_at), view_count(view_count), like_count(like_count), 
      answer_count(answer_count) {}

Question::Question(const std::string& title, const std::string& content, 
                   int author_id) 
    : title(title), content(content), author_id(author_id), view_count(0), 
      like_count(0), answer_count(0) {}

int Question::getId() const { return id; }

std::string Question::getTitle() const { return title; }

std::string Question::getContent() const { return content; }

int Question::getAuthorId() const { return author_id; }

std::string Question::getCreatedAt() const { return created_at; }

int Question::getViewCount() const { return view_count; }

int Question::getLikeCount() const { return like_count; }

int Question::getAnswerCount() const { return answer_count; }

void Question::setTitle(const std::string& title) { this->title = title; }

void Question::setContent(const std::string& content) { this->content = content; }

void Question::incrementViewCount() { view_count++; }

void Question::incrementLikeCount() { like_count++; }

void Question::decrementLikeCount() { like_count--; }

void Question::incrementAnswerCount() { answer_count++; }

void Question::decrementAnswerCount() { answer_count--; }
