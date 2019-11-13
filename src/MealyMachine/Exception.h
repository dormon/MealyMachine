#pragma once

#include <MealyMachine/Fwd.h>
#include <stdexcept>
#include <string>

class mealyMachine::ex::Exception : public std::runtime_error {
 public:
  Exception(std::string const& msg) : std::runtime_error(msg) {
    message = std::string("MealyMachine - ") + msg;
  }
  virtual ~Exception() throw() {}
  virtual char const* what() const throw() override {
    return message.c_str();
  }
  std::string message;
};

class mealyMachine::ex::ParsingError : public Exception {
 public:
  ParsingError(std::string const& msg) : Exception(msg) {
    message = std::string("MealyMachine::parsing - ") + msg;
  }
};
