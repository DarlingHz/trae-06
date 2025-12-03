#include "models/tag.h"

Tag::Tag(int id, const std::string& name, int question_count) 
    : id(id), name(name), question_count(question_count) {}

Tag::Tag(const std::string& name) 
    : name(name), question_count(0) {}

int Tag::getId() const { return id; }

std::string Tag::getName() const { return name; }

int Tag::getQuestionCount() const { return question_count; }

void Tag::setQuestionCount(int count) { question_count = count; }

void Tag::incrementQuestionCount() { question_count++; }

void Tag::decrementQuestionCount() { question_count--; }
