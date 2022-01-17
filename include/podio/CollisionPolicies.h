#ifndef PODIO_COLLISIONPOLICIES_H
#define PODIO_COLLISIONPOLICIES_H

#include "podio/CollectionBase.h"

#include <exception>
#include <memory>
#include <stdexcept>
#include <string>

// Collision policies define what should happen in case a collection is put into
// a frame under an already existing key. The expected interface looks like this
//
//     struct CollisionPolicy {
//       static void handleCollision(const std::string& name,
//                                   std::unique_ptr<podio::CollectionBase>& existingColl,
//                                   std::unique_ptr<podio::CollectionBase>& newColl);
//     };
//
// This should allow enough flexibility to even replace the existing collection
// if necessary (even though that might create dangling references if someone
// already holds a reference to that collection somewhere else). Note that the
// two unique_ptr are passed by reference!
//
// This header defines three policies: One that simply throws a
// std::runtime_error when a collision occurs, a second one that simply
// (silently) ignores the collision and a third one that replaces the original
// collection with the new collection.

namespace podio {

/**
 * Struct describing the "throw on collision" policy
 */
struct ThrowOnCollision {
  static void handleCollision(std::string const& name, std::unique_ptr<podio::CollectionBase>&,
                              std::unique_ptr<podio::CollectionBase>&) {
    throw std::runtime_error("Collection with name '" + name + "' already present in Frame");
  }
};

/**
 * Struct describing a collision policy where the Frame contents remain
 * unchanged and collisions are essentially silently ignored. Also resulting in
 * the user getting the existing collection back while discarding the collection
 * the user originally passed into Frame::put
 */
struct KeepOriginal {
  static void handleCollision(std::string const&, std::unique_ptr<podio::CollectionBase>&,
                              std::unique_ptr<podio::CollectionBase>&) {
    // Nothing to do in this case
    // TODO log this?
  }
};

/**
 * Struct describing a collision policy where the existing collection is
 * replaced by the new collection. The collectiont hat originally existed in the
 * Frame is discarded.
 */
struct ReplaceCollection {
  static void handleCollision(std::string const&, std::unique_ptr<podio::CollectionBase>& existingColl,
                              std::unique_ptr<podio::CollectionBase>& newColl) {
    existingColl = std::move(newColl);
  }
};

} // namespace podio

#endif // PODIO_COLLISIONPOLICIES_H
