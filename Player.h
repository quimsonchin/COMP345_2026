#pragma once;
#include <vector>;
#include <iostream>;
#include "Map.h"    // For Territory
#include "Cards.h"  // For Hand
#include "Orders.h" // For OrderList

class Player
{
private:
    std::vector<Territory *> *territories; // Pointer to a collection of Territory pointers
    Hand *hand;                            // Pointer to the player's hand of cards
    OrderList *orders;                     // Pointer to the player's list of OrdersList

public:
    // Constructor and Destructor
    Player();
    Player(std::vector<Territory *> territories, Hand *hand, OrderList *orders);
    Player(const Player &other); // Copy constructor
    ~Player();                   // Destructor

    // Assignmnet operator
    Player &operator=(const Player &other);

    // Stream insertion operator
    friend std::ostream &operator<<(std::ostream &os, const Player &player);

    // Required methods
    std::vector<Territory *> *toDefend();
    std::vector<Territory *> *toAttack();
    void issueOrder();

    // Getters
    std::vector<Territory *> *getTerritories() const;
    Hand *getHand() const;
    OrderList *getOrdersList() const;
}