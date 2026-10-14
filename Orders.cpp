#include "Orders.h"
#include "Map.h"
#include <algorithm>
#include <stdexcept>

namespace {
// Check an adjacency edge in the Map.h territory graph.
bool adjacentTo(const Territory* source, const Territory* target) {
    if (!source || !target) return false;
    const std::vector<Territory*>& neighbors = source->getAdjacent();
    return std::find(neighbors.begin(), neighbors.end(), target) != neighbors.end();
}
std::string territoryName(const Territory* territory) {
    return territory ? territory->getName() : "null territory";
}
}

Order::Order() : effect(""), executed(false) {}
Order::Order(const Order& other) : effect(other.effect), executed(other.executed) {}
Order& Order::operator=(const Order& other) {
    if (this != &other) { effect = other.effect; executed = other.executed; }
    return *this;
}
Order::~Order() {}
std::string Order::getEffect() const { return effect; }
bool Order::wasExecuted() const { return executed; }
std::ostream& operator<<(std::ostream& out, const Order& order) {
    out << order.getDescription();
    if (order.wasExecuted()) out << " | Effect: " << order.getEffect();
    return out;
}

Deploy::Deploy(Player* p, Territory* t, int n, int* pool)
    : Order(), player(p), target(t), armies(n), reinforcementPool(pool) {}
Deploy::Deploy(const Deploy& other) : Order(other), player(other.player),
    target(other.target), armies(other.armies), reinforcementPool(other.reinforcementPool) {}
Deploy& Deploy::operator=(const Deploy& other) {
    if (this != &other) { Order::operator=(other); player = other.player;
        target = other.target; armies = other.armies; reinforcementPool = other.reinforcementPool; }
    return *this;
}
Deploy::~Deploy() {}
bool Deploy::validate() const {
    return player && target && reinforcementPool && armies > 0 &&
           target->getOwner() == player && *reinforcementPool >= armies;
}
void Deploy::execute() {
    executed = false;
    effect = "Rejected: invalid deploy order";
    if (!validate()) return;
    // A1: report acceptance; no actual map changes or pool debit yet.
    executed = true;
    effect = "Accepted: deploy game action deferred (A1)";
}
Order* Deploy::clone() const { return new Deploy(*this); }
std::string Deploy::getDescription() const {
    return "Deploy " + std::to_string(armies) + " armies to " + territoryName(target);
}
std::ostream& operator<<(std::ostream& out, const Deploy& order) {
    return out << static_cast<const Order&>(order);
}

Advance::Advance(Player* p, Territory* s, Territory* t, int n)
    : Order(), player(p), source(s), target(t), armies(n) {}
Advance::Advance(const Advance& other) : Order(other), player(other.player),
    source(other.source), target(other.target), armies(other.armies) {}
Advance& Advance::operator=(const Advance& other) {
    if (this != &other) { Order::operator=(other); player = other.player; source = other.source;
        target = other.target; armies = other.armies; }
    return *this;
}
Advance::~Advance() {}
bool Advance::validate() const {
    return player && source && target && source != target && armies > 0 &&
           source->getOwner() == player && source->getArmies() >= armies &&
           adjacentTo(source, target);
}
void Advance::execute() {
    executed = false;
    effect = "Rejected: invalid advance order";
    if (!validate()) return;
    executed = true;
    effect = "Accepted: advance game action deferred (A1)";
}
Order* Advance::clone() const { return new Advance(*this); }
std::string Advance::getDescription() const {
    return "Advance " + std::to_string(armies) + " armies from " + territoryName(source) +
           " to " + territoryName(target);
}
std::ostream& operator<<(std::ostream& out, const Advance& order) {
    return out << static_cast<const Order&>(order);
}

Bomb::Bomb(Player* p, Territory* t, Territory* witness)
    : Order(), player(p), target(t), adjacentOwnedTerritory(witness) {}
Bomb::Bomb(const Bomb& other) : Order(other), player(other.player),
    target(other.target), adjacentOwnedTerritory(other.adjacentOwnedTerritory) {}
Bomb& Bomb::operator=(const Bomb& other) {
    if (this != &other) { Order::operator=(other); player = other.player;
        target = other.target; adjacentOwnedTerritory = other.adjacentOwnedTerritory; }
    return *this;
}
Bomb::~Bomb() {}
bool Bomb::validate() const {
    return player && target && adjacentOwnedTerritory && target->getOwner() &&
           target->getOwner() != player && adjacentOwnedTerritory->getOwner() == player &&
           adjacentTo(adjacentOwnedTerritory, target);
}
void Bomb::execute() {
    executed = false;
    effect = "Rejected: invalid bomb order";
    if (!validate()) return;
    executed = true;
    effect = "Accepted: bomb game action deferred (A1)";
}
Order* Bomb::clone() const { return new Bomb(*this); }
std::string Bomb::getDescription() const { return "Bomb " + territoryName(target); }
std::ostream& operator<<(std::ostream& out, const Bomb& order) {
    return out << static_cast<const Order&>(order);
}

Blockade::Blockade(Player* p, Territory* t) : Order(), player(p), target(t) {}
Blockade::Blockade(const Blockade& other) : Order(other), player(other.player), target(other.target) {}
Blockade& Blockade::operator=(const Blockade& other) {
    if (this != &other) { Order::operator=(other); player = other.player; target = other.target; }
    return *this;
}
Blockade::~Blockade() {}
bool Blockade::validate() const { return player && target && target->getOwner() == player; }
void Blockade::execute() {
    executed = false;
    effect = "Rejected: invalid blockade order";
    if (!validate()) return;
    executed = true;
    effect = "Accepted: blockade game action deferred (A1)";
}
Order* Blockade::clone() const { return new Blockade(*this); }
std::string Blockade::getDescription() const { return "Blockade " + territoryName(target); }
std::ostream& operator<<(std::ostream& out, const Blockade& order) {
    return out << static_cast<const Order&>(order);
}

Airlift::Airlift(Player* p, Territory* s, Territory* t, int n)
    : Order(), player(p), source(s), target(t), armies(n) {}
Airlift::Airlift(const Airlift& other) : Order(other), player(other.player),
    source(other.source), target(other.target), armies(other.armies) {}
Airlift& Airlift::operator=(const Airlift& other) {
    if (this != &other) { Order::operator=(other); player = other.player;
        source = other.source; target = other.target; armies = other.armies; }
    return *this;
}
Airlift::~Airlift() {}
bool Airlift::validate() const {
    return player && source && target && source != target && armies > 0 &&
           source->getOwner() == player && source->getArmies() >= armies;
}
void Airlift::execute() {
    executed = false;
    effect = "Rejected: invalid airlift order";
    if (!validate()) return;
    executed = true;
    effect = "Accepted: airlift game action deferred (A1)";
}
Order* Airlift::clone() const { return new Airlift(*this); }
std::string Airlift::getDescription() const {
    return "Airlift " + std::to_string(armies) + " armies from " + territoryName(source) +
           " to " + territoryName(target);
}
std::ostream& operator<<(std::ostream& out, const Airlift& order) {
    return out << static_cast<const Order&>(order);
}

Negotiate::Negotiate(Player* p, Player* t) : Order(), player(p), targetPlayer(t) {}
Negotiate::Negotiate(const Negotiate& other)
    : Order(other), player(other.player), targetPlayer(other.targetPlayer) {}
Negotiate& Negotiate::operator=(const Negotiate& other) {
    if (this != &other) { Order::operator=(other); player = other.player;
        targetPlayer = other.targetPlayer; }
    return *this;
}
Negotiate::~Negotiate() {}
bool Negotiate::validate() const { return player && targetPlayer && player != targetPlayer; }
void Negotiate::execute() {
    executed = false;
    effect = "Rejected: invalid negotiate order";
    if (!validate()) return;
    executed = true;
    effect = "Accepted: negotiate game action deferred (A1)";
}
Order* Negotiate::clone() const { return new Negotiate(*this); }
std::string Negotiate::getDescription() const { return "Negotiate with another player"; }
std::ostream& operator<<(std::ostream& out, const Negotiate& order) {
    return out << static_cast<const Order&>(order);
}

OrdersList::OrdersList() {}
OrdersList::OrdersList(const OrdersList& other) {
    try {
        orders.reserve(other.orders.size());
        for (Order* order : other.orders) orders.push_back(order->clone());
    } catch (...) {
        for (Order* order : orders) delete order;
        throw;
    }
}
OrdersList& OrdersList::operator=(const OrdersList& other) {
    if (this != &other) {
        OrdersList temp(other);
        orders.swap(temp.orders);
    }
    return *this;
}
OrdersList::~OrdersList() {
    for (Order* order : orders) delete order;
}
void OrdersList::addOrder(Order* order) {
    if (!order) return;
    if (std::find(orders.begin(), orders.end(), order) != orders.end())
        throw std::invalid_argument("The same Order* is already in this list");
    // Take ownership only after successfully adding the pointer.
    orders.push_back(order);
}
bool OrdersList::move(int fromIndex, int toIndex) {
    if (fromIndex < 0 || toIndex < 0 || fromIndex >= size() || toIndex >= size()) return false;
    if (fromIndex == toIndex) return true;
    if (fromIndex < toIndex) {
        std::rotate(orders.begin() + fromIndex, orders.begin() + fromIndex + 1,
                    orders.begin() + toIndex + 1);
    } else {
        std::rotate(orders.begin() + toIndex, orders.begin() + fromIndex,
                    orders.begin() + fromIndex + 1);
    }
    return true;
}
bool OrdersList::remove(int index) {
    if (index < 0 || index >= size()) return false;
    delete orders[index];
    orders.erase(orders.begin() + index);
    return true;
}
int OrdersList::size() const { return static_cast<int>(orders.size()); }
Order* OrdersList::getOrder(int index) const {
    return (index < 0 || index >= size()) ? nullptr : orders[index];
}
std::ostream& operator<<(std::ostream& out, const OrdersList& list) {
    out << "OrdersList contains " << list.orders.size() << " orders:\n";
    for (std::size_t i = 0; i < list.orders.size(); ++i)
        out << i << ": " << *list.orders[i] << '\n';
    return out;
}
