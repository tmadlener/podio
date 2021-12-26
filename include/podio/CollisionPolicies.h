#ifndef PODIO_COLLISIONPOLICIES_H
#define PODIO_COLLISIONPOLICIES_H

#include <iostream>

// Put policies control what should happen in case a user tries to put a
// collection into a Frame that is already present
namespace podio {
struct ThrowOnCollision {};

// template <typename LogStream = std::cout>
// struct LogIgnoreCollision {
//     static
// };

// /**
//  * Policy for overwriting an existing collection
//  */
// struct OverwriteExisting {};

} // namespace podio

#endif // PODIO_COLLISIONPOLICIES_H
