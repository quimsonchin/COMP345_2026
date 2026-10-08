//only includes this header file once in a single compilation
#pragma once
#include <string>
#include <vector>
#include <iostream>

class Player; //defined in Player.h
class Territory; //defined within this file

class Territory {
public:
    Territory(); //defaut constructor
    Territory(const std::string&name); //passes text without copying it, will not be modified, and will not be deleted
    Territory(const Territory&other); //copy constructor
    Territory&operator=(const Territory&other); //builds a new object by copying the values of an existing object
    ~Territory(); //destructor

    //getters
    std::string getName() const;
    Player*getOwner() const;
    int getArmies() const;
    Continent*getContinent() const;
    const std::vector<Territory*>&getAdjacentTerritories() const;

    //setters
    void setOwner(Player*player);
    void setArmies(int num);
    void setContinent(Continent*continent);
    void setAdjacent(Territory&other);

    friend std::ostream&operator<<(std::ostream&out, const Territory&t);
    
private:
    std::string name;
    Player* owner;
    int armies;
    Continent* continent;
    std::vector<Territory*> adjacent;
};
    
