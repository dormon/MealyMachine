#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

#include <MealyMachine/MapTransitionChooser.h>
#include <MealyMachine/MealyMachine.h>
#include <MealyMachine/TransitionChooser.h>
#include <MealyMachine/Exception.h>

using namespace mealyMachine;

std::string getHexRepresentation(MealyMachine::TransitionSymbol const& symbol,
                                 size_t                                size) {
  std::stringstream ss;
  for (size_t i = 0; i < size; ++i)
    ss << std::setfill('0') << std::setw(2) << std::hex
       << static_cast<uint32_t>(symbol[i]);
  return ss.str();
}

MealyMachine::MealyMachine(size_t largestState) {
  _symbolBuffer.resize(largestState);
}

MealyMachine::~MealyMachine() {}

inline void MealyMachine::_call(Transition const& transition) {
  auto clb = std::get<CALLBACK>(transition);
  if (clb) clb(this);
}

inline bool MealyMachine::_nextState(State const& state) {
  auto const& transitionIndex =
      std::get<CHOOSER>(state)->getTransition(_currentSymbol);
  Transition const* transition = nullptr;
  if (transitionIndex == MealyMachine::nonexistingTransition) {
    auto trans = std::get<ELSE_TRANSITION>(state);
    if (!trans) {
      if (_quiet) return false;
      std::stringstream ss;
      ss << "MealyMachine::_nextState - ";
      ss << "there is no suitable transition from state ";
      ss << _currentState << " using symbol: 0x"
         << getHexRepresentation(_currentSymbol, _currentSymbolSize);
      ss << " at position: " << _currentSymbol;
      throw ex::Exception(ss.str());
      return false;
    }
    transition = &*trans;
  } else
    transition = &std::get<TRANSITIONS>(state)[transitionIndex];
  _call(*transition);
  _currentState = std::get<STATE_INDEX>(*transition);
  return true;
}

/**
 * @brief This function adds state to Mealy machine.
 *
 * @param chooser instance of transition chooser.
 * @param name name of the added state
 *
 * @return This function returns Id of newly added state.
 */
MealyMachine::StateIndex MealyMachine::addState(
    std::shared_ptr<TransitionChooser> const& chooser,
    std::string const&                        name) {
  if (chooser == nullptr) {
    std::stringstream ss;
    ss << "MealyMachine::addState(" << name << ")";
    ss << " - transition chooser is nullptr";
    throw ex::Exception(ss.str());
  }

  if (chooser->getSize() > _symbolBuffer.size()) {
    std::stringstream ss;
    ss << "MealyMachine::addState(" << name << ")";
    ss << " - transition chooser's symbol size (" << chooser->getSize();
    ss << ") is greater that this MealyMachine symbol buffer size (";
    ss << _symbolBuffer.size() << ")";
    throw ex::Exception(ss.str());
  }

  auto id = _states.size();
  _states.emplace_back(TransitionVector(), chooser, nullptr, nullptr, name);
  return id;
}

MealyMachine::StateIndex MealyMachine::addState(std::string const& name) {
  return addState(std::make_shared<MapTransitionChooser<1>>(), name);
}

void MealyMachine::addTransition(StateIndex const&       from,
                                 TransitionSymbol const& lex,
                                 StateIndex const&       to,
                                 Callback const&         callback) {
  if (from >= _states.size()) {
    std::stringstream ss;
    ss << "MealyMachine::addTransition(" << from << "," << lex << "," << to
       << ")";
    ss << " - from symbol(" << from << " does not exists";
    throw ex::Exception(ss.str());
  }

  assert(from < _states.size());
  assert(to < _states.size());
  assert(std::get<CHOOSER>(_states[from]) != nullptr);
  std::get<CHOOSER>(_states[from])->addTransition(lex);
  std::get<TRANSITIONS>(_states[from]).push_back(Transition(to, callback));
}

void MealyMachine::addTransition(StateIndex const&                    from,
                                 std::vector<TransitionSymbol> const& symbols,
                                 StateIndex const&                    to,
                                 Callback const& callback) {
  for (auto const& x : symbols) addTransition(from, x, to, callback);
}

void MealyMachine::addTransition(StateIndex const&       from,
                                 TransitionSymbol const& symbolFrom,
                                 TransitionSymbol const& symbolTo,
                                 StateIndex const&       to,
                                 Callback const&         callback) {
  assert(from < _states.size());
  assert(std::get<CHOOSER>(_states.at(from)) != nullptr);
  size_t stateSize = std::get<CHOOSER>(_states.at(from))->getSize();
  for (size_t i = 1; i <= stateSize; ++i)
    if (symbolFrom[stateSize - i] > symbolTo[stateSize - i]) return;
  bool                   running = true;
  std::vector<BasicUnit> currentSymbol;
  currentSymbol.resize(stateSize);
  std::memcpy(currentSymbol.data(), symbolFrom, stateSize);
  do {
    addTransition(from, currentSymbol.data(), to, callback);
    size_t ii = 0;
    while (ii < currentSymbol.size() &&
           currentSymbol.at(ii) == std::numeric_limits<BasicUnit>::max())
      currentSymbol.at(ii++) = 0;
    if (ii < currentSymbol.size())
      currentSymbol.at(ii)++;
    else
      break;
    for (size_t i = 1; i <= stateSize; ++i)
      if (currentSymbol[stateSize - i] > symbolTo[stateSize - i]) {
        running = false;
        break;
      }
  } while (running);
}

void MealyMachine::addTransition(StateIndex const&  from,
                                 std::string const& lex,
                                 StateIndex const&  to,
                                 Callback const&    callback) {
  assert(from < _states.size());
  assert(std::get<CHOOSER>(_states.at(from)) != nullptr);
  size_t stateSize = std::get<CHOOSER>(_states.at(from))->getSize();
  if (lex.length() % stateSize != 0) {
    std::stringstream ss;
    ss << "MealyMachine::addTransition(";
    ss << from << ", " << lex << ", " << to << ") -";
    ss << "transition symbol length is not multiplication of state size: ";
    ss << stateSize;
    throw ex::Exception(ss.str());
    return;
  }
  for (size_t offset = 0; offset < lex.length(); offset += stateSize)
    addTransition(from, (TransitionSymbol)lex.c_str() + offset, to, callback);
}

void MealyMachine::addTransition(StateIndex const&               from,
                                 std::vector<std::string> const& symbols,
                                 StateIndex const&               to,
                                 Callback const&                 callback) {
  for (auto const& x : symbols) addTransition(from, x, to, callback);
}

void MealyMachine::addTransition(StateIndex const&  from,
                                 std::string const& symbolFrom,
                                 std::string const& symbolTo,
                                 StateIndex const&  to,
                                 Callback const&    callback) {
  addTransition(from, (TransitionSymbol)symbolFrom.c_str(),
                (TransitionSymbol)symbolTo.c_str(), to, callback);
}

void MealyMachine::addElseTransition(StateIndex const& from,
                                     StateIndex const& to,
                                     Callback const&   callback) {
  assert(from < _states.size());
  assert(to < _states.size());
  std::get<ELSE_TRANSITION>(_states[from]) =
      std::make_shared<Transition>(to, callback);
}

void MealyMachine::addEOFTransition(StateIndex const& from,
                                    Callback const&   callback) {
  assert(from < _states.size());
  std::get<EOF_TRANSITION>(_states[from]) =
      std::make_shared<Transition>(0, callback);
}

void MealyMachine::begin() {
  _currentState      = 0;
  _symbolBufferIndex = 0;
  _readingPosition   = 0;
}

bool MealyMachine::parse(BasicUnit const* data, size_t size) {
  assert(_currentState < _states.size());
  size_t      read       = 0;
  auto const& state      = _states[_currentState];
  auto const& chooser    = std::get<CHOOSER>(state);
  auto        symbolSize = chooser->getSize();
  while (_symbolBufferIndex > 0) {
    read = std::min(symbolSize - _symbolBufferIndex, size);
    std::memcpy(_symbolBuffer.data() + _symbolBufferIndex, data,
                sizeof(BasicUnit) * read);
    _symbolBufferIndex += read;
    if (_symbolBufferIndex < symbolSize) return true;

    _currentSymbol     = _symbolBuffer.data();
    _currentSymbolSize = symbolSize;
    _dontMove          = false;
    if (!_nextState(state)) return false;
    if (!_dontMove) {
      _readingPosition += symbolSize * sizeof(BasicUnit);
      _symbolBufferIndex = 0;
    }
  }

  do {
    auto const& state   = _states.at(_currentState);
    auto const& chooser = std::get<CHOOSER>(state);
    symbolSize          = chooser->getSize();

    if (size - read < symbolSize) {
      if (read == size) return true;
      std::memcpy(_symbolBuffer.data(), data + read, size - read);
      _symbolBufferIndex = size - read;
      return true;
    }

    _currentSymbol     = data + read;
    _currentSymbolSize = symbolSize;
    _dontMove          = false;
    if (!_nextState(state)) return false;
    if (!_dontMove) {
      _readingPosition += symbolSize * sizeof(BasicUnit);
      read += symbolSize * sizeof(BasicUnit);
    }
  } while (true);
}

bool MealyMachine::parse(char const* data) {
  return parse((MealyMachine::BasicUnit const*)data, std::strlen(data));
}

bool MealyMachine::end() {
  if (_symbolBufferIndex > 0) {
    if (_quiet) return false;
    std::stringstream ss;
    ss << "MealyMachine::end() - ";
    ss << "there are some unprocess bytes at the end of the stream";
    throw ex::ParsingError(ss.str());
    return false;
  }
  assert(_currentState < _states.size());
  auto const& state      = _states[_currentState];
  auto const& transition = std::get<EOF_TRANSITION>(state);
  if (!transition) return false;
  _call(*transition);
  return true;
}

bool MealyMachine::match(BasicUnit const* data, size_t size) {
  begin();
  return parse(data, size) && end();
}

bool MealyMachine::match(char const* data) {
  return match((BasicUnit const*)data, std::strlen(data));
}

const MealyMachine::TransitionIndex MealyMachine::nonexistingTransition =
    std::numeric_limits<MealyMachine::TransitionIndex>::max();

std::string MealyMachine::str() const {
  auto printTransition = [&](Transition const& t) {
    std::stringstream ss;
    auto              endStateIndex = std::get<STATE_INDEX>(t);
    assert(endStateIndex < _states.size());
    auto endState = _states.at(endStateIndex);
    if (std::get<NAME>(endState) != "")
      ss << std::get<NAME>(endState);
    else
      ss << endStateIndex;
    ss << std::endl;
    return ss.str();
  };
  auto printSymbol = [](TransitionSymbol symbol, size_t size) {
    std::stringstream ss;
    if (size == 1 && std::isprint(symbol[0])) {
      ss << "\'" << (char)symbol[0] << "\' ";
      return ss.str();
    }
    for (MealyMachine::TransitionIndex i = 0; i < size; ++i)
      ss << std::setfill('0') << std::setw(2) << std::hex
         << static_cast<uint32_t>(symbol[i]);
    return ss.str();
  };

  std::stringstream ss;
  size_t            stateCounter = 0;
  for (auto const& s : _states) {
    ss << "state ";
    if (std::get<NAME>(s) != "")
      ss << std::get<NAME>(s);
    else
      ss << stateCounter;
    ss << ": " << std::endl;
    auto const& transitions       = std::get<TRANSITIONS>(s);
    size_t      transitionCounter = 0;
    for (auto t : transitions) {
      auto const& chooser = std::get<CHOOSER>(s);
      ss << "  ";

      ss << printSymbol(chooser->getSymbol(transitionCounter),
                        chooser->getSize());
      if (chooser->getSize() < 2) ss << "  ";

      ss << " -> " << printTransition(t);
      transitionCounter++;
    }
    if (std::get<EOF_TRANSITION>(s)) {
      auto const& t = *std::get<EOF_TRANSITION>(s);
      ss << "  ";
      ss << "eof " << printTransition(t);
    }
    if (std::get<ELSE_TRANSITION>(s)) {
      auto const& t = *std::get<ELSE_TRANSITION>(s);

      auto const& chooser = std::get<CHOOSER>(s);
      ss << "  ";

      ss << "else";
      if (chooser->getSize() > 2)
        for (size_t i = 0; i < chooser->getSize() - 2; ++i) ss << "  ";

      ss << " -> " << std::get<ELSE_TRANSITION>(s) << " " << printTransition(t);
    }
    stateCounter++;
  }
  return ss.str();
}

void MealyMachine::setQuiet(bool quiet) { _quiet = quiet; }

bool MealyMachine::isQuiet() const { return _quiet; }
