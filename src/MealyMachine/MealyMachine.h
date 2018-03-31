/*!
 * @file
 * @brief This file contains the implementation of simple Mealy machine.
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz, amillhouse@seznam.cz
 */

#pragma once

#include <MealyMachine/Fwd.h>
#include <MealyMachine/mealymachine_export.h>
#include <functional>
#include <limits>
#include <memory>
#include <tuple>
#include <vector>

/**
 * @brief This class represents simple mealy machine.
 * It is able to parse tokens.
 * You can create Finite State Machines and specify actions for every
 * transition.
 */
class MEALYMACHINE_EXPORT mealyMachine::MealyMachine {
  public:
  using StateIndex       = size_t;
  using BasicUnit        = uint8_t;
  using TransitionSymbol = BasicUnit const*;
  using Callback         = std::function<void(MealyMachine*)>;
  using SimpleCallback   = std::function<void()>;
  MealyMachine(size_t largestState = 1);
  virtual ~MealyMachine();
  StateIndex addState(std::shared_ptr<TransitionChooser> const& chooser,
                      std::string const&                        name = "");
  StateIndex addState(std::string const& name = "");
  void addTransition(StateIndex const&       from,
                     TransitionSymbol const& symbol,
                     StateIndex const&       to,
                     Callback const&         callback = nullptr);
  void addTransition(StateIndex const&                    from,
                     std::vector<TransitionSymbol> const& symbols,
                     StateIndex const&                    to,
                     Callback const&                      callback = nullptr);
  void addTransition(StateIndex const&       from,
                     TransitionSymbol const& symbolFrom,
                     TransitionSymbol const& symbolTo,
                     StateIndex const&       to,
                     Callback const&         callback = nullptr);
  void addTransition(StateIndex const&  from,
                     std::string const& symbols,
                     StateIndex const&  to,
                     Callback const&    callback = nullptr);
  void addTransition(StateIndex const&               from,
                     std::vector<std::string> const& symbols,
                     StateIndex const&               to,
                     Callback const&                 callback = nullptr);
  void addTransition(StateIndex const&  from,
                     std::string const& symbolFrom,
                     std::string const& symbloTo,
                     StateIndex const&  to,
                     Callback const&    callback = nullptr);
  void addElseTransition(StateIndex const& from,
                         StateIndex const& to,
                         Callback const&   callback = nullptr);
  void addEOFTransition(StateIndex const& from,
                        Callback const&   callback = nullptr);
  virtual void begin();
  virtual bool parse(BasicUnit const* data, size_t size);
  bool parse(char const* data);
  virtual bool end();
  bool match(BasicUnit const* data, size_t size);
  bool match(char const* data);
  inline size_t const&     getReadingPosition() const;
  inline TransitionSymbol  getCurrentSymbol() const;
  inline StateIndex const& getCurrentState() const;
  inline void              dontMove();
  virtual std::string      str() const;
  void setQuiet(bool quiet);
  bool isQuiet() const;

  protected:
  using TransitionSymbolIndex = size_t;
  using Transition            = std::tuple<StateIndex, Callback>;
  using TransitionVector      = std::vector<Transition>;

  public:
  using TransitionIndex = TransitionVector::size_type;
  static const TransitionIndex nonexistingTransition;

  protected:
  using State = std::tuple<TransitionVector,
                           std::shared_ptr<TransitionChooser>,
                           std::shared_ptr<Transition>,
                           std::shared_ptr<Transition>,
                           std::string>;
  enum TransitionParts {
    STATE_INDEX = 0,
    CALLBACK    = 1,
  };
  enum StateParts {
    TRANSITIONS     = 0,
    CHOOSER         = 1,
    ELSE_TRANSITION = 2,
    EOF_TRANSITION  = 3,
    NAME            = 4,
  };
  inline void call(Transition const& transitions);
  inline bool nextState(State const& state);
  bool                   quiet             = false;
  bool                   dontMoveFlag      = false;
  size_t                 readingPosition   = 0;
  TransitionSymbol       currentSymbol     = nullptr;
  size_t                 currentSymbolSize = 0;
  std::vector<State>     states;
  StateIndex             currentState = 0;
  std::vector<BasicUnit> symbolBuffer;
  TransitionSymbolIndex  symbolBufferIndex = 0;
};

/**
 * @brief This function returns the position in input stream.
 *
 * @return position in input stream
 */
inline size_t const& mealyMachine::MealyMachine::getReadingPosition() const {
  return readingPosition;
}

/**
 * @brief This function returns current transition symbol.
 * This function can be called by callbacks, it returns symbol that triggers the
 * transtion.
 *
 * @return transition symbol
 */
inline mealyMachine::MealyMachine::TransitionSymbol
mealyMachine::MealyMachine::getCurrentSymbol() const {
  return currentSymbol;
}

/**
 * @brief This function returns current transition symbol.
 * This function can be called by callbacks, it returns the "from" state.
 *
 * @return "from" state id
 */
inline mealyMachine::MealyMachine::StateIndex const&
mealyMachine::MealyMachine::getCurrentState() const {
  return currentState;
}

/**
 * @brief This function can be called inside callbacks.
 * When a callback calls this function, the position in input stream is not
 * updated and
 * current transition symbol is used by consequent transition.
 */
inline void mealyMachine::MealyMachine::dontMove() { dontMoveFlag = true; }
