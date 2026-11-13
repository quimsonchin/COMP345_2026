#include "Orders.h"
#include "Map.h"
#include "Player.h"
#include <iostream>
#include <string>
#include <algorithm>

namespace {
int passed = 0;
int failed = 0;
void check(bool condition, const std::string& description) {
    std::cout << (condition ? "[PASS] " : "[FAIL] ") << description << '\n';
    if (condition) ++passed; else ++failed;
}
void testOrder(Order& order, bool expected, const std::string& name) {
    check(order.validate() == expected, name + " validation");
    order.execute();
    check(order.wasExecuted() == expected, name + " execution result");
    std::cout << "       " << order << "\n       " << order.getEffect() << '\n';
}
}

int main() {
    Player p1, p2;
    int reinforcements = 12;
    // Build the SAME connected Westeros/Essos graph with teammate's Map API.
    Map gameMap;
    Continent* westeros = gameMap.addContinent("Westeros", 5);
    Continent* essos = gameMap.addContinent("Essos", 3);
    Territory* winterfell = gameMap.addTerritory("Winterfell");
    Territory* kingsLanding = gameMap.addTerritory("King's Landing");
    Territory* dorne = gameMap.addTerritory("Dorne");
    Territory* pentos = gameMap.addTerritory("Pentos");
    Territory* braavos = gameMap.addTerritory("Braavos");
    Territory* meereen = gameMap.addTerritory("Meereen");
    Territory* qarth = gameMap.addTerritory("Qarth");
    for (Territory* t : {winterfell, kingsLanding, dorne})
        gameMap.assignToContinent(t, westeros);
    for (Territory* t : {pentos, braavos, meereen, qarth})
        gameMap.assignToContinent(t, essos);
    gameMap.addEdge(winterfell, kingsLanding);
    gameMap.addEdge(kingsLanding, dorne);
    gameMap.addEdge(kingsLanding, pentos);
    gameMap.addEdge(pentos, braavos);
    gameMap.addEdge(pentos, meereen);
    gameMap.addEdge(meereen, qarth);
    check(gameMap.validate(), "Teammate's Map validates with Westeros/Essos");
   
    p1.addTerritory(winterfell);
    winterfell->setArmies(15);

    p2.addTerritory(kingsLanding);
    kingsLanding->setArmies(8);

    p2.addTerritory(dorne);
    dorne->setArmies(5);

    p2.addTerritory(pentos);
    pentos->setArmies(7);

    p2.addTerritory(braavos);
    braavos->setArmies(4);

    p2.addTerritory(meereen);
    meereen->setArmies(6);

    p1.addTerritory(qarth);
    qarth->setArmies(3);

    // Neutral territory outside the validated game map for invalid Bomb test.
    Territory theWall("The Wall");

    std::cout << "=== Six order types: valid and invalid tests ===\n";
    Deploy deploy(&p1, winterfell, 5, &reinforcements);
    Advance advance(&p1, winterfell, kingsLanding, 3);
    Bomb bomb(&p1, kingsLanding, winterfell);
    Blockade blockade(&p1, qarth);
    Airlift airlift(&p1, winterfell, qarth, 2); // not adjacent
    Negotiate negotiate(&p1, &p2);
    testOrder(deploy, true, "Deploy valid");
    check(winterfell->getArmies() == 20,
      "Deploy adds armies to territory");

    check(reinforcements == 7,
      "Deploy deducts reinforcement pool");
    check(advance.validate(),
      "Advance attack validation before battle");
    testOrder(bomb, true, "Bomb valid");
    check(kingsLanding->getArmies() == 4,
      "Bomb removes half of enemy armies");
    //move airlift to before blockade
    testOrder(airlift, true, "Airlift valid (nonadjacent)");
    check(winterfell->getArmies() == 18,
      "Airlift removes armies from source");

    check(qarth->getArmies() == 5,
      "Airlift adds armies to target");
    testOrder(blockade, true, "Blockade valid");
    check(qarth->getArmies() == 10,
      "Blockade doubles armies");

    check(qarth->getOwner() != &p1 &&
      qarth->getOwner() != nullptr,
      "Blockade transfers territory to Neutral");
    
    testOrder(negotiate, true, "Negotiate valid");
    check(Order::areNegotiating(&p1, &p2),
      "Diplomacy registered");

    Advance blockedAttack(
    &p1, winterfell, kingsLanding, 1
    );

    check(!blockedAttack.validate(),
      "Negotiation prevents attack");

    Order::clearNegotiations();

    check(blockedAttack.validate(),
      "Attack allowed after turn reset");
    Deploy invalidDeploy(&p1, kingsLanding, 5, &reinforcements);
    Deploy overBudget(&p1, winterfell, 50, &reinforcements);
    Advance invalidAdvance(&p1, winterfell, kingsLanding, 100);
    Advance notAdjacent(&p1, winterfell, qarth, 2);
    Bomb invalidBomb(&p1, dorne, winterfell);
    Bomb neutralBomb(&p1, &theWall, winterfell);
    Blockade invalidBlockade(&p1, kingsLanding);
    Airlift invalidAirlift(&p1, kingsLanding, qarth, 2);
    Negotiate invalidNegotiate(&p1, &p1);
    testOrder(invalidDeploy, false, "Deploy enemy territory");
    testOrder(overBudget, false, "Deploy exceeds pool");
    testOrder(invalidAdvance, false, "Advance exceeds source armies");
    testOrder(notAdjacent, false, "Advance nonadjacent");
    testOrder(invalidBomb, false, "Bomb nonadjacent enemy");
    testOrder(neutralBomb, false, "Bomb neutral territory");
    testOrder(invalidBlockade, false, "Blockade enemy territory");
    testOrder(invalidAirlift, false, "Airlift enemy source");
    testOrder(invalidNegotiate, false, "Negotiate self");

    
    std::cout << "\n=== Advance Battle Tests ===\n";

    // TEST 1: Friendly movement
    // King's Landing and Pentos are adjacent and owned by p2.
    int sourceBefore = kingsLanding->getArmies();
    int targetBefore = pentos->getArmies();

    Advance friendlyMove(&p2, kingsLanding, pentos, 2);

    testOrder(friendlyMove, true, "Advance friendly");

    check(kingsLanding->getArmies() == sourceBefore - 2,
      "Friendly Advance deducts source armies");

    check(pentos->getArmies() == targetBefore + 2,
      "Friendly Advance increases target armies");


    // TEST 2: Guaranteed conquest
    // Zero defenders gives us a deterministic test.
    kingsLanding->setArmies(0);

    int winterfellBefore = winterfell->getArmies();

    Advance conquest(&p1, winterfell, kingsLanding, 3);

    testOrder(conquest, true, "Advance conquest");

    check(kingsLanding->getOwner() == &p1,
      "Conquest transfers territory ownership");

    check(kingsLanding->getArmies() == 3,
      "Surviving attackers occupy conquered territory");

    check(winterfell->getArmies() == winterfellBefore - 3,
      "Conquest removes armies from source");

    auto& p1Territories = *p1.getTerritories();
    auto& p2Territories = *p2.getTerritories();

    check(std::find(p1Territories.begin(),
                p1Territories.end(),
                kingsLanding) != p1Territories.end(),
      "Attacker territory list updated");

    check(std::find(p2Territories.begin(),
                p2Territories.end(),
                kingsLanding) == p2Territories.end(),
      "Defender territory list updated");

    std::cout << "\n=== Conquest Card Reward Tests ===\n";

    // Start with a fresh deck and an empty hand.
    Deck rewardDeck;

    int cardsBefore = p1.getHand()->size();
    int deckBefore = rewardDeck.size();

    // Player 1 has already conquered King's Landing.
    // Conquer another adjacent territory in the SAME turn.
    pentos->setArmies(0);

    Advance secondConquest(
        &p1, kingsLanding, pentos, 1
    );

    secondConquest.execute();

    check(pentos->getOwner() == &p1,
      "Second territory conquered in same turn");

    // End of turn: award cards.
    int awarded = Order::awardConquestCards(rewardDeck);

    check(awarded == 1,
      "Only one conquest card awarded per turn");

    check(p1.getHand()->size() == cardsBefore + 1,
      "Conquering player receives one card");

    check(rewardDeck.size() == deckBefore - 1,
      "One card removed from deck");

    // Calling again without a new conquest must give no card.
    int awardedAgain = Order::awardConquestCards(rewardDeck);

    check(awardedAgain == 0,
      "No duplicate card reward");

    check(p1.getHand()->size() == cardsBefore + 1,
      "Hand still contains only one reward card");
    
    std::cout << "\n=== Random Battle Test ===\n";

    // Dorne belongs to p2.
    // King's Landing belongs to p1.
    // They are adjacent in our map.

    dorne->setArmies(5);
    kingsLanding->setArmies(3);

    int sourceBeforeBattle = dorne->getArmies();

    Advance randomBattle(
        &p2, dorne, kingsLanding, 4 
    );

    testOrder(randomBattle, true, "Random battle");

    check(dorne->getArmies() >= 0 &&
      kingsLanding->getArmies() >= 0,
      "Battle never produces negative armies");

    check(dorne->getArmies() <= sourceBeforeBattle,
      "Battle does not create extra source armies");

    check(kingsLanding->getOwner() == &p1 ||
      kingsLanding->getOwner() == &p2,
      "Battle ownership remains valid");

    std::cout << "\n=== OrdersList: add, move, remove, copy ===\n";
    OrdersList list;
    list.addOrder(new Deploy(&p1, winterfell, 4, &reinforcements));
    list.addOrder(new Advance(&p1, winterfell, kingsLanding, 2));
    list.addOrder(new Bomb(&p1, kingsLanding, winterfell));
    list.addOrder(new Blockade(&p1, qarth));
    list.addOrder(new Airlift(&p1, winterfell, qarth, 2));
    list.addOrder(new Negotiate(&p1, &p2));
    check(list.size() == 6, "Six orders added sequentially");
    check(list.getOrder(-1) == nullptr && list.getOrder(99) == nullptr, "getOrder invalid indices");
    check(list.move(0, 5) && list.getOrder(5)->getDescription().find("Deploy") == 0,
          "move(0,5) moves Deploy to the end");
    check(!list.move(100, 0), "move rejects invalid index");
    check(list.remove(0) && list.size() == 5, "remove deletes first order");
    check(!list.remove(-1), "remove rejects invalid index");
    std::cout << list;

    OrdersList copied(list);
    check(copied.size() == list.size(), "Copy constructor preserves size");
    check(copied.getOrder(0) != list.getOrder(0), "Copy constructor deep-copies Order objects");
    check(copied.getOrder(0)->getDescription() == list.getOrder(0)->getDescription(),
          "Copied order has same description");
    copied.remove(0);
    check(copied.size() == list.size() - 1 && list.size() == 5,
          "Removing from copy does not modify original");

    OrdersList assigned;
    assigned.addOrder(new Negotiate(&p1, &p2));
    assigned = list;
    check(assigned.size() == list.size() && assigned.getOrder(0) != list.getOrder(0),
          "Assignment operator deep-copies and replaces previous contents");
    assigned = assigned;
    check(assigned.size() == list.size(), "Self-assignment safe");
    OrdersList empty, emptyCopy(empty);
    check(emptyCopy.size() == 0, "Copying an empty list works");
    std::cout << "\nResult: " << passed << " passed, " << failed << " failed.\n";
    return failed == 0 ? 0 : 1;
}

