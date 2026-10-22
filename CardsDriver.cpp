#include "Cards.h"
#include "Orders.h"
#include <iostream>

// Demonstrate all five cards using the real OrdersList, without Player objects.
int main() {
    Deck deck;
    Hand hand;
    OrdersList orders;

    std::cout << "Before drawing:\n" << deck << '\n' << hand << "\n\n";

    // The default deck has one of every card type. Draw all five.
    while (deck.draw(hand)) {
        std::cout << "Drew a card.\n";
    }
    std::cout << "\nAfter drawing:\n" << deck << '\n' << hand << "\n\n";
    if (deck.size() != 0 || hand.size() != 5) {
        std::cerr << "Drawing failed.\n";
        return 1;
    }

    // Playing removes the first card, so the next card moves into slot zero.
    while (hand.size() > 0) {
        Card* card = hand.getCard(0);
        std::cout << "Playing " << *card << '\n';
        if (!card->play(hand, deck, orders)) {
            std::cerr << "Playing failed.\n";
            return 1;
        }
    }

    std::cout << "\nAfter playing:\n" << deck << '\n' << hand << '\n' << orders;
    if (deck.size() != 5 || hand.size() != 0 || orders.size() != 5) {
        std::cerr << "Final counts are incorrect.\n";
        return 1;
    }

    // These are real orders with missing game inputs, so validation rejects them.
    std::cout << "\nOrders have no player/territory inputs in this standalone demo.\n";
    for (int i = 0; i < orders.size(); i++) {
        Order* order = orders.getOrder(i);
        if (order->validate()) {
            std::cerr << "An order without game inputs unexpectedly validated.\n";
            return 1;
        }
        std::cout << "Order " << i + 1 << ": invalid, as expected.\n";
    }
    std::cout << "\nCard demonstration passed.\n";
    return 0;
}
