#ifndef CARDS_H
#define CARDS_H

#include <iostream>

class Hand;
class Deck;
class OrdersList;

// The five types of cards.
enum class CardType { Bomb, Reinforcement, Blockade, Airlift, Diplomacy };

// Represents a single card.
class Card {
private:
    CardType type;

public:
    // Create a card with a given type.
    Card(CardType type);

    // Copy another card.
    Card(const Card& other);

    // Assign another card.
    Card& operator=(const Card& other);

    // Destroy the card.
    ~Card();

    // Play the card and return it to the deck.
    bool play(Hand& hand, Deck& deck, OrdersList& orders);

    // Print the card type.
    friend std::ostream& operator<<(std::ostream& out, const Card& card);
};

// Stores the cards available to draw.
class Deck {
private:
    Card* cards[5];
    int count;

public:
    // Create the starting deck.
    Deck();

    // Copy another deck.
    Deck(const Deck& other);

    // Assign another deck.
    Deck& operator=(const Deck& other);

    // Delete the cards in the deck.
    ~Deck();

    // Draw a random card into the hand.
    bool draw(Hand& hand);

    // Return the number of cards.
    int size() const;

    // Allow cards to access the deck.
    friend class Card;

    // Print the deck.
    friend std::ostream& operator<<(std::ostream& out, const Deck& deck);
};

// Stores the cards held by a player.
class Hand {
private:
    Card* cards[5];
    int count;

public:
    // Create an empty hand.
    Hand();

    // Copy another hand.
    Hand(const Hand& other);

    // Assign another hand.
    Hand& operator=(const Hand& other);

    // Delete the cards in the hand.
    ~Hand();

    // Return the number of cards.
    int size() const;

    // Get a card by its index.
    Card* getCard(int index) const;

    // Allow the deck and cards to access the hand.
    friend class Deck;
    friend class Card;

    // Print the hand.
    friend std::ostream& operator<<(std::ostream& out, const Hand& hand);
};

#endif