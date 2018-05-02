#pragma once

#include<cstdlib>

namespace mealyMachine{
  class TransitionChooser;
  class MealyMachine;
  template<size_t>
  class MapTransitionChooser;
  namespace ex{
    class Exception;
    class ParsingError;
  }
}
