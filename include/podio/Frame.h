#ifndef PODIO_FRAME_H
#define PODIO_FRAME_H

#include "podio/CollectionBase.h"
#include "podio/CollectionBuffers.h"
#include "podio/CollectionIDTable.h"
#include "podio/ICollectionProvider.h"
#include "podio/ReaderRawData.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace podio {
namespace detail {
  struct EmptyRawData {
    const std::vector<std::string>& getAvailableCollections() const {
      static auto emptyVec = std::vector<std::string>();
      return emptyVec;
    }

    std::optional<podio::CollectionBuffers> getCollectionBuffers(const std::string&) const {
      return std::nullopt;
    }
  };
} // namespace detail

/**
 * The Frame class that serves as a container of collections and meta data
 */
class Frame {
  /**
   * The frame concept that defines the basic functionality
   */
  struct FrameConcept {
    virtual ~FrameConcept() = default;
    virtual const podio::CollectionBase* get(const std::string& name) const = 0;
  };

  /**
   * The templated frame model that implements the concept and that is
   * constructed in the Frame constructor
   */
  template <typename RawDataT, typename UnpackingPolicy>
  struct FrameModel final : FrameConcept {
    using CollectionMapT = std::unordered_map<std::string, std::unique_ptr<podio::CollectionBase>>;

    /// Constructor for an empty event
    FrameModel() = default;

    /// Constructor from some raw data
    FrameModel(std::unique_ptr<RawDataT> rawData);

    /// Get the corresponding
    const podio::CollectionBase* get(const std::string& name) const final;

    // TODO: locking
    mutable CollectionMapT m_collections{};
    std::unique_ptr<RawDataT> m_rawData{nullptr};
  };

  std::unique_ptr<FrameConcept> m_self;

public:
  /// Construct an empty Frame (without any data)
  template <typename UnpackingPolicy>
  Frame(UnpackingPolicy);

  /// Construct a Frame from some raw data
  template <typename RawDataT, typename UnpackingPolicy>
  Frame(std::unique_ptr<RawDataT> rawData, UnpackingPolicy);

  /// Get a collection of a given type via its name
  template <typename CollT>
  const CollT& get(const std::string& name) const;
};

/////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATIONS BELOW (move to a .tpp file or siilar and include here?) //
/////////////////////////////////////////////////////////////////////////////
// =================== Frame implementations ==============================
template <typename UnpackingPolicy>
Frame::Frame(UnpackingPolicy) : m_self(std::make_unique<FrameModel<detail::EmptyRawData, UnpackingPolicy>>()) {
}

template <typename RawDataT, typename UnpackingPolicy>
Frame::Frame(std::unique_ptr<RawDataT> rawData, UnpackingPolicy) :
    m_self(std::make_unique<FrameModel<RawDataT, UnpackingPolicy>>(std::move(rawData))) {
}

template <typename CollT>
const CollT& Frame::get(const std::string& name) const {
  const auto* coll = static_cast<const CollT*>(m_self->get(name));
  if (coll) {
    return &coll;
  }
  // TODO: less happy case via policy?
  static auto& defaultColl = CollT();
  return defaultColl;
}

// =================== FrameModel implementations =========================
template <typename RawDataT, typename UnpackingPolicy>
Frame::FrameModel<RawDataT, UnpackingPolicy>::FrameModel(std::unique_ptr<RawDataT> rawData) :
    m_rawData(std::move(rawData)) {
  auto buffers = UnpackingPolicy::getCollectionBuffers(m_rawData.get());
  for (auto& b : buffers) {
    // TODO: schema evolution, emplacing in collection map
  }
}

template <typename RawDataT, typename UnpackingPolicy>
const podio::CollectionBase* Frame::FrameModel<RawDataT, UnpackingPolicy>::get(const std::string& name) const {
  if (const auto it = m_collections.find(name); it != m_collections.end()) {
    return it->second.get();
  }

  if (m_rawData) {
    auto buffers = UnpackingPolicy::unpack(m_rawData.get(), name);
    // TODO: schema evolution
    //
    // TODO; check return value of emplace? (At this point it is possible
    // that someone has already put in a collection before this one was
    // requested)
    const auto [it, success] = m_collections.emplace(name, buffers->createCollection());
    return it->second.get();
  }

  return nullptr;
}

} // namespace podio

#endif // PODIO_FRAME_H
