#include "GameEngine.h"
#include "Map.h"
#include "Orders.h"
#include "Player.h"
#include "Cards.h"      

#include <algorithm>
#include <filesystem>   
#include <random>       
#include <sstream>
#include <stdexcept>
#include <utility>

GameEngine::GameEngine()
    : currentState(GameState::Start), map(nullptr),
      activePlayers(new std::vector<Player *>()), issueOneOrder(nullptr),
      turnNumber(0),
      deck(new Deck()) {} // create the deck players draw their starting cards from

// Map and Player objects are borrowed, but the vector is deep-copied.
GameEngine::GameEngine(const GameEngine &other)
    : currentState(other.currentState), map(other.map),
      activePlayers(new std::vector<Player *>(*other.activePlayers)),
      issueOneOrder(other.issueOneOrder), turnNumber(other.turnNumber),
      deck(new Deck(*other.deck)) {} // deep copy: the copy gets its own deck

GameEngine &GameEngine::operator=(const GameEngine &other)
{
    if (this != &other)
    {
        std::vector<Player *> *replacement =
            new std::vector<Player *>(*other.activePlayers);
        delete activePlayers;
        activePlayers = replacement;
        currentState = other.currentState;
        map = other.map;
        issueOneOrder = other.issueOneOrder;
        turnNumber = other.turnNumber;

        Deck *newDeck = new Deck(*other.deck); // copy the other engine's deck first
        delete deck; // then free our old deck
        deck = newDeck; // and keep the copy
    }
    return *this;
}

GameEngine::~GameEngine()
{
    delete activePlayers; // Never delete a Player or Map you do not own.
    delete deck; // the engine owns the deck, so it frees it
}

// Events follow the A2 page-2 diagram. A2 renames three A1 events:
// assigncountries -> gamestart, play -> replay, end -> quit.
// Any arguments (e.g., "loadmap world.map") are processed by the startup
// controller; this function looks only at the event's first word.
bool GameEngine::transition(const std::string &event)
{
    std::istringstream input(event);
    std::string command;
    input >> command;
    GameState next = currentState;
    bool accepted = false;

    switch (currentState)
    {
    case GameState::Start:
        if (command == "loadmap")
        {
            next = GameState::MapLoaded;
            accepted = true;
        }
        break;
    case GameState::MapLoaded:
        if (command == "loadmap")
            accepted = true;
        else if (command == "validatemap")
        {
            next = GameState::MapValidated;
            accepted = true;
        }
        break;
    case GameState::MapValidated:
        if (command == "addplayer")
        {
            next = GameState::PlayersAdded;
            accepted = true;
        }
        break;
    case GameState::PlayersAdded:
        if (command == "addplayer")
            accepted = true;
        else if (command == "gamestart")
        {
            next = GameState::AssignReinforcement;
            accepted = true;
        }
        break;
    case GameState::AssignReinforcement:
        if (command == "issueorder")
        {
            next = GameState::IssueOrders;
            accepted = true;
        }
        break;
    case GameState::IssueOrders:
        if (command == "issueorder")
            accepted = true;
        else if (command == "endissueorders")
        {
            next = GameState::ExecuteOrders;
            accepted = true;
        }
        break;
    case GameState::ExecuteOrders:
        if (command == "execorder")
            accepted = true;
        else if (command == "endexecorders")
        {
            next = GameState::AssignReinforcement;
            accepted = true;
        }
        else if (command == "win")
        {
            next = GameState::Win;
            accepted = true;
        }
        break;
    case GameState::Win:
        if (command == "replay")
        {
            next = GameState::Start;
            accepted = true;
        }
        else if (command == "quit")
        {
            next = GameState::End;
            accepted = true;
        }
        break;
    case GameState::End:
        break;
    }

    if (!accepted)
        return false;
    currentState = next;
    if (command == "replay")
    {
        // Game assets must also be reset by startup integration before reuse.
        activePlayers->clear();
        map = nullptr;
        turnNumber = 0;
    }
    // TODO (Alexandre): Notify(this) here when Subject/ILoggable is merged.
    return true;
}

GameState GameEngine::getState() const { return currentState; }

std::string GameEngine::getStateName() const
{
    switch (currentState)
    {
    case GameState::Start:
        return "start";
    case GameState::MapLoaded:
        return "maploaded";
    case GameState::MapValidated:
        return "mapvalidated";
    case GameState::PlayersAdded:
        return "playersadded";
    case GameState::AssignReinforcement:
        return "assignreinforcement";
    case GameState::IssueOrders:
        return "issueorders";
    case GameState::ExecuteOrders:
        return "executeorders";
    case GameState::Win:
        return "win";
    case GameState::End:
        return "end";
    }
    return "unknown";
}

// Return ONLY commands a human may enter. The play-phase events in the
// A2 diagram are game-triggered, not console commands.
std::vector<std::string> GameEngine::getAvailableCommands() const
{
    switch (currentState)
    {
    case GameState::Start:
        return {"loadmap"};
    case GameState::MapLoaded:
        return {"loadmap", "validatemap"};
    case GameState::MapValidated:
        return {"addplayer"};
    case GameState::PlayersAdded:
        return {"addplayer", "gamestart"};
    case GameState::AssignReinforcement:
        return {};
    case GameState::IssueOrders:
        return {};
    case GameState::ExecuteOrders:
        return {};
    case GameState::Win:
        return {"replay", "quit"};
    case GameState::End:
        return {};
    }
    return {};
}

bool GameEngine::isFinished() const { return currentState == GameState::End; }

std::string GameEngine::stringToLog() const
{
    return "GameEngine state changed to: " + getStateName();
}

void GameEngine::setMap(Map *selectedMap) { map = selectedMap; }

void GameEngine::setPlayers(const std::vector<Player *> &players)
{
    // Preserve the startup/randomized play order; do not own Player objects.
    *activePlayers = players;
}

const std::vector<Player *> &GameEngine::getActivePlayers() const
{
    return *activePlayers;
}

void GameEngine::setIssueOrderCallback(bool (*callback)(Player &))
{
    issueOneOrder = callback;
}

unsigned int GameEngine::getTurnNumber() const { return turnNumber; }

// Use Map ownership as source of truth; the A1 Player territory list may
// become stale when an Advance/Blockade changes Territory::owner.
int GameEngine::countOwnedTerritories(const Player *player) const
{
    if (!map || !player)
        return 0;
    int owned = 0;
    for (const Territory *territory : map->getTerritories())
    {
        if (territory && territory->getOwner() == player)
            ++owned;
    }
    return owned;
}

// Count a continent bonus ONLY if its nonempty territory list is fully owned.
int GameEngine::continentBonus(const Player *player) const
{
    if (!map || !player)
        return 0;
    int bonus = 0;
    for (const Continent *continent : map->getContinents())
    {
        if (!continent)
            continue;
        const std::vector<Territory *> &members = continent->getTerritories();
        if (members.empty())
            continue;
        const bool ownsAll = std::all_of(
            members.begin(), members.end(),
            [player](const Territory *territory)
            {
                return territory && territory->getOwner() == player;
            });
        if (ownsAll)
            bonus += continent->getBonus();
    }
    return bonus;
}

// A2, p.4: max(3, floor(territories / 3)) plus continent bonuses.
void GameEngine::reinforcementPhase()
{
    if (currentState != GameState::AssignReinforcement)
        throw std::logic_error("reinforcementPhase: wrong game state");
    if (!map || activePlayers->empty())
        throw std::logic_error("reinforcementPhase: map/players not initialized");

    ++turnNumber;
    for (Player *player : *activePlayers)
    {
        if (!player)
            continue;
        const int territoryCount = countOwnedTerritories(player);
        if (territoryCount == 0)
            continue; // eliminated; don't grant armies
        const int base = std::max(3, territoryCount / 3);
        const int total = base + continentBonus(player);
        int *pool = player->getReinforcementsPool();
        if (!pool)
            throw std::logic_error("Player reinforcement pool is null");
        *pool += total;
        std::cout << "Turn " << turnNumber << ": player " << player
                  << " receives " << total << " armies (" << territoryCount
                  << " territories, continent bonus " << (total - base)
                  << ")\n";
    }
}

// Round-robin: one opportunity per player per pass, until EVERY active
// player has declared they are done for this turn.
void GameEngine::issueOrdersPhase()
{
    if (currentState != GameState::AssignReinforcement)
        throw std::logic_error("issueOrdersPhase: wrong game state");
    if (!issueOneOrder)
        throw std::logic_error("issueOrdersPhase: waiting for Phuc's issueOrder API");

    transition("issueorder");
    std::vector<bool> finished(activePlayers->size(), false);
    std::size_t remaining = activePlayers->size();

    while (remaining != 0)
    {
        for (std::size_t i = 0; i < activePlayers->size(); ++i)
        {
            if (finished[i])
                continue;
            Player *player = (*activePlayers)[i];
            if (!player || countOwnedTerritories(player) == 0 ||
                !issueOneOrder(*player))
            {
                finished[i] = true;
                --remaining;
            }
            else
            {
                transition("issueorder");
            }
        }
    }
    transition("endissueorders");
}

// Execute one order of the requested kind from one player's list.
// OrdersList::remove() in A1 DELETES the Order, so never use it afterward.
bool GameEngine::executeNextOrder(Player *player, bool deployOnly)
{
    if (!player || countOwnedTerritories(player) == 0)
        return false;
    OrdersList *orders = player->getOrdersList();
    if (!orders)
        return false;

    for (int i = 0; i < orders->size(); ++i)
    {
        Order *order = orders->getOrder(i);
        if (!order)
            continue;
        const bool isDeploy = dynamic_cast<Deploy *>(order) != nullptr;
        if (isDeploy != deployOnly)
            continue;

        order->execute(); // A2: this must validate and enact the effect.
        std::cout << "Executed: " << *order << '\n';
        transition("execorder");
        orders->remove(i); // destroys order; do not dereference again
        return true;
    }
    return false;
}

// Finish ALL players' Deploy orders before allowing ANY non-Deploy order.
void GameEngine::executeOrdersPhase()
{
    if (currentState != GameState::ExecuteOrders)
        throw std::logic_error("executeOrdersPhase: wrong game state");

    for (int pass = 0; pass < 2; ++pass)
    {
        const bool deployOnly = pass == 0;
        bool executedOne = false;
        do
        {
            executedOne = false;
            for (Player *player : *activePlayers)
            {
                if (executeNextOrder(player, deployOnly))
                    executedOne = true;
            }
        } while (executedOne);
    }

    // TODO integration: award max one card to each conqueror this turn;
    // clear per-turn diplomacy effects after all orders; coordinate with
    // Mengqi/Phuc/Alexandre to expose these APIs.
    removeEliminatedPlayers();
    if (checkWinner())
        transition("win");
    else
        transition("endexecorders");
}

// Elimination changes only the engine's ACTIVE list, never the owning
// startup list. Deleting Player objects here would leave dangling pointers
// inside previously issued Orders and Territory::owner.
void GameEngine::removeEliminatedPlayers()
{
    activePlayers->erase(
        std::remove_if(activePlayers->begin(), activePlayers->end(),
                       [this](Player *player)
                       {
                           return !player || countOwnedTerritories(player) == 0;
                       }),
        activePlayers->end());
}

// Winner exists only if the nonempty map has the SAME non-null owner for
// every territory, and that owner is still an active player.
Player *GameEngine::checkWinner() const
{
    if (!map || map->getTerritories().empty())
        return nullptr;
    const Territory *first = map->getTerritories().front();
    if (!first)
        return nullptr;
    Player *owner = first->getOwner();
    if (!owner)
        return nullptr;
    for (const Territory *territory : map->getTerritories())
    {
        if (!territory || territory->getOwner() != owner)
            return nullptr;
    }
    return std::find(activePlayers->begin(), activePlayers->end(), owner) !=
                   activePlayers->end()
               ? owner
               : nullptr;
}

// Do not run until orders actually change territories, otherwise an A1
// integration with placeholder Order::execute() can loop indefinitely.
void GameEngine::mainGameLoop()
{
    if (currentState != GameState::AssignReinforcement)
        throw std::logic_error("mainGameLoop: gamestart has not completed");
    if (!map || activePlayers->empty() || !issueOneOrder)
        throw std::logic_error("mainGameLoop: missing game dependencies");

    while (currentState != GameState::Win && currentState != GameState::End)
    {
        reinforcementPhase();
        issueOrdersPhase();
        executeOrdersPhase();
    }
    if (currentState == GameState::Win)
        std::cout << "Winner: " << checkWinner() << '\n';
}

std::ostream &operator<<(std::ostream &out, const GameEngine &engine)
{
    out << "GameEngine state: " << engine.getStateName()
        << " | Turn: " << engine.getTurnNumber();
    return out;
}

//  A2, Part 2: startup phase 

// Reads startup commands until gamestart moves the game to the play phase.
void GameEngine::startupPhase()
{
    listMapFiles(); // show which maps the user can pick
    std::string line; // holds one command typed by the user
    while (currentState != GameState::AssignReinforcement // keep going until the game has started
           && std::getline(std::cin, line))// read the next command from the console
        std::cout << handleStartupCommand(line) << "\n"; // run it and print what happened
}

// Splits a command into its word and argument, then runs it.
// Returns the effect of the command as a string.
std::string GameEngine::handleStartupCommand(const std::string &commandText)
{
    std::istringstream in(commandText);// lets us read the command word by word
    std::string command, argument; // e.g. "loadmap" and "westeros_essos.map"
    in >> command; // first word is the command
    std::getline(in >> std::ws, argument); // the rest of the line is the argument

    if (!isCommandAvailable(command)) // reject commands not allowed in this state
        return "Invalid command '" + command + "' in state " + getStateName();

    if (command == "loadmap") // pick the matching action for the command
        return loadMapCommand(argument);
    if (command == "validatemap")
        return validateMapCommand();
    if (command == "addplayer")
        return addPlayerCommand(argument);
    if (command == "gamestart")
        return gameStartCommand();
    return "Unknown startup command '" + command + "'"; // should not happen if the state table is right
}

// Loads a map from the maps folder, replacing any map loaded before.
std::string GameEngine::loadMapCommand(const std::string &fileName)
{
    if (fileName.empty()) // the user forgot the file name
        return "loadmap needs a file name";

    MapLoader loader; // reads and parses map files
    Map *loaded = loader.loadMap("maps/" + fileName); // try to load the chosen file
    if (!loaded)  // nullptr means the file was bad or missing
        return "Could not load map " + fileName;

    delete map; // free the previous map, if there was one
    map = loaded; // keep the new map in the engine
    transition("loadmap"); // move to the maploaded state
    return "Loaded map " + fileName;
}

// Checks that the loaded map is a valid connected graph.
std::string GameEngine::validateMapCommand()
{
    if (!map || !map->validate()) // no map, or the map failed validation
        return "Map is invalid, load another map";
    transition("validatemap"); // move to the mapvalidated state
    return "Map is valid";
}

// Adds a player, up to a maximum of 6.
std::string GameEngine::addPlayerCommand(const std::string &playerName)
{
    if (playerName.empty()) // the user forgot the player name
        return "addplayer needs a player name";
    if (activePlayers->size() >= 6) // the game allows 2 to 6 players
        return "Cannot add " + playerName + ": the maximum is 6 players";

    activePlayers->push_back(new Player()); // switch to Player(playerName) once Phuc adds it
    transition("addplayer"); // move to the playersadded state
    return "Added player " + playerName;
}

// Distributes territories, randomizes play order, gives 50 armies
// and 2 cards to each player, then switches to the play phase.
std::string GameEngine::gameStartCommand()
{
    if (activePlayers->size() < 2)  // the game needs at least 2 players
        return "gamestart needs at least 2 players";

    std::mt19937 rng(std::random_device{}()); // random number generator for shuffling

    // a) Shuffle the territories, then deal them out one at a time.
    std::vector<Territory *> territories = map->getTerritories(); // copy, so the map's own order is untouched
    std::shuffle(territories.begin(), territories.end(), rng);  // random order, so nobody gets a cluster
    for (std::size_t i = 0; i < territories.size(); ++i) // go through every territory
    {
        Player *owner = (*activePlayers)[i % activePlayers->size()]; // next player in turn (round-robin)
        territories[i]->setOwner(owner);  // the map records the new owner
        owner->addTerritory(territories[i]); // the player records it too
    }

    // b) Random order of play.
    std::shuffle(activePlayers->begin(), activePlayers->end(), rng);

    // c) and d) 50 armies and 2 cards each.
    for (Player *player : *activePlayers)
    {
        *player->getReinforcementsPool() = 50; // put 50 armies in the reinforcement pool
        deck->draw(*player->getHand()); // first starting card
        deck->draw(*player->getHand()); // second starting card
    }

    // e) Switch to the play phase.
    transition("gamestart"); // move to the assignreinforcement state
    return "Game started with " + std::to_string(activePlayers->size()) + " players";
}

// Returns true if the command is allowed in the current state.
bool GameEngine::isCommandAvailable(const std::string &command) const
{
    std::vector<std::string> available = getAvailableCommands(); // commands allowed right now
    return std::find(available.begin(), available.end(), command) != available.end(); // is ours in the list?
}

// Prints the map files the user can choose with loadmap.
void GameEngine::listMapFiles() const
{
    std::cout << "Available maps:\n";
    for (const auto &entry : std::filesystem::directory_iterator("maps")) // go through the maps folder
        std::cout << "  " << entry.path().filename().string() << "\n"; // print each file name
}