#include "Map.h"
#include <algorithm>
#include <cstddef>
#include <map>
#include <queue>
#include <set>

// TERRITORY
// Default constructor. Blank name, no owner, no armies, no continent, no neighbors.
Territory::Territory()
    : name(""), owner(nullptr), armies(0), continent(nullptr) {}

// Makes a territory with a name and nothing else filled in yet.
Territory::Territory(const std::string& n)
    : name(n), owner(nullptr), armies(0), continent(nullptr) {}

// Copy constructor. Copies the plain values. The owner, continent and neighbors
// are pointers, so the copy just points at the same things the original does.
Territory::Territory(const Territory& other)
    : name(other.name), owner(other.owner), armies(other.armies),
      continent(other.continent), adjacent(other.adjacent) {}

// Assignment operator. Does the same thing as the copy constructor.
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

// Destructor. Nothing to delete here, since the Map owns the territories
// and everything else we hold is only borrowed.
Territory::~Territory() {}

// Gives back the territory's name.
std::string Territory::getName() const { return name; }

// Gives back the player who owns it (can be null).
Player* Territory::getOwner() const { return owner; }

// Gives back how many armies are sitting here.
int Territory::getArmies() const { return armies; }

// Gives back the continent it belongs to (can be null).
Continent* Territory::getContinent() const { return continent; }

// Gives back the list of neighbors.
const std::vector<Territory*>& Territory::getAdjacent() const { return adjacent; }

// Sets who owns this territory.
void Territory::setOwner(Player* o) { owner = o; }

// Sets the army count.
void Territory::setArmies(int a) { armies = a; }

// Sets which continent this territory is in.
void Territory::setContinent(Continent* c) { continent = c; }

// Adds one neighbor. It skips null, itself, and anything already in the list,
// so listing the same edge twice is harmless.
void Territory::setAdjacent(Territory* other) {
    if (other == nullptr || other == this) return;
    if (std::find(adjacent.begin(), adjacent.end(), other) == adjacent.end())
        adjacent.push_back(other);
}

// Prints the name, army count, number of neighbors and continent name.
std::ostream& operator<<(std::ostream& out, const Territory& t) {
    out << t.name << " (armies: " << t.armies
        << ", neighbors: " << t.adjacent.size() << ", continent: ";
    if (t.continent != nullptr)
        out << t.continent->getName();
    else
        out << "none";
    out << ")";
    return out;
}

//Continent
// Default constructor. Blank name, bonus of 0, no territories.
Continent::Continent() : name(""), bonus(0) {}

// Makes a continent with a name and a bonus.
Continent::Continent(const std::string& n, int b) : name(n), bonus(b) {}

// Copy constructor. The territory list is copied as pointers, so both continents
// end up pointing at the same territories.
Continent::Continent(const Continent& other)
    : name(other.name), bonus(other.bonus), territories(other.territories) {}

// Assignment operator. Does the same thing as the copy constructor.
Continent& Continent::operator=(const Continent& other) {
    if (this != &other) {
        name = other.name;
        bonus = other.bonus;
        territories = other.territories;
    }
    return *this;
}

// Destructor. Nothing to delete, the Map owns the territories.
Continent::~Continent() {}

// Gives back the continent's name.
std::string Continent::getName() const { return name; }

// Gives back the army bonus.
int Continent::getBonus() const { return bonus; }

// Gives back the territories in this continent.
const std::vector<Territory*>& Continent::getTerritories() const { return territories; }

// Changes the name.
void Continent::setName(const std::string& n) { name = n; }

// Changes the bonus.
void Continent::setBonus(int b) { bonus = b; }

// Adds a territory to the list. Null and duplicates are ignored.
void Continent::addTerritory(Territory* territory) {
    if (territory == nullptr) return;
    if (std::find(territories.begin(), territories.end(), territory) == territories.end())
        territories.push_back(territory);
}

// Prints the name, bonus, and the names of the territories inside.
std::ostream& operator<<(std::ostream& out, const Continent& c) {
    out << c.name << " (bonus: " << c.bonus << ", territories: ";
    for (std::size_t i = 0; i < c.territories.size(); ++i) {
        if (i > 0) out << ", ";
        out << c.territories[i]->getName();
    }
    out << ")";
    return out;
}

//MAP
// Default constructor. Starts out empty.
Map::Map() {}

// Copy constructor. Makes a full deep copy, see copyFrom below.
Map::Map(const Map& other) {
    copyFrom(other);
}

// Assignment operator. Throws away what we have, then deep copies the other map.
Map& Map::operator=(const Map& other) {
    if (this != &other) {
        clear();
        copyFrom(other);
    }
    return *this;
}

// Destructor. Deletes every territory and continent we own.
Map::~Map() {
    clear();
}

// Deletes everything we own and empties both lists.
void Map::clear() {
    for (Territory* t : territories) delete t;
    for (Continent* c : continents) delete c;
    territories.clear();
    continents.clear();
}

// The deep copy. First we make brand new continents and territories and keep a
// lookup from each old pointer to its new one. Then we use that lookup to rewire
// the neighbors, the continent links and the member lists, so nothing in the copy
// still points back at the original map.
void Map::copyFrom(const Map& other) {
    std::map<const Territory*, Territory*> newTerritory;
    std::map<const Continent*, Continent*> newContinent;

    for (Continent* c : other.continents) {
        Continent* copy = new Continent(c->getName(), c->getBonus());
        continents.push_back(copy);
        newContinent[c] = copy;
    }

    for (Territory* t : other.territories) {
        Territory* copy = new Territory(t->getName());
        copy->setArmies(t->getArmies());
        copy->setOwner(t->getOwner());
        territories.push_back(copy);
        newTerritory[t] = copy;
    }

    for (Territory* t : other.territories) {
        Territory* copy = newTerritory[t];
        for (Territory* neighbor : t->getAdjacent()) {
            if (newTerritory.count(neighbor) > 0)
                copy->setAdjacent(newTerritory[neighbor]);
        }
        if (t->getContinent() != nullptr && newContinent.count(t->getContinent()) > 0)
            copy->setContinent(newContinent[t->getContinent()]);
    }

    for (Continent* c : other.continents) {
        Continent* copy = newContinent[c];
        for (Territory* member : c->getTerritories()) {
            if (newTerritory.count(member) > 0)
                copy->addTerritory(newTerritory[member]);
        }
    }
}

// Creates a territory and adds it to the map. If one with that name already
// exists, you get that one back instead of a duplicate.
Territory* Map::addTerritory(const std::string& name) {
    Territory* existing = findTerritory(name);
    if (existing != nullptr) return existing;
    Territory* created = new Territory(name);
    territories.push_back(created);
    return created;
}

// Creates a continent and adds it to the map. If one with that name already
// exists, you get that one back and its bonus is left alone.
Continent* Map::addContinent(const std::string& name, int bonus) {
    Continent* existing = findContinent(name);
    if (existing != nullptr) return existing;
    Continent* created = new Continent(name, bonus);
    continents.push_back(created);
    return created;
}

// Puts a territory in a continent. It sets the territory's continent and also
// adds the territory to the continent's list.
void Map::assignToContinent(Territory* territory, Continent* continent) {
    if (territory == nullptr || continent == nullptr) return;
    territory->setContinent(continent);
    continent->addTerritory(territory);
}

// Connects two territories, one link in each direction.
void Map::addEdge(Territory* a, Territory* b) {
    if (a == nullptr || b == nullptr) return;
    a->setAdjacent(b);
    b->setAdjacent(a);
}

// Looks a territory up by name. Returns null if there isn't one.
Territory* Map::findTerritory(const std::string& name) const {
    for (Territory* t : territories) {
        if (t->getName() == name) return t;
    }
    return nullptr;
}

// Looks a continent up by name. Returns null if there isn't one.
Continent* Map::findContinent(const std::string& name) const {
    for (Continent* c : continents) {
        if (c->getName() == name) return c;
    }
    return nullptr;
}

// Gives back all the territories.
const std::vector<Territory*>& Map::getTerritories() const { return territories; }

// Gives back all the continents.
const std::vector<Continent*>& Map::getContinents() const { return continents; }

namespace {

// Plain breadth-first search starting at 'start'. It only steps onto territories
// that are in 'allowed', so the same function works for the whole map and for a
// single continent. Returns how many territories it reached, counting 'start'.
std::size_t countReachable(Territory* start, const std::set<const Territory*>& allowed) {
    std::set<const Territory*> visited;
    std::queue<Territory*> toVisit;
    visited.insert(start);
    toVisit.push(start);
    while (!toVisit.empty()) {
        Territory* current = toVisit.front();
        toVisit.pop();
        for (Territory* next : current->getAdjacent()) {
            if (allowed.count(next) > 0 && visited.count(next) == 0) {
                visited.insert(next);
                toVisit.push(next);
            }
        }
    }
    return visited.size();
}

}  // namespace

// Runs the three checks and prints PASS or FAIL for each:
//   1. the whole map is connected
//   2. every continent is connected on its own
//   3. every territory is in exactly one continent
// Returns true only if all three pass. An empty map counts as invalid.
bool Map::validate() const {
    if (territories.empty()) {
        std::cout << "  [FAIL] the map has no territories\n";
        return false;
    }

    bool valid = true;

    // Check 1: starting from the first territory, can we reach all the others?
    std::set<const Territory*> everything(territories.begin(), territories.end());
    if (countReachable(territories[0], everything) == territories.size()) {
        std::cout << "  [PASS] the map is a connected graph\n";
    } else {
        std::cout << "  [FAIL] the map is not connected\n";
        valid = false;
    }

    // Check 2: same idea, but only inside each continent. An empty continent fails.
    bool continentsConnected = true;
    for (Continent* c : continents) {
        const std::vector<Territory*>& members = c->getTerritories();
        if (members.empty()) {
            std::cout << "  [FAIL] continent " << c->getName() << " has no territories\n";
            continentsConnected = false;
            continue;
        }
        std::set<const Territory*> memberSet(members.begin(), members.end());
        if (countReachable(members[0], memberSet) != memberSet.size()) {
            std::cout << "  [FAIL] continent " << c->getName() << " is not connected\n";
            continentsConnected = false;
        }
    }
    if (continentsConnected) {
        std::cout << "  [PASS] every continent is a connected subgraph\n";
    } else {
        valid = false;
    }

    // Check 3: count how many continents list each territory. It has to be exactly one.
    bool oneContinentEach = true;
    for (Territory* t : territories) {
        int count = 0;
        for (Continent* c : continents) {
            const std::vector<Territory*>& members = c->getTerritories();
            if (std::find(members.begin(), members.end(), t) != members.end()) ++count;
        }
        if (count != 1) {
            std::cout << "  [FAIL] territory " << t->getName() << " is in " << count
                      << " continents (must be exactly 1)\n";
            oneContinentEach = false;
        }
    }
    if (oneContinentEach) {
        std::cout << "  [PASS] every territory is in exactly one continent\n";
    } else {
        valid = false;
    }

    return valid;
}

// Prints a quick summary: continents first, then territories.
std::ostream& operator<<(std::ostream& out, const Map& m) {
    out << "Map with " << m.territories.size() << " territories and "
        << m.continents.size() << " continents\n";
    for (Continent* c : m.continents) out << "  " << *c << "\n";
    for (Territory* t : m.territories) out << "  " << *t << "\n";
    return out;
}