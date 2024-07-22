#ifndef PODIO_TESTS_SCHEMAEVOLUTION_WRITEOLDDATA_H // NOLINT(llvm-header-guard): folder structure not suitable
#define PODIO_TESTS_SCHEMAEVOLUTION_WRITEOLDDATA_H // NOLINT(llvm-header-guard): folder structure not suitable

#include "datamodel/ExampleAssociationCollection.h"
#include "datamodel/ExampleClusterCollection.h"
#include "datamodel/ExampleHitCollection.h"
#include "datamodel/ExampleWithARelationCollection.h"
#include "datamodel/ExampleWithArrayComponentCollection.h"
#include "datamodel/ExampleWithNamespaceCollection.h"

#include "podio/Frame.h"

#include <string>

using TestAssocCollection = ExampleAssociationCollection;

/// This is a datatype that holds a SimpleStruct component
auto writeSimpleStruct() {
  ExampleWithArrayComponentCollection coll;
  auto elem = coll.create();
  auto sstruct = SimpleStruct();
  sstruct.x = 42;
  sstruct.z = 123;
  elem.s(sstruct);

  return coll;
}

auto writeExampleHit() {
  ExampleHitCollection coll;
  auto elem = coll.create();
  elem.x(1.23);
  elem.y(1.23);
  elem.z(1.23);
  elem.cellID(0xcaffee);

  return coll;
}

auto writeExampleCluster() {
  ExampleClusterCollection coll;
  auto elem = coll.create();

  return coll;
}

auto createAssociationCollection(const ExampleHitCollection& hits, const ExampleClusterCollection& clusters) {
  TestAssocCollection associations{};
  const auto nAssocs = std::min(clusters.size(), hits.size());
  for (size_t iA = 0; iA < nAssocs; ++iA) {
    auto assoc = associations.create();
    assoc.weight(0.5 * iA);
    // assoc.setWeight(0.5 * iA);

    // Fill in opposite "order" to at least make sure that we uncover issues
    // that would otherwise be masked by parallel running of indices
    // assoc.setFrom(hits[iA]);
    // assoc.setTo(clusters[nAssocs - 1 - iA]);
    assoc.from(hits[iA]);
    assoc.to(clusters[nAssocs - 1 - iA]);
  }

  return associations;
}

auto writeExampleWithNamespace() {
  ex42::ExampleWithNamespaceCollection coll;
  auto elem = coll.create();
  elem.y(42);
  elem.x(123);

  return coll;
}

auto writeExamplewWithARelation() {
  ex42::ExampleWithARelationCollection coll;
  auto elem = coll.create();
  elem.number(3.14f);

  return coll;
}

podio::Frame createFrame() {
  podio::Frame event;

  event.put(writeSimpleStruct(), "simpleStructTest");
  const auto& hits = event.put(writeExampleHit(), "datatypeMemberAdditionTest");
  event.put(writeExampleWithNamespace(), "componentMemberRenameTest");
  event.put(writeExamplewWithARelation(), "floatToDoubleMemberTest");
  const auto& clusters = event.put(writeExampleCluster(), "clusters");
  event.put(createAssociationCollection(hits, clusters), "associations");

  return event;
}

template <typename WriterT>
int writeData(const std::string& filename) {
  WriterT writer(filename);

  auto event = createFrame();
  writer.writeFrame(event, "events");

  writer.finish();

  return 0;
}

#endif // PODIO_TESTS_SCHEMAEVOLUTION_WRITEOLDDATA_H
