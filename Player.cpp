#include "Player.h"
#include <algorithm> // For std::find

/* Default constructor:
   Every pointer gets initialized, so none is a dangling pointer or garbage. */
Player::Player()
{
    territories = new std::vector<Territory *>();
    hand = new Hand();
    orders = new OrdersList();
    reinforcementsPool = new int(0); // Initialize reinforcements pool to 0
}

/* Parameterized constructor:
   Player makes its own copies of the arguments, so the caller keeps ownership of whatever is passed in.
   This avoids a double delete when both caller and player try to free the same memory. */
Player::Player(std::vector<Territory *> *territories, Hand *hand, OrdersList *orders)
{
    // Check if the passed in pointers are not null, if they are not, make a copy of the objects they point to. If they are null, initialize to empty objects.
    this->territories = territories ? new std::vector<Territory *>(*territories) : new std::vector<Territory *>();
    this->hand = hand ? new Hand(*hand) : new Hand();
    this->orders = orders ? new OrdersList(*orders) : new OrdersList();
    reinforcementsPool = new int(0); // Initialize reinforcements pool to 0
}

/* Copy constructor:
   Player makes its own copies of the other player's data, so the caller keeps ownership of whatever is passed in.
   This avoids a double delete when both caller and player try to free the same memory.     */
Player::Player(const Player &other)
{
    // Shallow copy of Territory* values, but it's okay because Map owns the Territory objects.
    territories = new std::vector<Territory *>(*other.territories);
    // The others are deep copies, so Player owns them and is responsible for freeing them in the destructor.
    hand = new Hand(*other.hand);
    orders = new OrdersList(*other.orders);
    reinforcementsPool = new int(*other.reinforcementsPool);
}

/* Destructor */
Player::~Player()
{
    delete territories; // Delete the vector, NOT the Territory objects
    delete hand;
    delete orders; // OrdersList's destructor deletes each Order
    delete reinforcementsPool;
}

/* Assignment operator */
Player &Player::operator=(const Player &other)
{
    if (this != &other)
    { // Self-assignment check
        // Delete old members to avoid memory leaks
        delete territories;
        delete hand;
        delete orders;
        delete reinforcementsPool;

        // Allocate new copies from other
        territories = new std::vector<Territory *>(*other.territories);
        hand = new Hand(*other.hand);
        orders = new OrdersList(*other.orders);
        reinforcementsPool = new int(*other.reinforcementsPool);
    }
    return *this;
}

// Stream insertion operator
std::ostream &operator<<(std::ostream &os, const Player &player)
{
    // Print the number of territories, the hand, and the orders list. The reinforcements pool is also printed.
    os << "Player with " << player.territories->size() << " territories, "
       << "hand: " << *player.hand << ", "
       << "orders: " << *player.orders << ", "
       << "reinforcements pool: " << *player.reinforcementsPool;
    return os;
}

/* Required methods */
std::vector<Territory *> *Player::toDefend()
{
    return new std::vector<Territory *>(*territories); // For now, just return all territories. This can be improved later.
}

std::vector<Territory *> *Player::toAttack()
{
    std::vector<Territory *> *attackable = new std::vector<Territory *>(); // Create a new, empty list of attackable territories.
    for (Territory *mine : *territories)
    { // For each territory owned by the player
        for (Territory *neighbor : mine->getAdjacent())
        { // For each adjacent territory
            // If the neighbor is not owned by the player and is not already in the attackable list, add it to the list.
            if (neighbor->getOwner() != this && std::find(attackable->begin(), attackable->end(), neighbor) == attackable->end())
            {
                attackable->push_back(neighbor);
            }
        }
    }
    return attackable; // Return the list of territories that can be attacked. The caller owns this vector and is responsible for deleting it.
}

void Player::issueOrder(const std::string &type)
{
    // Get the territories to defend and attack. The caller owns these vectors and is responsible for deleting them.
    std::vector<Territory *> *mine = toDefend();
    std::vector<Territory *> *enemy = toAttack();

    if (mine->empty())
    {
        std::cout << "Cannot issue order: player owns no territories." << std::endl;
    }
    else if (type == "deploy")
    {
        orders->addOrder(new Deploy(this, (*mine)[0], 1, reinforcementsPool));
    }
    else if (type == "advance" && !enemy->empty())
    {
        orders->addOrder(new Advance(this, (*mine)[0], (*enemy)[0], 1));
    }
    else if (type == "bomb" && !enemy->empty())
    {
        orders->addOrder(new Bomb(this, (*enemy)[0], (*mine)[0]));
    }
    else if (type == "blockade")
    {
        orders->addOrder(new Blockade(this, (*mine)[0]));
    }
    else if (type == "airlift" && !enemy->empty())
    {
        orders->addOrder(new Airlift(this, (*mine)[0], (*enemy)[0], 1));
    }
    else if (type == "negotiate" && !enemy->empty())
    {
        orders->addOrder(new Negotiate(this, (*enemy)[0]->getOwner()));
    }
    else
    {
        std::cout << "Cannot issue order: invalid order type or no valid targets." << std::endl;
    }

    // Clean up the temporary vectors. The caller owns these vectors and is responsible for deleting them.
    delete mine;
    delete enemy;
}

// Getters
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
