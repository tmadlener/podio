#ifndef PODIO_UTILITIES_INTERFACEWRAPPER_H
#define PODIO_UTILITIES_INTERFACEWRAPPER_H

#include "podio/ObjectID.h"
#include "podio/utilities/MaybeSharedPtr.h"
#include "podio/utilities/TypeHelpers.h"

#include <iostream>
#include <type_traits>
#include <variant>

namespace podio::utils {

namespace detail {
  /**
   * Variable template to determine whether all passed types have an
   * obj_ptr_type member type. We use this to determine whether the
   * GenericWraper is used for wrapping generated EDM types only
   */
  template <typename... Ts>
  constexpr bool allHaveObjPtr = (podio::detail::hasObjPtr<Ts> && ...);

  template <typename... Ts>
  constexpr bool allAreImmutable = (!Ts::is_mutable && ...);
} // namespace detail

/**
 * Generic wrapper class that can wrap any number of data types as long as they
 * are generated via podio. The wrapper internally stores an immutable handle
 * (by value) of the passed value into a std::variant containing all the passed
 * types.
 */
template <typename... WrappedTypes>
class InterfaceWrapper {
  static_assert(detail::allHaveObjPtr<WrappedTypes...>, "All WrappedTypes must be podio generated data types");
  static_assert(detail::allAreImmutable<WrappedTypes...>, "All WrappedTypes must be immutable");

  /// The variant type that is used internally is using the MaybeSharedPtr for
  /// managing the Obj*
  using VariantT = std::variant<WrappedTypes...>;

  /// Helper type to enable some member functions only for the types that are
  /// actually wrapped
  template <typename T>
  using EnableIfValidType = std::enable_if_t<podio::detail::isAnyOf<T, WrappedTypes...>>;

public:
  /// Helper type to make the signature of the main constructor a bit more
  /// readable. Essentially makes the wrapper constructible from any type that
  /// has been used to specify it, including a Mutable value.
  template <typename T>
  using EnableIfConstructibleFrom =
      std::enable_if_t<podio::detail::isAnyOf<T, WrappedTypes..., podio::detail::GetMutableType<WrappedTypes>...>>;

  /// The default interface wrapper will be initialized to an empty handle of
  /// the first type that is in the WrappedTypes type list
  InterfaceWrapper() = default;

  ~InterfaceWrapper() = default;
  InterfaceWrapper(const InterfaceWrapper&) = default;            // ?
  InterfaceWrapper& operator=(const InterfaceWrapper&) = default; // ?

  /// Main constructor from any object that is wrapped by this, including
  /// Mutable objects.
  template <typename T, typename = EnableIfConstructibleFrom<T>>
  InterfaceWrapper(T value) : m_obj(value) {
  }

  /// Get the object id of the contained value
  const podio::ObjectID getObjectID() const {
    return std::visit([](auto&& obj) { return obj.getObjectID(); }, m_obj);
  }
  /// Get the object id of the contained value
  const podio::ObjectID id() const {
    return getObjectID();
  }

  /// Check if the wrapper currently holds the requested type
  template <typename T, typename = EnableIfValidType<T>>
  bool holds() const {
    return std::holds_alternative<T>(m_obj);
  }

  /// Get the contained value as the concrete type it was put in. This will
  /// throw a std::bad_variant_access if T is not the type of the currently held
  /// value. Use holds to check beforehand if necessary
  template <typename T, typename = EnableIfValidType<T>>
  T getValue() const {
    return std::get<T>(m_obj);
  }

  bool operator==(const InterfaceWrapper& other) const {
    return m_obj == other.m_obj;
  }

  bool operator!=(const InterfaceWrapper& other) const {
    return !(*this == other);
  }

  /// Disconnect from the underlying value
  void unlink() {
    std::visit([](auto&& obj) { obj.unlink(); }, m_obj);
  }

  /// Check whether the wrapper currently wraps an actual value
  bool isAvailable() const {
    return std::visit([](auto&& obj) { return obj.isAvailable(); }, m_obj);
  }

  VariantT m_obj{podio::detail::FirstType<WrappedTypes...>::makeEmpty()};
};
} // namespace podio::utils

#endif // PODIO_UTILITIES_INTERFACEWRAPPER_H
