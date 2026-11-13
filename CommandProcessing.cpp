#include "CommandProcessing.h"

// Constructor: store the command text.
Command::Command(const std::string& value)
    : text(new std::string(value)), effect(new std::string) {}
// Copy constructor: copy the text and effect.
Command::Command(const Command& other)
    : Subject(other), text(new std::string(*other.text)), effect(new std::string(*other.effect)) {}
// Assignment operator: copy the text and effect.
Command& Command::operator=(const Command& other) {
    if (this != &other) {
        *text = *other.text;
        *effect = *other.effect;
    }
    return *this;
}
// Destructor: delete the text and effect.
Command::~Command() {
    delete text;
    delete effect;
}
void Command::saveEffect(const std::string& value) {
    *effect = value;
    Notify(this); // Tell the logger the command effect is ready.
}
std::string Command::getCommand() const { return *text; }
std::string Command::stringToLog() const {
    return "Command effect: " + *text + " -> " + *effect;
}
std::ostream& operator<<(std::ostream& out, const Command& command) {
    return out << command.stringToLog();
}
// Default constructor: create an empty command list.
CommandProcessor::CommandProcessor() : commands(new std::vector<Command*>) {}
// Copy constructor: make separate copies of the commands.
CommandProcessor::CommandProcessor(const CommandProcessor& other) : CommandProcessor() {
    // Reserve space first so adding copied pointers needs no allocation.
    commands->reserve(other.commands->size());
    for (Command* command : *other.commands) {
        commands->push_back(new Command(*command));
    }
}
// Assignment operator: replace commands with copies.
CommandProcessor& CommandProcessor::operator=(const CommandProcessor& other) {
    if (this != &other) {
        CommandProcessor copy(other);
        // Give the old list to the temporary object for cleanup.
        std::vector<Command*>* oldCommands = commands;
        commands = copy.commands;
        copy.commands = oldCommands;
    }
    return *this;
}
// Destructor: delete saved commands and their list.
CommandProcessor::~CommandProcessor() {
    for (Command* command : *commands) delete command;
    delete commands;
}
std::string CommandProcessor::readCommand() {
    std::string text;
    std::getline(std::cin, text);
    return text;
}
Command* CommandProcessor::saveCommand(const std::string& text) {
    Command* command = new Command(text);
    try {
        commands->push_back(command);
    } catch (...) {
        delete command; // The list did not take ownership.
        throw;
    }
    Notify(this); // Tell the logger a new command was saved.
    return command;
}
Command* CommandProcessor::getCommand() {
    std::string text = readCommand();
    if (!std::cin) return nullptr;
    return saveCommand(text);
}
std::string CommandProcessor::stringToLog() const {
    return commands->empty() ? "No commands" : "Command saved: " + commands->back()->getCommand();
}
std::ostream& operator<<(std::ostream& out, const CommandProcessor& processor) {
    return out << processor.stringToLog();
}
