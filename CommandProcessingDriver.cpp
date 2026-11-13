#include "CommandProcessing.h"

#include <iostream>
#include <string>

// State names as they appear in the handout's command table.
std::string stateName(GameState state) {
    switch (state) {
        case GameState::Start: return "start";
        case GameState::MapLoaded: return "maploaded";
        case GameState::MapValidated: return "mapvalidated";
        case GameState::PlayersAdded: return "playersadded";
        case GameState::AssignReinforcement: return "assignreinforcement";
        case GameState::IssueOrders: return "issueorders";
        case GameState::ExecuteOrders: return "executeorders";
        case GameState::Win: return "win";
        case GameState::End: return "end";
    }
    return "unknown";
}

// Stands in for the GameEngine: moves to the next state and saves the
// effect. Actually loading maps and adding players is Part 2's job.
void executeCommand(Command* command, GameState& state) {
    const std::string name = command->getName();
    const std::string previous = stateName(state);

    if (name == "loadmap") {
        state = GameState::MapLoaded;
    } else if (name == "validatemap") {
        state = GameState::MapValidated;
    } else if (name == "addplayer") {
        state = GameState::PlayersAdded;
    } else if (name == "gamestart") {
        state = GameState::AssignReinforcement;
    } else if (name == "replay") {
        state = GameState::Start;
    } else if (name == "quit") {
        state = GameState::End;
    }

    command->saveEffect(previous + " -> " + stateName(state));
}

// Runs commands until quit or the input runs out. Works the same for the
// console and the file adapter.
void runCommands(CommandProcessor& processor) {
    LogObserver logger;
    processor.Attach(&logger);
    GameState state = GameState::Start;

    while (state != GameState::End) {
        std::cout << "[" << stateName(state) << "] > " << std::flush;
        Command* command = processor.getCommand();
        if (command == nullptr) {
            std::cout << "\nOut of commands.\n";
            break;
        }

        if (processor.validate(command, state)) {
            executeCommand(command, state);
            std::cout << "OK: " << command->getEffect() << '\n';
        } else {
            std::cout << "Rejected: " << command->getEffect() << '\n';
        }

        // Game play is Part 3, so skip ahead to win to show replay and quit.
        if (state == GameState::AssignReinforcement) {
            state = GameState::Win;
            std::cout << "(Skipping game play, jumping to win)\n";
        }
    }

    std::cout << '\n' << processor << '\n';
    for (Command* command : processor.getCommands()) {
        command->Detach(&logger);
    }
    processor.Detach(&logger);
}

// Shows that copies get their own commands.
void demoCopy(const CommandProcessor& processor) {
    CommandProcessor copy(processor);
    CommandProcessor assigned;
    assigned = processor;
    std::cout << "\nCopy has " << copy.getCommands().size()
              << " commands, assigned copy has " << assigned.getCommands().size()
              << ".\n";
}

// Shows how to run the program.
void printUsage(const char* program) {
    std::cerr << "Usage: " << program << " -console\n"
              << "       " << program << " -file <filename>\n";
}

// -console reads from the keyboard, -file <filename> from a saved file.
int main(int argc, char* argv[]) {
    const std::string mode = argc >= 2 ? argv[1] : "";

    if (argc == 2 && mode == "-console") {
        std::cout << "Reading commands from the console.\n"
                  << "Commands: loadmap <mapfile>, validatemap, addplayer <playername>,\n"
                  << "          gamestart, replay, quit\n\n";
        CommandProcessor processor;
        runCommands(processor);
        demoCopy(processor);
        return 0;
    }

    if (argc == 3 && mode == "-file") {
        FileCommandProcessorAdapter adapter(argv[2]);
        if (!adapter.isOpen()) {
            std::cerr << "Couldn't open " << argv[2] << '\n';
            return 1;
        }
        std::cout << "Reading commands from " << argv[2] << "\n\n";
        runCommands(adapter);
        demoCopy(adapter);
        FileCommandProcessorAdapter adapterCopy(adapter);
        std::cout << adapterCopy << '\n';
        return 0;
    }

    printUsage(argv[0]);
    return 1;
}
