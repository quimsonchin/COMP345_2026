#ifndef ORDERS_H
#define ORDERS_H

#include <iostream>
#include <string>
#include <vector>

class Player;
class Territory;

// Base type for all orders. Orders borrow Player/Territory pointers.
class Order {
public:
    Order();
    Order(const Order& other);
    Order& operator=(const Order& other);
    virtual ~Order();

    virtual bool validate() const = 0;
    virtual void execute() = 0;
    virtual Order* clone() const = 0;
    virtual std::string getDescription() const = 0;

    std::string getEffect() const;
    bool wasExecuted() const;
    friend std::ostream& operator<<(std::ostream& out, const Order& order);
protected:
    std::string effect;
    bool executed;
};

// Deploy needs a shared reinforcement-pool counter until Player is integrated.
class Deploy : public Order {
public:
    Deploy(Player* player, Territory* target, int armies, int* reinforcementPool);
    Deploy(const Deploy& other);
    Deploy& operator=(const Deploy& other);
    ~Deploy() override;
    bool validate() const override;
    void execute() override;
    Order* clone() const override;
    std::string getDescription() const override;
    friend std::ostream& operator<<(std::ostream& out, const Deploy& order);
private:
    Player* player;
    Territory* target;
    int armies;
    int* reinforcementPool; // Borrowed shared counter (A1 integration seam)
};

class Advance : public Order {
public:
    Advance(Player* player, Territory* source, Territory* target, int armies);
    Advance(const Advance& other);
    Advance& operator=(const Advance& other);
    ~Advance() override;
    bool validate() const override;
    void execute() override;
    Order* clone() const override;
    std::string getDescription() const override;
    friend std::ostream& operator<<(std::ostream& out, const Advance& order);
private:
    Player* player;
    Territory* source;
    Territory* target;
    int armies;
};

// Adjacent owned territory is a witness for the bombing eligibility rule.
class Bomb : public Order {
public:
    Bomb(Player* player, Territory* target, Territory* adjacentOwnedTerritory);
    Bomb(const Bomb& other);
    Bomb& operator=(const Bomb& other);
    ~Bomb() override;
    bool validate() const override;
    void execute() override;
    Order* clone() const override;
    std::string getDescription() const override;
    friend std::ostream& operator<<(std::ostream& out, const Bomb& order);
private:
    Player* player;
    Territory* target;
    Territory* adjacentOwnedTerritory;
};

class Blockade : public Order {
public:
    Blockade(Player* player, Territory* target);
    Blockade(const Blockade& other);
    Blockade& operator=(const Blockade& other);
    ~Blockade() override;
    bool validate() const override;
    void execute() override;
    Order* clone() const override;
    std::string getDescription() const override;
    friend std::ostream& operator<<(std::ostream& out, const Blockade& order);
private:
    Player* player;
    Territory* target;
};

class Airlift : public Order {
public:
    Airlift(Player* player, Territory* source, Territory* target, int armies);
    Airlift(const Airlift& other);
    Airlift& operator=(const Airlift& other);
    ~Airlift() override;
    bool validate() const override;
    void execute() override;
    Order* clone() const override;
    std::string getDescription() const override;
    friend std::ostream& operator<<(std::ostream& out, const Airlift& order);
private:
    Player* player;
    Territory* source;
    Territory* target;
    int armies;
};

class Negotiate : public Order {
public:
    Negotiate(Player* player, Player* targetPlayer);
    Negotiate(const Negotiate& other);
    Negotiate& operator=(const Negotiate& other);
    ~Negotiate() override;
    bool validate() const override;
    void execute() override;
    Order* clone() const override;
    std::string getDescription() const override;
    friend std::ostream& operator<<(std::ostream& out, const Negotiate& order);
private:
    Player* player;
    Player* targetPlayer;
};

// Owns Orders; do not insert the same Order* into two owning lists.
class OrdersList {
public:
    OrdersList();
    OrdersList(const OrdersList& other);
    OrdersList& operator=(const OrdersList& other);
    ~OrdersList();
    void addOrder(Order* order); // Transfers ownership to the list
    bool move(int fromIndex, int toIndex);
    bool remove(int index);
    int size() const;
    Order* getOrder(int index) const; // Non-owning, do not delete
    friend std::ostream& operator<<(std::ostream& out, const OrdersList& list);
private:
    std::vector<Order*> orders;
};

#endif
