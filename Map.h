//only includes this header file once in a single compilation
#pragma once
#include <string>

class Territory {
public:
    Territory(const std::string & name);
private:
    std::string* name;
};
    
