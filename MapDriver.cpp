#include "Map.h"
#include <iostream>
#include <string>
#include <vector>

// Loads one map file and validates it. Returns true if the map is accepted.
bool testMapFile(MapLoader& loader, const std::string& fileName) {
    std::cout << "\n=== " << fileName << " ===\n";
    Map* map = loader.loadMap(fileName);
    if (map == nullptr) {
        std::cout << "RESULT: rejected by the loader (" << loader.getLastError() << ")\n";
        return false;
    }
    std::cout << *map;
    bool valid = map->validate();
    std::cout << (valid ? "RESULT: valid map, accepted\n" : "RESULT: invalid map, rejected\n");
    delete map;
    return valid;
}

// Tests all the map files, then the copy constructor and assignment operator.
int main() {
    MapLoader loader;

    // paths are relative to the project root
    std::vector<std::string> mapFiles = {
        // valid map
        "maps/westeros_essos.map",
        // load fine but fail validate()
        "maps/disconnected.map",
        "maps/split_continent.map",
        "maps/two_continents.map",
        // rejected by the loader
        "maps/not_a_map.txt"
    };

    int accepted = 0;
    for (const std::string& fileName : mapFiles) {
        if (testMapFile(loader, fileName)) ++accepted;
    }
    std::cout << "\n" << accepted << " of " << mapFiles.size() << " map files were accepted\n";

    // the copy should still work after the original is deleted
    std::cout << "\n=== Deep copy of a loaded map ===\n";
    Map* original = loader.loadMap("maps/westeros_essos.map");
    if (original != nullptr) {
        Map copy(*original);
        std::cout << "Original Winterfell at " << original->findTerritory("Winterfell")
                  << ", copy Winterfell at " << copy.findTerritory("Winterfell")
                  << " (addresses differ)\n";
        delete original;
        std::cout << "Original deleted, copy still works:\n" << copy;
        std::cout << (copy.validate() ? "RESULT: valid map\n" : "RESULT: invalid map\n");
    }

    std::cout << "\n=== MapLoader copy and assignment ===\n";
    MapLoader copiedLoader(loader);
    MapLoader assignedLoader;
    assignedLoader = copiedLoader;
    std::cout << "Original: " << loader << "\n";
    std::cout << "Copy:     " << copiedLoader << "\n";
    std::cout << "Assigned: " << assignedLoader << "\n";

    return 0;
}
