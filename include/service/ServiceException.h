#ifndef SERVICE_EXCEPTION_H
#define SERVICE_EXCEPTION_H

#include <exception>
#include <string>

namespace service {

class ServiceException : public std::exception {
public:
    ServiceException(const std::string& message) : message_(message) {}
    ~ServiceException() = default;

    const char* what() const noexcept override {
        return message_.c_str();
    }

private:
    std::string message_;
};

} // namespace service

#endif // SERVICE_EXCEPTION_H