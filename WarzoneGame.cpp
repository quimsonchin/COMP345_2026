// Interactive or file-driven Warzone game. Run from repository root.
#include "GameEngine.h"
#include "CommandProcessing.h"
#include "LoggingObserver.h"
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

// -console and -file use exactly the same command/validation flow.
int main(int argc, char *argv[])
{
    std::unique_ptr<CommandProcessor> commands;
    const std::string mode = argc >= 2 ? argv[1] : "";
    if (argc == 1 || (argc == 2 && mode == "-console"))
    {
        commands = std::make_unique<CommandProcessor>();
    }
    else if (argc == 3 && mode == "-file")
    {
        auto adapter = std::make_unique<FileCommandProcessorAdapter>(argv[2]);
        if (!adapter->isOpen())
        {
            std::cerr << "Cannot open command file: " << argv[2] << '\n';
            return 1;
        }
        commands = std::move(adapter);
    }
    else
    {
        std::cerr << "Usage: " << argv[0]
                  << " -console | -file <filename>\n";
        return 1;
    }

    try
    {
        GameEngine engine;
        LogObserver logger;
        engine.Attach(&logger);
        commands->Attach(&logger);

        while (engine.getState() != GameState::End)
        {
            if (engine.getState() == GameState::Start)
            {
                engine.startupPhase(*commands, std::cout);
                if (engine.getState() != GameState::AssignReinforcement)
                {
                    std::cout << "Startup ended before gamestart.\n";
                    break;
                }
            }

            if (engine.getState() == GameState::AssignReinforcement)
            {
                engine.mainGameLoop();
                std::cout << "Game completed: " << engine.getStateName() << '\n';
            }

            // After victory, accept only replay/quit and retain command logging.
            if (engine.getState() == GameState::Win)
            {
                std::cout << "[win] Enter replay or quit: " << std::flush;
                Command *command = commands->getCommand();
                if (!command)
                {
                    std::cout << "No further commands; exiting.\n";
                    break;
                }
                if (!commands->validate(command, engine.getState()))
                {
                    std::cout << "Rejected: " << command->getEffect() << '\n';
                    continue;
                }
                const bool transitioned = engine.transition(command->getName());
                command->saveEffect(transitioned
                    ? "Accepted: " + engine.getStateName()
                    : "Rejected: state transition failed");
                std::cout << command->getEffect() << '\n';
            }
        }

        engine.Detach(&logger);
        commands->Detach(&logger);
    }
    catch (const std::exception &error)
    {
        std::cerr << "Game error: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
