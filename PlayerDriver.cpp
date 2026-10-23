#include "Player.h"
#include "Map.h"
#include <iostream>

int main()
{
    // Map owns the territories. Layout: Winterfell - King's Landing - Dorne, King's Landing - Pentos
    Map *gameMap = new Map();
    Territory *winterfell = gameMap->addTerritory("Winterfell");
    Territory *kingsLanding = gameMap->addTerritory("King's Landing");
    Territory *dorne = gameMap->addTerritory("Dorne");
    Territory *pentos = gameMap->addTerritory("Pentos");
    gameMap->addEdge(winterfell, kingsLanding);
    gameMap->addEdge(kingsLanding, dorne);
    gameMap->addEdge(kingsLanding, pentos);

    Player *p1 = new Player();
    Player *p2 = new Player();
    // Add King's Landing and Winterfell to p1's territories, and Dorne and Pentos to p2's territories
    p1->addTerritory(kingsLanding);
    p1->addTerritory(winterfell);
    p2->addTerritory(dorne);
    p2->addTerritory(pentos);

    // Orders are only valid with armies available
    kingsLanding->setArmies(10);
    *p1->getReinforcementsPool() = 10; // Give p1 some reinforcements

    // Demo 1: print the player
    std::cout << "=== Player 1 ===\n"
              << *p1 << std::endl;

    // Demo 2: toDefend and toAttack
    std::vector<Territory *> *p1Defend = p1->toDefend();
    std::vector<Territory *> *p1Attack = p1->toAttack();
    std::cout << "Player 1 territories to defend:\n";
    for (Territory *t : *p1Defend)
    {
        std::cout << " " << t->getName() << "\n";
    }
    std::cout << "Player 1 territories to attack:\n";
    for (Territory *t : *p1Attack)
    {
        std::cout << " " << t->getName() << "\n";
    }
    delete p1Defend;
    delete p1Attack;

    // Demo 3: issueOrder
    p1->issueOrder("deploy");
    p1->issueOrder("advance");
    p1->issueOrder("bomb");
    p1->issueOrder("blockade");
    p1->issueOrder("airlift");
    p1->issueOrder("negotiate");

    // Execute the valid orders to see the effects
    for (int i = 0; i < p1->getOrdersList()->size(); i++)
    {
        p1->getOrdersList()->getOrder(i)->execute();
    }

    p1->issueOrder("invalid"); // Invalid order type: should print an error message
    Player *p3 = new Player(); // Player with no territories
    p3->issueOrder("deploy");  // Player with no territories: should print an error message
    std::cout << "=== Player 1 Orders List ===\n"
              << *p1->getOrdersList() << std::endl; // Print the orders list for p1

    // Demo 4: copy constructor and assignment operator
    Player *copyPlayer = new Player(*p1);                                                                                 // Copy constructor
    copyPlayer->issueOrder("deploy");                                                                                     // Issue an order for the copy
    std::cout << "p1 orders: " << p1->getOrdersList()->size() << ", copy orders: " << copyPlayer->getOrdersList()->size() // Print the number of orders for p1 and the copy
              << std::endl;
    std::cout << "\n=== Copy of Player 1 ===\n"
              << *copyPlayer << std::endl; // Print the copy

    Player *assignedPlayer = new Player(); // Assignment operator
    *assignedPlayer = *p1;                 // Assign p1 to assignedPlayer
    *assignedPlayer = *assignedPlayer;     // Self-assignment test
    std::cout << "=== Assigned Player ===\n"
              << *assignedPlayer << std::endl; // Print the assigned player

    // Clean up
    delete p1;
    delete p2;
    delete p3;
    delete assignedPlayer;
    delete copyPlayer;
    delete gameMap; // Map destructor deletes all territories

    return 0;
}