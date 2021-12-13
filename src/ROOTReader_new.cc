#include "podio/ROOTReader_new.h"

#include "podio/CollectionBase.h"
#include "rootUtils.h"

#include "TFile.h"
#include "TTree.h"

namespace podio {

std::unique_ptr<podio::ROOTRawData> ROOTReader_new::readNextEventRawData() {
  std::unordered_map<std::string, size_t> idxMap;
  std::vector<podio::CollectionBuffers> buffers;
  size_t currColl = 0;

  for (const auto& [name, collInfo] : m_collectionInfo) {
    const auto& [bufferClass, collectionClass, branches] = collInfo;

    auto* collection = static_cast<CollectionBase*>(collectionClass->New());
    auto collBuffers = collection->getBuffers();
    // If we have a valid data buffer class we know that have to read data,
    // otherwise we are handling a subset collection
    if (bufferClass) {
      collBuffers.data = bufferClass->New();
    } else {
      collection->setSubsetCollection();
    }

    const auto localEntry = m_chain->LoadTree(m_eventNumber);
    // After switching trees in the chain, branch pointers get invalidated so
    // they need to be reassigned.
    // NOTE: root 6.22/06 requires that we get completely new branches here,
    // with 6.20/04 we could just re-set them
    if (localEntry == 0) {
      branches.data = root_utils::getBranch(m_chain.get(), name.c_str());

      // reference collections
      if (auto* refCollections = collBuffers.references) {
        for (size_t i = 0; i < refCollections->size(); ++i) {
          const auto brName = root_utils::refBranch(name, i);
          branches.refs[i] = root_utils::getBranch(m_chain.get(), brName.c_str());
        }
      }

      // vector members
      if (auto* vecMembers = collBuffers.vectorMembers) {
        for (size_t i = 0; i < vecMembers->size(); ++i) {
          const auto brName = root_utils::vecBranch(name, i);
          branches.vecs[i] = root_utils::getBranch(m_chain.get(), brName.c_str());
        }
      }
    }

    // set the addresses
    root_utils::setCollectionAddresses(collection, branches);

    // Read all data
    if (branches.data) {
      branches.data->GetEntry(m_eventNumber);
    }
    for (auto* br : branches.refs) {
      br->GetEntry(m_eventNumber);
    }
    for (auto* br : branches.vecs) {
      br->GetEntry(m_eventNumber);
    }

    buffers.push_back(collBuffers);
    idxMap.emplace(name, currColl++);
  }

  m_eventNumber++;
  return std::make_unique<ROOTRawData>(std::move(idxMap), std::move(buffers));
}

void ROOTReader_new::openFile(std::string const& filename) {
  openFiles({filename});
}

void ROOTReader_new::openFiles(std::vector<std::string> const& fileNames) {
  m_chain = std::make_unique<TChain>("events");
  for (const auto& filename : fileNames) {
    m_chain->Add(filename.c_str());
  }

  // read the meta data and build the collectionBranches cache
  // NOTE: This is a small pessimization, if we do not read all collections
  // afterwards, but it makes the handling much easier in general
  auto metadatatree = static_cast<TTree*>(m_chain->GetFile()->Get("metadata"));
  m_idTable = std::make_shared<CollectionIDTable>();
  auto tblPtr = m_idTable.get();
  metadatatree->SetBranchAddress("CollectionIDs", &tblPtr);

  auto collectionInfo = std::make_unique<CollectionTypeInfo>();
  auto infoPtr = collectionInfo.get();
  metadatatree->SetBranchAddress("CollectionTypeInfo", &infoPtr);
  metadatatree->GetEntry(0);
}

void ROOTReader_new::createCollectionBranches(std::vector<CollectionTypeInfo> const& collInfo) {
  for (const auto& [collID, collType, isSubsetColl] : collInfo) {
    // We only write collections that are in the collectionIDTable, so no need
    // to check here
    const auto name = m_idTable->name(collID);

    root_utils::CollectionBranches branches{};
    const auto collectionClass = TClass::GetClass(collType.c_str());

    // Need the collection here to setup all the branches. Have to manage the
    // temporary collection ourselves
    auto collection =
        std::unique_ptr<podio::CollectionBase>(static_cast<podio::CollectionBase*>(collectionClass->New()));
    collection->setSubsetCollection(isSubsetColl);

    if (!isSubsetColl) {
      // This branch is guaranteed to exist since only collections that are
      // also written to file are in the info metadata that we work with here
      branches.data = root_utils::getBranch(m_chain.get(), name.c_str());
    }

    const auto buffers = collection->getBuffers();
    for (size_t i = 0; i < buffers.references->size(); ++i) {
      const auto brName = root_utils::refBranch(name, i);
      branches.refs.push_back(root_utils::getBranch(m_chain.get(), brName.c_str()));
    }

    for (size_t i = 0; i < buffers.vectorMembers->size(); ++i) {
      const auto brName = root_utils::vecBranch(name, i);
      branches.vecs.push_back(root_utils::getBranch(m_chain.get(), brName.c_str()));
    }

    const std::string bufferClassName = "std::vector<" + collection->getValueTypeName() + "Data>";
    const auto bufferClass = isSubsetColl ? nullptr : TClass::GetClass(bufferClassName.c_str());

    // TODO: switch to try_emplace?
    m_collectionInfo.emplace(std::piecewise_construct, std::forward_as_tuple(std::move(name)),
                             std::forward_as_tuple(bufferClass, collectionClass, std::move(branches)));
  }
}

} // namespace podio
