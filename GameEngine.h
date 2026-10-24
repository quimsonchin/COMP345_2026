#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include <iostream>
#include <string>
#include <vector>

// Assignment 1, Part 5: the nine states in the supplied diagram.
// GameState is an enumeration, not a class-type data member.
enum class GameState {
    Start,
    MapLoaded,
    MapValidated,
    PlayersAdded,
    AssignReinforcement,
    IssueOrders,
    ExecuteOrders,
    Win,
    End
};

// Controls the game's flow by accepting only valid transitions.
// This A1 class does not load maps or execute game actions: those
// responsibilities are demonstrated separately by the other parts.
class GameEngine {
public:
    // Initialize an engine in the start state.
    GameEngine();

    // Copy the current state from another engine.
    GameEngine(const GameEngine& other);

    // Copy the current state, supporting self-assignment.
    GameEngine& operator=(const GameEngine& other);

    // No heap-allocated resources are owned by this class.
    ~GameEngine();

    // Apply a console command. Return false without changing state
    // when the command is not an outgoing edge of the current state.
    bool transition(const std::string& command);

    // Get the current state and its human-readable name.
    GameState getState() const;
    std::string getStateName() const;

    // Return the valid outgoing commands for the current state.
    std::vector<std::string> getAvailableCommands() const;

    // The end state has no outgoing transitions.
    bool isFinished() const;

    // Print the current engine state.
    friend std::ostream& operator<<(std::ostream& out, const GameEngine& engine);

private:
    GameState currentState;
};

#endif
