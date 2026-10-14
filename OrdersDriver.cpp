#include "Orders.h"
#include "Map.h"
#include <iostream>
#include <string>

// TEST-ONLY player identity. Remove this when real Player.h becomes available.
// This defines no teammate API; Orders.cpp never dereferences Player*.
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
    Territory canada("Canada"), usa("USA"), mexico("Mexico"), japan("Japan"), unowned("Unowned");
    canada.setOwner(&p1); canada.setArmies(15);
    usa.setOwner(&p2); usa.setArmies(8);
    mexico.setOwner(&p2); mexico.setArmies(5);
    japan.setOwner(&p1); japan.setArmies(3);
    canada.setAdjacent(&usa); usa.setAdjacent(&canada);
    usa.setAdjacent(&mexico); mexico.setAdjacent(&usa);

    std::cout << "=== Six order types: valid and invalid tests ===\n";
    Deploy deploy(&p1, &canada, 5, &reinforcements);
    Advance advance(&p1, &canada, &usa, 3);
    Bomb bomb(&p1, &usa, &canada);
    Blockade blockade(&p1, &japan);
    Airlift airlift(&p1, &canada, &japan, 2); // not adjacent
    Negotiate negotiate(&p1, &p2);
    testOrder(deploy, true, "Deploy valid");
    testOrder(advance, true, "Advance valid");
    testOrder(bomb, true, "Bomb valid");
    testOrder(blockade, true, "Blockade valid");
    testOrder(airlift, true, "Airlift valid (nonadjacent)");
    testOrder(negotiate, true, "Negotiate valid");

    Deploy invalidDeploy(&p1, &usa, 5, &reinforcements);
    Deploy overBudget(&p1, &canada, 50, &reinforcements);
    Advance invalidAdvance(&p1, &canada, &usa, 100);
    Advance notAdjacent(&p1, &canada, &japan, 2);
    Bomb invalidBomb(&p1, &mexico, &canada);
    Bomb neutralBomb(&p1, &unowned, &canada);
    Blockade invalidBlockade(&p1, &usa);
    Airlift invalidAirlift(&p1, &usa, &japan, 2);
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
    list.addOrder(new Deploy(&p1, &canada, 4, &reinforcements));
    list.addOrder(new Advance(&p1, &canada, &usa, 2));
    list.addOrder(new Bomb(&p1, &usa, &canada));
    list.addOrder(new Blockade(&p1, &japan));
    list.addOrder(new Airlift(&p1, &canada, &japan, 2));
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
