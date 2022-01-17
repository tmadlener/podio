#ifndef PODIO_FRAME_H
#define PODIO_FRAME_H

#include "podio/CollectionBase.h"
#include "podio/CollectionBuffers.h"
#include "podio/CollectionIDTable.h"
#include "podio/ICollectionProvider.h"
// Some policies
#include "podio/CollisionPolicies.h"
#include "podio/UnpackingPolicies.h"
#include "podio/UserDataCollection.h"

#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace podio {
namespace detail {
  /**
   * An empty raw data stub that is used as default for Frames that are created empty
   */
  struct EmptyRawData {
    const std::vector<std::string>& getAvailableCollections() const {
      static auto emptyVec = std::vector<std::string>();
      return emptyVec;
    }

    std::optional<podio::CollectionBuffers> getCollectionBuffers(const std::string&) const {
      return std::nullopt;
    }
  };

  // TODO: Is there a better way for checking the presence of different policies
  // in a type? The approach below is not rocket science and basically can be
  // copy - pasted easily, but it is quite a bit of boiler plate, and I have the
  // feeling it should be possible to get something where adding a new policy
  // should only be declaring one additional struct inheriting from another one.
  // However, I haven't found one yet and I am not entirely sure we will need
  // such fine grained checks in the end, since we basically just want to SFINAE
  // out a few constructor signatures below to make sure that the default
  // constructors always do the "sensible" thing for the users.

  /**
   * Helper struct to determine whether a type has an UnpackingPolicy
   */
  template <typename T, typename = void>
  struct HasUnpackingPolicyT : std::false_type {};

  template <typename T>
  struct HasUnpackingPolicyT<T, std::void_t<typename T::UnpackingPolicy>> : std::true_type {};

  template <typename T>
  constexpr bool HasUnpackingPolicy = HasUnpackingPolicyT<T>::value;

  /**
   * Helper struct to determine whetehr a type has a CollissionPolicy
   */
  template <typename T, typename = void>
  struct HasCollisionPolicyT : std::false_type {};

  template <typename T>
  struct HasCollisionPolicyT<T, std::void_t<typename T::CollisionPolicy>> : std::true_type {};

  template <typename T>
  constexpr bool HasCollisionPolicy = HasCollisionPolicyT<T>::value;

  /**
   * Alias template to be used for enabling/disabling certain constructors below
   * to make sure that the arguments match their types (and avoid trying to call
   * mismatching constructors with the same number of arguments)
   */
  template <typename T>
  using EnableIfValidFramePolicies = std::enable_if<HasUnpackingPolicy<T> && HasCollisionPolicy<T>>;

} // namespace detail

/**
 * The default frame policies. Defines typedefs for policies that will then be
 * used in the frame where necessary. The main purpose of this is to not have
 * tons of policies, that have to be passed everywhere, but to rather collect
 * them in one place and pass that.
 */
struct FrameDefaultPolicies {
  using UnpackingPolicy = EagerUnpacking;
  using CollisionPolicy = ThrowOnCollision;
};

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
    virtual const podio::CollectionBase* put(std::unique_ptr<podio::CollectionBase> coll, const std::string& name) = 0;
    virtual bool contains(const std::string& name) const = 0;
  };

  /**
   * The templated frame model that implements the concept and that is
   * constructed in the Frame constructor
   */
  template <typename RawDataT, typename FramePolicies>
  struct FrameModel final : FrameConcept {
    using CollectionMapT = std::unordered_map<std::string, std::unique_ptr<podio::CollectionBase>>;

    /// Constructor for an empty event
    FrameModel() = default;

    /// Constructor from some raw data
    FrameModel(std::unique_ptr<RawDataT> rawData);

    /// Destructor
    ~FrameModel() = default;

    /// Get the corresponding collection
    const podio::CollectionBase* get(const std::string& name) const final;

    /// Put a collection into the Frame (and return a const ref to it for further usage)
    const podio::CollectionBase* put(std::unique_ptr<podio::CollectionBase> coll, const std::string& name) final;

    /// Check whether the Frame contains a collection with this name
    bool contains(const std::string& name) const final;

    // TODO: locking
    mutable CollectionMapT m_collections{};
    std::unique_ptr<RawDataT> m_rawData{nullptr};
  };

  std::unique_ptr<FrameConcept> m_self;

public:
  /// Construct an empty Frame (without any data)
  template <typename FramePolicies = FrameDefaultPolicies,
            typename = typename detail::EnableIfValidFramePolicies<FramePolicies>::type>
  Frame(FramePolicies = FramePolicies{});

  /// Construct a Frame from some raw data
  template <typename RawDataT, typename FramePolicies = FrameDefaultPolicies>
  Frame(std::unique_ptr<RawDataT> rawData, FramePolicies = FramePolicies{});

  /// Get a collection of a given type via its name
  template <typename CollT>
  const CollT& get(const std::string& name) const;

  /// Put a collection into the Frame and get a const reference for further use back
  template <typename CollT, typename = std::enable_if_t<!std::is_lvalue_reference_v<CollT>, bool>>
  const CollT& put(CollT&& coll, const std::string& name);

  /// Check whether the Frame contains a collection with this name
  bool contains(const std::string& name) const;
};

/////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATIONS BELOW (move to a .tpp file or siilar and include here?) //
/////////////////////////////////////////////////////////////////////////////
// =================== Frame implementations ==============================
template <typename FramePolicies, typename>
Frame::Frame(FramePolicies) : m_self(std::make_unique<FrameModel<detail::EmptyRawData, FramePolicies>>()) {
}

template <typename RawDataT, typename FramePolicies>
Frame::Frame(std::unique_ptr<RawDataT> rawData, FramePolicies) :
    m_self(std::make_unique<FrameModel<RawDataT, FramePolicies>>(std::move(rawData))) {
}

template <typename CollT>
const CollT& Frame::get(const std::string& name) const {
  const auto* coll = static_cast<const CollT*>(m_self->get(name));
  if (coll) {
    return *coll;
  }
  // TODO: less happy case via policy?
  static auto defaultColl = CollT();
  return defaultColl;
}

template <typename CollT, typename>
const CollT& Frame::put(CollT&& coll, const std::string& name) {
  auto collPtr = std::make_unique<CollT>(std::move(coll));
  const auto* retColl = static_cast<const CollT*>(m_self->put(std::move(collPtr), name));
  if (retColl) {
    return *retColl;
  }

  // TODO: less happy case via policy?
  static auto defaultColl = CollT();
  return defaultColl;
}

bool Frame::contains(const std::string& name) const {
  return m_self->contains(name);
}

// =================== FrameModel implementations =========================
template <typename RawDataT, typename FramePolicies>
Frame::FrameModel<RawDataT, FramePolicies>::FrameModel(std::unique_ptr<RawDataT> rawData) :
    m_rawData(std::move(rawData)) {
  auto buffers = FramePolicies::UnpackingPolicy::getCollectionBuffers(m_rawData.get());
  for (auto& b : buffers) {
    // TODO: schema evolution, emplacing in collection map
    (void)b; // silence unused warning
  }
}

template <typename RawDataT, typename FramePolicies>
const podio::CollectionBase* Frame::FrameModel<RawDataT, FramePolicies>::get(const std::string& name) const {
  if (const auto it = m_collections.find(name); it != m_collections.end()) {
    return it->second.get();
  }

  if (m_rawData) {
    auto buffers = FramePolicies::UnpackingPolicy::unpack(m_rawData.get(), name);
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

template <typename RawDataT, typename FramePolicies>
const podio::CollectionBase*
Frame::FrameModel<RawDataT, FramePolicies>::put(std::unique_ptr<podio::CollectionBase> coll, const std::string& name) {
  auto [it, success] = m_collections.try_emplace(name, std::move(coll));
  if (!success) {
    // TODO: handle collision
    // Should policy also decide what to return in this case, or do we simply
    // treat collisions as "hard errors" and the policy only sets how "hard"
    // that error is going to be?
    //
    // Major question when doing more than just erroring out: How to handle
    // collection lifetime? All collections managed by the Frame have a clearly
    // defined lifetime, but failed inserts not so much.

    FramePolicies::CollisionPolicy::handleCollision(it->first, it->second, coll);
  }

  return it->second.get();
}

template <typename RawDataT, typename FramePolicies>
bool Frame::FrameModel<RawDataT, FramePolicies>::contains(const std::string& name) const {
  return m_collections.find(name) != m_collections.end();
}

} // namespace podio

#endif // PODIO_FRAME_H
