#include "Player.h"
#include <algorithm> // For std::find, std::stable_sort
#include <utility>   // For std::swap

namespace
{
    // Number of armies a Reinforcement card adds to the reinforcement pool.
    const int REINFORCEMENT_CARD_ARMIES = 5;

    // True if 'from' has 'to' in its adjacency list.
    bool isAdjacent(const Territory *from, const Territory *to)
    {
        const std::vector<Territory *> &neighbors = from->getAdjacent();
        return std::find(neighbors.begin(), neighbors.end(), to) != neighbors.end();
    }

    // Number of neighbors of 't' that belong to somebody other than 'player'.
    int enemyNeighborCount(const Territory *t, const Player *player)
    {
        int count = 0;
        for (const Territory *neighbor : t->getAdjacent())
        {
            if (neighbor->getOwner() != player)
            {
                count++;
            }
        }
        return count;
    }

    // Among the player's territories, find the one adjacent to 'target' holding
    // the most armies (at least minArmies). Returns nullptr if there is none.
    Territory *bestSourceFor(const Player *player, const std::vector<Territory *> &owned,
                             const Territory *target, int minArmies)
    {
        Territory *best = nullptr;
        for (Territory *mine : owned)
        {
            if (mine->getOwner() == player && mine->getArmies() >= minArmies && isAdjacent(mine, target))
            {
                if (best == nullptr || mine->getArmies() > best->getArmies())
                {
                    best = mine;
                }
            }
        }
        return best;
    }
} // namespace

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
       << "reinforcements pool: " << *player.reinforcementsPool;
    return os;
}

/* Territories the player should protect, most threatened first: the ones with the most
   enemy neighbors, then the ones with the fewest armies. The caller owns the returned vector. */
std::vector<Territory *> *Player::toDefend()
{
    std::vector<Territory *> *defend = new std::vector<Territory *>();
    for (Territory *mine : *territories)
    {
        if (mine->getOwner() == this) // Ignore territories lost since they were added
        {
            defend->push_back(mine);
        }
    }
    std::stable_sort(defend->begin(), defend->end(), [this](const Territory *a, const Territory *b)
                     {
                         int threatA = enemyNeighborCount(a, this);
                         int threatB = enemyNeighborCount(b, this);
                         if (threatA != threatB)
                         {
                             return threatA > threatB;
                         }
                         return a->getArmies() < b->getArmies(); });
    return defend;
}

/* Neighboring territories the player could attack, weakest first. Territories of a player
   that negotiated with this one are left out. The caller owns the returned vector. */
std::vector<Territory *> *Player::toAttack()
{
    std::vector<Territory *> *attackable = new std::vector<Territory *>();
    for (Territory *mine : *territories)
    {
        if (mine->getOwner() != this)
        {
            continue; // Ignore territories lost since they were added
        }
        for (Territory *neighbor : mine->getAdjacent())
        {
            if (neighbor->getOwner() != this && !hasNegotiatedWith(neighbor->getOwner()) &&
                std::find(attackable->begin(), attackable->end(), neighbor) == attackable->end())
            {
                attackable->push_back(neighbor);
            }
        }
    }
    std::stable_sort(attackable->begin(), attackable->end(), [](const Territory *a, const Territory *b)
                     { return a->getArmies() < b->getArmies(); });
    return attackable;
}

/* Deploy part of the armies that are still free, spread over the toDefend() list.
   Armies are only committed here; the pool is debited when the order executes. */
bool Player::issueDeploy()
{
    bool issued = false;
    int available = getAvailableArmies();
    std::vector<Territory *> *mine = toDefend();
    if (available > 0 && !mine->empty())
    {
        int armies = (available + 1) / 2; // Half of what is left (rounded up), so deploys are spread out
        Territory *target = (*mine)[orders->size() % mine->size()];
        orders->addOrder(new Deploy(this, target, armies, reinforcementsPool));
        *pendingDeployments += armies;
        issued = true;
    }
    delete mine;
    return issued;
}

/* Move half the armies of the strongest neighboring own territory to the
   highest-priority territory of the toDefend() list. */
bool Player::issueDefendAdvance()
{
    bool issued = false;
    std::vector<Territory *> *mine = toDefend();
    if (!mine->empty())
    {
        Territory *target = (*mine)[0];
        Territory *source = bestSourceFor(this, *mine, target, 2);
        if (source != nullptr)
        {
            orders->addOrder(new Advance(this, source, target, source->getArmies() / 2));
            issued = true;
        }
    }
    delete mine;
    return issued;
}

/* Attack the weakest reachable territory of the toAttack() list with all but one
   army of the strongest neighboring own territory. */
bool Player::issueAttackAdvance()
{
    bool issued = false;
    std::vector<Territory *> *mine = toDefend();
    std::vector<Territory *> *enemy = toAttack();
    for (std::size_t i = 0; i < enemy->size() && !issued; i++)
    {
        Territory *target = (*enemy)[i];
        Territory *source = bestSourceFor(this, *mine, target, 2);
        if (source != nullptr)
        {
            orders->addOrder(new Advance(this, source, target, source->getArmies() - 1));
            issued = true;
        }
    }
    delete mine;
    delete enemy;
    return issued;
}

/* Play the first card in the hand that can be turned into a sensible order.
   The played card goes back to the deck. */
bool Player::issueCardOrder(Deck *deck)
{
    if (deck == nullptr)
    {
        return false;
    }
    bool issued = false;
    std::vector<Territory *> *mine = toDefend();
    std::vector<Territory *> *enemy = toAttack();

    for (int i = 0; i < hand->size() && !issued; i++)
    {
        Card *card = hand->getCard(i);
        Order *order = nullptr;
        bool isReinforcement = false;

        switch (card->getType())
        {
        case CardType::Bomb:
            for (Territory *target : *enemy)
            {
                // Bomb needs an enemy-owned target next to one of our territories
                Territory *witness = bestSourceFor(this, *mine, target, 0);
                if (target->getOwner() != nullptr && witness != nullptr)
                {
                    order = new Bomb(this, target, witness);
                    break;
                }
            }
            break;
        case CardType::Reinforcement:
            isReinforcement = true;
            break;
        case CardType::Blockade:
            if (mine->size() >= 2) // Never blockade the only territory we have
            {
                order = new Blockade(this, mine->back()); // Least threatened territory
            }
            break;
        case CardType::Airlift:
        {
            Territory *source = nullptr;
            for (Territory *t : *mine)
            {
                if (t->getArmies() >= 2 && (source == nullptr || t->getArmies() > source->getArmies()))
                {
                    source = t;
                }
            }
            if (source != nullptr && !mine->empty())
            {
                Territory *target = (*mine)[0] != source ? (*mine)[0] : (mine->size() > 1 ? (*mine)[1] : nullptr);
                if (target != nullptr)
                {
                    order = new Airlift(this, source, target, source->getArmies() / 2);
                }
            }
            break;
        }
        case CardType::Diplomacy:
            for (Territory *target : *enemy)
            {
                if (target->getOwner() != nullptr)
                {
                    order = new Negotiate(this, target->getOwner());
                    break;
                }
            }
            break;
        }

        if (isReinforcement || order != nullptr)
        {
            // Card::play() puts its own placeholder order in the list it is given,
            // so give it a scratch list and add the real order to the player's list.
            OrdersList *scratch = new OrdersList();
            if (card->play(*hand, *deck, *scratch))
            {
                if (isReinforcement)
                {
                    *reinforcementsPool += REINFORCEMENT_CARD_ARMIES; // Adds armies to the pool, no order is created
                }
                else
                {
                    orders->addOrder(order);
                }
                issued = true;
            }
            else
            {
                delete order; // The card was not played, so the order was never added
            }
            delete scratch; // Deletes the placeholder order
        }
    }
    delete mine;
    delete enemy;
    return issued;
}

/* Issue one order. While armies are still left to deploy, only deploy orders are issued.
   After that: one defending advance, one attacking advance, then card orders.
   Orders are not validated here; that happens when they are executed. */
bool Player::issueOrder(Deck *deck)
{
    if (getAvailableArmies() > 0)
    {
        return issueDeploy();
    }
    while (*issueStage < 3)
    {
        bool issued = false;
        switch (*issueStage)
        {
        case 0:
            issued = issueDefendAdvance();
            break;
        case 1:
            issued = issueAttackAdvance();
            break;
        case 2:
            issued = issueCardOrder(deck);
            if (issued)
            {
                return true; // Stay on the card stage until no card can be played
            }
            break;
        }
        (*issueStage)++;
        if (issued)
        {
            return true;
        }
    }
    return false; // Nothing more to issue this turn
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
int Player::getAvailableArmies() const
{
    return std::max(0, *reinforcementsPool - *pendingDeployments);
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