#ifndef PODIO_COLLECTIONBUFFERREGISTRY_H
#define PODIO_COLLECTIONBUFFERREGISTRY_H

#include "podio/CollectionBuffers.h"

#include <functional>
#include <optional>
#include <string>
#include <unordered_map>

namespace podio {

class CollectionBufferRegistry {
  using KeyT = std::string;
  using FunctionT = std::function<podio::CollectionReadBuffers(bool)>;
  using MapT = std::unordered_map<KeyT, FunctionT>;

public:
  CollectionBufferRegistry(CollectionBufferRegistry const&) = delete;
  CollectionBufferRegistry& operator=(CollectionBufferRegistry const&) = delete;
  CollectionBufferRegistry(CollectionBufferRegistry&&) = delete;
  CollectionBufferRegistry& operator=(CollectionBufferRegistry&&) = delete;
  ~CollectionBufferRegistry() = default;

  static CollectionBufferRegistry& mutInstance();

  static CollectionBufferRegistry const& instance();

  void registerCollType(const std::string& collType, FunctionT createFunc);

  std::optional<podio::CollectionReadBuffers> createBuffers(const std::string& collType, bool subsetColl) const;

private:
  CollectionBufferRegistry() = default;

  MapT m_functionMap{};
};

} // namespace podio

#endif // PODIO_COLLECTIONBUFFERREGISTRY_H
