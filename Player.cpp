#include "Player.h"
#include "PlayerStrategies.h"
#include <algorithm> // For std::find, std::stable_sort
#include <utility>   // For std::swap

/* Shared setup: allocates every member as an empty/default object. */
void Player::init(const std::string &playerName)
{
    name = new std::string(playerName);
    territories = new std::vector<Territory *>();
    hand = new Hand();
    orders = new OrdersList();
    reinforcementsPool = new int(0);
    pendingDeployments = new int(0);
    issueStage = new int(0);
    conqueredThisTurn = new bool(false);
    negotiatedWith = new std::vector<Player *>();
    strategy = new DefaultPlayerStrategy(this); // Keeps the A2 behavior until setStrategy() is called
}

/* Deep copy of every member of 'other' into this (uninitialized) player. */
void Player::copyFrom(const Player &other)
{
    name = new std::string(*other.name);
    // Shallow copy of Territory* and Player* values, but it's okay because they are not owned by Player.
    territories = new std::vector<Territory *>(*other.territories);
    negotiatedWith = new std::vector<Player *>(*other.negotiatedWith);
    // The others are deep copies, so Player owns them and is responsible for freeing them.
    hand = new Hand(*other.hand);
    orders = new OrdersList(*other.orders);
    reinforcementsPool = new int(*other.reinforcementsPool);
    pendingDeployments = new int(*other.pendingDeployments);
    issueStage = new int(*other.issueStage);
    conqueredThisTurn = new bool(*other.conqueredThisTurn);
    strategy = other.strategy->clone(); // Deep copy through the virtual copy constructor
    strategy->setPlayer(this);          // The copy's strategy must work for the copy, not for 'other'
}

/* Frees everything the player owns. */
void Player::clear()
{
    delete name;
    delete territories; // Delete the vector, NOT the Territory objects
    delete hand;
    delete orders; // OrdersList's destructor deletes each Order
    delete reinforcementsPool;
    delete pendingDeployments;
    delete issueStage;
    delete conqueredThisTurn;
    delete negotiatedWith; // Delete the vector, NOT the Player objects
    delete strategy;       // The player owns its strategy
}

/* Default constructor */
Player::Player()
{
    init("Player");
}

/* Constructor taking the player's name */
Player::Player(const std::string &playerName)
{
    init(playerName);
}

/* Parameterized constructor:
   Player makes its own copies of the arguments, so the caller keeps ownership of whatever is passed in.
   This avoids a double delete when both caller and player try to free the same memory. */
Player::Player(std::vector<Territory *> *territories, Hand *hand, OrdersList *orders)
{
    init("Player");
    // If an argument is null, the member stays an empty object.
    if (territories)
    {
        *this->territories = *territories;
    }
    if (hand)
    {
        *this->hand = *hand;
    }
    if (orders)
    {
        *this->orders = *orders;
    }
}

/* Copy constructor */
Player::Player(const Player &other)
{
    copyFrom(other);
}

/* Destructor */
Player::~Player()
{
    clear();
}

/* Assignment operator (copy-and-swap: safe on self-assignment and on exceptions) */
Player &Player::operator=(const Player &other)
{
    if (this != &other)
    {
        Player copy(other);
        std::swap(name, copy.name);
        std::swap(territories, copy.territories);
        std::swap(hand, copy.hand);
        std::swap(orders, copy.orders);
        std::swap(reinforcementsPool, copy.reinforcementsPool);
        std::swap(pendingDeployments, copy.pendingDeployments);
        std::swap(issueStage, copy.issueStage);
        std::swap(conqueredThisTurn, copy.conqueredThisTurn);
        std::swap(negotiatedWith, copy.negotiatedWith);
        std::swap(strategy, copy.strategy);
        // Each strategy still points at the player it was created for, so point them at their new owners.
        strategy->setPlayer(this);
        copy.strategy->setPlayer(&copy);
        // 'copy' now holds the old members and frees them when it goes out of scope.
    }
    return *this;
}

// Stream insertion operator
std::ostream &operator<<(std::ostream &os, const Player &player)
{
    os << "Player " << *player.name << " with " << player.territories->size() << " territories, "
       << "hand: " << *player.hand << ", "
       << "orders: " << *player.orders << ", "
       << "reinforcements pool: " << *player.reinforcementsPool << ", "
       << "strategy: " << player.strategy->getName();
    return os;
}

/* Territories the player should protect. Delegates to the strategy; the caller owns the returned vector. */
std::vector<Territory *> *Player::toDefend()
{
    return strategy->toDefend();
}

/* Territories the player could attack. Delegates to the strategy; the caller owns the returned vector. */
std::vector<Territory *> *Player::toAttack()
{
    return strategy->toAttack();
}

/* Issue one order. Delegates to the strategy: returns true if an order was issued, or false when the
   player has nothing more to issue this turn. */
bool Player::issueOrder(Deck *deck)
{
    return strategy->issueOrder(deck);
}

/* Forget everything that only lasts for one turn. */
void Player::resetTurnState()
{
    *pendingDeployments = 0;
    *issueStage = 0;
    *conqueredThisTurn = false;
    negotiatedWith->clear();
}

// Getters
std::string Player::getName() const
{
    return *name;
}
std::vector<Territory *> *Player::getTerritories() const
{
    return territories; // Return player's territories. The caller does not own the vector, so DO NOT DELETE IT
}
Hand *Player::getHand() const
{
    return hand; // Return player's hand. The caller does not own the hand, so DO NOT DELETE IT
}
OrdersList *Player::getOrdersList() const
{
    return orders; // Return player's orders list. The caller does not own the orders list, so DO NOT DELETE IT
}
int *Player::getReinforcementsPool() const
{
    return reinforcementsPool; // Return player's reinforcements pool. The caller does not own the pool, so DO NOT DELETE IT
}
bool Player::hasConqueredThisTurn() const
{
    return *conqueredThisTurn;
}
int *Player::getPendingDeployments() const
{
    return pendingDeployments;
}
int Player::getAvailableArmies() const
{
    return std::max(0, *reinforcementsPool - *pendingDeployments);
}
int *Player::getIssueStage() const
{
    return issueStage;
}
PlayerStrategy *Player::getStrategy() const
{
    return strategy;
}

/* Replaces the strategy at runtime. The old strategy is deleted and the player owns the new one. */
void Player::setStrategy(PlayerStrategy *newStrategy)
{
    if (newStrategy == nullptr || newStrategy == strategy)
    {
        return; // A player always keeps a strategy
    }
    delete strategy;
    strategy = newStrategy;
    strategy->setPlayer(this);
}

/* Add a territory to the player's list of territories */
void Player::addTerritory(Territory *territory)
{
    if (territory == nullptr)
    {
        return; // Do not add null territories
    }
    if (std::find(territories->begin(), territories->end(), territory) == territories->end())
    {
        territories->push_back(territory); // Add the territory if it's not already in the list
    }
    territory->setOwner(this); // Set the owner of the territory to this player
}

/* Remove a territory from the player's list. The caller sets the territory's new owner. */
void Player::removeTerritory(Territory *territory)
{
    territories->erase(std::remove(territories->begin(), territories->end(), territory), territories->end());
}

/* Record whether the player conquered a territory this turn (for the end-of-turn card). */
void Player::setConqueredThisTurn(bool conquered)
{
    *conqueredThisTurn = conquered;
}

/* Record that this player cannot attack 'other' (and vice versa is set by the caller) this turn. */
void Player::addNegotiatedPlayer(Player *other)
{
    if (other != nullptr && !hasNegotiatedWith(other))
    {
        negotiatedWith->push_back(other);
    }
}

bool Player::hasNegotiatedWith(const Player *other) const
{
    if (other == nullptr)
    {
        return false;
    }
    return std::find(negotiatedWith->begin(), negotiatedWith->end(), other) != negotiatedWith->end();
}