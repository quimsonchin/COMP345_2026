#ifndef COMMAND_PROCESSING_H
#define COMMAND_PROCESSING_H

#include "LoggingObserver.h"
#include <vector>

// Minimal command support for Part 5; command validation belongs to Part 1.
class Command : public Subject, public ILoggable {
public:
    explicit Command(const std::string& text = "");
    Command(const Command& other);
    Command& operator=(const Command& other);
    ~Command() override;
    void saveEffect(const std::string& effect);
    std::string getCommand() const;
    std::string stringToLog() const override;
    friend std::ostream& operator<<(std::ostream& out, const Command& command);
private:
    std::string* text;
    std::string* effect;
};

// Owns saved commands. Returned command pointers are borrowed.
class CommandProcessor : public Subject, public ILoggable {
public:
    CommandProcessor();
    CommandProcessor(const CommandProcessor& other);
    CommandProcessor& operator=(const CommandProcessor& other);
    ~CommandProcessor() override;
    Command* getCommand();
    std::string stringToLog() const override;
    friend std::ostream& operator<<(std::ostream& out, const CommandProcessor& processor);
private:
    std::string readCommand();
    Command* saveCommand(const std::string& text);
    std::vector<Command*>* commands;
};

#endif
