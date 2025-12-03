#include "models/like.h"

Like::Like(int id, int user_id, int question_id, const std::string& created_at) 
    : id(id), user_id(user_id), question_id(question_id), created_at(created_at) {}

Like::Like(int user_id, int question_id) 
    : user_id(user_id), question_id(question_id) {}

int Like::getId() const { return id; }

int Like::getUserId() const { return user_id; }

int Like::getQuestionId() const { return question_id; }

std::string Like::getCreatedAt() const { return created_at; }
