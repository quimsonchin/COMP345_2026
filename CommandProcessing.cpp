#include "CommandProcessing.h"
#include <memory>
#include <utility>

// Constructor: store the command text.
Command::Command(const std::string& value)
    : text(new std::string(value)), effect(new std::string) {}
// Copy constructor: copy the text and effect.
Command::Command(const Command& other)
    : Subject(other), text(new std::string(*other.text)), effect(new std::string(*other.effect)) {}
// Assignment operator: copy the text and effect.
Command& Command::operator=(const Command& other) {
    if (this != &other) { *text = *other.text; *effect = *other.effect; }
    return *this;
}
// Destructor: delete the text and effect.
Command::~Command() { delete text; delete effect; }
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
    for (Command* command : *other.commands) {
        std::unique_ptr<Command> copy(new Command(*command));
        commands->push_back(copy.get());
        copy.release();
    }
}
// Assignment operator: replace commands with copies.
CommandProcessor& CommandProcessor::operator=(const CommandProcessor& other) {
    if (this != &other) {
        CommandProcessor copy(other);
        std::swap(commands, copy.commands);
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
    std::unique_ptr<Command> command(new Command(text));
    commands->push_back(command.get());
    Command* saved = command.release();
    Notify(this); // Tell the logger a new command was saved.
    return saved;
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
