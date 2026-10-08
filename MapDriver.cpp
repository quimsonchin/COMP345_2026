#include "Map.h"
#include <iostream>

int main() {
    // --- Territory test ---
    Territory a("A"), b("B");
    a.setAdjacent(&b);
    b.setAdjacent(&a);

    Territory c(a);   // copy constructor

    std::cout << a << "\n";
    std::cout << c << "\n";

    return 0;
}