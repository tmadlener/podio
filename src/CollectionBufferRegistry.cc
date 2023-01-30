#include "podio/CollectionBufferRegistry.h"
#include "podio/CollectionBuffers.h"

namespace podio {

CollectionBufferRegistry const& CollectionBufferRegistry::instance() {
  static CollectionBufferRegistry registry;
  return registry;
}

CollectionBufferRegistry& CollectionBufferRegistry::mutInstance() {
  return const_cast<CollectionBufferRegistry&>(CollectionBufferRegistry::instance());
}

void CollectionBufferRegistry::registerCollType(const std::string& collType, FunctionT createFunc) {
  const auto& [it, inserted] = m_functionMap.emplace(collType, std::move(createFunc));
  if (!inserted) {
    // TODO: what do we actually want to do here? What can we do?
  }
}

std::optional<podio::CollectionReadBuffers> CollectionBufferRegistry::createBuffers(const std::string& collType,
                                                                                    bool subsetColl) const {
  if (const auto it = m_functionMap.find(collType); it != m_functionMap.end()) {
    return it->second(subsetColl);
  }

  return std::nullopt;
}

} // namespace podio
