# MealyMachine
This library contains simple configurable mealy machine for token parsing.
Mealy machine contains states, transitions and actions.
A transition connects two states and can perform an action.
Each transition consumes N bytes of an input stream.

## How to use this library

Basic floating point machine with no actions:
```cpp
// Parsing floating point number

MealyMachine mm;
// add all states
auto start            = mm.addState("start"           );
auto sign             = mm.addState("sign"            );
auto immediateDot     = mm.addState("immediateDot"    );
auto fractionalNumber = mm.addState("fractionalNumber");
auto wholeNumber      = mm.addState("whileNumber"     );
auto exponent         = mm.addState("exponent"        );
auto postfix          = mm.addState("postfix"         );
auto exponentSign     = mm.addState("exponentSign"    );
auto exponentNumber   = mm.addState("exponentNumber"  );

// add transitions
mm.addTransition   (start           ,"+-"   ,sign            );
mm.addTransition   (start           ,"."    ,immediateDot    );
mm.addTransition   (start           ,"0","9",wholeNumber     );
mm.addTransition   (sign            ,"."    ,immediateDot    );
mm.addTransition   (sign            ,"0","9",wholeNumber     );
mm.addTransition   (immediateDot    ,"0","9",fractionalNumber);
mm.addTransition   (wholeNumber     ,"0","9",wholeNumber     );
mm.addTransition   (wholeNumber     ,"."    ,fractionalNumber);
mm.addTransition   (wholeNumber     ,"fF"   ,postfix         );
mm.addTransition   (wholeNumber     ,"eE"   ,exponent        );
mm.addEOFTransition(wholeNumber                              );
mm.addTransition   (fractionalNumber,"0","9",fractionalNumber);
mm.addTransition   (fractionalNumber,"fF"   ,postfix         );
mm.addTransition   (fractionalNumber,"eE"   ,exponent        );
mm.addEOFTransition(fractionalNumber                         );
mm.addEOFTransition(postfix                                  );
mm.addTransition   (exponent        ,"+-"   ,exponentSign    );
mm.addTransition   (exponent        ,"0","9",exponentNumber  );
mm.addTransition   (exponentSign    ,"0","9",exponentNumber  );
mm.addTransition   (exponentNumber  ,"0","9",exponentNumber  );
mm.addTransition   (exponentNumber  ,"fF"   ,postfix         );
mm.addEOFTransition(exponentNumber                           );
mm.setQuiet(true);

//parse
mm.begin()
mm.parse("+1.1e-3f")
mm.end(); //this will return true if the string was parsed
```

More advance machine with actions: <br>
<img style="float:left"  src="https://github.com/dormon/MealyMachine/blob/master/img/example1.svg" width="720px">
```cpp

//This machine reads operators: + - ++ -- and count their numbers. 
//When other symbol is recived, the machine reads everything and remembers
//the first position of non +,- symbol and count number of characters

MealyMachine mm;
size_t plusCounter       = 0;
size_t plusPlusCounter   = 0;
size_t minusCounter      = 0;
size_t minusMinusCounter = 0;
size_t position          = 0;
size_t length            = 0;

auto S = mm.addState();
auto P = mm.addState();
auto M = mm.addState();
auto E = mm.addState();

mm.addTransition    (S,"+",P);
mm.addTransition    (S,"-",M);
mm.addElseTransition(S    ,E,[&](MealyMachine*){position = mm.getReadingPosition();length++;});
mm.addEOFTransition (S);

mm.addTransition    (P,"+",S,[&](MealyMachine*){plusPlusCounter++;});
mm.addTransition    (P,"-",M,[&](MealyMachine*){plusCounter++;});
mm.addElseTransition(P,    S,[&](MealyMachine*){mm.dontMove();plusCounter++;});
mm.addEOFTransition (P,      [&](MealyMachine*){plusCounter++;});

mm.addTransition    (M,"-",S,[&](MealyMachine*){minusMinusCounter++;});
mm.addTransition    (M,"+",P,[&](MealyMachine*){minusCounter++;});
mm.addElseTransition(M,    S,[&](MealyMachine*){mm.dontMove();minusCounter++;});
mm.addEOFTransition (M,      [&](MealyMachine*){minusCounter++;});

//if there are no other transitions, ElseTransition behaves as AllTransition
mm.addElseTransition(E,E,[&](MealyMachine*){length++;});
mm.addEOFTransition (E);

auto str0 = "++--+-+-++-a++-+";
mm.begin();
auto result = mm.parse(str0);
mm.end();
```
