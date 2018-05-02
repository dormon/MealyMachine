#pragma once

#include <MealyMachine/Fwd.h>
#include <stdexcept>
#include <string>

class mealyMachine::ex::Exception : public std::runtime_error {
 public:
  Exception(std::string const& msg) : std::runtime_error(msg) {}
  virtual ~Exception() throw() {}
  virtual char const* what() const throw() override {
    return std::string(std::string("MealyMachine - ") +
                       std::runtime_error::what())
        .c_str();
  }
};

class mealyMachine::ex::ParsingError : public Exception {
 public:
  ParsingError(std::string const& msg) : Exception(msg) {}
  virtual char const* what() const throw() override {
    return std::string(std::string("MealyMachine::parsing - ") +
                       std::runtime_error::what())
        .c_str();
  }
};
