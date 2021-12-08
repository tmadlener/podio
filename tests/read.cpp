#include "podio/ROOTReader.h"
#include "read_test.h"

int main() {
  auto reader = podio::ROOTReader();
  reader.openFile("example.root");

  run_read_test(reader);

  reader.closeFile();
  return 0;
}
