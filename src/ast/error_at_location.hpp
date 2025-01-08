#pragma once

#include <exception>
#include <string>

#include "lex/location.hpp"

class ErrorAtLocation : public std::exception {
public:
    ErrorAtLocation(lex::Location location, std::string description)
      : loc_(std::move(location)), desc_(std::move(description)) {}

    ErrorAtLocation(ErrorAtLocation&&) = default;
    ErrorAtLocation(const ErrorAtLocation&) = delete;

    virtual ~ErrorAtLocation() = default;

    const char* what() const noexcept override {
        return desc_.c_str();
    }

    lex::Location where() const noexcept {
        return loc_;
    }

private:
    lex::Location loc_;
    std::string desc_;
};
