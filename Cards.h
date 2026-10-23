#ifndef CARDS_H
#define CARDS_H

#include <iostream>

class Hand;
class Deck;
class OrdersList;

enum class CardType { Bomb, Reinforcement, Blockade, Airlift, Diplomacy };

class Card {
private:
    CardType type;
public:
    Card(CardType type);
    Card(const Card& other);
    Card& operator=(const Card& other);
    ~Card();
    bool play(Hand& hand, Deck& deck, OrdersList& orders);
    friend std::ostream& operator<<(std::ostream& out, const Card& card);
};

class Deck {
private:
    Card* cards[5];
    int count;
public:
    Deck();
    Deck(const Deck& other);
    Deck& operator=(const Deck& other);
    ~Deck();
    bool draw(Hand& hand);
    int size() const;
    friend class Card;
    friend std::ostream& operator<<(std::ostream& out, const Deck& deck);
};

class Hand {
private:
    Card* cards[5];
    int count;
public:
    Hand();
    Hand(const Hand& other); 
    Hand& operator=(const Hand& other);
    ~Hand();
    int size() const;
    Card* getCard(int index) const;
    friend class Deck; //Gives the permission to access members of the class Hand to the class Deck
    friend class Card;
    friend std::ostream& operator<<(std::ostream& out, const Hand& hand);
};

#endif

