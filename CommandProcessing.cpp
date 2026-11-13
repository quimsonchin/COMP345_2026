#include "CommandProcessing.h"

#include <sstream>

// State names as they appear in the handout's command table.
static std::string stateLabel(GameState state) {
  switch (state) {
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

// Strips surrounding whitespace, including the \r from Windows line endings.
static std::string trim(const std::string &text) {
  const std::string whitespace = " \t\r\n";
  const std::size_t first = text.find_first_not_of(whitespace);
  if (first == std::string::npos)
    return "";
  const std::size_t last = text.find_last_not_of(whitespace);
  return text.substr(first, last - first + 1);
}

// ---------------------------------------------------------------- Command

Command::Command(const std::string &command)
    : command(trim(command)), effect("") {}

Command::Command(const Command &other)
    : command(other.command), effect(other.effect) {}

Command &Command::operator=(const Command &other) {
  if (this != &other) {
    command = other.command;
    effect = other.effect;
  }
  return *this;
}

Command::~Command() {}

void Command::saveEffect(const std::string &effect) { this->effect = effect; }

std::string Command::getCommand() const { return command; }

std::string Command::getName() const {
  std::istringstream words(command);
  std::string name;
  words >> name;
  return name;
}

// Takes the whole rest of the line, so names with spaces still work.
std::string Command::getArgument() const {
  const std::size_t space = command.find_first_of(" \t");
  if (space == std::string::npos)
    return "";
  return trim(command.substr(space));
}

std::string Command::getEffect() const { return effect; }

// Prints: loadmap x.map: effect
std::ostream &operator<<(std::ostream &out, const Command &command) {
  out << command.command << ": "
      << (command.effect.empty() ? "(not run yet)" : command.effect);
  return out;
}

// ------------------------------------------------------- CommandProcessor

CommandProcessor::CommandProcessor() : commands() {}

CommandProcessor::CommandProcessor(const CommandProcessor &other) : commands() {
  copyCommands(other.commands);
}

CommandProcessor &CommandProcessor::operator=(const CommandProcessor &other) {
  if (this != &other) {
    clearCommands();
    copyCommands(other.commands);
  }
  return *this;
}

CommandProcessor::~CommandProcessor() { clearCommands(); }

Command *CommandProcessor::getCommand() {
  const std::string commandText = readCommand();
  if (commandText.empty()) {
    return nullptr;
  }
  return saveCommand(commandText);
}

// Command table from the handout:
//   loadmap <mapfile>      start, maploaded
//   validatemap            maploaded
//   addplayer <playername> mapvalidated, playersadded
//   gamestart              playersadded
//   replay                 win
//   quit                   win
bool CommandProcessor::validate(Command *command, GameState state) {
  if (command == nullptr) {
    return false;
  }

  const std::string name = command->getName();
  const bool hasArgument = !command->getArgument().empty();
  bool validState = false;
  bool needsArgument = false;

  if (name == "loadmap") {
    validState = state == GameState::Start || state == GameState::MapLoaded;
    needsArgument = true;
  } else if (name == "validatemap") {
    validState = state == GameState::MapLoaded;
  } else if (name == "addplayer") {
    validState =
        state == GameState::MapValidated || state == GameState::PlayersAdded;
    needsArgument = true;
  } else if (name == "gamestart") {
    validState = state == GameState::PlayersAdded;
  } else if (name == "replay" || name == "quit") {
    validState = state == GameState::Win;
  } else {
    command->saveEffect("\"" + name + "\" isn't a command");
    return false;
  }

  if (!validState) {
    command->saveEffect("Can't " + name + " in " + stateLabel(state));
    return false;
  }
  if (needsArgument && !hasArgument) {
    command->saveEffect(name + " needs a " +
                        (name == "loadmap" ? "map file" : "player name"));
    return false;
  }
  if (!needsArgument && hasArgument) {
    command->saveEffect(name + " doesn't take an argument");
    return false;
  }
  return true;
}

const std::vector<Command *> &CommandProcessor::getCommands() const {
  return commands;
}

// One saved command per line.
std::ostream &operator<<(std::ostream &out, const CommandProcessor &processor) {
  out << "Saved commands (" << processor.commands.size() << "):";
  for (std::size_t i = 0; i < processor.commands.size(); ++i) {
    out << "\n  " << (i + 1) << ". " << *processor.commands[i];
  }
  return out;
}

// Skips blank lines so hitting Enter doesn't make an empty command.
std::string CommandProcessor::readCommand() {
  std::string line;
  while (std::getline(std::cin, line)) {
    line = trim(line);
    if (!line.empty()) {
      return line;
    }
  }
  return "";
}

// Invalid commands are kept too, so their effect can be shown later.
Command *CommandProcessor::saveCommand(const std::string &commandText) {
  Command *command = new Command(commandText);
  commands.push_back(command);
  return command;
}

void CommandProcessor::clearCommands() {
  for (Command *command : commands) {
    delete command;
  }
  commands.clear();
}

void CommandProcessor::copyCommands(const std::vector<Command *> &source) {
  for (const Command *command : source) {
    commands.push_back(new Command(*command));
  }
}

// --------------------------------------------------------- FileLineReader

FileLineReader::FileLineReader(const std::string &fileName)
    : fileName(fileName), file(new std::ifstream(fileName)) {}

FileLineReader::FileLineReader(const FileLineReader &other)
    : fileName(other.fileName), file(new std::ifstream(other.fileName)) {}

FileLineReader &FileLineReader::operator=(const FileLineReader &other) {
  if (this != &other) {
    delete file;
    fileName = other.fileName;
    file = new std::ifstream(fileName);
  }
  return *this;
}

FileLineReader::~FileLineReader() { delete file; }

std::string FileLineReader::readLineFromFile() {
  std::string line;
  while (std::getline(*file, line)) {
    line = trim(line);
    if (!line.empty()) {
      return line;
    }
  }
  return "";
}

bool FileLineReader::isOpen() const { return file->is_open(); }

// Prints: file.txt (open)
std::ostream &operator<<(std::ostream &out, const FileLineReader &reader) {
  out << reader.fileName << (reader.isOpen() ? " (open)" : " (couldn't open)");
  return out;
}

// ---------------------------------------------- FileCommandProcessorAdapter

FileCommandProcessorAdapter::FileCommandProcessorAdapter(
    const std::string &fileName)
    : CommandProcessor(), flr(new FileLineReader(fileName)) {}

FileCommandProcessorAdapter::FileCommandProcessorAdapter(
    const FileCommandProcessorAdapter &other)
    : CommandProcessor(other), flr(new FileLineReader(*other.flr)) {}

FileCommandProcessorAdapter &FileCommandProcessorAdapter::operator=(
    const FileCommandProcessorAdapter &other) {
  if (this != &other) {
    CommandProcessor::operator=(other);
    *flr = *other.flr;
  }
  return *this;
}

// The base destructor takes care of the commands.
FileCommandProcessorAdapter::~FileCommandProcessorAdapter() { delete flr; }

bool FileCommandProcessorAdapter::isOpen() const { return flr->isOpen(); }

// Prints the file being read, then the saved commands.
std::ostream &operator<<(std::ostream &out,
                         const FileCommandProcessorAdapter &adapter) {
  out << "Reading from " << *adapter.flr << '\n'
      << static_cast<const CommandProcessor &>(adapter);
  return out;
}

std::string FileCommandProcessorAdapter::readCommand() {
  return flr->readLineFromFile();
}
