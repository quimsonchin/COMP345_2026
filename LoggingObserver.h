#ifndef LOGGING_OBSERVER_H
#define LOGGING_OBSERVER_H

#include <iostream>
#include <string>

// Interface for objects that can describe a logging event.
class ILoggable {
public:
    virtual ~ILoggable();
    virtual std::string stringToLog() const = 0;
    friend std::ostream& operator<<(std::ostream& out, const ILoggable& object);
};

// Interface for observers receiving logging events.
class Observer {
public:
    virtual ~Observer();
    virtual void Update(const ILoggable* object) = 0;
    friend std::ostream& operator<<(std::ostream& out, const Observer& observer);
};

// Holds one borrowed observer; copies start without an observer.
class Subject {
public:
    Subject();
    Subject(const Subject& other);
    Subject& operator=(const Subject& other);
    virtual ~Subject();
    void Attach(Observer* observer);
    void Detach(Observer* observer);
    void Notify(const ILoggable* object) const;
    friend std::ostream& operator<<(std::ostream& out, const Subject& subject);
private:
    Observer* observer;
};

// Appends each notification to gamelog.txt in the working directory.
class LogObserver : public Observer {
public:
    LogObserver();
    LogObserver(const LogObserver& other);
    LogObserver& operator=(const LogObserver& other);
    ~LogObserver() override;
    void Update(const ILoggable* object) override;
    friend std::ostream& operator<<(std::ostream& out, const LogObserver& observer);
};

#endif
