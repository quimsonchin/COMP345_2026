#include "GameEngine.h"
#include "Map.h"
#include "Orders.h"
#include "Player.h"

#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace
{
      int failures = 0;
      std::map<Player *, int> calls;

      void check(bool condition, const std::string &explanation)
      {
            std::cout << (condition ? "[PASS] " : "[FAIL] ") << explanation << '\n';
            if (!condition)
                  ++failures;
      }

      bool step(GameEngine &engine, const std::string &event, GameState expected)
      {
            const bool accepted = engine.transition(event);
            check(accepted && engine.getState() == expected,
                  event + " -> " + engine.getStateName());
            return accepted;
      }

      // Demonstrates the CURRENT A1 issueOrder(string) integration seam.
      // At most one order is issued per player, then the player says "done".
      bool issueOneForDemo(Player &player)
      {
            if (calls[&player]++ != 0)
                  return false;
            const int before = player.getOrdersList()->size();
            player.issueOrder("deploy");
            return player.getOrdersList()->size() > before;
      }

      void testStateMachine()
      {
            std::cout << "\n=== A2 states, valid/invalid events, Rule of Three ===\n";
            GameEngine engine;
            check(engine.getState() == GameState::Start, "initial state: start");
            check(!engine.transition("gamestart"), "reject gamestart from start");
            step(engine, "loadmap world.map", GameState::MapLoaded);
            step(engine, "loadmap other.map", GameState::MapLoaded);
            step(engine, "validatemap", GameState::MapValidated);
            step(engine, "addplayer Alice", GameState::PlayersAdded);
            step(engine, "addplayer Bob", GameState::PlayersAdded);
            check(!engine.transition("assigncountries"),
                  "reject obsolete A1 assigncountries event");

            GameEngine copy(engine);
            check(copy.getState() == engine.getState(), "copy constructor");
            GameEngine assigned;
            assigned = engine;
            check(assigned.getState() == engine.getState(), "copy assignment");
            GameEngine &alias = assigned;
            assigned = alias;
            check(assigned.getState() == engine.getState(), "self assignment");

            step(engine, "gamestart", GameState::AssignReinforcement);
            step(engine, "issueorder", GameState::IssueOrders);
            step(engine, "endissueorders", GameState::ExecuteOrders);
            step(engine, "execorder", GameState::ExecuteOrders);
            step(engine, "endexecorders", GameState::AssignReinforcement);
            step(engine, "issueorder", GameState::IssueOrders);
            step(engine, "endissueorders", GameState::ExecuteOrders);
            step(engine, "win", GameState::Win);
            step(engine, "replay", GameState::Start);
            step(engine, "loadmap", GameState::MapLoaded);
            step(engine, "validatemap", GameState::MapValidated);
            step(engine, "addplayer", GameState::PlayersAdded);
            step(engine, "gamestart", GameState::AssignReinforcement);
            step(engine, "issueorder", GameState::IssueOrders);
            step(engine, "endissueorders", GameState::ExecuteOrders);
            step(engine, "win", GameState::Win);
            step(engine, "quit", GameState::End);
            check(engine.isFinished(), "quit reaches terminal state");
            check(!engine.transition("replay"), "terminal state rejects commands");
      }

      void testReinforcementBoundaries()
      {
            std::cout << "\n=== Reinforcement integer division boundary ===\n";
            Map map;
            Continent *neutral = map.addContinent("NoBonus", 0);
            Player alice;
            Player bob;
            for (int i = 0; i < 12; ++i)
            {
                  Territory *territory = map.addTerritory("T" + std::to_string(i));
                  map.assignToContinent(territory, neutral);
                  alice.addTerritory(territory);
            }
            GameEngine engine;
            engine.setMap(&map);
            engine.setPlayers(std::vector<Player *>{&alice, &bob});
            engine.transition("loadmap");
            engine.transition("validatemap");
            engine.transition("addplayer");
            engine.transition("gamestart");
            engine.reinforcementPhase();
            check(*alice.getReinforcementsPool() == 4,
                  "12 / 3 = 4, with zero continent bonus");
            check(*bob.getReinforcementsPool() == 0,
                  "player with zero territories receives zero reinforcements");
      }

      void testPhases()
      {
            std::cout << "\n=== Real A1 Map/Player/Orders interface: phase tests ===\n";
            Map map;
            Continent *north = map.addContinent("North", 5);
            Continent *south = map.addContinent("South", 2);
            Territory *a = map.addTerritory("A");
            Territory *b = map.addTerritory("B");
            Territory *c = map.addTerritory("C");
            Territory *d = map.addTerritory("D");
            map.assignToContinent(a, north);
            map.assignToContinent(b, north);
            map.assignToContinent(c, north);
            map.assignToContinent(d, south);
            map.addEdge(a, b);
            map.addEdge(b, c);
            map.addEdge(c, d);
            map.addEdge(a, d);
            Player alice;
            Player bob;
            alice.addTerritory(a);
            alice.addTerritory(b);
            alice.addTerritory(c);
            bob.addTerritory(d);
            a->setArmies(2);

            GameEngine engine;
            engine.setMap(&map);
            engine.setPlayers(std::vector<Player *>{&alice, &bob});
            engine.setIssueOrderCallback(&issueOneForDemo);
            check(engine.getActivePlayers().size() == 2, "two initial players");

            engine.transition("loadmap");
            engine.transition("validatemap");
            engine.transition("addplayer");
            engine.transition("addplayer");
            engine.transition("gamestart");

            engine.reinforcementPhase();
            check(*alice.getReinforcementsPool() == 8,
                  "Alice: max(3, 3/3) + North bonus 5 = 8");
            check(*bob.getReinforcementsPool() == 5,
                  "Bob: max(3, 1/3) + South bonus 2 = 5");

            // Advance is put FIRST, but must execute AFTER every Deploy.
            alice.getOrdersList()->addOrder(new Advance(&alice, a, d, 1));
            calls.clear();
            engine.issueOrdersPhase();
            check(engine.getState() == GameState::ExecuteOrders,
                  "issue phase ends in executeorders state");
            check(alice.getOrdersList()->size() == 2,
                  "Alice has Advance + Deploy before execution");
            check(bob.getOrdersList()->size() == 1,
                  "Bob has Deploy before execution");

            std::cout << "Execution order below must show Deploy, Deploy, Advance:\n";
            engine.executeOrdersPhase();
            check(alice.getOrdersList()->size() == 0 &&
                      bob.getOrdersList()->size() == 0,
                  "execution clears both order lists");
            check(engine.getState() == GameState::AssignReinforcement,
                  "without winner, engine returns to reinforcement state");

            // Simulate a completed conquest (actual A2 Advance::execute will do it).
            d->setOwner(&alice);
            engine.removeEliminatedPlayers();
            check(engine.getActivePlayers().size() == 1,
                  "defeated Bob removed from active players");
            check(engine.checkWinner() == &alice,
                  "Alice wins when she owns every territory");
            check(engine.stringToLog().find("assignreinforcement") != std::string::npos,
                  "state can be represented as a log entry");
      }
} // namespace

// Build with A1 Map/Player/Orders/Cards source files, NOT the other drivers.
// Note: A1 Order::execute still defers actual world changes; test the full mainGameLoop only after Part 4 integration.
int main()
{
      testStateMachine();
      testReinforcementBoundaries();
      testPhases();
      std::cout << "\n"
                << (failures == 0 ? "ALL STARTER TESTS PASSED" : "STARTER TESTS FAILED") << " (" << failures << " failures)\n";
      return failures == 0 ? 0 : 1;
}
