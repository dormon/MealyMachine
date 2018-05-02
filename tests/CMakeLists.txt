cmake_minimum_required(VERSION 3.10.1)

add_executable(tests tests.cpp catch.hpp)

target_link_libraries(tests MealyMachine::MealyMachine)
