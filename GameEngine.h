#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>
#include "LoggingObserver.h"

class Map;
class Player;
class Deck;

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
class GameEngine : public Subject, public ILoggable
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
    std::string stringToLog() const override;

    // Kaila supplies the map and the list of Player objects after startup.
    // They must outlive the GameEngine and its game loop.
    void setMap(Map *map);
    void setDeck(Deck *deck);
    void setPlayers(const std::vector<Player *> &players);
    const std::vector<Player *> &getActivePlayers() const;

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
    Deck *deck;                           // borrowed, do not delete
    std::vector<Player *> *activePlayers; // owned vector; borrowed elements
    unsigned int turnNumber;

    int countOwnedTerritories(const Player *player) const;
    int continentBonus(const Player *player) const;
    bool executeNextOrder(Player *player, bool deployOnly);
};

#endif
