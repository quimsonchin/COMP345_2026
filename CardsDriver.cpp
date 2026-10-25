#include "Cards.h"
#include "Orders.h"
#include <iostream>

int main() {
    // Create the deck, hand, and orders list.
    Deck deck;
    Hand hand;
    OrdersList orders;

    // Show the starting cards.
    std::cout << "Before drawing:\n" << deck << '\n' << hand << "\n\n";

    // Draw cards until the deck is empty.
    while (deck.draw(hand)) {
        std::cout << "Drew a card.\n";
    }
    std::cout << "\nAfter drawing:\n" << deck << '\n' << hand << "\n\n";

    // Check that all five cards were drawn.
    if (deck.size() != 0 || hand.size() != 5) {
        std::cerr << "Drawing failed.\n";
        return 1;
    }

    // Play every card in the hand.
    while (hand.size() > 0) {
        Card* card = hand.getCard(0);
        std::cout << "Playing " << *card << '\n';
        if (!card->play(hand, deck, orders)) {
            std::cerr << "Playing failed.\n";
            return 1;
        }
    }

    // Show the results after playing.
    std::cout << "\nAfter playing:\n" << deck << '\n' << hand << '\n' << orders;

    // Check the final card and order counts.
    if (deck.size() != 5 || hand.size() != 0 || orders.size() != 5) {
        std::cerr << "Final counts are incorrect.\n";
        return 1;
    }

    // Check that orders without game inputs are invalid.
    std::cout << "\nOrders have no player/territory inputs in this standalone demo.\n";
    for (int i = 0; i < orders.size(); i++) {
        Order* order = orders.getOrder(i);
        if (order->validate()) {
            std::cerr << "An order without game inputs unexpectedly validated.\n";
            return 1;
        }
        std::cout << "Order " << i + 1 << ": invalid, as expected.\n";
    }

    // All checks passed.
    std::cout << "\nCard demonstration passed.\n";
    return 0;
}