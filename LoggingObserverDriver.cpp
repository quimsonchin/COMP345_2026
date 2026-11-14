#include "LoggingObserver.h"
#include "CommandProcessing.h"
#include "GameEngine.h"
#include "Orders.h"
#include "Map.h"
#include "Player.h"
// Part 5 driver.
int main() {
    LogObserver logger; // Shared logger.
    GameEngine engine;
    CommandProcessor processor;
    engine.Attach(&logger);
    processor.Attach(&logger);
    std::cout << "Enter an engine command (for example loadmap world.map): ";
    Command* command = processor.getCommand();
    if (!command) return 1;
    // Process the command.
    bool accepted = processor.validate(command, engine.getState());
    if (accepted) {
        accepted = engine.transition(command->getName());
    }
    if (accepted) {
        command->saveEffect("Accepted: " + engine.getStateName());
    } else if (command->getEffect().empty()) {
        command->saveEffect("Rejected in " + engine.getStateName());
    }

    // Add and execute a deploy order.
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
