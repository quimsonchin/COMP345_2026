#pragma once
#include <string>
#include <vector>
#include <iostream>

class Player;     // defined in Player.h by teammate C
class Continent;  // full class comes later in this file

/** A node in the map graph. Owned by Map, not by its neighbors. */
class Territory {
public:
    Territory();
    Territory(const std::string& name);
    Territory(const Territory& other);
    Territory& operator=(const Territory& other);
    ~Territory();

    std::string getName() const;
    Player* getOwner() const;
    int getArmies() const;
    Continent* getContinent() const;
    const std::vector<Territory*>& getAdjacent() const;

    void setOwner(Player* owner);
    void setArmies(int armies);
    void setContinent(Continent* continent);
    void setAdjacent(Territory* other);  // adds one neighbor to the list

    friend std::ostream& operator<<(std::ostream& out, const Territory& t);

private:
    std::string name;
    Player* owner;                    // not owned
    int armies;
    Continent* continent;             // not owned
    std::vector<Territory*> adjacent; // not owned
};