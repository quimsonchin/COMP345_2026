#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

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
    void startupPhase(); //runs loadmap, validatemap, addplayer and gamestart.
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

    Deck *deck; // the deck players draw their starting cards from (owned by the engine)
    std::string handleStartupCommand(const std::string &commandText); // reads one command and runs it
    std::string loadMapCommand(const std::string &fileName);          // loads a map file
    std::string validateMapCommand();                                  // checks the loaded map is valid
    std::string addPlayerCommand(const std::string &playerName);      // adds one player to the game
    std::string gameStartCommand();                                    // sets up the game and starts play
    bool isCommandAvailable(const std::string &command) const;         // true if the command is allowed now
    void listMapFiles() const;                                         // prints the maps the user can load
    int countOwnedTerritories(const Player *player) const;
    int continentBonus(const Player *player) const;
    bool executeNextOrder(Player *player, bool deployOnly);
};

#endif
