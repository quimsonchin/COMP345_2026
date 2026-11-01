#include "GameEngine.h"
#include "Map.h"
#include "Orders.h"
#include "Player.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <utility>

GameEngine::GameEngine()
    : currentState(GameState::Start), map(nullptr),
      activePlayers(new std::vector<Player *>()), issueOneOrder(nullptr),
      turnNumber(0) {}

// Map and Player objects are borrowed, but the vector is deep-copied.
GameEngine::GameEngine(const GameEngine &other)
    : currentState(other.currentState), map(other.map),
      activePlayers(new std::vector<Player *>(*other.activePlayers)),
      issueOneOrder(other.issueOneOrder), turnNumber(other.turnNumber) {}

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
    }
    return *this;
}

GameEngine::~GameEngine()
{
    delete activePlayers; // Never delete a Player or Map you do not own.
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