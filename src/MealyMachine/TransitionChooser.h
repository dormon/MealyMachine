#pragma once

#include <MealyMachine/MealyMachine.h>

class mealyMachine::TransitionChooser {
 public:
  inline TransitionChooser(size_t size);
  virtual inline ~TransitionChooser();
  inline size_t                         getSize() const;
  virtual MealyMachine::TransitionIndex getTransition(
      MealyMachine::TransitionSymbol const& data) const                  = 0;
  virtual bool addTransition(MealyMachine::TransitionSymbol const& data) = 0;
  virtual MealyMachine::TransitionSymbol const& getSymbol(
      MealyMachine::TransitionIndex const& index) const = 0;

 protected:
  size_t _size;
};

inline mealyMachine::TransitionChooser::TransitionChooser(size_t size) {
  _size = size;
}

inline mealyMachine::TransitionChooser::~TransitionChooser() {}

inline size_t mealyMachine::TransitionChooser::getSize() const {
  return _size;
}
