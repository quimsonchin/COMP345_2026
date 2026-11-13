#include "Player.h"
#include "Map.h"
#include <iostream>

// Prints the territories of a list, one per line, with owner armies.
void printTerritories(const std::string &title, const std::vector<Territory *> *list)
{
    std::cout << title << "\n";
    for (const Territory *t : *list)
    {
        std::cout << "  " << t->getName() << " (" << t->getArmies() << " armies)\n";
    }
}

// Calls issueOrder() until the player is done and prints every order as it is issued.
// Returns the number of orders issued.
int issueAllOrders(Player *player, Deck *deck)
{
    int issued = 0;
    int before = player->getOrdersList()->size();
    while (player->issueOrder(deck))
    {
        int now = player->getOrdersList()->size();
        if (now > before)
        {
            std::cout << "  issued: " << *player->getOrdersList()->getOrder(now - 1)
                      << "   [armies left to deploy: " << player->getAvailableArmies() << "]\n";
            issued++;
        }
        else
        {
            std::cout << "  played a card that created no order [armies left to deploy: "
                      << player->getAvailableArmies() << "]\n";
        }
        before = now;
    }
    std::cout << "  -> player has nothing more to issue this turn\n";
    return issued;
}

int main()
{
    // Map owns the territories. Layout:
    // Winterfell - King's Landing - Dorne, King's Landing - Pentos - Braavos
    Map *gameMap = new Map();
    Territory *winterfell = gameMap->addTerritory("Winterfell");
    Territory *kingsLanding = gameMap->addTerritory("King's Landing");
    Territory *dorne = gameMap->addTerritory("Dorne");
    Territory *pentos = gameMap->addTerritory("Pentos");
    Territory *braavos = gameMap->addTerritory("Braavos");
    gameMap->addEdge(winterfell, kingsLanding);
    gameMap->addEdge(kingsLanding, dorne);
    gameMap->addEdge(kingsLanding, pentos);
    gameMap->addEdge(pentos, braavos);

    Player *p1 = new Player("Alice");
    Player *p2 = new Player("Bob");
    p1->addTerritory(winterfell);
    p1->addTerritory(kingsLanding);
    p2->addTerritory(dorne);
    p2->addTerritory(pentos);
    p2->addTerritory(braavos);
    winterfell->setArmies(4);
    kingsLanding->setArmies(10);
    dorne->setArmies(3);
    pentos->setArmies(6);
    braavos->setArmies(2);

    Deck *deck = new Deck();

    // Demo 1: print the player
    std::cout << "=== Demo 1: print the player ===\n"
              << *p1 << "\n";

    // Demo 2: toDefend and toAttack
    std::cout << "\n=== Demo 2: toDefend() and toAttack() for Alice ===\n";
    std::vector<Territory *> *p1Defend = p1->toDefend();
    std::vector<Territory *> *p1Attack = p1->toAttack();
    printTerritories("Alice defends (most threatened first):", p1Defend);
    printTerritories("Alice attacks (weakest first):", p1Attack);
    delete p1Defend;
    delete p1Attack;

    // Demo 3: with armies in the pool, only deploy orders are issued
    std::cout << "\n=== Demo 3: deploy orders only while armies remain ===\n";
    *p1->getReinforcementsPool() = 10;
    p1->resetTurnState();
    std::cout << "Alice has " << *p1->getReinforcementsPool() << " armies in her pool\n";
    int deployCount = 0;
    while (p1->getAvailableArmies() > 0 && p1->issueOrder(deck))
    {
        deployCount++;
        std::cout << "  issued: " << *p1->getOrdersList()->getOrder(p1->getOrdersList()->size() - 1)
                  << "   [armies left to deploy: " << p1->getAvailableArmies() << "]\n";
    }
    std::cout << "Alice issued " << deployCount << " deploy orders and has no armies left to deploy\n";

    // Demo 4: advance orders to defend and to attack
    std::cout << "\n=== Demo 4: advance orders (defend, then attack) ===\n";
    int advances = 0;
    int ordersBefore = p1->getOrdersList()->size();
    while (advances < 2 && p1->issueOrder(nullptr)) // nullptr deck: no cards are played yet
    {
        advances++;
        std::cout << "  issued: " << *p1->getOrdersList()->getOrder(p1->getOrdersList()->size() - 1) << "\n";
    }
    std::cout << "Advance orders issued: " << p1->getOrdersList()->size() - ordersBefore << "\n";

    // Demo 5: playing cards issues orders. Draw every card so all five types are shown.
    std::cout << "\n=== Demo 5: playing cards to issue orders ===\n";
    while (deck->draw(*p1->getHand()))
    {
    }
    std::cout << *p1->getHand() << "\n";
    issueAllOrders(p1, deck);
    std::cout << "Hand after playing: " << *p1->getHand() << "\n"
              << *deck << "\n";

    std::cout << "\n=== Alice's final orders list ===\n"
              << *p1->getOrdersList() << std::endl;

    // Demo 6: copy constructor and assignment operator
    std::cout << "=== Demo 6: copy constructor and assignment operator ===\n";
    Player *copyPlayer = new Player(*p1);
    int originalSize = p1->getOrdersList()->size();
    copyPlayer->getOrdersList()->remove(0);
    std::cout << "Alice orders: " << p1->getOrdersList()->size() << " (was " << originalSize
              << "), copy orders: " << copyPlayer->getOrdersList()->size() << " (deep copy)\n";
    Player *assignedPlayer = new Player("Temp");
    *assignedPlayer = *p1;
    *assignedPlayer = *assignedPlayer; // Self-assignment test
    std::cout << "Assigned player: " << assignedPlayer->getName() << " with "
              << assignedPlayer->getOrdersList()->size() << " orders\n";

    // Demo 7: a player without territories cannot issue anything
    std::cout << "\n=== Demo 7: player with no territories ===\n";
    Player *p3 = new Player("Carol");
    *p3->getReinforcementsPool() = 5;
    std::cout << "Carol issued an order: " << (p3->issueOrder(deck) ? "yes" : "no") << "\n";

    // Clean up
    delete p1;
    delete p2;
    delete p3;
    delete assignedPlayer;
    delete copyPlayer;
    delete deck;
    delete gameMap; // Map destructor deletes all territories

    return 0;
}