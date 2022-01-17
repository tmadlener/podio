#ifndef PODIO_UNPACKINGPOLICIES_H
#define PODIO_UNPACKINGPOLICIES_H

#include "podio/CollectionBuffers.h"

#include <cassert>
#include <optional>

// Unpacking policies define when the collection buffers are obtained from the
// raw data of a given reader. They define two static functions. One controls
// what happens during event initialization / construction, while the second
// controls what happens when a collection is requested from an Event. The basic
// interface is the following
//
//     struct UnpackingPolicy {
//       template <typename RawDataT>
//       static std::vector<podio::CollectionBuffers> getCollectionBuffers(RawDataT*);
//
//       template <typename RawDataT>
//       static std::optional<podio::CollectionBuffers> unpack(RawDataT*, const std::string&);
//     };
//
// This header defines two default policies: One where everything happens during
// initialization and another one, where everything happens on demand only.

namespace podio {
/**
 * Struct describing the eager unpacking policy, i.e. everything gets done
 * during initialization / construction of an Event, leaving nothing to be done
 * for later.
 */
struct EagerUnpacking {
  template <typename RawDataT>
  static std::vector<podio::CollectionBuffers> getCollectionBuffers(RawDataT* rawData) {
    std::vector<podio::CollectionBuffers> allBuffers;
    for (const auto& name : rawData->getAvailableCollections()) {
      auto buffers = rawData->getCollectionBuffers(name);
      allBuffers.push_back(buffers.value()); // Can safely get the value here
    }
    return allBuffers;
  }

  template <typename RawDataT>
  static std::optional<podio::CollectionBuffers> unpack(RawDataT*, const std::string&) {
    // Since everything is unpacked already during initialization this is
    // basically unreachable
    assert(false);
    return std::nullopt;
  }
};

/**
 * Struct describing the lazy unpacking policy, i.e. nothing gets done during
 * the initialization / construction of an Event, and everything is done
 * "lazily" on demand when a collection is first requested.
 */
struct LazyUnpacking {
  template <typename RawDataT>
  static std::vector<podio::CollectionBuffers> getCollectionBuffers(RawDataT*) {
    return {};
  }

  template <typename RawDataT>
  static std::optional<podio::CollectionBuffers> unpack(RawDataT* rawData, const std::string& name) {
    return rawData->getCollectionBuffers(name);
  }
};
} // namespace podio

#endif // PODIO_UNPACKINGPOLICIES_H
