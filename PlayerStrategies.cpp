#include "PlayerStrategies.h"
#include "Player.h"
#include <algorithm> // For std::find, std::stable_sort
#include <sstream>   // For std::istringstream

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

    /* Territories of 'player' sorted by how much they need defending: the ones with the most
       enemy neighbors first, then the ones with the fewest armies. The caller owns the vector. */
    std::vector<Territory *> *buildDefendList(const Player *player)
    {
        std::vector<Territory *> *defend = new std::vector<Territory *>();
        for (Territory *mine : *player->getTerritories())
        {
            if (mine->getOwner() == player) // Ignore territories lost since they were added
            {
                defend->push_back(mine);
            }
        }
        std::stable_sort(defend->begin(), defend->end(), [player](const Territory *a, const Territory *b)
                         {
                             int threatA = enemyNeighborCount(a, player);
                             int threatB = enemyNeighborCount(b, player);
                             if (threatA != threatB)
                             {
                                 return threatA > threatB;
                             }
                             return a->getArmies() < b->getArmies(); });
        return defend;
    }

    /* Neighboring territories 'player' could attack, weakest first. Territories of a player
       that negotiated with this one are left out. The caller owns the vector. */
    std::vector<Territory *> *buildAttackList(const Player *player)
    {
        std::vector<Territory *> *attackable = new std::vector<Territory *>();
        for (Territory *mine : *player->getTerritories())
        {
            if (mine->getOwner() != player)
            {
                continue; // Ignore territories lost since they were added
            }
            for (Territory *neighbor : mine->getAdjacent())
            {
                if (neighbor->getOwner() != player && !player->hasNegotiatedWith(neighbor->getOwner()) &&
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

    /* Plays 'card' for 'player' and files the result. Card::play() puts its own placeholder order
       in the list it is given, so it gets a scratch list and the real order goes to the player's list.
       'order' is the order built for the card (nullptr for none). It is handed to the player's orders
       list when the card was played and deleted otherwise. A Reinforcement card creates no order and
       adds armies to the pool instead. Returns true if the card was played. */
    bool playCard(Player *player, Deck *deck, Card *card, Order *order, bool isReinforcement)
    {
        bool played = false;
        OrdersList *scratch = new OrdersList();
        if (card->play(*player->getHand(), *deck, *scratch))
        {
            if (isReinforcement)
            {
                *player->getReinforcementsPool() += REINFORCEMENT_CARD_ARMIES;
            }
            else
            {
                player->getOrdersList()->addOrder(order);
            }
            played = true;
        }
        else
        {
            delete order; // The card was not played, so the order was never added
        }
        delete scratch; // Deletes the placeholder order
        return played;
    }

    // Name of a card type for the menus of the human player.
    std::string cardTypeName(CardType type)
    {
        switch (type)
        {
        case CardType::Bomb:
            return "Bomb";
        case CardType::Reinforcement:
            return "Reinforcement";
        case CardType::Blockade:
            return "Blockade";
        case CardType::Airlift:
            return "Airlift";
        case CardType::Diplomacy:
            return "Diplomacy";
        }
        return "Unknown";
    }

    /* Asks the user for a whole number between low and high (included) and repeats until the
       answer is valid. Returns false if the input stream is closed. */
    bool readInt(const std::string &prompt, int low, int high, int &value)
    {
        std::string line;
        while (true)
        {
            std::cout << prompt << " [" << low << "-" << high << "]: ";
            if (!std::getline(std::cin, line))
            {
                return false;
            }
            std::istringstream input(line);
            int number = 0;
            char extra = 0;
            if ((input >> number) && !(input >> extra) && number >= low && number <= high)
            {
                value = number;
                return true;
            }
            std::cout << "  Invalid input, try again.\n";
        }
    }

    // Prints a numbered list of territories with their armies.
    void printTerritoryList(const std::vector<Territory *> &list)
    {
        for (std::size_t i = 0; i < list.size(); i++)
        {
            std::cout << "  " << (i + 1) << ") " << list[i]->getName() << " (" << list[i]->getArmies() << " armies)";
            if (list[i]->getOwner() != nullptr)
            {
                std::cout << " owner: " << list[i]->getOwner()->getName();
            }
            std::cout << "\n";
        }
    }

    /* Shows 'list' and asks the user to pick one territory. Returns nullptr if the list is empty
       or the input stream is closed. */
    Territory *chooseTerritory(const std::string &prompt, const std::vector<Territory *> &list)
    {
        if (list.empty())
        {
            return nullptr;
        }
        std::cout << prompt << "\n";
        printTerritoryList(list);
        int choice = 0;
        if (!readInt("Territory number", 1, static_cast<int>(list.size()), choice))
        {
            return nullptr;
        }
        return list[choice - 1];
    }

    // Asks the user for a deploy order. Returns true if one was issued.
    bool humanDeploy(Player *player)
    {
        bool issued = false;
        std::vector<Territory *> *mine = buildDefendList(player);
        int available = player->getAvailableArmies();
        std::cout << "\n"
                  << player->getName() << ": " << available << " armies left to deploy.\n";
        Territory *target = chooseTerritory("Deploy to which territory?", *mine);
        int armies = 0;
        if (target != nullptr && readInt("Number of armies to deploy", 1, available, armies))
        {
            player->getOrdersList()->addOrder(new Deploy(player, target, armies, player->getReinforcementsPool()));
            *player->getPendingDeployments() += armies;
            issued = true;
        }
        delete mine;
        return issued;
    }

    // Asks the user for an advance order. Returns true if one was issued.
    bool humanAdvance(Player *player)
    {
        bool issued = false;
        std::vector<Territory *> *mine = buildDefendList(player);
        Territory *source = chooseTerritory("Advance from which territory?", *mine);
        if (source != nullptr && source->getArmies() < 1)
        {
            std::cout << "  That territory has no armies.\n";
        }
        else if (source != nullptr)
        {
            Territory *target = chooseTerritory("Advance to which neighbor?", source->getAdjacent());
            int armies = 0;
            if (target != nullptr && readInt("Number of armies to advance", 1, source->getArmies(), armies))
            {
                player->getOrdersList()->addOrder(new Advance(player, source, target, armies));
                issued = true;
            }
        }
        delete mine;
        return issued;
    }

    /* Asks the user which card to play and what to do with it. Returns true if the card was played.
       Orders are not validated here; that happens when they are executed. */
    bool humanCard(Player *player, Deck *deck)
    {
        Hand *hand = player->getHand();
        if (deck == nullptr || hand->size() == 0)
        {
            std::cout << "  You have no card to play.\n";
            return false;
        }
        std::cout << "Your cards:\n";
        for (int i = 0; i < hand->size(); i++)
        {
            std::cout << "  " << (i + 1) << ") " << cardTypeName(hand->getCard(i)->getType()) << "\n";
        }
        int choice = 0;
        if (!readInt("Card number (0 to go back)", 0, hand->size(), choice) || choice == 0)
        {
            return false;
        }
        Card *card = hand->getCard(choice - 1);
        Order *order = nullptr;
        bool isReinforcement = false;
        std::vector<Territory *> *mine = buildDefendList(player);
        std::vector<Territory *> *enemy = buildAttackList(player);

        switch (card->getType())
        {
        case CardType::Bomb:
        {
            Territory *target = chooseTerritory("Bomb which territory?", *enemy);
            // Bomb needs one of our territories next to the target
            Territory *witness = target != nullptr ? bestSourceFor(player, *mine, target, 0) : nullptr;
            if (target != nullptr && witness != nullptr && target->getOwner() != nullptr)
            {
                order = new Bomb(player, target, witness);
            }
            break;
        }
        case CardType::Reinforcement:
            isReinforcement = true;
            break;
        case CardType::Blockade:
        {
            Territory *target = chooseTerritory("Blockade which of your territories?", *mine);
            if (target != nullptr)
            {
                order = new Blockade(player, target);
            }
            break;
        }
        case CardType::Airlift:
        {
            Territory *source = chooseTerritory("Airlift from which territory?", *mine);
            Territory *target = source != nullptr ? chooseTerritory("Airlift to which of your territories?", *mine) : nullptr;
            int armies = 0;
            if (target != nullptr && source->getArmies() >= 1 &&
                readInt("Number of armies to airlift", 1, source->getArmies(), armies))
            {
                order = new Airlift(player, source, target, armies);
            }
            break;
        }
        case CardType::Diplomacy:
        {
            // The players that own the territories we could attack
            std::vector<Player *> opponents;
            for (const Territory *t : *enemy)
            {
                if (t->getOwner() != nullptr &&
                    std::find(opponents.begin(), opponents.end(), t->getOwner()) == opponents.end())
                {
                    opponents.push_back(t->getOwner());
                }
            }
            if (!opponents.empty())
            {
                std::cout << "Negotiate with which player?\n";
                for (std::size_t i = 0; i < opponents.size(); i++)
                {
                    std::cout << "  " << (i + 1) << ") " << opponents[i]->getName() << "\n";
                }
                int who = 0;
                if (readInt("Player number", 1, static_cast<int>(opponents.size()), who))
                {
                    order = new Negotiate(player, opponents[who - 1]);
                }
            }
            break;
        }
        }

        bool played = false;
        if (isReinforcement || order != nullptr)
        {
            played = playCard(player, deck, card, order, isReinforcement);
        }
        else
        {
            std::cout << "  That card cannot be played right now.\n";
        }
        delete mine;
        delete enemy;
        return played;
    }
} // namespace

// ---------------------------------------------------------------------------------------
// PlayerStrategy (abstract base class)
// ---------------------------------------------------------------------------------------

/* Constructor: the strategy works for 'owner' (borrowed, may be set later). */
PlayerStrategy::PlayerStrategy(Player *owner) : p(owner)
{
}

/* Copy constructor: copies the borrowed player pointer; Player re-binds it to itself after cloning. */
PlayerStrategy::PlayerStrategy(const PlayerStrategy &other) : p(other.p)
{
}

/* Destructor: the player is not owned, so there is nothing to free. */
PlayerStrategy::~PlayerStrategy()
{
}

/* Assignment operator */
PlayerStrategy &PlayerStrategy::operator=(const PlayerStrategy &other)
{
    if (this != &other)
    {
        p = other.p;
    }
    return *this;
}

Player *PlayerStrategy::getPlayer() const
{
    return p;
}

void PlayerStrategy::setPlayer(Player *owner)
{
    p = owner;
}

/* Stream insertion operator: prints the strategy name. */
std::ostream &operator<<(std::ostream &os, const PlayerStrategy &strategy)
{
    os << strategy.getName() << " strategy";
    return os;
}

// ---------------------------------------------------------------------------------------
// DefaultPlayerStrategy (Assignment 2 behavior)
// ---------------------------------------------------------------------------------------

DefaultPlayerStrategy::DefaultPlayerStrategy(Player *owner) : PlayerStrategy(owner)
{
}

DefaultPlayerStrategy::DefaultPlayerStrategy(const DefaultPlayerStrategy &other) : PlayerStrategy(other)
{
}

DefaultPlayerStrategy::~DefaultPlayerStrategy()
{
}

DefaultPlayerStrategy &DefaultPlayerStrategy::operator=(const DefaultPlayerStrategy &other)
{
    PlayerStrategy::operator=(other);
    return *this;
}

PlayerStrategy *DefaultPlayerStrategy::clone() const
{
    return new DefaultPlayerStrategy(*this);
}

std::string DefaultPlayerStrategy::getName() const
{
    return "Default";
}

/* Territories to protect, most threatened first. The caller owns the returned vector. */
std::vector<Territory *> *DefaultPlayerStrategy::toDefend()
{
    return buildDefendList(p);
}

/* Neighboring territories that could be attacked, weakest first. The caller owns the returned vector. */
std::vector<Territory *> *DefaultPlayerStrategy::toAttack()
{
    return buildAttackList(p);
}

/* Deploy part of the armies that are still free, spread over the toDefend() list.
   Armies are only committed here; the pool is debited when the order executes. */
bool DefaultPlayerStrategy::issueDeploy()
{
    bool issued = false;
    int available = p->getAvailableArmies();
    std::vector<Territory *> *mine = toDefend();
    if (available > 0 && !mine->empty())
    {
        int armies = (available + 1) / 2; // Half of what is left (rounded up), so deploys are spread out
        Territory *target = (*mine)[p->getOrdersList()->size() % mine->size()];
        p->getOrdersList()->addOrder(new Deploy(p, target, armies, p->getReinforcementsPool()));
        *p->getPendingDeployments() += armies;
        issued = true;
    }
    delete mine;
    return issued;
}

/* Move half the armies of the strongest neighboring own territory to the
   highest-priority territory of the toDefend() list. */
bool DefaultPlayerStrategy::issueDefendAdvance()
{
    bool issued = false;
    std::vector<Territory *> *mine = toDefend();
    if (!mine->empty())
    {
        Territory *target = (*mine)[0];
        Territory *source = bestSourceFor(p, *mine, target, 2);
        if (source != nullptr)
        {
            p->getOrdersList()->addOrder(new Advance(p, source, target, source->getArmies() / 2));
            issued = true;
        }
    }
    delete mine;
    return issued;
}

/* Attack the weakest reachable territory of the toAttack() list with all but one
   army of the strongest neighboring own territory. */
bool DefaultPlayerStrategy::issueAttackAdvance()
{
    bool issued = false;
    std::vector<Territory *> *mine = toDefend();
    std::vector<Territory *> *enemy = toAttack();
    for (std::size_t i = 0; i < enemy->size() && !issued; i++)
    {
        Territory *target = (*enemy)[i];
        Territory *source = bestSourceFor(p, *mine, target, 2);
        if (source != nullptr)
        {
            p->getOrdersList()->addOrder(new Advance(p, source, target, source->getArmies() - 1));
            issued = true;
        }
    }
    delete mine;
    delete enemy;
    return issued;
}

/* Play the first card in the hand that can be turned into a sensible order.
   The played card goes back to the deck. */
bool DefaultPlayerStrategy::issueCardOrder(Deck *deck)
{
    if (deck == nullptr)
    {
        return false;
    }
    bool issued = false;
    Hand *hand = p->getHand();
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
                Territory *witness = bestSourceFor(p, *mine, target, 0);
                if (target->getOwner() != nullptr && witness != nullptr)
                {
                    order = new Bomb(p, target, witness);
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
                order = new Blockade(p, mine->back()); // Least threatened territory
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
                    order = new Airlift(p, source, target, source->getArmies() / 2);
                }
            }
            break;
        }
        case CardType::Diplomacy:
            for (Territory *target : *enemy)
            {
                if (target->getOwner() != nullptr)
                {
                    order = new Negotiate(p, target->getOwner());
                    break;
                }
            }
            break;
        }

        if (isReinforcement || order != nullptr)
        {
            issued = playCard(p, deck, card, order, isReinforcement);
        }
    }
    delete mine;
    delete enemy;
    return issued;
}

/* Issue one order. While armies are still left to deploy, only deploy orders are issued.
   After that: one defending advance, one attacking advance, then card orders.
   Orders are not validated here; that happens when they are executed. */
bool DefaultPlayerStrategy::issueOrder(Deck *deck)
{
    if (p->getAvailableArmies() > 0)
    {
        return issueDeploy();
    }
    int *stage = p->getIssueStage();
    while (*stage < 3)
    {
        bool issued = false;
        switch (*stage)
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
        (*stage)++;
        if (issued)
        {
            return true;
        }
    }
    return false; // Nothing more to issue this turn
}

// ---------------------------------------------------------------------------------------
// HumanPlayerStrategy
// ---------------------------------------------------------------------------------------

HumanPlayerStrategy::HumanPlayerStrategy(Player *owner) : PlayerStrategy(owner)
{
}

HumanPlayerStrategy::HumanPlayerStrategy(const HumanPlayerStrategy &other) : PlayerStrategy(other)
{
}

HumanPlayerStrategy::~HumanPlayerStrategy()
{
}

HumanPlayerStrategy &HumanPlayerStrategy::operator=(const HumanPlayerStrategy &other)
{
    PlayerStrategy::operator=(other);
    return *this;
}

PlayerStrategy *HumanPlayerStrategy::clone() const
{
    return new HumanPlayerStrategy(*this);
}

std::string HumanPlayerStrategy::getName() const
{
    return "Human";
}

/* Territories the user may defend: all of the player's territories, most threatened first. */
std::vector<Territory *> *HumanPlayerStrategy::toDefend()
{
    return buildDefendList(p);
}

/* Territories the user may attack: the enemy neighbors, weakest first. */
std::vector<Territory *> *HumanPlayerStrategy::toAttack()
{
    return buildAttackList(p);
}

/* Ask the user for the next order. Deploying is forced while armies are left to deploy; after that
   the user chooses between an advance, a card and ending the turn. Returns false when the user ends
   the turn (or the input is closed). */
bool HumanPlayerStrategy::issueOrder(Deck *deck)
{
    if (p->getAvailableArmies() > 0)
    {
        return humanDeploy(p);
    }
    while (true)
    {
        std::cout << "\n"
                  << p->getName() << ", choose an order: 1) Advance  2) Play a card  3) End turn\n";
        int choice = 0;
        if (!readInt("Choice", 1, 3, choice) || choice == 3)
        {
            return false;
        }
        bool issued = (choice == 1) ? humanAdvance(p) : humanCard(p, deck);
        if (issued)
        {
            return true;
        }
    }
}

// makes a cheater for the given player
CheaterPlayerStrategy::CheaterPlayerStrategy(Player *owner) : PlayerStrategy(owner) {}

// copy constructor, the base class does the copying
CheaterPlayerStrategy::CheaterPlayerStrategy(const CheaterPlayerStrategy &other) : PlayerStrategy(other) {}

// destructor, nothing to delete because the player isn't ours
CheaterPlayerStrategy::~CheaterPlayerStrategy() {}

// assignment operator
CheaterPlayerStrategy &CheaterPlayerStrategy::operator=(const CheaterPlayerStrategy &other)
{
    if (this != &other)
        PlayerStrategy::operator=(other);
    return *this;
}

// goes through my territories and collects the neighbors I don't own
std::vector<Territory *> CheaterPlayerStrategy::enemyNeighbors() const
{
    std::vector<Territory *> result;
    if (p == nullptr)
        return result;
    for (Territory *mine : *p->getTerritories())
    {
        for (Territory *next : mine->getAdjacent())
        {
            bool notMine = next->getOwner() != p;
            bool notAddedYet = std::find(result.begin(), result.end(), next) == result.end(); // a territory can touch several of mine
            if (notMine && notAddedYet)
                result.push_back(next);
        }
    }
    return result;
}

// takes all the enemy neighbors, only once per turn
// returns true if it took at least one
bool CheaterPlayerStrategy::issueOrder(Deck *)
{
    if (p == nullptr || *p->getIssueStage() > 0) // already did it this turn
        return false;
    *p->getIssueStage() = 1; // so we don't do it twice

    std::vector<Territory *> targets = enemyNeighbors(); // get the list first, before ownership changes
    for (Territory *t : targets)
    {
        Player *oldOwner = t->getOwner();
        if (oldOwner != nullptr)
            oldOwner->removeTerritory(t); // old owner loses it
        t->setOwner(p);
        p->addTerritory(t);
        std::cout << p->getName() << " (Cheater) conquered " << t->getName() << "\n";
    }
    if (!targets.empty())
        p->setConqueredThisTurn(true); // so it still gets a card at the end of the turn
    return !targets.empty();
}

// the enemy territories it takes over
std::vector<Territory *> *CheaterPlayerStrategy::toAttack()
{
    return new std::vector<Territory *>(enemyNeighbors());
}

// just my own territories, a cheater doesn't really defend
std::vector<Territory *> *CheaterPlayerStrategy::toDefend()
{
    if (p == nullptr)
        return new std::vector<Territory *>();
    return new std::vector<Territory *>(*p->getTerritories());
}

// makes a copy of this strategy
PlayerStrategy *CheaterPlayerStrategy::clone() const
{
    return new CheaterPlayerStrategy(*this);
}

// the name shown in logs and in the tournament results
std::string CheaterPlayerStrategy::getName() const
{
    return "Cheater";
}

AggressivePlayerStrategy::AggressivePlayerStrategy(Player *owner)
    : PlayerStrategy(owner) {}

AggressivePlayerStrategy::AggressivePlayerStrategy(
    const AggressivePlayerStrategy &other)
    : PlayerStrategy(other) {}

AggressivePlayerStrategy::~AggressivePlayerStrategy() {}

AggressivePlayerStrategy &
AggressivePlayerStrategy::operator=(
    const AggressivePlayerStrategy &other)
{
    if (this != &other)
        PlayerStrategy::operator=(other);
    return *this;
}

PlayerStrategy *AggressivePlayerStrategy::clone() const
{
    return new AggressivePlayerStrategy(*this);
}

std::string AggressivePlayerStrategy::getName() const
{
    return "Aggressive";
}

std::vector<Territory *> *AggressivePlayerStrategy::toDefend()
{
    auto *result = new std::vector<Territory *>();

    if (p == nullptr)
        return result;

    for (Territory *t : *p->getTerritories())
    {
        if (t != nullptr && t->getOwner() == p)
            result->push_back(t);
    }

    std::stable_sort(result->begin(), result->end(),
        [](const Territory *a, const Territory *b)
        {
            return a->getArmies() > b->getArmies();
        });

    return result;
}

std::vector<Territory *> *AggressivePlayerStrategy::toAttack()
{
    auto *targets = new std::vector<Territory *>();
    auto *owned = toDefend();

    // Prefer the strongest territory that can attack.
    for (Territory *source : *owned)
    {
        for (Territory *neighbor : source->getAdjacent())
        {
            if (neighbor != nullptr &&
                neighbor->getOwner() != p &&
                !p->hasNegotiatedWith(neighbor->getOwner()))
            {
                targets->push_back(neighbor);
            }
        }

        if (!targets->empty())
            break;
    }

    // Attack weaker targets first.
    std::stable_sort(targets->begin(), targets->end(),
        [](const Territory *a, const Territory *b)
        {
            return a->getArmies() < b->getArmies();
        });

    delete owned;
    return targets;
}

bool AggressivePlayerStrategy::issueOrder(Deck *deck)
{
    (void)deck;

    if (p == nullptr)
        return false;

    auto *owned = toDefend();

    if (owned->empty())
    {
        delete owned;
        return false;
    }

    Territory *strongest = (*owned)[0];
    int available = p->getAvailableArmies();

    // Deploy all available reinforcements to the strongest territory.
    if (available > 0)
    {
        p->getOrdersList()->addOrder(
            new Deploy(p, strongest, available,
                       p->getReinforcementsPool()));

        *p->getPendingDeployments() += available;

        delete owned;
        return true;
    }

    auto *targets = toAttack();

    // Find the strongest territory that can attack these targets.
    Territory *source = nullptr;

    for (Territory *candidate : *owned)
    {
        for (Territory *target : *targets)
        {
            const auto &neighbors = candidate->getAdjacent();

            if (std::find(neighbors.begin(), neighbors.end(), target)
                != neighbors.end())
            {
                source = candidate;
                break;
            }
        }

        if (source != nullptr)
            break;
    }

    bool issued = false;

    if (source != nullptr && !targets->empty())
    {
        int plannedArmies = source->getArmies();

        // Deploy orders execute before Advance orders.
        if (source == strongest)
            plannedArmies += *p->getPendingDeployments();

        int attackArmies = std::max(0, plannedArmies - 1);
        int targetCount = std::min(
            static_cast<int>(targets->size()), attackArmies);

        int index = *p->getIssueStage();

        if (index < targetCount)
        {
            // Reserve enough armies for subsequent attack orders.
            int armies = (index == 0)
                ? attackArmies - (targetCount - 1)
                : 1;

            p->getOrdersList()->addOrder(
                new Advance(p, source, (*targets)[index], armies));

            ++(*p->getIssueStage());
            issued = true;
        }
    }

    delete targets;
    delete owned;

    return issued;
}

/* Neutral player strategy */

NeutralPlayerStrategy::NeutralPlayerStrategy(Player *owner)
    : PlayerStrategy(owner) {}

NeutralPlayerStrategy::NeutralPlayerStrategy(
    const NeutralPlayerStrategy &other)
    : PlayerStrategy(other) {}

NeutralPlayerStrategy::~NeutralPlayerStrategy() {}

NeutralPlayerStrategy &
NeutralPlayerStrategy::operator=(
    const NeutralPlayerStrategy &other)
{
    if (this != &other)
        PlayerStrategy::operator=(other);
    return *this;
}

bool NeutralPlayerStrategy::issueOrder(Deck *deck)
{
    (void)deck;
    return false;
}

std::vector<Territory *> *NeutralPlayerStrategy::toAttack()
{
    return new std::vector<Territory *>();
}

std::vector<Territory *> *NeutralPlayerStrategy::toDefend()
{
    auto *result = new std::vector<Territory *>();

    if (p == nullptr)
        return result;

    for (Territory *territory : *p->getTerritories())
    {
        if (territory != nullptr &&
            territory->getOwner() == p)
        {
            result->push_back(territory);
        }
    }

    return result;
}

PlayerStrategy *NeutralPlayerStrategy::clone() const
{
    return new NeutralPlayerStrategy(*this);
}

std::string NeutralPlayerStrategy::getName() const
{
    return "Neutral";
}
