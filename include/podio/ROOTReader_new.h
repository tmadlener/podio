#ifndef PODIO_ROOTREADER_NEW_H
#define PODIO_ROOTREADER_NEW_H

#include "podio/CollectionBranches.h"
#include "podio/CollectionIDTable.h"
#include "podio/ROOTRawData.h"

#include "TChain.h"

#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace podio {
class ROOTReader_new {
public:
  ROOTReader_new() = default;
  ~ROOTReader_new() = default;

  ROOTReader_new(ROOTReader_new const&) = delete;
  ROOTReader_new& operator=(ROOTReader_new const&) = delete;

  void openFile(const std::string& filename);
  void openFiles(const std::vector<std::string>& filenames);

  unsigned getEntries() const {
    return m_chain->GetEntries();
  }

  // Get all available collection buffers
  std::unique_ptr<ROOTRawData> readNextEventRawData();

  std::shared_ptr<const CollectionIDTable> getIdTable() {
    return m_idTable;
  }

private:
  // Metadata information to identify the type of each collection
  // collection id, name and subsetCollection
  using CollectionTypeInfo = std::tuple<int, std::string, bool>;

  // Information about the data vector as wall as the collection class type
  // and the collection ID
  using CollectionInfo = std::tuple<const TClass*, const TClass*, int>;

  void createCollectionBranches(const std::vector<CollectionTypeInfo>& collInfo);

  std::unique_ptr<TChain> m_chain{nullptr};
  std::shared_ptr<CollectionIDTable> m_idTable{nullptr};
  unsigned m_eventNumber{0};

  struct CollectionReadInfo {
    CollectionReadInfo(const TClass* bc, const TClass* cc, root_utils::CollectionBranches&& br) :
        bufferClass(bc), collectionClass(cc), branches(std::move(br)) {
    }
    CollectionReadInfo(CollectionReadInfo const&) = delete;
    CollectionReadInfo(CollectionReadInfo&&) = delete;
    CollectionReadInfo& operator=(CollectionReadInfo const&) = delete;
    CollectionReadInfo& operator=(CollectionReadInfo&&) = delete;
    ~CollectionReadInfo() = default;

    const TClass* bufferClass{nullptr};
    const TClass* collectionClass{nullptr};
    mutable root_utils::CollectionBranches branches{};
  };

  std::unordered_map<std::string, CollectionReadInfo> m_collectionInfo{};
};
} // namespace podio

#endif // PODIO_ROOTREADER_NEW_H
