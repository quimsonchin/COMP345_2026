#include "LoggingObserver.h"
#include <algorithm>
#include <fstream>
#include <stdexcept>

// Destructor.
ILoggable::~ILoggable() {}
std::ostream& operator<<(std::ostream& out, const ILoggable& object) {
    return out << object.stringToLog();
}
// Destructor.
Observer::~Observer() {}
std::ostream& operator<<(std::ostream& out, const Observer&) {
    return out << "Observer";
}
// Default constructor: create an empty observer list.
Subject::Subject() : observers(new std::vector<Observer*>) {}
// Copy constructor: start without attached observers.
Subject::Subject(const Subject&) : observers(new std::vector<Observer*>) {}
// Assignment operator: keep existing observer attachments.
Subject& Subject::operator=(const Subject&) {
    // Keep this object's subscriptions when its game data is assigned.
    return *this;
}
// Destructor: delete the list, not the observers.
Subject::~Subject() { delete observers; }
// Connect an observer once; the subject does not own it.
void Subject::Attach(Observer* observer) {
    if (observer && std::find(observers->begin(), observers->end(), observer) == observers->end())
        observers->push_back(observer);
}
// Disconnect the observer without deleting it.
void Subject::Detach(Observer* observer) {
    observers->erase(std::remove(observers->begin(), observers->end(), observer), observers->end());
}
// Send the event to every attached observer.
void Subject::Notify(const ILoggable* object) const {
    for (Observer* observer : *observers) observer->Update(object);
}
std::ostream& operator<<(std::ostream& out, const Subject& subject) {
    return out << "Subject with " << subject.observers->size() << " observers";
}
// Default constructor.
LogObserver::LogObserver() {}
// Copy constructor.
LogObserver::LogObserver(const LogObserver&) {}
// Assignment operator.
LogObserver& LogObserver::operator=(const LogObserver&) { return *this; }
// Destructor.
LogObserver::~LogObserver() {}
void LogObserver::Update(const ILoggable* object) {
    if (!object) return;
    // Append the message, keeping earlier log entries.
    std::ofstream log("gamelog.txt", std::ios::app);
    if (!log) throw std::runtime_error("Cannot open gamelog.txt");
    log << object->stringToLog() << '\n';
}
std::ostream& operator<<(std::ostream& out, const LogObserver&) {
    return out << "LogObserver: gamelog.txt";
}
