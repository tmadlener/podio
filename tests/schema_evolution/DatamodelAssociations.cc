#include "DatamodelAssociations.h"

#include "datamodel/ExampleClusterCollection.h"
#include "datamodel/ExampleHitCollection.h"

#include "podio/AssociationCollection.h"
#include "podio/CollectionBufferFactory.h"
#include "podio/CollectionBuffers.h"
#include "podio/DatamodelRegistry.h"
#include "podio/SchemaEvolution.h"
#include "podio/detail/AssociationFwd.h"

#include <vector>

PODIO_DECLARE_ASSOCIATION(ExampleHit, ExampleCluster)

// using ExampleAssociationContainer = podio::AssociationDataContainer;

namespace {
auto evolveAssociations(podio::CollectionReadBuffers oldBuffers, podio::SchemaVersionT) {
  podio::CollectionReadBuffers newBuffers{};
  newBuffers.type = "podio::AssociationCollection<ExampleHit,ExampleCluster>";
  newBuffers.schemaVersion = 3;
  // For testing assume: no subset colletions!
  // Otherwise we don't need to do anything here

  newBuffers.references = oldBuffers.references;
  oldBuffers.references = nullptr;
  newBuffers.vectorMembers = newBuffers.vectorMembers;
  oldBuffers.vectorMembers = nullptr;

  // We can simply "recast" from the old struct to the new one here since memory
  // layout doesn't change
  newBuffers.data = oldBuffers.dataAsVector<podio::AssociationData>();
  oldBuffers.data = nullptr;

  newBuffers.createCollection = [](const podio::CollectionReadBuffers& buffers, bool isSubsetColl) {
    podio::AssociationCollectionData<ExampleHit, ExampleCluster> data(buffers, isSubsetColl);
    return std::make_unique<podio::AssociationCollection<ExampleHit, ExampleCluster>>(std::move(data), isSubsetColl);
  };

  newBuffers.recast = [](podio::CollectionReadBuffers& buffers) {
    if (buffers.data) {
      buffers.data = podio::CollectionWriteBuffers::asVector<podio::AssociationData>(buffers.data);
    }
  };

  newBuffers.deleteBuffers = [](podio::CollectionReadBuffers& buffers) {
    if (buffers.data) {
      delete static_cast<std::vector<podio::AssociationData>*>(buffers.data);
    }

    delete buffers.references;
    delete buffers.vectorMembers;
  };

  return newBuffers;
}

auto createAssocBuffers(bool isSubset) {
  auto readBuffers = podio::CollectionReadBuffers{};
  readBuffers.type = "ExampleAssociationCollection";
  readBuffers.schemaVersion = 2;
  readBuffers.data = isSubset ? nullptr : new podio::AssociationDataContainer;
  // The number of ObjectID vectors is either 1 or the sum of OneToMany and
  // OneToOne relations
  const auto nRefs = isSubset ? 1 : 0 + 2;
  readBuffers.references = new podio::CollRefCollection(nRefs);
  for (auto& ref : *readBuffers.references) {
    // Make sure to place usable buffer pointers here
    ref = std::make_unique<std::vector<podio::ObjectID>>();
  }

  readBuffers.vectorMembers = new podio::VectorMembersInfo();
  if (!isSubset) {
    readBuffers.vectorMembers->reserve(0);
  }

  // In the example here we will never create a collection from this
  readBuffers.createCollection = [](const podio::CollectionReadBuffers&, bool) { return nullptr; };

  readBuffers.recast = [](podio::CollectionReadBuffers& buffers) {
    // We only have any of these buffers if this is not a subset collection
    if (buffers.data) {
      buffers.data = podio::CollectionWriteBuffers::asVector<podio::AssociationData>(buffers.data);
    }
  };

  readBuffers.deleteBuffers = [](podio::CollectionReadBuffers& buffers) {
    if (buffers.data) {
      // If we have data then we are not a subset collection and we have to
      // clean up all type erased buffers by casting them back to something that
      // we can delete
      delete static_cast<podio::AssociationDataContainer*>(buffers.data);
    }
    delete buffers.references;
    delete buffers.vectorMembers;
  };

  return readBuffers;
}

bool reg() {
  const static auto registerSchemaAssociation = []() {
    podio::CollectionBufferFactory::mutInstance().registerCreationFunc("ExampleAssociationCollection", 2,
                                                                       createAssocBuffers);

    using namespace std::string_view_literals;

    static const auto relNames = podio::RelationNameMapping{{"ExampleAssociation"sv, {"from"sv, "to"sv}, {}}};

    podio::DatamodelRegistry::mutInstance().registerDatamodel("meta_datamodel", "{}", relNames);

    podio::SchemaEvolution::mutInstance().registerEvolutionFunc(
        "ExampleAssociationCollection", 2, 3, evolveAssociations, podio::SchemaEvolution::Priority::UserDefined);

    return true;
  }();
  return registerSchemaAssociation;
}

const auto regis = reg();
} // namespace
