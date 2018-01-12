/*!
 * @file 
 * @brief This file contains the implementation of simple Mealy machine.
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz, amillhouse@seznam.cz
 */

#pragma once

#include<vector>
#include<tuple>
#include<memory>
#include<limits>
#include<functional>
#include<MealyMachine/Fwd.h>
#include<MealyMachine/mealymachine_export.h>

/**
 * @brief This class represents simple mealy machine.
 * It is able to parse tokens.
 * You can create Finite State Machines and specify actions for every transition.
 */
class MEALYMACHINE_EXPORT dsl::MealyMachine{
  public:
    using StateIndex       = size_t;
    using BasicUnit        = uint8_t;
    using TransitionSymbol = BasicUnit const*;
    using Callback         = std::function<void(MealyMachine*)>;
    using SimpleCallback   = std::function<void()>;
    MealyMachine(size_t largestState = 1);
    virtual ~MealyMachine();

    StateIndex addState(
        std::shared_ptr<TransitionChooser>const&chooser     ,
        std::string                       const&name    = "");

    /**
     * @brief This function adds new state to Mealy machine.
     * This function selects MapTransitionChooser as TransitionChooser.
     *
     * @param name name of the added state
     *
     * @return id of new state
     */
    StateIndex addState(std::string const&name = "");

    /**
     * @brief This function adds/creates transition between two states
     *
     * @param from Id of start state
     * @param symbol transition symbol that is needed in order to perform transition
     * @param to id of end state
     * @param callback when the transition happens, this callback is executed.
     */
    void addTransition(
        StateIndex                   const&from                ,
        TransitionSymbol             const&symbol              ,
        StateIndex                   const&to                  ,
        Callback                     const&callback   = nullptr);

    /**
     * @brief This function adds/creates transition between two states.
     *
     * @param from id of start state
     * @param symbols vector of accepted transition symbols
     * @param to id of end state
     * @param callback when the transition happens, this callback is executed.
     */
    void addTransition(
        StateIndex                   const&from                ,
        std::vector<TransitionSymbol>const&symbols             ,
        StateIndex                   const&to                  ,
        Callback                     const&callback   = nullptr);


    /**
     * @brief This function adds/creates transition between two states.
     *
     * @param from id of start state
     * @param symbolFrom start of range of accepted transition symbols
     * @param symbolTo end of range of accepted transition symbols
     * @param to id of end state
     * @param callback when the transition happens, this callback is executed.
     */
    void addTransition(
        StateIndex                   const&from                ,
        TransitionSymbol             const&symbolFrom          ,
        TransitionSymbol             const&symbolTo            ,
        StateIndex                   const&to                  ,
        Callback                     const&callback   = nullptr);

    /**
     * @brief This function adds/creates transition between two states.
     *
     * @param from id of start state
     * @param symbols transition symbol or symbols. The lenght of symbol must be
     * multiplication of transitionChooser's state size.
     * For example: transitionChooser in "from" state requires 2 bytes for every
     * transition symbols; this implies that lenght of symbols needs to be 2,4,6,8,...
     * If the lenght of symbols strings is greates than transitionChooser state
     * size, then the string represents a set of transition symbols.
     * @param to id of end state
     * @param callback when the transition happens, this callback is executed.
     */
    void addTransition(
        StateIndex                   const&from                ,
        std::string                  const&symbols             ,
        StateIndex                   const&to                  ,
        Callback                     const&callback   = nullptr);

    /**
     * @brief This function adds/creates transition between two states.
     *
     * @param from id of start state
     * @param symbols vector of symbols. Every element of symbols has to have lenght equal to
     * multiplication of transitionChooser state size in "from" state.
     * @param to id of end state
     * @param callback when the transition happens, this callback is executed.
     */
    void addTransition(
        StateIndex                   const&from                ,
        std::vector<std::string>     const&symbols             ,
        StateIndex                   const&to                  ,
        Callback                     const&callback   = nullptr);

    /**
     * @brief This function adds/creates transition between two states.
     *
     * @param from id of start state
     * @param symbolFrom start of range of accepted transition symbols, it's lenght
     * has to be equal to transitionChooser's state size
     * @param symbolTo end of range of accepted transition symbols, it's lenght 
     * has to be equal to transitionChooser's state size
     * @param to id of end state
     * @param callback when the transition happens, this callback is executed.
     */
    void addTransition(
        StateIndex                   const&from                ,
        std::string                  const&symbolFrom          ,
        std::string                  const&symbloTo            ,
        StateIndex                   const&to                  ,
        Callback                     const&callback   = nullptr);

    /**
     * @brief This function adds/creates else transiton between two states.
     * Else transition is executed if input does not correspond to any other transition in "from"
     * state.
     *
     * @param from id of start state 
     * @param to id of end state
     * @param callback whet the transition happens, this callback is executed.
     */
    void addElseTransition(
        StateIndex                   const&from                ,
        StateIndex                   const&to                  ,
        Callback                     const&callback   = nullptr);


    /**
     * @brief This function adds/creates EOF transition.
     * EOF transition is executed when the mealy machine reaches the end of input stream.
     *
     * @param from id of start state
     * @param callback when the transition happens, this callback is executed.
     */
    void addEOFTransition(
        StateIndex                   const&from                ,
        Callback                     const&callback   = nullptr);


    virtual void begin();
    virtual bool parse(BasicUnit const*data,size_t size);
    bool parse(char const*data);
    virtual bool end();
    bool match(BasicUnit const*data,size_t size);
    bool match(char const*data);

    /**
     * @brief This function returns the position in input stream.
     *
     * @return position in input stream
     */
    inline size_t     const&getReadingPosition()const;

    /**
     * @brief This function returns current transition symbol.
     * This function can be called by callbacks, it returns symbol that triggers the transtion.
     *
     * @return transition symbol
     */
    inline TransitionSymbol getCurrentSymbol()const;

    /**
     * @brief This function returns current transition symbol.
     * This function can be called by callbacks, it returns the "from" state.
     *
     * @return "from" state id
     */
    inline StateIndex const&getCurrentState()const;

    /**
     * @brief This function can be called inside callbacks.
     * When a callback calls this function, the position in input stream is not updated and
     * current transition symbol is used by consequent transition.
     */
    inline void dontMove();

    /**
     * @brief This function returns string representation of the Mealy Machine.
     *
     * @return string representation
     */
    virtual std::string str()const;
    void setQuiet(bool quiet);
    bool isQuiet()const;
  protected:
    using TransitionSymbolIndex = size_t;
    using Transition            = std::tuple<StateIndex,Callback>;
    using TransitionVector      = std::vector<Transition>;
  public:
    using TransitionIndex       = TransitionVector::size_type;
    static const TransitionIndex nonexistingTransition;
  protected:
    using State = std::tuple<
      TransitionVector,
      std::shared_ptr<TransitionChooser>,
      std::shared_ptr<Transition>,
      std::shared_ptr<Transition>,
      std::string>;
    enum TransitionParts{
      STATE_INDEX = 0,
      CALLBACK    = 1,
    };
    enum StateParts{
      TRANSITIONS     = 0,
      CHOOSER         = 1,
      ELSE_TRANSITION = 2,
      EOF_TRANSITION  = 3,
      NAME            = 4,
    };
    inline void _call(Transition const&transitions);
    inline bool _nextState(State const&state);
    bool                    _quiet             = false  ;
    bool                    _dontMove          = false  ;
    size_t                  _readingPosition   = 0      ;
    TransitionSymbol        _currentSymbol     = nullptr;
    size_t                  _currentSymbolSize = 0      ;
    std::vector<State>      _states                     ;
    StateIndex              _currentState      = 0      ;
    std::vector<BasicUnit>  _symbolBuffer               ;
    TransitionSymbolIndex   _symbolBufferIndex = 0      ;
};

inline size_t const& dsl::MealyMachine::getReadingPosition()const{
  return this->_readingPosition;
}

inline dsl::MealyMachine::TransitionSymbol dsl::MealyMachine::getCurrentSymbol()const{
  return this->_currentSymbol;
}

inline dsl::MealyMachine::StateIndex const&dsl::MealyMachine::getCurrentState()const{
  return this->_currentState;
}

inline void dsl::MealyMachine::dontMove(){
  this->_dontMove = true;
}
