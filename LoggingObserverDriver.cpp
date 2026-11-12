#include "LoggingObserver.h"
#include "CommandProcessing.h"
#include "GameEngine.h"
#include "Orders.h"
#include "Map.h"
#include "Player.h"
#include <type_traits>

// Verify that every required subject supports both interfaces.
static_assert(std::is_base_of<Subject, Command>::value && std::is_base_of<ILoggable, Command>::value);
static_assert(std::is_base_of<Subject, CommandProcessor>::value && std::is_base_of<ILoggable, CommandProcessor>::value);
static_assert(std::is_base_of<Subject, Order>::value && std::is_base_of<ILoggable, Order>::value);
static_assert(std::is_base_of<Subject, OrdersList>::value && std::is_base_of<ILoggable, OrdersList>::value);
static_assert(std::is_base_of<Subject, GameEngine>::value && std::is_base_of<ILoggable, GameEngine>::value);

// Demonstrate console commands, effects, order addition/execution and state changes.
int main() {
    LogObserver logger;
    GameEngine engine;
    CommandProcessor processor;
    engine.Attach(&logger);
    processor.Attach(&logger);
    std::cout << "Enter an engine command (for example loadmap): ";
    Command* command = processor.getCommand();
    if (!command) return 1;
    command->Attach(&logger);
    bool accepted = engine.transition(command->getCommand());
    command->saveEffect(accepted ? "Accepted: " + engine.getStateName() : "Rejected in " + engine.getStateName());

    OrdersList orders;
    orders.Attach(&logger);
    // A rejected order still has an effect that must be logged.
    Order* order = new Blockade(nullptr, nullptr);
    order->Attach(&logger);
    orders.addOrder(order);
    order->execute();

    // Demonstrate a valid order using a real player's order list.
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
