#include "GameEngine.h"
#include "Cards.h"
#include "Map.h"
#include "Player.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace
{
int failures = 0;
void check(bool okay, const std::string &message)
{
    std::cout << (okay ? "[PASS] " : "[FAIL] ") << message << '\n';
    if (!okay)
        ++failures;
}

class CountObserver : public Observer
{
public:
    int count = 0;
    void Update(const ILoggable *value) override
    {
        if (value)
            ++count;
    }
};

void testStartupToReinforcement()
{
    std::cout << "\n=== Interactive startup -> reinforcement ===\n";
    GameEngine engine;
    CountObserver observer;
    engine.Attach(&observer);
    std::istringstream commands(
        "gamestart\n"                    // invalid in Start
        "loadmap missing.map\n"         // missing file, stay Start
        "loadmap westeros_essos.map\n"  // valid
        "validatemap extra\n"          // invalid argument
        "validatemap\n"                // valid
        "addplayer Alice\n"            // valid
        "addplayer Alice\n"            // duplicate (reject)
        "gamestart\n"                  // too few players (reject)
        "addplayer Bob\n"              // valid
        "gamestart\n");                // valid
    std::ostringstream output;
    engine.startupPhase(commands, output);
    const std::string transcript = output.str();
    std::cout << transcript;

    check(engine.getState() == GameState::AssignReinforcement,
          "gamestart moves to assignreinforcement");
    check(transcript.find("Invalid command 'gamestart'") != std::string::npos,
          "reject gamestart before loading a map");
    check(transcript.find("Could not load map") != std::string::npos,
          "reject missing map without changing state");
    check(transcript.find("does not accept arguments") != std::string::npos,
          "reject extraneous command arguments");
    check(transcript.find("Duplicate player name") != std::string::npos,
          "reject duplicate player names");
    check(transcript.find("requires 2 to 6 players") != std::string::npos,
          "reject gamestart with one player");
    check(observer.count == 4, "notify exactly on 4 successful state changes");

    const std::vector<Player *> &players = engine.getActivePlayers();
    check(players.size() == 2, "exactly two players registered");
    if (players.size() == 2)
    {
        check(players[0]->getName() != players[1]->getName(),
              "players preserve distinct names");
        const std::size_t a = players[0]->getTerritories()->size();
        const std::size_t b = players[1]->getTerritories()->size();
        check(a + b == 7, "all seven test-map territories assigned");
        check(std::max(a, b) - std::min(a, b) <= 1,
              "fair territory distribution: difference at most one");
        for (Player *player : players)
        {
            check(*player->getReinforcementsPool() == 50,
                  player->getName() + " receives the initial 50 armies");
            check(player->getHand()->size() == 2,
                  player->getName() + " draws two cards");
        }
    }
    engine.reinforcementPhase();
    check(engine.getTurnNumber() == 1, "gameplay reinforcement phase runs after startup");
    engine.Detach(&observer);
}

void testReplayRestartsSafely()
{
    std::cout << "\n=== Replay and resource cleanup ===\n";
    GameEngine engine;
    std::ostringstream log;
    std::istringstream first(
        "loadmap westeros_essos.map\nvalidatemap\naddplayer One\naddplayer Two\ngamestart\n");
    engine.startupPhase(first, log);
    check(engine.getState() == GameState::AssignReinforcement,
          "first startup finishes");
    check(engine.transition("issueorder") &&
          engine.transition("endissueorders") && engine.transition("win") &&
          engine.transition("replay"), "replay from win is accepted");
    check(engine.getActivePlayers().empty() && engine.getTurnNumber() == 0,
          "replay resets players and turn number");
    std::istringstream second(
        "loadmap westeros_essos.map\nvalidatemap\naddplayer Three\naddplayer Four\ngamestart\n");
    engine.startupPhase(second, log);
    check(engine.getState() == GameState::AssignReinforcement &&
          engine.getActivePlayers().size() == 2,
          "second startup creates a fresh game");
}
} // namespace

int main()
{
    testStartupToReinforcement();
    testReplayRestartsSafely();
    std::cout << '\n' << (failures == 0 ? "ALL STARTUP TESTS PASSED" : "STARTUP TESTS FAILED")
              << " (" << failures << " failures)\n";
    return failures == 0 ? 0 : 1;
}
