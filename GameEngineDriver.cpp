#include "GameEngine.h"
#include <iostream>
#include <string>
#include <vector>

// Show the commands permitted by the current engine state.
void showCommands(const GameEngine& engine) {
    const std::vector<std::string> commands = engine.getAvailableCommands();
    std::cout << "Available commands: ";
    if (commands.empty()) {
        std::cout << "(none)";
    } else {
        for (std::size_t i = 0; i < commands.size(); ++i) {
            if (i != 0) std::cout << ", ";
            std::cout << commands[i];
        }
    }
    std::cout << '\n';
}

// Interactive demonstration required by the assignment.
// Any unrecognized command is rejected without a state transition.
int runInteractive() {
    GameEngine engine;
    std::cout << "=== COMP 345 Assignment 1: Game Engine ===\n";
    std::cout << "Type commands exactly as shown (lowercase).\n";
    std::cout << "Press Ctrl+D (macOS/Linux) to exit early.\n\n";

    while (!engine.isFinished()) {
        std::cout << engine << '\n';
        showCommands(engine);
        std::cout << "> ";

        std::string command;
        if (!std::getline(std::cin, command)) {
            std::cout << "\nInput closed; exiting demo.\n";
            return 0;
        }

        const std::string previousState = engine.getStateName();
        if (engine.transition(command)) {
            std::cout << "ACCEPTED: " << command << " | " << previousState
                      << " -> " << engine.getStateName() << "\n\n";
        } else {
            std::cout << "REJECTED: '" << command << "' is not valid in '"
                      << previousState << "'. State unchanged.\n\n";
        }
    }
    std::cout << "Game ended.\n";
    return 0;
}

// Check that a command is accepted/rejected and leaves the expected state.
// This helper is only used by --test and does not affect interactive mode.
bool expect(GameEngine& engine, const std::string& command,
            bool expectedAcceptance, GameState expectedState) {
    const bool accepted = engine.transition(command);
    const bool success = accepted == expectedAcceptance &&
                         engine.getState() == expectedState;
    std::cout << (success ? "[PASS] " : "[FAIL] ")
              << "'" << command << "' -> " << engine.getStateName()
              << (accepted ? " (accepted)" : " (rejected)") << '\n';
    return success;
}

// Automated tests cover every arrow in the state diagram, invalid commands,
// restarting, terminal behavior, copying, assignment and printing.
int runTests() {
    int failures = 0;
    GameEngine engine;
    const auto check = [&failures](bool success) {
        if (!success) ++failures;
    };

    check(expect(engine, "validatemap", false, GameState::Start));
    check(expect(engine, "", false, GameState::Start));
    check(expect(engine, "loadmap", true, GameState::MapLoaded));
    check(expect(engine, "loadmap", true, GameState::MapLoaded));
    check(expect(engine, "assigncountries", false, GameState::MapLoaded));
    check(expect(engine, "validatemap", true, GameState::MapValidated));
    check(expect(engine, "issueorder", false, GameState::MapValidated));
    check(expect(engine, "addplayer", true, GameState::PlayersAdded));
    check(expect(engine, "addplayer", true, GameState::PlayersAdded));

    // Exercise copy constructor, assignment operator and <<.
    GameEngine copy(engine);
    check(copy.getState() == engine.getState());
    GameEngine assigned;
    assigned = engine;
    check(assigned.getState() == engine.getState());
    GameEngine& selfAlias = assigned;
    assigned = selfAlias;  // Test self-assignment through a reference.
    check(assigned.getState() == engine.getState());
    std::cout << "Copy: " << copy << " | Assigned: " << assigned << '\n';

    check(expect(engine, "assigncountries", true, GameState::AssignReinforcement));
    check(expect(engine, "endissueorders", false, GameState::AssignReinforcement));
    check(expect(engine, "issueorder", true, GameState::IssueOrders));
    check(expect(engine, "issueorder", true, GameState::IssueOrders));
    check(expect(engine, "win", false, GameState::IssueOrders));
    check(expect(engine, "endissueorders", true, GameState::ExecuteOrders));
    check(expect(engine, "execorder", true, GameState::ExecuteOrders));
    check(expect(engine, "endexecorders", true, GameState::AssignReinforcement));
    check(expect(engine, "issueorder", true, GameState::IssueOrders));
    check(expect(engine, "endissueorders", true, GameState::ExecuteOrders));
    check(expect(engine, "win", true, GameState::Win));
    check(expect(engine, "loadmap", false, GameState::Win));
    check(expect(engine, "play", true, GameState::Start));
    check(expect(engine, "loadmap", true, GameState::MapLoaded));
    check(expect(engine, "validatemap", true, GameState::MapValidated));
    check(expect(engine, "addplayer", true, GameState::PlayersAdded));
    check(expect(engine, "assigncountries", true, GameState::AssignReinforcement));
    check(expect(engine, "issueorder", true, GameState::IssueOrders));
    check(expect(engine, "endissueorders", true, GameState::ExecuteOrders));
    check(expect(engine, "win", true, GameState::Win));
    check(expect(engine, "end", true, GameState::End));
    check(expect(engine, "play", false, GameState::End));
    check(engine.isFinished());
    check(engine.getAvailableCommands().empty());

    if (failures == 0) {
        std::cout << "\nAll Game Engine tests passed.\n";
    } else {
        std::cout << "\n" << failures << " Game Engine test(s) failed.\n";
    }
    return failures == 0 ? 0 : 1;
}

// Run an interactive console by default; use --test for an automated demo.
int main(int argc, char* argv[]) {
    if (argc == 2 && std::string(argv[1]) == "--test") {
        return runTests();
    }
    if (argc != 1) {
        std::cerr << "Usage: ./game_engine_demo [--test]\n";
        return 1;
    }
    return runInteractive();
}
