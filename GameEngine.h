#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

class Map;
class Player;

// Exact A2 states ("End" represents quitting the program).
enum class GameState
{
    Start,
    MapLoaded,
    MapValidated,
    PlayersAdded,
    AssignReinforcement,
    IssueOrders,
    ExecuteOrders,
    Win,
    End
};

// A2 game controller.
class GameEngine
{
public:
    GameEngine();
    GameEngine(const GameEngine &other);
    GameEngine &operator=(const GameEngine &other);
    ~GameEngine();

    // Process a STATE EVENT, not the physical game action itself.
    // Kaila's startupPhase() should call this only AFTER a successful action.
    bool transition(const std::string &event);
    GameState getState() const;
    std::string getStateName() const;
    std::vector<std::string> getAvailableCommands() const;
    bool isFinished() const;
    std::string stringToLog() const;

    // Kaila supplies the map and the list of Player objects after startup.
    // They must outlive the GameEngine and its game loop.
    void setMap(Map *map);
    void setPlayers(const std::vector<Player *> &players);
    const std::vector<Player *> &getActivePlayers() const;

    // TEMPORARY A1->A2 integration seam. True: issued one order;
    // false: finished issuing orders for this turn.
    // Replace with direct Player::issueOrder() calls once Phuc finalizes
    // an API that reports when a player is done.
    void setIssueOrderCallback(bool (*issueOneOrder)(Player &));

    // A2, Part 3b
    void reinforcementPhase();
    void issueOrdersPhase();
    void executeOrdersPhase();
    void mainGameLoop();
    void removeEliminatedPlayers();
    Player *checkWinner() const;
    unsigned int getTurnNumber() const;

    friend std::ostream &operator<<(std::ostream &out,
                                    const GameEngine &engine);

private:
    GameState currentState;
    Map *map;                             // borrowed, do not delete
    std::vector<Player *> *activePlayers; // owned vector; borrowed elements
    bool (*issueOneOrder)(Player &);      // non-owning function pointer
    unsigned int turnNumber;

    int countOwnedTerritories(const Player *player) const;
    int continentBonus(const Player *player) const;
    bool executeNextOrder(Player *player, bool deployOnly);
};

#endif
