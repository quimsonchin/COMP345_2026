cmake_minimum_required(VERSION 3.20)
project(Warzone CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall)

add_executable(MapDriver        MapDriver.cpp        Map.cpp)
add_executable(OrdersDriver     OrdersDriver.cpp     Orders.cpp Map.cpp)
add_executable(CardsDriver      CardsDriver.cpp      Cards.cpp Orders.cpp Map.cpp Player.cpp)
add_executable(PlayerDriver     PlayerDriver.cpp     Player.cpp Cards.cpp Orders.cpp Map.cpp)
add_executable(GameEngineDriver GameEngineDriver.cpp GameEngine.cpp)
add_executable(CommandProcessingDriver CommandProcessingDriver.cpp CommandProcessing.cpp)
