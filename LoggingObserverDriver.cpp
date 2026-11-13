#include "LoggingObserver.h"
#include "CommandProcessing.h"
#include "GameEngine.h"
#include "Orders.h"
#include "Map.h"
#include "Player.h"
// Demonstrate console commands, effects, order addition/execution and state changes.
int main() {
    LogObserver logger; // One logger shared by all attached subjects.
    GameEngine engine;
    CommandProcessor processor;
    engine.Attach(&logger);
    processor.Attach(&logger);
    std::cout << "Enter an engine command (for example loadmap): ";
    Command* command = processor.getCommand();
    if (!command) return 1;
    command->Attach(&logger);
    // Ask the engine to process the text and report success or failure.
    bool accepted = engine.transition(command->getCommand());
    if (accepted) {
        command->saveEffect("Accepted: " + engine.getStateName());
    } else {
        command->saveEffect("Rejected in " + engine.getStateName());
    }

    // Add and execute one order through a real player's list.
    Player player;
    Territory territory("Home");
    territory.setOwner(&player);
    int pool = 5;
    OrdersList* playerOrders = player.getOrdersList();
    playerOrders->Attach(&logger);
    Order* deploy = new Deploy(&player, &territory, 3, &pool);
    deploy->Attach(&logger);
    playerOrders->addOrder(deploy);
    deploy->execute();

    std::cout << "Saved command, effect, order and execution events to gamelog.txt.\n";
    return 0;
}
