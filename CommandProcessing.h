#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "GameEngine.h"

// A command typed by the user or read from a file, plus its effect.
class Command {
public:
  // Effect starts empty until the command runs.
  Command(const std::string &command);

  // Copies the text and effect.
  Command(const Command &other);

  // Copies the text and effect.
  Command &operator=(const Command &other);

  // Nothing to free.
  ~Command();

  // Record what happened when the command ran (or why it was rejected).
  void saveEffect(const std::string &effect);

  // Whole command, e.g. "loadmap westeros_essos.map".
  std::string getCommand() const;

  // First word, e.g. "loadmap".
  std::string getName() const;

  // Rest of the line after the first word, or "" if there isn't one.
  std::string getArgument() const;

  // "" until saveEffect() is called.
  std::string getEffect() const;

  // Prints the command and its effect.
  friend std::ostream &operator<<(std::ostream &out, const Command &command);

private:
  std::string command;
  std::string effect;
};

// Reads commands from the console and keeps every one it reads.
// The GameEngine and Player get their commands through getCommand().
class CommandProcessor {
public:
  // Starts with no commands.
  CommandProcessor();

  // Deep copies the saved commands.
  CommandProcessor(const CommandProcessor &other);

  // Deep copies the saved commands.
  CommandProcessor &operator=(const CommandProcessor &other);

  // Deletes the saved commands.
  virtual ~CommandProcessor();

  // Reads and saves the next command. Returns nullptr once input runs out.
  // The processor still owns the returned command.
  Command *getCommand();

  // Is this command allowed in this state? If not, the reason is saved as
  // the command's effect.
  bool validate(Command *command, GameState state);

  // Every command read so far, valid or not.
  const std::vector<Command *> &getCommands() const;

  // Prints every saved command with its effect.
  friend std::ostream &operator<<(std::ostream &out,
                                  const CommandProcessor &processor);

protected:
  // Next non-blank line from the console, or "" when input is closed.
  virtual std::string readCommand();

  // Adds the command to the list.
  Command *saveCommand(const std::string &commandText);

private:
  // Deletes every command and empties the list.
  void clearCommands();

  // Appends a deep copy of each command in source.
  void copyCommands(const std::vector<Command *> &source);

  std::vector<Command *> commands; // owned
};

// Reads a text file line by line.
class FileLineReader {
public:
  // Opens the file at the start.
  FileLineReader(const std::string &fileName);

  // Streams can't be copied, so this opens the same file from the start.
  FileLineReader(const FileLineReader &other);

  // Opens the other reader's file from the start.
  FileLineReader &operator=(const FileLineReader &other);

  // Closes the file.
  ~FileLineReader();

  // Next non-blank line, or "" at end of file.
  std::string readLineFromFile();

  // False if the file couldn't be opened.
  bool isOpen() const;

  // Prints the file name and whether it's open.
  friend std::ostream &operator<<(std::ostream &out,
                                  const FileLineReader &reader);

private:
  std::string fileName;
  std::ifstream *file; // owned
};

// Adapter: same interface as CommandProcessor, but the commands come from
// a saved text file instead of the console.
class FileCommandProcessorAdapter : public CommandProcessor {
public:
  // Reads commands from fileName.
  FileCommandProcessorAdapter(const std::string &fileName);

  // Copies the saved commands and opens the file from the start.
  FileCommandProcessorAdapter(const FileCommandProcessorAdapter &other);

  // Copies the saved commands and reopens the file from the start.
  FileCommandProcessorAdapter &
  operator=(const FileCommandProcessorAdapter &other);

  // Deletes the file reader.
  ~FileCommandProcessorAdapter() override;

  // False if the command file couldn't be opened.
  bool isOpen() const;

  // Prints the file being read, then the saved commands.
  friend std::ostream &operator<<(std::ostream &out,
                                  const FileCommandProcessorAdapter &adapter);

protected:
  // Reads from the file instead of the console.
  std::string readCommand() override;

private:
  FileLineReader *flr; // owned
};
