#include "GameEngine.h"
#include "Cards.h"
#include "Map.h"
#include "Orders.h"
#include "Player.h"

#include <algorithm>
#include <filesystem>
#include <random>
#include <sstream>
#include <stdexcept>
#include <utility>

GameEngine::GameEngine()
    : currentState(GameState::Start), map(nullptr), deck(nullptr),
      activePlayers(new std::vector<Player *>()), turnNumber(0),
      ownedMap(nullptr), ownedDeck(nullptr),
      ownedPlayers(new std::vector<Player *>()) {}

// Map and Player objects are borrowed, but the vector is deep-copied.
GameEngine::GameEngine(const GameEngine &other)
    : Subject(other), currentState(other.currentState), map(other.map),
      deck(other.deck), activePlayers(new std::vector<Player *>(*other.activePlayers)),
      turnNumber(other.turnNumber), ownedMap(nullptr), ownedDeck(nullptr),
      ownedPlayers(new std::vector<Player *>()) {}
// The copied engine borrows the original's game objects: it never deletes them.

GameEngine &GameEngine::operator=(const GameEngine &other)
{
    if (this != &other)
    {
        // Allocate before touching this, including when other borrows our objects.
        std::vector<Player *> *replacement =
            new std::vector<Player *>(*other.activePlayers);
        releaseStartupResources();
        delete activePlayers;
        activePlayers = replacement;
        currentState = other.currentState;
        map = other.map;
        deck = other.deck;
        turnNumber = other.turnNumber;
    }
    return *this;
}

GameEngine::~GameEngine()
{
    releaseStartupResources();
    delete activePlayers;
    delete ownedPlayers;
}

void GameEngine::releaseStartupResources()
{
    // Remove borrowed references before freeing startup-owned objects.
    activePlayers->clear();
    for (Player *player : *ownedPlayers)
        delete player;
    ownedPlayers->clear();

    if (map == ownedMap)
        map = nullptr;
    delete ownedMap;
    ownedMap = nullptr;

    if (deck == ownedDeck)
        deck = nullptr;
    delete ownedDeck;
    ownedDeck = nullptr;
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

    const bool stateChanged = (currentState != next);
    currentState = next;
    if (command == "replay")
    {
        // Game assets must also be reset by startup integration before reuse.
        releaseStartupResources();
        map = nullptr;  // Also stop borrowing externally supplied game assets.
        deck = nullptr;
        turnNumber = 0;
        // Clear shared per-turn order bookkeeping for a new game.
        Order::clearNegotiations();
        Order::clearConquests();
    }
    if (stateChanged)
    {
        Notify(this);
    }

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

void GameEngine::setMap(Map *selectedMap)
{
    if (selectedMap != map && ownedMap)
    {
        delete ownedMap;
        ownedMap = nullptr;
    }
    map = selectedMap;
}

void GameEngine::setDeck(Deck *selectedDeck)
{
    if (selectedDeck != deck && ownedDeck)
    {
        delete ownedDeck;
        ownedDeck = nullptr;
    }
    deck = selectedDeck;
}

void GameEngine::setPlayers(const std::vector<Player *> &players)
{
    // Startup-created Players must not silently be replaced (would invalidate
    // Territory::owner); test fixtures can set borrowed players before startup.
    if (!ownedPlayers->empty())
        throw std::logic_error("setPlayers: cannot replace startup-created players");
    *activePlayers = players;
}

const std::vector<Player *> &GameEngine::getActivePlayers() const
{
    return *activePlayers;
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
        std::cout << "Turn " << turnNumber << ": player " << player->getName()
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
    if (!deck)
        throw std::logic_error("issueOrdersPhase: deck not initialized");

    // Reset order-issuing and per-turn Player bookkeeping before the first pass.
    for (Player *player : *activePlayers)
    {
        if (player)
            player->resetTurnState();
    }

    transition("issueorder");
    std::vector<bool> finished(activePlayers->size(), false);
    std::size_t remaining = activePlayers->size();

    // One Player::issueOrder() call per active player per pass.  The Player
    // decides which order to create; false means it is done for this turn.
    while (remaining != 0)
    {
        for (std::size_t i = 0; i < activePlayers->size(); ++i)
        {
            if (finished[i])
                continue;
            Player *player = (*activePlayers)[i];
            if (!player || countOwnedTerritories(player) == 0 ||
                !player->issueOrder(deck))
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

    // Orders records a conquering player once, so each may draw at most
    // one card this turn. awardConquestCards() also clears that record.
    if (deck)
        Order::awardConquestCards(*deck);
    else
        Order::clearConquests();

    // Diplomacy expires at the end of a turn.  Each Player clears its own
    // per-turn negotiated-player list in resetTurnState() next issue phase.
    Order::clearNegotiations();
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

// Run with the A2 Orders implementation (which enacts territory changes).
void GameEngine::mainGameLoop()
{
    if (currentState != GameState::AssignReinforcement)
        throw std::logic_error("mainGameLoop: gamestart has not completed");
    if (!map || !deck || activePlayers->empty())
        throw std::logic_error("mainGameLoop: missing game dependencies");

    while (currentState != GameState::Win && currentState != GameState::End)
    {
        reinforcementPhase();
        issueOrdersPhase();
        executeOrdersPhase();
    }
    if (currentState == GameState::Win)
        std::cout << "Winner: " << checkWinner()->getName() << '\n';
}

std::ostream &operator<<(std::ostream &out, const GameEngine &engine)
{
    out << "GameEngine state: " << engine.getStateName()
        << " | Turn: " << engine.getTurnNumber();
    return out;
}
// ------------------- A2 Part 2: startup integration -------------------
// Startup is interactive by default; the stream overload makes it testable.
void GameEngine::startupPhase()
{
    startupPhase(std::cin, std::cout);
}

void GameEngine::startupPhase(std::istream &input, std::ostream &output)
{
    if (currentState != GameState::Start &&
        currentState != GameState::MapLoaded &&
        currentState != GameState::MapValidated &&
        currentState != GameState::PlayersAdded)
        throw std::logic_error("startupPhase: game is not in startup state");

    listMapFiles(output);
    std::string line;
    while (currentState != GameState::AssignReinforcement &&
           std::getline(input, line))
    {
        output << handleStartupCommand(line) << '\n';
    }
}

namespace
{
std::string trimStartupArgument(const std::string &value)
{
    const std::string whitespace = " \t\r\n";
    const std::size_t start = value.find_first_not_of(whitespace);
    if (start == std::string::npos)
        return "";
    const std::size_t end = value.find_last_not_of(whitespace);
    return value.substr(start, end - start + 1);
}
} // namespace

std::string GameEngine::handleStartupCommand(const std::string &line)
{
    std::istringstream input(line);
    std::string command;
    input >> command;
    std::string argument;
    std::getline(input, argument);
    argument = trimStartupArgument(argument);

    if (command.empty())
        return "Empty command";

    const std::vector<std::string> allowed = getAvailableCommands();
    if (std::find(allowed.begin(), allowed.end(), command) == allowed.end())
        return "Invalid command '" + command + "' in state " + getStateName();

    if (command == "loadmap")
        return loadMapCommand(argument);
    if (command == "addplayer")
        return addPlayerCommand(argument);

    if (!argument.empty())
        return command + " does not accept arguments";
    if (command == "validatemap")
        return validateMapCommand();
    if (command == "gamestart")
        return gameStartCommand();
    return "Unknown startup command";
}

std::string GameEngine::loadMapCommand(const std::string &fileName)
{
    if (fileName.empty())
        return "loadmap needs a file name";

    // Only accept filenames within maps/, not absolute or parent paths.
    const std::filesystem::path relative(fileName);
    if (relative != relative.filename())
        return "loadmap expects a filename inside maps/";

    MapLoader loader;
    Map *loaded = loader.loadMap((std::filesystem::path("maps") / relative).string());
    if (!loaded)
        return "Could not load map '" + fileName + "': " + loader.getLastError();

    // Don't delete an external Map supplied by setMap().
    if (ownedMap)
        delete ownedMap;
    map = loaded;
    ownedMap = loaded;
    transition("loadmap");
    return "Loaded map " + fileName;
}

std::string GameEngine::validateMapCommand()
{
    if (!map || !map->validate())
        return "Map validation failed";
    transition("validatemap");
    return "Map validated";
}

std::string GameEngine::addPlayerCommand(const std::string &playerName)
{
    if (playerName.empty())
        return "addplayer needs a player name";
    if (activePlayers->size() >= 6)
        return "Cannot add more than 6 players";

    const bool duplicate = std::any_of(activePlayers->begin(), activePlayers->end(),
                                       [&playerName](const Player *player)
                                       {
                                           return player && player->getName() == playerName;
                                       });
    if (duplicate)
        return "Duplicate player name: " + playerName;

    Player *newPlayer = new Player(playerName);
    try
    {
        ownedPlayers->push_back(newPlayer);
        activePlayers->push_back(newPlayer);
    }
    catch (...)
    {
        ownedPlayers->erase(std::remove(ownedPlayers->begin(), ownedPlayers->end(), newPlayer),
                            ownedPlayers->end());
        delete newPlayer;
        throw;
    }

    transition("addplayer");
    return "Added player " + playerName;
}

std::string GameEngine::gameStartCommand()
{
    if (!map || !map->validate())
        return "gamestart requires a validated map";
    if (activePlayers->size() < 2 || activePlayers->size() > 6)
        return "gamestart requires 2 to 6 players";
    if (map->getTerritories().size() < activePlayers->size())
        return "gamestart requires at least one territory per player";

    // Ensure each player receives two cards before changing world ownership.
    if (!deck)
    {
        ownedDeck = new Deck();
        deck = ownedDeck;
    }
    if (deck->size() < 2 * static_cast<int>(activePlayers->size()))
        return "gamestart: not enough cards in deck";

    Order::clearNegotiations();
    Order::clearConquests();
    std::mt19937 rng(std::random_device{}());

    // Fair territory assignment: territory counts differ by at most one.
    std::vector<Territory *> territories(map->getTerritories());
    std::shuffle(territories.begin(), territories.end(), rng);
    for (std::size_t i = 0; i < territories.size(); ++i)
    {
        Player *player = (*activePlayers)[i % activePlayers->size()];
        player->addTerritory(territories[i]);
    }

    // Randomize the turn order after territory distribution.
    std::shuffle(activePlayers->begin(), activePlayers->end(), rng);
    for (Player *player : *activePlayers)
    {
        *player->getReinforcementsPool() = 50;
        player->resetTurnState();
        deck->draw(*player->getHand());
        deck->draw(*player->getHand());
    }
    transition("gamestart");
    return "Game started with " + std::to_string(activePlayers->size()) + " players";
}

void GameEngine::listMapFiles(std::ostream &output) const
{
    output << "Available maps in maps/:\n";
    const std::filesystem::path dir("maps");
    if (!std::filesystem::is_directory(dir))
    {
        output << "  (maps/ directory not found; run from repository root)\n";
        return;
    }
    for (const std::filesystem::directory_entry &entry : std::filesystem::directory_iterator(dir))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".map")
            output << "  " << entry.path().filename().string() << '\n';
    }
}
