#include "Map.h"
#include <iostream>
#include <string>
#include <vector>

// Builds the good map. Westeros has Winterfell, King's Landing and Dorne. Essos
// has Pentos, Braavos, Meereen and Qarth. The links are:
//   Winterfell - King's Landing - Dorne
//   King's Landing - Pentos (the Narrow Sea crossing)
//   Pentos - Braavos, Pentos - Meereen, Meereen - Qarth
// If linkNarrowSea is false, the King's Landing - Pentos link is left out.
// The caller has to delete what comes back.
Map *buildBaseMap(bool linkNarrowSea) {
  Map *map = new Map();
  Continent *westeros = map->addContinent("Westeros", 5);
  Continent *essos = map->addContinent("Essos", 3);

  Territory *winterfell = map->addTerritory("Winterfell");
  Territory *kingsLanding = map->addTerritory("King's Landing");
  Territory *dorne = map->addTerritory("Dorne");
  Territory *pentos = map->addTerritory("Pentos");
  Territory *braavos = map->addTerritory("Braavos");
  Territory *meereen = map->addTerritory("Meereen");
  Territory *qarth = map->addTerritory("Qarth");

  map->assignToContinent(winterfell, westeros);
  map->assignToContinent(kingsLanding, westeros);
  map->assignToContinent(dorne, westeros);
  map->assignToContinent(pentos, essos);
  map->assignToContinent(braavos, essos);
  map->assignToContinent(meereen, essos);
  map->assignToContinent(qarth, essos);

  map->addEdge(winterfell, kingsLanding);
  map->addEdge(kingsLanding, dorne);
  map->addEdge(pentos, braavos);
  map->addEdge(pentos, meereen);
  map->addEdge(meereen, qarth);
  if (linkNarrowSea)
    map->addEdge(kingsLanding, pentos);
  return map;
}

// Prints a title, runs validate() on the map, then says whether it passed
// overall.
void runValidation(const std::string &title, const Map &map) {
  std::cout << "\n=== " << title << " ===\n";
  bool ok = map.validate();
  std::cout << (ok ? "RESULT: valid map\n" : "RESULT: invalid map\n");
}

// Kaila's tests: builds maps by hand and runs validate() on each of them.
void testHandBuiltMaps() {
  // Test 1: the good map. Everything should pass.
  Map *valid = buildBaseMap(true);
  std::cout << *valid;
  runValidation("Test 1: valid map", *valid);

  // Test 2: deep copy. If it's really deep, the copy keeps working after we
  // delete the original.
  Map copy(*valid);
  std::cout << "\n=== Test 2: deep copy ===\n";
  std::cout << "Original Winterfell at " << valid->findTerritory("Winterfell")
            << ", copy Winterfell at " << copy.findTerritory("Winterfell")
            << " (addresses differ)\n";
  delete valid;
  std::cout << "Original deleted, copy still works:\n";
  std::cout << copy;
  runValidation("Test 2: copy after original deleted", copy);

  // Test 3: same map but with no King's Landing - Pentos link, so the Narrow
  // Sea splits it into a Westeros half and an Essos half with no way across.
  Map *disconnected = buildBaseMap(false);
  runValidation("Test 3: disconnected map", *disconnected);
  delete disconnected;

  // Test 4: the whole map is connected, but Westeros isn't. King's Landing and
  // Dragonstone have no link to each other, so the only way between them is
  // through Pentos, which belongs to Essos. Essos is fine: Pentos reaches
  // Braavos and Meereen, and Meereen reaches Qarth.
  Map *splitContinent = new Map();
  Continent *westeros = splitContinent->addContinent("Westeros", 5);
  Continent *essos = splitContinent->addContinent("Essos", 3);
  Territory *kingsLanding = splitContinent->addTerritory("King's Landing");
  Territory *dragonstone = splitContinent->addTerritory("Dragonstone");
  Territory *pentos = splitContinent->addTerritory("Pentos");
  Territory *braavos = splitContinent->addTerritory("Braavos");
  Territory *meereen = splitContinent->addTerritory("Meereen");
  Territory *qarth = splitContinent->addTerritory("Qarth");
  splitContinent->assignToContinent(kingsLanding, westeros);
  splitContinent->assignToContinent(dragonstone, westeros);
  splitContinent->assignToContinent(pentos, essos);
  splitContinent->assignToContinent(braavos, essos);
  splitContinent->assignToContinent(meereen, essos);
  splitContinent->assignToContinent(qarth, essos);
  splitContinent->addEdge(pentos, kingsLanding);
  splitContinent->addEdge(pentos, dragonstone);
  splitContinent->addEdge(pentos, braavos);
  splitContinent->addEdge(pentos, meereen);
  splitContinent->addEdge(meereen, qarth);
  runValidation("Test 4: Westeros not connected", *splitContinent);
  delete splitContinent;

  // Test 5: the Narrow Sea gets claimed by both Westeros and Essos, so it ends
  // up in two continents at once.
  Map *twoContinents = buildBaseMap(true);
  Territory *narrowSea = twoContinents->addTerritory("Narrow Sea");
  twoContinents->addEdge(narrowSea,
                         twoContinents->findTerritory("King's Landing"));
  twoContinents->addEdge(narrowSea, twoContinents->findTerritory("Pentos"));
  twoContinents->assignToContinent(narrowSea,
                                   twoContinents->findContinent("Westeros"));
  twoContinents->assignToContinent(narrowSea,
                                   twoContinents->findContinent("Essos"));
  runValidation("Test 5: Narrow Sea in two continents", *twoContinents);
  delete twoContinents;

  // Test 6: The Wall is hooked up to Winterfell but someone forgot to put it in
  // a continent.
  Map *noContinent = buildBaseMap(true);
  Territory *theWall = noContinent->addTerritory("The Wall");
  noContinent->addEdge(theWall, noContinent->findTerritory("Winterfell"));
  runValidation("Test 6: The Wall in no continent", *noContinent);
  delete noContinent;
}

// Loads one map file and validates it. Returns true if the map is accepted.
bool testMapFile(MapLoader &loader, const std::string &fileName) {
  std::cout << "\n=== " << fileName << " ===\n";
  Map *map = loader.loadMap(fileName);
  if (map == nullptr) {
    std::cout << "RESULT: rejected by the loader (" << loader.getLastError()
              << ")\n";
    return false;
  }
  std::cout << *map;
  bool valid = map->validate();
  std::cout << (valid ? "RESULT: valid map, accepted\n"
                      : "RESULT: invalid map, rejected\n");
  delete map;
  return valid;
}

// Loads the map files, then tests the MapLoader copy constructor and assignment
// operator.
void testMapFiles() {
  MapLoader loader;

  // paths are relative to the project root
  std::vector<std::string> mapFiles = {// valid map
                                       "maps/westeros_essos.map",
                                       // load fine but fail validate()
                                       "maps/disconnected.map",
                                       "maps/split_continent.map",
                                       "maps/two_continents.map",
                                       // rejected by the loader
                                       "maps/not_a_map.txt"};

  int accepted = 0;
  for (const std::string &fileName : mapFiles) {
    if (testMapFile(loader, fileName))
      ++accepted;
  }
  std::cout << "\n"
            << accepted << " of " << mapFiles.size()
            << " map files were accepted\n";

  std::cout << "\n=== MapLoader copy and assignment ===\n";
  MapLoader copiedLoader(loader);
  MapLoader assignedLoader;
  assignedLoader = copiedLoader;
  std::cout << "Original: " << loader << "\n";
  std::cout << "Copy:     " << copiedLoader << "\n";
  std::cout << "Assigned: " << assignedLoader << "\n";
}

// Runs the hand-built map tests, then the map file tests.
int main() {
  std::cout << "##### Hand-built maps #####\n";
  testHandBuiltMaps();
  std::cout << "\n##### Map files #####\n";
  testMapFiles();
  return 0;
}
