// Optional interactive demo. Run from repository root so maps/ is found.
#include "GameEngine.h"
#include "LoggingObserver.h"
#include <exception>
#include <iostream>

int main()
{
    try
    {
        GameEngine engine;
        LogObserver logger;
        engine.Attach(&logger);
        engine.startupPhase();
        if (engine.getState() != GameState::AssignReinforcement)
        {
            std::cout << "Startup ended before gamestart.\n";
            return 0;
        }
        engine.mainGameLoop();
        std::cout << "Game completed: " << engine.getStateName() << '\n';
        engine.Detach(&logger);
    }
    catch (const std::exception &error)
    {
        std::cerr << "Game error: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
