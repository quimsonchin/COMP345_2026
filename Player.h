#pragma once
#include <vector>
#include <iostream>
#include <string>
#include "Map.h"    // For Territory
#include "Cards.h"  // For Hand
#include "Orders.h" // For OrdersList

class Player
{
private:
    std::vector<Territory *> *territories; // Pointer to a collection of Territory pointers
    Hand *hand;                            // Pointer to the player's hand of cards
    OrdersList *orders;                    // Pointer to the player's list of orders
    int *reinforcementsPool;               // Pointer to the player's reinforcements pool

public:
    // Constructor and Destructor
    Player();
    Player(std::vector<Territory *> *territories, Hand *hand, OrdersList *orders);
    Player(const Player &other); // Copy constructor
    ~Player();                   // Destructor

    // Assignmnet operator
    Player &operator=(const Player &other);

    // Stream insertion operator
    friend std::ostream &operator<<(std::ostream &os, const Player &player);

    // Required methods
    std::vector<Territory *> *toDefend();
    std::vector<Territory *> *toAttack();
    void issueOrder(const std::string &type);

    // Getters
    std::vector<Territory *> *getTerritories() const;
    Hand *getHand() const;
    OrdersList *getOrdersList() const;
    int *getReinforcementsPool() const;

    // Other methods
    void addTerritory(Territory *territory);
};