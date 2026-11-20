# COMP 345 Team Project: Warzone

C++ implementation of a simplified Warzone/Risk game, built in three assignments.
Course: COMP 345, Advanced Program Design with C++, Concordia University.

| Assignment | Topic | Deadline |
|---|---|---|
| A1 | Core classes: Map, Player, Orders, Cards, Game Engine | Oct 25, 2026 |
| A2 | Command processing, startup phase, game loop, orders, logging | Nov 15, 2026 |
| A3 | Player strategies and tournament mode | Dec 6, 2026 |

## Team

Kaila, Rifat, Mengqi, Phuc, Alexandre, Jiaju.

## Assignment 1

| # | Member | Responsibility | Files |
|---|---|---|---|
| 1 | Kaila | Map graph (Territory, Continent, Map, validate) | `Map.h`, `Map.cpp` |
| 2 | Rifat | MapLoader and Map driver | `Map.h`, `Map.cpp`, `MapDriver.cpp` |
| 3 | Mengqi | Orders | `Orders.h`, `Orders.cpp`, `OrdersDriver.cpp` |
| 4 | Phuc | Player | `Player.h`, `Player.cpp`, `PlayerDriver.cpp` |
| 5 | Alexandre | Cards, deck and hand | `Cards.h`, `Cards.cpp`, `CardsDriver.cpp` |
| 6 | Jiaju | Game engine | `GameEngine.h`, `GameEngine.cpp`, `GameEngineDriver.cpp` |

Kaila and Rifat both work in `Map.h` and `Map.cpp` (the whole Map part must be in one file pair). Kaila owns `Territory`, `Continent`, `Map` and `validate()`. Rifat owns `MapLoader` and the driver. Pull before every edit and commit small changes often. Map files used by the Map driver are in the `maps/` folder.

## Assignment 2

| Part | Files | Driver |
|---|---|---|
| 1. Command processing | `CommandProcessing.h`, `CommandProcessing.cpp` | `CommandProcessingDriver.cpp` |
| 2. Game startup phase | `GameEngine.h`, `GameEngine.cpp` | `GameEngineStartupDriver.cpp` |
| 3. Main game loop | `GameEngine.h`, `GameEngine.cpp`, `Player.cpp` | `GameEngineDriver.cpp` |
| 4. Order execution | `Orders.h`, `Orders.cpp` | `OrdersDriver.cpp` |
| 5. Game log observer | `LoggingObserver.h`, `LoggingObserver.cpp` | `LoggingObserverDriver.cpp` |

## Assignment 3

Part 1 adds player strategies (Strategy pattern) and Part 2 adds tournament mode. All strategy classes live in the new `PlayerStrategies.h` and `PlayerStrategies.cpp`.

| Member | Tasks |
|---|---|
| Phuc | `PlayerStrategy` base class, `Player` delegation, `HumanPlayerStrategy` |
| Mengqi | `AggressivePlayerStrategy`, `NeutralPlayerStrategy` (an attacked Neutral becomes Aggressive) |
| Alexandre | `BenevolentPlayerStrategy`, tournament results in the log, tournament driver |
| Rifat | `tournament` command parsing and validation, strategy driver |
| Jiaju | Tournament execution in `GameEngine`, CMake targets, final zip |
| Kaila | `CheaterPlayerStrategy`, automated per-game setup for the tournament |

Tournament command: `tournament -M <maps> -P <strategies> -G <games> -D <max turns>` (1 to 5 maps, 2 to 4 computer strategies, 1 to 5 games per map, 10 to 50 turns).

## Build and run

Requires CMake 3.20 or newer and g++ with C++23 support (tested with MinGW-w64 on Windows). Run these from the project root.

    cmake -S . -B build -G "MinGW Makefiles"
    cmake --build build

On Mac or Linux, use `cmake -S . -B build` without the `-G` option. The executables are created in `build/`.

| Driver | Run |
|---|---|
| Map | `.\build\MapDriver.exe` |
| Orders | `.\build\OrdersDriver.exe` |
| Cards | `.\build\CardsDriver.exe` |
| Player | `.\build\PlayerDriver.exe` |
| Game engine | `.\build\GameEngineDriver.exe` |
| Startup phase | `.\build\GameEngineStartupDriver.exe` |
| Logging observer | `.\build\LoggingObserverDriver.exe` |
| Command processing | `.\build\CommandProcessingDriver.exe` |
| Full game | `.\build\WarzoneGame.exe` |

Do not commit `build/` or any `.exe` files. Assignment 3 drivers will be added to this table as their targets are created.
