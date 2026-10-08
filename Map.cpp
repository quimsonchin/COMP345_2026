#include "Map.h"
#include <algorithm>

Territory::Territory()
    : name(""), owner(nullptr), armies(0), continent(nullptr) {}

Territory::Territory(const std::string& n)
    : name(n), owner(nullptr), armies(0), continent(nullptr) {}

Territory::Territory(const Territory& other)
    : name(other.name), owner(other.owner), armies(other.armies),
      continent(other.continent), adjacent(other.adjacent) {}

Territory& Territory::operator=(const Territory& other) {
    if (this != &other) {
        name = other.name;
        owner = other.owner;
        armies = other.armies;
        continent = other.continent;
        adjacent = other.adjacent;
    }
    return *this;
}

Territory::~Territory() {
    // Nothing to delete: Map owns territories; owner, continent and neighbors are borrowed.
}

std::string Territory::getName() const { return name; }
Player* Territory::getOwner() const { return owner; }
int Territory::getArmies() const { return armies; }
Continent* Territory::getContinent() const { return continent; }
const std::vector<Territory*>& Territory::getAdjacent() const { return adjacent; }

void Territory::setOwner(Player* o) { owner = o; }
void Territory::setArmies(int a) { armies = a; }
void Territory::setContinent(Continent* c) { continent = c; }

void Territory::setAdjacent(Territory* other) {
    if (other == nullptr || other == this) return;
    if (std::find(adjacent.begin(), adjacent.end(), other) == adjacent.end())
        adjacent.push_back(other);
}

std::ostream& operator<<(std::ostream& out, const Territory& t) {
    out << t.name << " (armies: " << t.armies
        << ", neighbors: " << t.adjacent.size() << ")";
    return out;
}