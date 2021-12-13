#ifndef PODIO_ROOTRAWDATA_H
#define PODIO_ROOTRAWDATA_H

#include "podio/CollectionBuffers.h"

// #include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace podio {

/**
 * The raw data format returned by the ROOTReader
 */
class ROOTRawData {
public:
  ROOTRawData() = default;
  ROOTRawData(const ROOTRawData&) = delete;
  ROOTRawData& operator=(const ROOTRawData&) = delete;
  ROOTRawData(ROOTRawData&&) = default;
  ROOTRawData& operator=(ROOTRawData&&) = default;
  ~ROOTRawData() = default;

  const std::vector<std::string>& getAvailableCollections() const {
    return m_collectionNames;
  }

  ROOTRawData(std::unordered_map<std::string, size_t>&& idxMap, std::vector<podio::CollectionBuffers>&& buffers) :
      m_idToIdxMap(std::move(idxMap)), m_buffers(std::move(buffers)) {
    // TODO: Make this filling "lazy" and only do it once this is actually
    // requested, needs some locking for multithreading then, because
    // getAvailableCollections should be const
    m_collectionNames.reserve(m_idToIdxMap.size());
    for (const auto& [name, index] : m_idToIdxMap) {
      m_collectionNames.push_back(name);
    }
  }

  std::optional<podio::CollectionBuffers> getCollectionBuffers(const std::string& name) const {
    if (const auto it = m_idToIdxMap.find(name); it != m_idToIdxMap.end()) {
      return {m_buffers[it->second]};
    }
    return std::nullopt;
  }

private:
  std::unordered_map<std::string, size_t> m_idToIdxMap{};
  std::vector<podio::CollectionBuffers> m_buffers{};
  std::vector<std::string> m_collectionNames{};
};

} // namespace podio

#endif // PODIO_ROOTRAWDATA_H
