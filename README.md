# COMP 345 Team Project: Warzone (Assignment 1)

C++ implementation of the building blocks of a simplified Warzone/Risk game.
Course: COMP 345, Advanced Program Design with C++, Concordia University.
Deadline: October 25, 2026.

## Team

| Member | Parts |
|---|---|
| Kaila | Part 1: Map |
| Teammate B | Part 3: Orders list, Part 4: Cards |
| Teammate C | Part 2: Player, Part 5: Game Engine |

(Replace the teammate names above with the real ones.)

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

## Code rules (from the assignment)

- Data members of user-defined class type are pointers.
- Each part lives in its own `.h`/`.cpp` pair; no inline functions.
- Every class has a copy constructor, assignment operator, stream insertion operator and destructor.
- No memory leaks.
- Every class, method, free function and operator is commented; no commented-out code.

