#pragma once

#include<MealyMachine/MealyMachine.h>

class dsl::TransitionChooser{
  public:
    inline TransitionChooser(size_t size);
    virtual inline ~TransitionChooser();
    inline size_t  getSize()const;
    virtual MealyMachine::TransitionIndex getTransition(
        MealyMachine::TransitionSymbol const&data)const = 0;
    virtual bool addTransition(
        MealyMachine::TransitionSymbol const&data) = 0;
    virtual MealyMachine::TransitionSymbol const& getSymbol(
        MealyMachine::TransitionIndex const&index)const = 0;
  protected:
    size_t _size;
};

inline dsl::TransitionChooser::TransitionChooser(size_t size){
  this->_size = size;
}

inline dsl::TransitionChooser::~TransitionChooser(){}

inline size_t dsl::TransitionChooser::getSize()const{
  return this->_size;
}

