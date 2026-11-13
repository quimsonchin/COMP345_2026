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
| 5 | Alexandre | Cards, deck and hand | `Cards.h`, `Cards.cpp`, `CardsDriver.cpp` |
| 6 | Jiaju | Game engine | `GameEngine.h`, `GameEngine.cpp`, `GameEngineDriver.cpp` |

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

Requires CMake 3.20+ and a compiler with C++23 support (e.g. `g++`).
Each driver has its own `main()`, so `CMakeLists.txt` builds one executable per driver, each with only the files it needs.

Build all drivers from the project root:

```
cmake -S . -B build
cmake --build build
```

Run the drivers from the project root (the Map driver reads files from `maps/`, relative to the current folder):

```
./build/MapDriver
./build/OrdersDriver
./build/CardsDriver
./build/PlayerDriver
./build/GameEngineDriver
```

To build a single driver, use `cmake --build build --target MapDriver` (or any other driver name).

On Windows, run the executables as `.\build\MapDriver.exe` (some generators put them in `build\Debug\` instead).

If you add a new `.cpp` file or a new dependency between parts, add it to the matching `add_executable` line in `CMakeLists.txt`.

## Assignment 2, Part 5: logging

`LoggingObserver.h/.cpp` implements Subject, Observer, ILoggable and LogObserver.
Attach a LogObserver to each object whose events should be recorded. The observer
must remain alive while attached; use Detach before destroying it if necessary.
Copies start without observers; assignment preserves existing subscriptions.

Build and run the demonstration:

```
cmake --build build --target LoggingObserverDriver
./build/LoggingObserverDriver
```

Enter `loadmap` when prompted. The driver appends command, command effect,
state change, order addition and order execution entries to `gamelog.txt` in
the current folder. One deploy order demonstrates addition and execution.
The minimal CommandProcessing classes support this logging demonstration;
Part 1 validation and file adapter, and Parts 2-4, are left to their owners.
