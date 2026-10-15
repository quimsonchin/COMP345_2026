#include "Orders.h"
#include "Map.h"
#include <iostream>
#include <string>

// TEST-ONLY player identity. Remove this when real Player.h becomes available.

class Player {};

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
    winterfell->setOwner(&p1); winterfell->setArmies(15);
    kingsLanding->setOwner(&p2); kingsLanding->setArmies(8);
    dorne->setOwner(&p2); dorne->setArmies(5);
    pentos->setOwner(&p2); pentos->setArmies(7);
    braavos->setOwner(&p2); braavos->setArmies(4);
    meereen->setOwner(&p2); meereen->setArmies(6);
    qarth->setOwner(&p1); qarth->setArmies(3);
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
    testOrder(advance, true, "Advance valid");
    testOrder(bomb, true, "Bomb valid");
    testOrder(blockade, true, "Blockade valid");
    testOrder(airlift, true, "Airlift valid (nonadjacent)");
    testOrder(negotiate, true, "Negotiate valid");

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

