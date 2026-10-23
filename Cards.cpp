#include "Cards.h"
#include "Orders.h"
#include <random>

Card::Card(CardType type) { this->type = type; }
Card::Card(const Card& other) { type = other.type; }
Card& Card::operator=(const Card& other) {
    type = other.type;
    return *this;
}
Card::~Card() {}

std::ostream& operator<<(std::ostream& out, const Card& card) {
    switch (card.type) {
        case CardType::Bomb: out << "bomb"; break;
        case CardType::Reinforcement: out << "reinforcement"; break;
        case CardType::Blockade: out << "blockade"; break;
        case CardType::Airlift: out << "airlift"; break;
        case CardType::Diplomacy: out << "diplomacy"; break;
    }
    return out;
}

bool Card::play(Hand& hand, Deck& deck, OrdersList& orders) {
    int position = -1;
    for (int i = 0; i < hand.count; i++) {
        if (hand.cards[i] == this) {
            position = i;
            break;
        }
    }
    if (position == -1 || deck.count == 5) {
        return false;
    }

    Order* order = nullptr;
    switch (type) {
        case CardType::Bomb: order = new Bomb(nullptr, nullptr, nullptr); break;
        // Explicit demo assumption: reinforcement creates a Deploy order.
        case CardType::Reinforcement: order = new Deploy(nullptr, nullptr, 0, nullptr); break;
        case CardType::Blockade: order = new Blockade(nullptr, nullptr); break;
        case CardType::Airlift: order = new Airlift(nullptr, nullptr, nullptr, 0); break;
        case CardType::Diplomacy: order = new Negotiate(nullptr, nullptr); break;
    }
    if (order == nullptr) {
        return false;
    }
    try {
        orders.addOrder(order); // The list now owns the order.
    } catch (...) {
        delete order; 
        throw;
    }

    deck.cards[deck.count] = this;
    deck.count++;

    // Close the gap left in the hand by shifting later pointers left.
    for (int i = position; i < hand.count - 1; i++) {
        hand.cards[i] = hand.cards[i + 1];
    }
    hand.count--;
    hand.cards[hand.count] = nullptr;
    return true;
}

Deck::Deck() : cards{}, count(0) {
    try {
        cards[count++] = new Card(CardType::Bomb);
        cards[count++] = new Card(CardType::Reinforcement);
        cards[count++] = new Card(CardType::Blockade);
        cards[count++] = new Card(CardType::Airlift);
        cards[count++] = new Card(CardType::Diplomacy);
    } catch (...) {
        for (int i = 0; i < count; i++) { delete cards[i]; }
        throw;
    }
}

Deck::Deck(const Deck& other) : cards{}, count(0) {
    try {
        for (int i = 0; i < other.count; i++) {
            cards[i] = new Card(*other.cards[i]);
            count++;
        }
    } catch (...) {
        for (int i = 0; i < count; i++) { delete cards[i]; }
        throw;
    }
}

Deck& Deck::operator=(const Deck& other) {
    if (this != &other) {
        Deck copy(other);
        for (int i = 0; i < 5; i++) {
            Card* temporary = cards[i];
            cards[i] = copy.cards[i];
            copy.cards[i] = temporary;
        }
        int temporaryCount = count;
        count = copy.count;
        copy.count = temporaryCount;
    }
    return *this;
}

Deck::~Deck() {
    for (int i = 0; i < count; i++) { delete cards[i]; }
}

int Deck::size() const { return count; }

std::ostream& operator<<(std::ostream& out, const Deck& object) {
    out << "Deck (" << object.count << "): ";
    for (int i = 0; i < object.count; i++) {
        out << *object.cards[i] << ' ';
    }
    return out;
}

Hand::Hand() : cards{}, count(0) {}

Hand::Hand(const Hand& other) : cards{}, count(0) {
    try {
        for (int i = 0; i < other.count; i++) {
            cards[i] = new Card(*other.cards[i]);
            count++;
        }
    } catch (...) {
        for (int i = 0; i < count; i++) { delete cards[i]; }
        throw;
    }
}

Hand& Hand::operator=(const Hand& other) {
    if (this != &other) {
        Hand copy(other);
        for (int i = 0; i < 5; i++) {
            Card* temporary = cards[i];
            cards[i] = copy.cards[i];
            copy.cards[i] = temporary;
        }
        int temporaryCount = count;
        count = copy.count;
        copy.count = temporaryCount;
    }
    return *this;
}

Hand::~Hand() {
    for (int i = 0; i < count; i++) { delete cards[i]; }
}

int Hand::size() const { return count; }

std::ostream& operator<<(std::ostream& out, const Hand& object) {
    out << "Hand (" << object.count << "): ";
    for (int i = 0; i < object.count; i++) {
        out << *object.cards[i] << ' ';
    }
    return out;
}

bool Deck::draw(Hand& hand) {
    if (count == 0 || hand.count == 5) { //count is from the Deck variable
        return false;
    }
    static std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<int> randomIndex(0, count - 1);
    int index = randomIndex(generator);

    hand.cards[hand.count] = cards[index];
    hand.count++;

    cards[index] = cards[count - 1];
    count--;
    cards[count] = nullptr;
    return true;
}

Card* Hand::getCard(int index) const {
    if (index < 0 || index >= count) {
        return nullptr;
    }
    return cards[index];
}
