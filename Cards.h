#ifndef CARDS_H
#define CARDS_H

#include <iostream>

class Hand;
class Deck;
class OrdersList;

// Each card has one of these five types.
enum class CardType { Bomb, Reinforcement, Blockade, Airlift, Diplomacy };

// A single card. The deck or hand containing it owns it.
class Card {
private:
    CardType type;
public:
    Card(CardType type);
    Card(const Card& other);
    Card& operator=(const Card& other);
    ~Card();
    // Create an order and move this card from the hand to the deck.
    bool play(Hand& hand, Deck& deck, OrdersList& orders);
    friend std::ostream& operator<<(std::ostream& out, const Card& card);
};

// A small deck with room for five cards, initially one of every type.
class Deck {
private:
    Card* cards[5];
    int count;
public:
    Deck();
    Deck(const Deck& other);
    Deck& operator=(const Deck& other);
    ~Deck();
    // Move a random card into the hand. Return false if transfer is impossible.
    bool draw(Hand& hand);
    int size() const;
    friend class Card;
    friend std::ostream& operator<<(std::ostream& out, const Deck& deck);
};

// A player's hand has room for five cards and starts empty.
class Hand {
private:
    Card* cards[5]; // Array of 5 cards pointers 
    int count;
public:
    Hand();
    Hand(const Hand& other); //Copy constructor
    Hand& operator=(const Hand& other);
    ~Hand();
    int size() const;
    // Return a borrowed pointer, or nullptr if the index is invalid.
    Card* getCard(int index) const;
    friend class Deck; //Gives the permission to access members of the class Hand to the class Deck
    friend class Card;
    friend std::ostream& operator<<(std::ostream& out, const Hand& hand);
};

#endif


//Bro code 
// Pointer is a variable that store a memory address of another variable.
//Example of the pizza, instead of taking the pizza just tell the address of the pizza to the person.
// & is the address of operator, it gives the address of the variable.
// * is the dereference operator, it gives the value at the address stored in the pointer variable.

// std:string name = "Bro"
// std::string *pname = &name

//std::cout << pname; this will print the memory address 
//std::cout << *pname; this will print bro 

//int age  = 12 
//int *page = &age;
//std::count *page; // this will print 12

//An array is already an Address 