#ifndef PODIO_DETAIL_ASSOCIATIONCOLLECTIONITERATOR_H
#define PODIO_DETAIL_ASSOCIATIONCOLLECTIONITERATOR_H

#include "podio/detail/AssociationFwd.h"
#include "podio/utilities/MaybeSharedPtr.h"

namespace podio {
template <typename FromT, typename ToT, bool Mutable>
class AssociationCollectionIteratorT {
  using AssocT = AssociationT<FromT, ToT, Mutable>;
  using AssociationObjT = AssociationObj<FromT, ToT>;

public:
  AssociationCollectionIteratorT(size_t index, const AssociationObjPointerContainer<FromT, ToT>* coll) :
      m_index(index), m_object(nullptr), m_collection(coll) {
  }

  AssociationCollectionIteratorT(const AssociationCollectionIteratorT&) = delete;
  AssociationCollectionIteratorT& operator=(const AssociationCollectionIteratorT&) = delete;

  bool operator!=(const AssociationCollectionIteratorT& other) const {
    return m_index != other.m_index; // TODO: may not be complete
  }

  AssocT operator*() {
    m_object.m_obj = podio::utils::MaybeSharedPtr<AssociationObjT>((*m_collection)[m_index]);
    return m_object;
  }

  AssocT* operator->() {
    m_object.m_obj = podio::utils::MaybeSharedPtr<AssociationObjT>((*m_collection)[m_index]);
    return &m_object;
  }

  AssociationCollectionIteratorT& operator++() {
    ++m_index;
    return *this;
  }

private:
  size_t m_index;
  AssocT m_object;
  const AssociationObjPointerContainer<FromT, ToT>* m_collection;
};
} // namespace podio

#endif // PODIO_DETAIL_ASSOCIATIONCOLLECTIONITERATOR_H
