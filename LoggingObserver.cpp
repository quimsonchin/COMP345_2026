#include "LoggingObserver.h"
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
// Default constructor.
Subject::Subject() : observer(nullptr) {}
// Copy constructor: no observer attached.
Subject::Subject(const Subject&) : observer(nullptr) {}
// Assignment operator: keep the attached observer.
Subject& Subject::operator=(const Subject&) {
    return *this;
}
// Destructor: observer is not owned here.
Subject::~Subject() {}
// Attach an observer.
void Subject::Attach(Observer* observer) {
    this->observer = observer;
}
// Detach the matching observer.
void Subject::Detach(Observer* observer) {
    if (this->observer == observer) {
        this->observer = nullptr;
    }
}
// Notify the observer.
void Subject::Notify(const ILoggable* object) const {
    if (observer != nullptr) {
        observer->Update(object);
    }
}
std::ostream& operator<<(std::ostream& out, const Subject& subject) {
    if (subject.observer != nullptr) {
        return out << "Subject with one observer";
    }
    return out << "Subject without an observer";
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
    // Append to the log file.
    std::ofstream log("gamelog.txt", std::ios::app);
    if (!log) throw std::runtime_error("Cannot open gamelog.txt");
    log << object->stringToLog() << '\n';
}
std::ostream& operator<<(std::ostream& out, const LogObserver&) {
    return out << "LogObserver: gamelog.txt";
}

// Return the attached observer.
Observer* Subject::getObserver() const {
    return observer;
}
