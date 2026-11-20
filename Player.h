#pragma once
#include <vector>
#include <iostream>
#include <string>
#include "Map.h"    // For Territory
#include "Cards.h"  // For Hand
#include "Orders.h" // For OrdersList

class PlayerStrategy;

// A player of the game. Owns its hand, orders list, reinforcement pool and
// per-turn bookkeeping, and its PlayerStrategy, which makes all the decisions of the issuing phase (Strategy pattern).
// Territory objects are owned by the Map, not the Player.
class Player
{
private:
    std::string *name;                     // Player's name (from the addplayer command)
    std::vector<Territory *> *territories; // Pointer to a collection of Territory pointers
    Hand *hand;                            // Pointer to the player's hand of cards
    OrdersList *orders;                    // Pointer to the player's list of orders
    int *reinforcementsPool;               // Armies available to deploy
    int *pendingDeployments;               // Armies already committed to deploy orders this turn
    int *issueStage;                       // Which kind of order issueOrder() tries next this turn
    bool *conqueredThisTurn;               // True if the player conquered a territory this turn
    std::vector<Player *> *negotiatedWith; // Players this player cannot attack this turn (not owned)
    PlayerStrategy *strategy;              // How this player decides (owned, never null)

    // Helpers used by the copy constructor, assignment operator and destructor
    void init(const std::string &playerName);
    void copyFrom(const Player &other);
    void clear();

public:
    // Constructor and Destructor
    Player();
    explicit Player(const std::string &name); // explicit to avoid accidental implicit conversions from string to Player
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

    /* Issue the player's next order of the turn. Returns true if an order was
       issued, or false when the player has nothing more to issue this turn.
       The deck is where played cards are returned (may be nullptr: no cards played). */
    bool issueOrder(Deck *deck);

    // Called by the game engine at the start of every issuing phase
    void resetTurnState();

    // Getters
    std::string getName() const;
    std::vector<Territory *> *getTerritories() const;
    Hand *getHand() const;
    OrdersList *getOrdersList() const;
    int *getReinforcementsPool() const;
    bool hasConqueredThisTurn() const;
    int *getPendingDeployments() const; // Armies already committed to deploy orders this turn
    int *getIssueStage() const;         // Progress counter that strategies may use during the issuing phase
    PlayerStrategy *getStrategy() const;

    // Replaces the player's strategy at runtime. The player takes ownership of 'newStrategy'.
    void setStrategy(PlayerStrategy *newStrategy);

    // Armies still free to be committed to a new deploy order
    int getAvailableArmies() const;

    // Territory ownership
    void addTerritory(Territory *territory);
    void removeTerritory(Territory *territory);

    // Used by order execution
    void setConqueredThisTurn(bool conquered);
    void addNegotiatedPlayer(Player *other);
    bool hasNegotiatedWith(const Player *other) const;
};