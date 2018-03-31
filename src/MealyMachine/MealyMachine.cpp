#include<limits>
#include<iostream>
#include<cassert>
#include<cstring>
#include<sstream>
#include<iomanip>
#include<algorithm>
#include<cctype>

#include<MealyMachine/MealyMachine.h>
#include<MealyMachine/MapTransitionChooser.h>
#include<MealyMachine/TransitionChooser.h>

using namespace mealyMachine;

std::string getHexRepresentation(
    MealyMachine::TransitionSymbol const&symbol,
    size_t size){
  std::stringstream ss;
  for(size_t i=0;i<size;++i)
    ss<<std::setfill('0')<<std::setw(2)<<std::hex<<
      static_cast<uint32_t>(symbol[i]);
  return ss.str();
}

MealyMachine::MealyMachine(size_t largestState){
  symbolBuffer.resize(largestState);
}

MealyMachine::~MealyMachine(){
}

inline void MealyMachine::call(Transition const&transition){
  auto clb = std::get<CALLBACK>(transition);
  if(clb)clb(this);
}

inline bool MealyMachine::nextState(State const&state){
  auto const& transitionIndex = 
    std::get<CHOOSER>(state)->getTransition(currentSymbol);
  Transition const* transition = nullptr;
  if(transitionIndex == MealyMachine::nonexistingTransition){
    auto trans = std::get<ELSE_TRANSITION>(state);
    if(!trans){
      if(quiet)return false;
      std::stringstream ss;
      ss<<"mealyMachine::_nextState - ";
      ss<<"there is no suitable transition from state ";
      ss<<currentState<<" using symbol: 0x"<<
        getHexRepresentation(currentSymbol,currentSymbolSize);
      ss<<" at position: "<<currentSymbol;
      throw std::invalid_argument(ss.str());
      return false;
    }
    transition = &*trans;
  }else transition = &std::get<TRANSITIONS>(state)[transitionIndex];
  call(*transition);
  currentState = std::get<STATE_INDEX>(*transition);
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
    std::shared_ptr<TransitionChooser>const&chooser,
    std::string                       const&name   ){

  if(chooser == nullptr){
    std::stringstream ss;
    ss << "mealyMachine::addState(" << name << ")";
    ss << " - transition chooser is nullptr";
    throw std::invalid_argument(ss.str());
  }

  if(chooser->getSize() > symbolBuffer.size()){
    std::stringstream ss;
    ss << "mealyMachine::addState(" << name << ")";
    ss << " - transition chooser's symbol size (" << chooser->getSize();
    ss << ") is greater that this mealyMachine symbol buffer size (";
    ss << symbolBuffer.size() << ")";
    throw std::invalid_argument(ss.str());
  }

  auto id = states.size();
  states.emplace_back(TransitionVector(),chooser,nullptr,nullptr,name);
  return id;
}

/**
 * @brief This function adds new state to Mealy machine.
 * This function selects MapTransitionChooser as TransitionChooser.
 *
 * @param name name of the added state
 *
 * @return id of new state
 */
MealyMachine::StateIndex MealyMachine::addState(std::string const&name){
  return this->addState(std::make_shared<MapTransitionChooser<1>>(),name);
}

/**
 * @brief This function adds/creates transition between two states
 *
 * @param from Id of start state
 * @param symbol transition symbol that is needed in order to perform transition
 * @param to id of end state
 * @param callback when the transition happens, this callback is executed.
 */
void MealyMachine::addTransition(
    StateIndex       const&from    ,
    TransitionSymbol const&lex     ,
    StateIndex       const&to      ,
    Callback         const&callback){
  if(from >= states.size()){
    std::stringstream ss;
    ss << "mealyMachine::addTransition(" << from << "," <<  lex << "," << to << ")";
    ss << " - from symbol(" << from << " does not exists";
    throw std::invalid_argument(ss.str());
  }

  assert(from<states.size());
  assert(to<states.size());
  assert(std::get<CHOOSER>(states[from])!=nullptr);
  std::get<CHOOSER>(states[from])->addTransition(lex);
  std::get<TRANSITIONS>(states[from]).push_back(
      Transition(to,callback));
}

/**
 * @brief This function adds/creates transition between two states.
 *
 * @param from id of start state
 * @param symbols vector of accepted transition symbols
 * @param to id of end state
 * @param callback when the transition happens, this callback is executed.
 */
void MealyMachine::addTransition(
    StateIndex                   const&from    ,
    std::vector<TransitionSymbol>const&symbols ,
    StateIndex                   const&to      ,
    Callback                     const&callback){
  for(auto const&x:symbols)
    this->addTransition(from,x,to,callback);
}

/**
 * @brief This function adds/creates transition between two states.
 *
 * @param from id of start state
 * @param symbolFrom start of range of accepted transition symbols
 * @param symbolTo end of range of accepted transition symbols
 * @param to id of end state
 * @param callback when the transition happens, this callback is executed.
 */
void MealyMachine::addTransition(
    StateIndex                   const&from      ,
    TransitionSymbol             const&symbolFrom,
    TransitionSymbol             const&symbolTo  ,
    StateIndex                   const&to        ,
    Callback                     const&callback  ){
  assert(from<states.size());
  assert(std::get<CHOOSER>(states.at(from))!=nullptr);
  size_t stateSize = std::get<CHOOSER>(states.at(from))->getSize();
  for(size_t i=1;i<=stateSize;++i)
    if(symbolFrom[stateSize-i]>symbolTo[stateSize-i])return;
  bool running=true;
  std::vector<BasicUnit>currentSymbol;
  currentSymbol.resize(stateSize);
  std::memcpy(currentSymbol.data(),symbolFrom,stateSize);
  do{
    this->addTransition(from,currentSymbol.data(),to,callback);
    size_t ii=0;
    while(ii<currentSymbol.size()&&currentSymbol.at(ii)==std::numeric_limits<BasicUnit>::max())
      currentSymbol.at(ii++)=0;
    if(ii<currentSymbol.size())currentSymbol.at(ii)++;
    else break;
    for(size_t i=1;i<=stateSize;++i)
      if(currentSymbol[stateSize-i]>symbolTo[stateSize-i]){
        running=false;
        break;
      }
  }while(running);
}

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
void MealyMachine::addTransition(
    StateIndex  const&from    ,
    std::string const&lex     ,
    StateIndex  const&to      ,
    Callback    const&callback){
  assert(from<states.size());
  assert(std::get<CHOOSER>(states.at(from))!=nullptr);
  size_t stateSize = std::get<CHOOSER>(states.at(from))->getSize();
  if(lex.length()%stateSize!=0){
    std::stringstream ss;
    ss << "mealyMachine::addTransition(";
    ss << from << ", " << lex << ", " << to << ") -";
    ss << "transition symbol length is not multiplication of state size: ";
    ss << stateSize;
    throw std::invalid_argument(ss.str());
    return;
  }
  for(size_t offset=0;offset<lex.length();offset+=stateSize)
    this->addTransition(from,(TransitionSymbol)lex.c_str()+offset,to,callback);
}

/**
 * @brief This function adds/creates transition between two states.
 *
 * @param from id of start state
 * @param symbols vector of symbols. Every element of symbols has to have lenght equal to
 * multiplication of transitionChooser state size in "from" state.
 * @param to id of end state
 * @param callback when the transition happens, this callback is executed.
 */
void MealyMachine::addTransition(
    StateIndex              const&from    ,
    std::vector<std::string>const&symbols ,
    StateIndex              const&to      ,
    Callback                const&callback){
  for(auto const&x:symbols)
    this->addTransition(from,x,to,callback);
}

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
void MealyMachine::addTransition(
    StateIndex  const&from      ,
    std::string const&symbolFrom,
    std::string const&symbolTo  ,
    StateIndex  const&to        ,
    Callback    const&callback  ){
  this->addTransition(from,(TransitionSymbol)symbolFrom.c_str(),(TransitionSymbol)symbolTo.c_str(),to,callback);
}

/**
 * @brief This function adds/creates else transiton between two states.
 * Else transition is executed if input does not correspond to any other transition in "from"
 * state.
 *
 * @param from id of start state 
 * @param to id of end state
 * @param callback whet the transition happens, this callback is executed.
 */
void MealyMachine::addElseTransition(
    StateIndex   const&from    ,
    StateIndex   const&to      ,
    Callback     const&callback){
  assert(from<states.size());
  assert(to<states.size());
  std::get<ELSE_TRANSITION>(states[from]) = 
    std::make_shared<Transition>(to,callback);
}

/**
 * @brief This function adds/creates EOF transition.
 * EOF transition is executed when the mealy machine reaches the end of input stream.
 *
 * @param from id of start state
 * @param callback when the transition happens, this callback is executed.
 */
void MealyMachine::addEOFTransition(
    StateIndex   const&from    ,
    Callback     const&callback){
  assert(from<states.size());
  std::get<EOF_TRANSITION>(states[from]) = 
    std::make_shared<Transition>(0,callback);
}

void MealyMachine::begin(){
  currentState      = 0;
  symbolBufferIndex = 0;
  readingPosition   = 0;
}

bool MealyMachine::parse(BasicUnit const*data,size_t size){
  assert(currentState<states.size());
  size_t read = 0;
  auto const& state = states[currentState];
  auto const& chooser = std::get<CHOOSER>(state);
  auto symbolSize = chooser->getSize();
  while(symbolBufferIndex>0){
    read = std::min(symbolSize-symbolBufferIndex,size);
    std::memcpy(symbolBuffer.data()+symbolBufferIndex,data,
        sizeof(BasicUnit)*read);
    symbolBufferIndex+=read;
    if(symbolBufferIndex<symbolSize)return true;

    currentSymbol = symbolBuffer.data();
    currentSymbolSize = symbolSize;
    dontMoveFlag = false;
    if(!nextState(state))
      return false;
    if(!dontMoveFlag){
      readingPosition += symbolSize*sizeof(BasicUnit);
      symbolBufferIndex = 0;
    }
  }

  do{
    auto const&state   = states.at(currentState);
    auto const&chooser = std::get<CHOOSER>(state);
    symbolSize = chooser->getSize();

    if(size-read<symbolSize){
      if(read == size)return true;
      std::memcpy(symbolBuffer.data(),data+read,size-read);
      symbolBufferIndex = size-read;
      return true;
    }

    currentSymbol = data+read;
    currentSymbolSize = symbolSize;
    dontMoveFlag = false;
    if(!nextState(state))
      return false;
    if(!dontMoveFlag){
      readingPosition += symbolSize*sizeof(BasicUnit);
      read += symbolSize*sizeof(BasicUnit);
    }
  }while(true);
}

bool MealyMachine::parse(char const*data){
  assert(this);
  return this->parse((MealyMachine::BasicUnit const*)data,std::strlen(data));
}

bool MealyMachine::end(){
  if(symbolBufferIndex>0){
    if(quiet)return false;
    std::stringstream ss;
    ss << "mealyMachine::end() - ";
    ss << "there are some unprocess bytes at the end of the stream";
    throw std::runtime_error(ss.str());
    return false;
  }
  assert(currentState<states.size());
  auto const&state = states[currentState];
  auto const&transition = std::get<EOF_TRANSITION>(state);
  if(!transition)return false;
  call(*transition);
  return true;
}

bool MealyMachine::match(BasicUnit const*data,size_t size){
  assert(this);
  this->begin();
  return this->parse(data,size)&&this->end();
}

bool MealyMachine::match(char const*data){
  assert(this);
  return this->match((BasicUnit const*)data,std::strlen(data));
}

const MealyMachine::TransitionIndex MealyMachine::nonexistingTransition = 
std::numeric_limits<MealyMachine::TransitionIndex>::max();


/**
 * @brief This function returns string representation of the Mealy Machine.
 *
 * @return string representation
 */
std::string MealyMachine::str()const{
  auto printTransition = [&](Transition const&t){
    std::stringstream ss;
    auto endStateIndex = std::get<STATE_INDEX>(t);
    assert(endStateIndex<states.size());
    auto endState = states.at(endStateIndex);
    if(std::get<NAME>(endState)!="")ss<<std::get<NAME>(endState);
    else ss<<endStateIndex;
    ss<<std::endl;
    return ss.str();
  };
  auto printSymbol = [](TransitionSymbol symbol,size_t size){
    std::stringstream ss;
    if(size==1&&std::isprint(symbol[0])){
      ss<<"\'"<<(char)symbol[0]<<"\' ";
      return ss.str();
    }
    for(MealyMachine::TransitionIndex i = 0;i<size;++i)
      ss<<std::setfill('0')<<std::setw(2)<<std::hex<<
        static_cast<uint32_t>(symbol[i]);
    return ss.str();
  };

  std::stringstream ss;
  size_t stateCounter = 0;
  for(auto const&s:states){
    ss<<"state ";
    if(std::get<NAME>(s)!="")ss<<std::get<NAME>(s);
    else ss<<stateCounter;
    ss<<": "<<std::endl;
    auto const&transitions = std::get<TRANSITIONS>(s);
    size_t transitionCounter = 0;
    for(auto t:transitions){
      auto const&chooser = std::get<CHOOSER>(s);
      ss<<"  ";

      ss<<printSymbol(chooser->getSymbol(transitionCounter),chooser->getSize());
      if(chooser->getSize()<2)
        ss<<"  ";

      ss<<" -> "<<printTransition(t);
      transitionCounter++;
    }
    if(std::get<EOF_TRANSITION>(s)){
      auto const&t = *std::get<EOF_TRANSITION>(s);
      ss<<"  ";
      ss<<"eof "<<printTransition(t);
    }
    if(std::get<ELSE_TRANSITION>(s)){
      auto const&t = *std::get<ELSE_TRANSITION>(s);

      auto const&chooser = std::get<CHOOSER>(s);
      ss<<"  ";

      ss<<"else";
      if(chooser->getSize()>2)
        for(size_t i=0;i<chooser->getSize()-2;++i)
          ss<<"  ";

      ss<<" -> "<<std::get<ELSE_TRANSITION>(s)<<" "<<printTransition(t);
    }
    stateCounter++;
  }
  return ss.str();
}

void MealyMachine::setQuiet(bool q){
  assert(this);
  quiet = q;
}

bool MealyMachine::isQuiet()const{
  assert(this);
  return quiet;
}

