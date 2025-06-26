#include "read_new_data.h"

#include "podio/ROOTReader.h"

#include "TSystem.h"

int main() {
  // auto res = gSystem->Load("libmanualDatamodelEvolution.so");
  // std::cout << res << '\n';
  return read_new_data<podio::ROOTReader>("example_data_old_schema.root");
}
