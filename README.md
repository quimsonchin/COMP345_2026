# COMP 345 Team Project: Warzone (Assignment 1)

C++ implementation of the building blocks of a simplified Warzone/Risk game.
Course: COMP 345, Advanced Program Design with C++, Concordia University.
Deadline: October 25, 2026.

## Team and work division

| # | Member | Responsibility | Files |
|---|---|---|---|
| 1 | Kaila | Map graph implementation | `Map.h`, `Map.cpp` |
| 2 | Rifat | MapLoader and MapDriver | `Map.h`, `Map.cpp`, `MapDriver.cpp` |
| 3 | Mengqi | Orders system | `Orders.h`, `Orders.cpp`, `OrdersDriver.cpp` |
| 4 | Phuc | Player class | `Player.h`, `Player.cpp`, `PlayerDriver.cpp` |
| 5 | Member 5 | Cards, deck and hand | `Cards.h`, `Cards.cpp`, `CardsDriver.cpp` |
| 6 | Member 6 | Game engine | `GameEngine.h`, `GameEngine.cpp`, `GameEngineDriver.cpp` |

(Replace the member names above with the real ones.)

Members 1 and 2 both work in `Map.h` and `Map.cpp` (the assignment requires the whole Map part in one file pair). Member 1 owns `Territory`, `Continent`, `Map` and `validate()`. Member 2 owns `MapLoader` and the driver. Pull before every edit and commit small changes often to avoid merge conflicts in these two files.

## Project structure

| Part | Files | Driver |
|---|---|---|
| 1. Map | `Map.h`, `Map.cpp` | `MapDriver.cpp` |
| 2. Player | `Player.h`, `Player.cpp` | `PlayerDriver.cpp` |
| 3. Orders list | `Orders.h`, `Orders.cpp` | `OrdersDriver.cpp` |
| 4. Cards deck/hand | `Cards.h`, `Cards.cpp` | `CardsDriver.cpp` |
| 5. Game Engine | `GameEngine.h`, `GameEngine.cpp` | `GameEngineDriver.cpp` |

Map files used by the Map driver are in the `maps/` folder.

## How to build and run

Requires `g++` with C++17 support (tested with MinGW-w64 on Windows).
Each driver has its own `main()`, so compile each driver only with the files it needs. Run these from the project root.

```
g++ -std=c++17 -Wall MapDriver.cpp Map.cpp -o MapDriver.exe
.\MapDriver.exe
```

```
g++ -std=c++17 -Wall OrdersDriver.cpp Orders.cpp -o OrdersDriver.exe
.\OrdersDriver.exe
```

```
g++ -std=c++17 -Wall CardsDriver.cpp Cards.cpp Orders.cpp -o CardsDriver.exe
.\CardsDriver.exe
```

```
g++ -std=c++17 -Wall PlayerDriver.cpp Player.cpp Map.cpp Orders.cpp Cards.cpp -o PlayerDriver.exe
.\PlayerDriver.exe
```

```
g++ -std=c++17 -Wall GameEngineDriver.cpp GameEngine.cpp -o GameEngineDriver.exe
.\GameEngineDriver.exe
```

Note: the exact list of `.cpp` files for each driver depends on how the parts depend on each other. If a build fails with "undefined reference" errors, add the missing part's `.cpp` file to the command.
