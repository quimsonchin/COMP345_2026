#pragma once
#include <iostream>
#include <string>
#include <vector>

class Player;
class Deck;
class Territory;

/* Abstract base class of the Strategy pattern (A3 Part 1). A Player owns exactly one
   PlayerStrategy and delegates issueOrder(), toAttack() and toDefend() to it, so the
   behavior of a player can be replaced at runtime with Player::setStrategy().
   The strategy only borrows the Player it works for: it is never deleted by the strategy. */
class PlayerStrategy
{
protected:
    Player *p; // The player this strategy decides for (not owned)

public:
    // Constructors, destructor and assignment operator
    explicit PlayerStrategy(Player *owner = nullptr);
    PlayerStrategy(const PlayerStrategy &other);
    virtual ~PlayerStrategy();
    PlayerStrategy &operator=(const PlayerStrategy &other);

    // The three behaviors every concrete strategy must implement
    virtual bool issueOrder(Deck *deck) = 0;
    virtual std::vector<Territory *> *toDefend() = 0;
    virtual std::vector<Territory *> *toAttack() = 0;

    // Virtual copy constructor, used by Player's copy constructor and assignment operator
    virtual PlayerStrategy *clone() const = 0;

    // Name of the strategy ("Human", "Aggressive", ...)
    virtual std::string getName() const = 0;

    // The player this strategy decides for
    Player *getPlayer() const;
    void setPlayer(Player *owner);

    // Prints the strategy name
    friend std::ostream &operator<<(std::ostream &os, const PlayerStrategy &strategy);
};

/* Computer strategy that keeps the Assignment 2 behavior (deploy, then one defending
   advance, one attacking advance and cards). Used by every new Player until another strategy
   is set. */
class DefaultPlayerStrategy : public PlayerStrategy
{
private:
    // Helpers for issueOrder(): each returns true if it issued an order
    bool issueDeploy();
    bool issueDefendAdvance();
    bool issueAttackAdvance();
    bool issueCardOrder(Deck *deck);

public:
    explicit DefaultPlayerStrategy(Player *owner = nullptr);
    DefaultPlayerStrategy(const DefaultPlayerStrategy &other);
    ~DefaultPlayerStrategy() override;
    DefaultPlayerStrategy &operator=(const DefaultPlayerStrategy &other);

    bool issueOrder(Deck *deck) override;
    std::vector<Territory *> *toAttack() override;
    std::vector<Territory *> *toDefend() override;
    PlayerStrategy *clone() const override;
    std::string getName() const override;
};

/* Human strategy: every decision is asked from the user on the console.
   Reading stops (the player ends its turn) when the input stream is closed. */
class HumanPlayerStrategy : public PlayerStrategy
{
public:
    explicit HumanPlayerStrategy(Player *owner = nullptr);
    HumanPlayerStrategy(const HumanPlayerStrategy &other);
    ~HumanPlayerStrategy() override;
    HumanPlayerStrategy &operator=(const HumanPlayerStrategy &other);

    bool issueOrder(Deck *deck) override;
    std::vector<Territory *> *toAttack() override;
    std::vector<Territory *> *toDefend() override;
    PlayerStrategy *clone() const override;
    std::string getName() const override;
};

// Cheater strategy: once per turn it takes every territory next to one of its own
class CheaterPlayerStrategy : public PlayerStrategy
{
public:
    // makes a cheater for the given player
    explicit CheaterPlayerStrategy(Player *owner = nullptr);
    // copy constructor
    CheaterPlayerStrategy(const CheaterPlayerStrategy &other);
    // destructor
    ~CheaterPlayerStrategy() override;
    // assignment operator
    CheaterPlayerStrategy &operator=(const CheaterPlayerStrategy &other);

    // takes all the enemy neighbors (once per turn), true if it took any
    bool issueOrder(Deck *deck) override;
    // the enemy territories it takes over
    std::vector<Territory *> *toAttack() override;
    // the player's own territories
    std::vector<Territory *> *toDefend() override;
    // makes a copy of this strategy
    PlayerStrategy *clone() const override;
    // returns "Cheater"
    std::string getName() const override;

private:
    // enemy territories next to mine, no doubles
    std::vector<Territory *> enemyNeighbors() const;
};

/* Aggressive strategy: concentrates reinforcements on the strongest
   territory and prioritizes attacks against neighboring enemies. */
class AggressivePlayerStrategy : public PlayerStrategy
{
public:
    explicit AggressivePlayerStrategy(Player *owner = nullptr);
    AggressivePlayerStrategy(const AggressivePlayerStrategy &other);
    ~AggressivePlayerStrategy() override;
    AggressivePlayerStrategy &operator=(const AggressivePlayerStrategy &other);

    bool issueOrder(Deck *deck) override;
    std::vector<Territory *> *toAttack() override;
    std::vector<Territory *> *toDefend() override;

    PlayerStrategy *clone() const override;
    std::string getName() const override;
};

/* Neutral strategy: never issues orders.
   An attacked Neutral player switches to Aggressive. */
class NeutralPlayerStrategy : public PlayerStrategy
{
public:
    explicit NeutralPlayerStrategy(Player *owner = nullptr);
    NeutralPlayerStrategy(const NeutralPlayerStrategy &other);
    ~NeutralPlayerStrategy() override;
    NeutralPlayerStrategy &operator=(const NeutralPlayerStrategy &other);

    bool issueOrder(Deck *deck) override;
    std::vector<Territory *> *toAttack() override;
    std::vector<Territory *> *toDefend() override;

    PlayerStrategy *clone() const override;
    std::string getName() const override;
};
