# Build folder
mkdir -p build

# Compile
## Map
clang++ -std=c++17 -Wall -Wextra MapDriver.cpp Map.cpp -o build/MapDriver
## Player
clang++ -std=c++17 -Wall -Wextra PlayerDriver.cpp Player.cpp Cards.cpp Orders.cpp Map.cpp -o build/PlayerDriver
## Oders
clang++ -std=c++17 -Wall -Wextra OrdersDriver.cpp Orders.cpp Map.cpp -o build/OrdersDriver
## Cards
clang++ -std=c++17 -Wall -Wextra CardsDriver.cpp Cards.cpp Player.cpp Orders.cpp Map.cpp -o build/CardsDriver
## GameEngine
clang++ -std=c++17 -Wall -Wextra GameEngineDriver.cpp GameEngine.cpp -o build/GameEngineDriver

# Test
./build/MapDriver
./build/PlayerDriver
./build/OrdersDriver
./build/CardsDriver
./build/GameEngineDriver --test