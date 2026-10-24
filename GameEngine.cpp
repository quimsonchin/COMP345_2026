#include "GameEngine.h"

// A new game begins in the start state.
GameEngine::GameEngine() : currentState(GameState::Start) {}

// The enum has value semantics, so copying it is sufficient.
GameEngine::GameEngine(const GameEngine& other)
    : currentState(other.currentState) {}

// Copy assignment safely handles an object assigned to itself.
GameEngine& GameEngine::operator=(const GameEngine& other) {
    if (this != &other) {
        currentState = other.currentState;
    }
    return *this;
}

// No explicit resource cleanup is required.
GameEngine::~GameEngine() {}

// Apply exactly the arrows shown in the Assignment 1 state diagram.
// Unknown commands, including commands valid in different states,
// are rejected without changing the current state.
bool GameEngine::transition(const std::string& command) {
    switch (currentState) {
        case GameState::Start:
            if (command == "loadmap") {
                currentState = GameState::MapLoaded;
                return true;
            }
            break;

        case GameState::MapLoaded:
            if (command == "loadmap") {
                return true;  // The diagram explicitly permits reloading.
            }
            if (command == "validatemap") {
                currentState = GameState::MapValidated;
                return true;
            }
            break;

        case GameState::MapValidated:
            if (command == "addplayer") {
                currentState = GameState::PlayersAdded;
                return true;
            }
            break;

        case GameState::PlayersAdded:
            if (command == "addplayer") {
                return true;  // Additional player, same state.
            }
            if (command == "assigncountries") {
                currentState = GameState::AssignReinforcement;
                return true;
            }
            break;

        case GameState::AssignReinforcement:
            if (command == "issueorder") {
                currentState = GameState::IssueOrders;
                return true;
            }
            break;

        case GameState::IssueOrders:
            if (command == "issueorder") {
                return true;  // Issue another order, same state.
            }
            if (command == "endissueorders") {
                currentState = GameState::ExecuteOrders;
                return true;
            }
            break;

        case GameState::ExecuteOrders:
            if (command == "execorder") {
                return true;  // Execute another order, same state.
            }
            if (command == "endexecorders") {
                currentState = GameState::AssignReinforcement;
                return true;  // Start the next turn.
            }
            if (command == "win") {
                currentState = GameState::Win;
                return true;
            }
            break;

        case GameState::Win:
            if (command == "play") {
                currentState = GameState::Start;
                return true;  // Start a new game.
            }
            if (command == "end") {
                currentState = GameState::End;
                return true;
            }
            break;

        case GameState::End:
            break;  // Terminal state: no valid commands.
    }
    return false;
}

// Read-only access to the current state.
GameState GameEngine::getState() const {
    return currentState;
}

// Convert the enum into labels matching the assignment diagram.
std::string GameEngine::getStateName() const {
    switch (currentState) {
        case GameState::Start: return "start";
        case GameState::MapLoaded: return "map loaded";
        case GameState::MapValidated: return "map validated";
        case GameState::PlayersAdded: return "players added";
        case GameState::AssignReinforcement: return "assign reinforcement";
        case GameState::IssueOrders: return "issue orders";
        case GameState::ExecuteOrders: return "execute orders";
        case GameState::Win: return "win";
        case GameState::End: return "end";
    }
    return "unknown";
}

// Help the console driver display only commands permitted now.
std::vector<std::string> GameEngine::getAvailableCommands() const {
    switch (currentState) {
        case GameState::Start:
            return {"loadmap"};
        case GameState::MapLoaded:
            return {"loadmap", "validatemap"};
        case GameState::MapValidated:
            return {"addplayer"};
        case GameState::PlayersAdded:
            return {"addplayer", "assigncountries"};
        case GameState::AssignReinforcement:
            return {"issueorder"};
        case GameState::IssueOrders:
            return {"issueorder", "endissueorders"};
        case GameState::ExecuteOrders:
            return {"execorder", "endexecorders", "win"};
        case GameState::Win:
            return {"play", "end"};
        case GameState::End:
            return {};
    }
    return {};
}

// Report whether the game has reached its terminal state.
bool GameEngine::isFinished() const {
    return currentState == GameState::End;
}

// Let callers print an engine using std::cout << engine.
std::ostream& operator<<(std::ostream& out, const GameEngine& engine) {
    out << "GameEngine state: " << engine.getStateName();
    return out;
}
