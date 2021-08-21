#pragma once

#include <iostream>
#include <exception>
#include <string>

namespace knit
{
    namespace actor
    {
        enum class ExceptionType
        {
            INITIAL,
            RUNTIME,
            DESTORY
        };

        class Exception : public std::exception
        {
        public:
            Exception(const ExceptionType &type, const std::string &message) : type_(type), message_(message){};
            std::string what() { return message_; }

        private:
            ExceptionType type_;
            std::string message_;
        };
    }
}