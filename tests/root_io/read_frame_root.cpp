#include "read_frame.h"
#include "read_frame_auxiliary.h"

#include "podio/Frame.h"
#include "podio/ROOTFrameReader.h"

#include <iostream>
#include <set>
#include <string>
#include <vector>

int test_read_frame_limited(const std::string& inputFile) {
  auto reader = podio::ROOTFrameReader();
  reader.openFile(inputFile);
  const std::vector<std::string> collsToRead = {"mcparticles", "clusters"};

  const auto event = podio::Frame(reader.readNextEntry("events", collsToRead));

  const auto& availColls = event.getAvailableCollections();

  const bool validColls =
      std::set(availColls.begin(), availColls.end()) != std::set(collsToRead.begin(), collsToRead.end());

  if (validColls) {
    std::cerr << "The available collections are not as expected" << std::endl;
    return 1;
  }

  if (!event.get("mcparticles")) {
    std::cerr << "Collection 'mcparticles' should be available" << std::endl;
    return 1;
  }

  if (event.get("hits")) {
    std::cerr << "Collection 'hits' is available, but should not be" << std::endl;
    return 1;
  }

  const auto& clusters = event.get<ExampleClusterCollection>("clusters");
  const auto clu0 = clusters[0];
  const auto hits = clu0.Hits();
  if (hits.size() != 1 || hits[0].isAvailable()) {
    std::cerr << "Hit in clusters are available but shouldn't be" << std::endl;
    return 1;
  }

  return 0;
}

int main(int argc, char* argv[]) {
  std::string inputFile = "example_frame.root";
  bool assertBuildVersion = true;
  if (argc == 2) {
    inputFile = argv[1];
    assertBuildVersion = false;
  }

  return read_frames<podio::ROOTFrameReader>(inputFile, assertBuildVersion) +
      test_frame_aux_info<podio::ROOTFrameReader>(inputFile) + test_read_frame_limited(inputFile);
}
