#include "GameEngine.h"
#include "Cards.h"
#include "Map.h"
#include "Orders.h"
#include "Player.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace
{
int failures = 0;

void check(bool condition, const std::string &description)
{
    std::cout << (condition ? "[PASS] " : "[FAIL] ") << description << '\n';
    if (!condition)
        ++failures;
}

// Test observer. Each order-list notification records the player whose
// turn it was; the engine test uses this to verify round-robin issuing.
class Recorder : public Observer
{
public:
    Recorder(char playerLabel, std::vector<char> *events)
        : label(playerLabel), events(events) {}

    void Update(const ILoggable *object) override
    {
        if (object && events)
            events->push_back(label);
    }

private:
    char label;
    std::vector<char> *events; // Borrowed test log.
};

void reachGameplay(GameEngine &engine)
{
    check(engine.transition("loadmap test.map"), "loadmap accepted");
    check(engine.transition("validatemap"), "validatemap accepted");
    check(engine.transition("addplayer Alice"), "first addplayer accepted");
    check(engine.transition("addplayer Bob"), "second addplayer accepted");
    check(engine.transition("gamestart"), "gamestart accepted");
    check(engine.getState() == GameState::AssignReinforcement,
          "reached reinforcement state");
}

int countDeploys(const Player &player)
{
    int count = 0;
    OrdersList *orders = player.getOrdersList();
    for (int i = 0; i < orders->size(); ++i)
    {
        if (dynamic_cast<Deploy *>(orders->getOrder(i)))
            ++count;
    }
    return count;
}

bool containsAdvance(const Player &player)
{
    OrdersList *orders = player.getOrdersList();
    for (int i = 0; i < orders->size(); ++i)
    {
        if (dynamic_cast<Advance *>(orders->getOrder(i)))
            return true;
    }
    return false;
}

void testStatesAndLogging()
{
    std::cout << "\n=== A2 states, copy operations, observer ===\n";
    GameEngine engine;
    std::vector<char> notifications;
    Recorder logger('S', &notifications);
    engine.Attach(&logger);

    check(!engine.transition("quit"), "reject quit from start");
    check(notifications.empty(), "invalid command is not logged");
    reachGameplay(engine);
    check(notifications.size() == 4, "notify only on state changes");
    check(!engine.transition("assigncountries"), "reject obsolete A1 event");

    GameEngine copy(engine);
    check(copy.getState() == engine.getState(), "copy constructor preserves state");
    GameEngine assigned;
    assigned = engine;
    GameEngine &alias = assigned;
    assigned = alias;
    check(assigned.getState() == engine.getState(), "assignment and self-assignment");
    check(engine.transition("issueorder"), "begin issuing orders");
    check(engine.transition("endissueorders"), "begin executing orders");
    check(engine.transition("win"), "transition to win");
    check(engine.transition("replay"), "replay returns to start");
    check(engine.getState() == GameState::Start, "replay resets state");

    engine.Detach(&logger);
}

void testReinforcementCases()
{
    std::cout << "\n=== Reinforcements: minimum, territories, continents ===\n";
    Map map;
    Continent *north = map.addContinent("North", 5);
    Continent *south = map.addContinent("South", 2);
    Player alice("Alice");
    Player bob("Bob");
    for (int i = 0; i < 3; ++i)
    {
        Territory *t = map.addTerritory("North" + std::to_string(i));
        map.assignToContinent(t, north);
        alice.addTerritory(t);
    }
    Territory *last = map.addTerritory("South0");
    map.assignToContinent(last, south);
    bob.addTerritory(last);
    GameEngine engine;
    engine.setMap(&map);
    engine.setPlayers({&alice, &bob});
    reachGameplay(engine);
    engine.reinforcementPhase();
    check(*alice.getReinforcementsPool() == 8,
          "3 territories: min 3 + North bonus 5 = 8");
    check(*bob.getReinforcementsPool() == 5,
          "1 territory: min 3 + South bonus 2 = 5");
    check(engine.getTurnNumber() == 1, "reinforcement advances turn number");

    Map map12;
    Continent *zeroBonus = map12.addContinent("Zero", 0);
    Player twelve("Twelve");
    Player eliminated("Eliminated");
    for (int i = 0; i < 12; ++i)
    {
        Territory *t = map12.addTerritory("T" + std::to_string(i));
        map12.assignToContinent(t, zeroBonus);
        twelve.addTerritory(t);
    }
    GameEngine engine12;
    engine12.setMap(&map12);
    engine12.setPlayers({&twelve, &eliminated});
    reachGameplay(engine12);
    engine12.reinforcementPhase();
    check(*twelve.getReinforcementsPool() == 4,
          "12 territories: 12 / 3 = 4 armies");
    check(*eliminated.getReinforcementsPool() == 0,
          "player with no territory receives no armies");
}

void testIssueAndExecute()
{
    std::cout << "\n=== Round-robin issue, Deploy-first execution ===\n";
    Map map;
    Continent *north = map.addContinent("North", 5);
    Continent *south = map.addContinent("South", 2);
    Territory *a = map.addTerritory("A");
    Territory *b = map.addTerritory("B");
    Territory *c = map.addTerritory("C");
    Territory *d = map.addTerritory("D");
    for (Territory *t : {a, b, c})
        map.assignToContinent(t, north);
    map.assignToContinent(d, south);
    map.addEdge(a, b);
    map.addEdge(b, c);
    map.addEdge(c, d);
    map.addEdge(a, d);

    Player alice("Alice");
    Player bob("Bob");
    alice.addTerritory(a);
    alice.addTerritory(b);
    alice.addTerritory(c);
    bob.addTerritory(d);
    a->setArmies(7);
    b->setArmies(3);
    c->setArmies(2);
    d->setArmies(75);

    Deck deck;
    GameEngine engine;
    engine.setMap(&map);
    engine.setDeck(&deck);
    engine.setPlayers({&alice, &bob});

    std::vector<char> issuedBy;
    Recorder aliceLogger('A', &issuedBy);
    Recorder bobLogger('B', &issuedBy);
    alice.getOrdersList()->Attach(&aliceLogger);
    bob.getOrdersList()->Attach(&bobLogger);

    reachGameplay(engine);
    engine.reinforcementPhase();
    engine.issueOrdersPhase();
    check(engine.getState() == GameState::ExecuteOrders, "issuing phase completed");
    check(issuedBy.size() >= 4 && issuedBy[0] == 'A' && issuedBy[1] == 'B' &&
              issuedBy[2] == 'A' && issuedBy[3] == 'B',
          "first two issuing rounds alternate Alice/Bob");
    check(countDeploys(alice) >= 2 && countDeploys(bob) >= 2,
          "each player can issue multiple Deploy orders");
    check(alice.getAvailableArmies() == 0 && bob.getAvailableArmies() == 0,
          "all reinforcement armies committed before non-Deploy orders");
    check(containsAdvance(alice), "Player creates Advance using its priorities");
    check(*alice.getReinforcementsPool() == 8 &&
              *bob.getReinforcementsPool() == 5,
          "issuing reserves armies without deducting the pool");

    // Capture real execution output to check global Deploy-first ordering.
    std::ostringstream executionTrace;
    std::streambuf *previous = std::cout.rdbuf(executionTrace.rdbuf());
    engine.executeOrdersPhase();
    std::cout.rdbuf(previous);
    std::cout << executionTrace.str();
    const std::string trace = executionTrace.str();
    const std::size_t lastDeploy = trace.rfind("Executed: Deploy ");
    const std::size_t firstAdvance = trace.find("Executed: Advance ");
    check(lastDeploy != std::string::npos &&
              firstAdvance != std::string::npos && lastDeploy < firstAdvance,
          "every Deploy order executes before any Advance order");
    check(alice.getOrdersList()->size() == 0 &&
              bob.getOrdersList()->size() == 0,
          "execution drains both active players' order lists");
    check(*alice.getReinforcementsPool() == 0 &&
              *bob.getReinforcementsPool() == 0,
          "executed Deploy orders debit each reinforcement pool");
    check(engine.getState() == GameState::AssignReinforcement,
          "next turn begins when no player owns all territories");
}

void testCardIssuing()
{
    std::cout << "\n=== Player issues an order from a card ===\n";
    Map map;
    Continent *continent = map.addContinent("All", 0);
    Territory *a = map.addTerritory("A");
    Territory *b = map.addTerritory("B");
    Territory *c = map.addTerritory("C");
    for (Territory *t : {a, b, c})
        map.assignToContinent(t, continent);
    map.addEdge(a, b);
    map.addEdge(a, c);
    map.addEdge(b, c);
    Player alice("Alice");
    Player bob("Bob");
    alice.addTerritory(a);
    alice.addTerritory(b);
    bob.addTerritory(c);
    a->setArmies(5);
    b->setArmies(3);
    c->setArmies(3);

    Deck deck;
    check(deck.draw(*alice.getHand()), "draw card into Alice's hand");
    alice.resetTurnState();
    bool eventuallyDone = false;
    for (int i = 0; i < 50; ++i)
    {
        if (!alice.issueOrder(&deck))
        {
            eventuallyDone = true;
            break;
        }
    }
    check(eventuallyDone, "Player finishes issuing in a bounded number of calls");
    check(alice.getHand()->size() == 0, "player plays its card");
    check(deck.size() == 25, "played card returned to the deck");
}

void testEliminationAndCardAward()
{
    std::cout << "\n=== Real conquest, elimination, card award, win ===\n";
    Order::clearConquests();
    Map map;
    Continent *continent = map.addContinent("Pair", 0);
    Territory *a = map.addTerritory("A");
    Territory *b = map.addTerritory("B");
    map.assignToContinent(a, continent);
    map.assignToContinent(b, continent);
    map.addEdge(a, b);
    Player alice("Alice");
    Player bob("Bob");
    alice.addTerritory(a);
    bob.addTerritory(b);
    a->setArmies(3);
    b->setArmies(0); // Guarantees a conquest: no defending armies can fire.
    Deck deck;
    GameEngine engine;
    engine.setMap(&map);
    engine.setDeck(&deck);
    engine.setPlayers({&alice, &bob});
    reachGameplay(engine);
    alice.getOrdersList()->addOrder(new Advance(&alice, a, b, 1));
    engine.transition("issueorder");
    engine.transition("endissueorders");
    engine.executeOrdersPhase();
    check(b->getOwner() == &alice, "actual order execution changes ownership");
    check(engine.getActivePlayers().size() == 1, "eliminate player with zero territories");
    check(engine.checkWinner() == &alice && engine.getState() == GameState::Win,
          "engine declares victory when one player owns every territory");
    check(alice.getHand()->size() == 1 && deck.size() == 24,
          "conquering player gets one card at end of turn");
}

void testMainGameLoopStopsOnWin()
{
    std::cout << "\n=== mainGameLoop terminates on victory ===\n";
    Map map;
    Continent *continent = map.addContinent("One", 0);
    Territory *a = map.addTerritory("A");
    map.assignToContinent(a, continent);
    Player alice("Alice");
    alice.addTerritory(a);
    a->setArmies(2);
    Deck deck;
    GameEngine engine;
    engine.setMap(&map);
    engine.setDeck(&deck);
    engine.setPlayers({&alice});
    reachGameplay(engine);
    // A terminal-state test fixture: only one player remains when loop starts.
    engine.mainGameLoop();
    check(engine.getState() == GameState::Win, "game loop stops on win");
    check(engine.getTurnNumber() == 1, "game loop required one turn");
}
} // namespace

int main()
{
    testStatesAndLogging();
    testReinforcementCases();
    testIssueAndExecute();
    testCardIssuing();
    testEliminationAndCardAward();
    testMainGameLoopStopsOnWin();
    std::cout << "\n" << (failures == 0 ? "ALL A2 TESTS PASSED" : "A2 TESTS FAILED")
              << " (" << failures << " failures)\n";
    return failures == 0 ? 0 : 1;
}
