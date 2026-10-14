#pragma once
#include <string>
#include <vector>
#include <iostream>

class Player;     // defined in Player.h by teammate C
class Continent;  // full class comes later in this file

// A node in the map graph. Owned by Map, not by its neighbors. 
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
    Player* owner;                   
    int armies;
    Continent* continent;             
    std::vector<Territory*> adjacent; 
};


 // A group of territories with an army bonus attached. Same idea as Territory:
 // the Map owns the continent, and the continent only points at its territories.

class Continent {
public:
    Continent();
    Continent(const std::string& name, int bonus);
    Continent(const Continent& other);
    Continent& operator=(const Continent& other);
    ~Continent();
 
    std::string getName() const;
    int getBonus() const;
    const std::vector<Territory*>& getTerritories() const;
 
    void setName(const std::string& name);
    void setBonus(int bonus);
    void addTerritory(Territory* territory);
 
    friend std::ostream& operator<<(std::ostream& out, const Continent& c);
 
private:
    std::string name;
    int bonus;
    std::vector<Territory*> territories; // not owned
};
 

 // The whole map: the territories, the continents, and the graph that connects them.
 // It owns everything it creates and deletes it all in the destructor.

class Map {
public:
    Map();
    Map(const Map& other);
    Map& operator=(const Map& other);
    ~Map();
 
    Territory* addTerritory(const std::string& name);
    Continent* addContinent(const std::string& name, int bonus);
    void assignToContinent(Territory* territory, Continent* continent);
    void addEdge(Territory* a, Territory* b);
 
    Territory* findTerritory(const std::string& name) const;
    Continent* findContinent(const std::string& name) const;
    const std::vector<Territory*>& getTerritories() const;
    const std::vector<Continent*>& getContinents() const;
 
    bool validate() const;
 
    friend std::ostream& operator<<(std::ostream& out, const Map& m);
 
private:
    std::vector<Territory*> territories; // owned
    std::vector<Continent*> continents;  // owned
 
    void clear();
    void copyFrom(const Map& other);
};