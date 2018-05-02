#pragma once

#include <MealyMachine/TransitionChooser.h>
#include <cstring>
#include <map>

template <size_t N>
class mealyMachine::MapTransitionChooser
    : public mealyMachine::TransitionChooser {
 public:
  MapTransitionChooser() : TransitionChooser(N) {}
  virtual ~MapTransitionChooser() override {
    for (auto& x : _keys) delete[] x;
  }
  virtual MealyMachine::TransitionIndex getTransition(
      MealyMachine::TransitionSymbol const& data) const override {
    auto ii = _translator.find(data);
    if (ii == _translator.end()) return MealyMachine::nonexistingTransition;
    return ii->second;
    return 0;
  }
  virtual bool addTransition(
      MealyMachine::TransitionSymbol const& data) override {
    auto key = new MealyMachine::BasicUnit[N];
    std::memcpy(key, data, N * sizeof(MealyMachine::BasicUnit));
    _keys.push_back(key);

    auto id = _translator.size();
    _translator[(MealyMachine::BasicUnit const*)_keys.back()] = id;
    return true;
  }
  virtual MealyMachine::TransitionSymbol const& getSymbol(
      MealyMachine::TransitionIndex const& i) const override {
    return _keys.at(i);
  }

 protected:
  struct Comparer {
    bool operator()(MealyMachine::BasicUnit const* const& a,
                    MealyMachine::BasicUnit const* const& b) const {
      for (size_t i = 0; i < N; ++i) {
        if (a[i] < b[i]) return true;
        if (a[i] > b[i]) return false;
      }
      return false;
    }
  };
  std::vector<MealyMachine::TransitionSymbol> _keys;
  std::map<MealyMachine::BasicUnit const*,
           MealyMachine::TransitionIndex,
           Comparer>
      _translator;
};
