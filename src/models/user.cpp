#include "models/user.h"

User::User(int id, const std::string& username, const std::string& email, 
           const std::string& password_hash, const std::string& salt, 
           const std::string& created_at, int question_count, int answer_count) 
    : id(id), username(username), email(email), password_hash(password_hash), 
      salt(salt), created_at(created_at), question_count(question_count), 
      answer_count(answer_count) {}

User::User(const std::string& username, const std::string& email, 
           const std::string& password_hash, const std::string& salt) 
    : username(username), email(email), password_hash(password_hash), 
      salt(salt), question_count(0), answer_count(0) {}

int User::getId() const { return id; }

std::string User::getUsername() const { return username; }

std::string User::getEmail() const { return email; }

std::string User::getPasswordHash() const { return password_hash; }

std::string User::getSalt() const { return salt; }

std::string User::getCreatedAt() const { return created_at; }

int User::getQuestionCount() const { return question_count; }

int User::getAnswerCount() const { return answer_count; }

void User::setQuestionCount(int count) { question_count = count; }

void User::setAnswerCount(int count) { answer_count = count; }

void User::incrementQuestionCount() { question_count++; }

void User::incrementAnswerCount() { answer_count++; }
