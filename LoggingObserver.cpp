#include "LoggingObserver.h"
#include <algorithm>
#include <fstream>
#include <stdexcept>

ILoggable::~ILoggable() {}
std::ostream& operator<<(std::ostream& out, const ILoggable& object) {
    return out << object.stringToLog();
}
Observer::~Observer() {}
std::ostream& operator<<(std::ostream& out, const Observer&) {
    return out << "Observer";
}
Subject::Subject() : observers(new std::vector<Observer*>) {}
Subject::Subject(const Subject&) : observers(new std::vector<Observer*>) {}
Subject& Subject::operator=(const Subject&) {
    // Keep this object's subscriptions when its game data is assigned.
    return *this;
}
Subject::~Subject() { delete observers; }
void Subject::Attach(Observer* observer) {
    if (observer && std::find(observers->begin(), observers->end(), observer) == observers->end())
        observers->push_back(observer);
}
void Subject::Detach(Observer* observer) {
    observers->erase(std::remove(observers->begin(), observers->end(), observer), observers->end());
}
void Subject::Notify(const ILoggable* object) const {
    for (Observer* observer : *observers) observer->Update(object);
}
std::ostream& operator<<(std::ostream& out, const Subject& subject) {
    return out << "Subject with " << subject.observers->size() << " observers";
}
LogObserver::LogObserver() {}
LogObserver::LogObserver(const LogObserver&) {}
LogObserver& LogObserver::operator=(const LogObserver&) { return *this; }
LogObserver::~LogObserver() {}
void LogObserver::Update(const ILoggable* object) {
    if (!object) return;
    std::ofstream log("gamelog.txt", std::ios::app);
    if (!log) throw std::runtime_error("Cannot open gamelog.txt");
    log << object->stringToLog() << '\n';
}
std::ostream& operator<<(std::ostream& out, const LogObserver&) {
    return out << "LogObserver: gamelog.txt";
}
