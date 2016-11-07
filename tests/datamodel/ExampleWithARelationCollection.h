//AUTOMATICALLY GENERATED - DO NOT EDIT

#ifndef ExampleWithARelationCollection_H
#define  ExampleWithARelationCollection_H

#include <string>
#include <vector>
#include <deque>
#include <array>
#include <algorithm>

// podio specific includes
#include "podio/ICollectionProvider.h"
#include "podio/CollectionBase.h"
#include "podio/CollectionIDTable.h"

// datamodel specific includes
#include "ExampleWithARelationData.h"
#include "ExampleWithARelation.h"
#include "ExampleWithARelationObj.h"

namespace ex {
typedef std::vector<ExampleWithARelationData> ExampleWithARelationDataContainer;
typedef std::deque<ExampleWithARelationObj*> ExampleWithARelationObjPointerContainer;

class ExampleWithARelationCollectionIterator {

  public:
    ExampleWithARelationCollectionIterator(int index, const ExampleWithARelationObjPointerContainer* collection) : m_index(index), m_object(nullptr), m_collection(collection) {}

    bool operator!=(const ExampleWithARelationCollectionIterator& x) const {
      return m_index != x.m_index; //TODO: may not be complete
    }

    const ExampleWithARelation operator*() const;
    const ExampleWithARelation* operator->() const;
    const ExampleWithARelationCollectionIterator& operator++() const;

  private:
    mutable int m_index;
    mutable ExampleWithARelation m_object;
    const ExampleWithARelationObjPointerContainer* m_collection;
};

/**
A Collection is identified by an ID.
*/

class ExampleWithARelationCollection : public podio::CollectionBase {

public:
  typedef const ExampleWithARelationCollectionIterator const_iterator;

  ExampleWithARelationCollection();
//  ExampleWithARelationCollection(const ExampleWithARelationCollection& ) = delete; // deletion doesn't work w/ ROOT IO ! :-(
//  ExampleWithARelationCollection(ExampleWithARelationVector* data, int collectionID);
  ~ExampleWithARelationCollection();

  void clear();
  /// Append a new object to the collection, and return this object.
  ExampleWithARelation create();

  /// Append a new object to the collection, and return this object.
  /// Initialized with the parameters given
  template<typename... Args>
  ExampleWithARelation create(Args&&... args);
  int size() const;

  /// Returns the object of given index
  const ExampleWithARelation operator[](unsigned int index) const;
  /// Returns the object of given index
  const ExampleWithARelation at(unsigned int index) const;


  /// Append object to the collection
  void push_back(ExampleWithARelation object);

  void prepareForWrite();
  void prepareAfterRead();
  void setBuffer(void* address);
  bool setReferences(const podio::ICollectionProvider* collectionProvider);

  podio::CollRefCollection* referenceCollections() { return &m_refCollections;};

  void setID(unsigned ID){
    m_collectionID = ID;
    std::for_each(m_entries.begin(),m_entries.end(),
                 [ID](ExampleWithARelationObj* obj){obj->id = {obj->id.index,static_cast<int>(ID)}; }
    );
  };

  void setReadOnly();

  bool isValid() const {
    return m_isValid;
  }

  // support for the iterator protocol
  const const_iterator begin() const {
    return const_iterator(0, &m_entries);
  }
  const const_iterator end() const {
    return const_iterator(m_entries.size(), &m_entries);
  }

  /// returns the address of the pointer to the data buffer
  void* getBufferAddress() { return (void*)&m_data;};

  /// returns the pointer to the data buffer
  std::vector<ExampleWithARelationData>* _getBuffer() { return m_data;};

    template<size_t arraysize>
  const std::array<float,arraysize> number() const;


private:
  bool m_isValid;
  bool m_isReadOnly{false};
  int m_collectionID;
  ExampleWithARelationObjPointerContainer m_entries;
  // members to handle 1-to-N-relations
  std::vector<ex::ExampleWithNamespace>* m_rel_refs; ///< Relation buffer for read / write
  std::vector<std::vector<ex::ExampleWithNamespace>*> m_rel_refs_tmp; ///< Relation buffer for internal book-keeping
  std::vector<ex::ExampleWithNamespace>* m_rel_ref; ///< Relation buffer for read / write

  // members to handle streaming
  podio::CollRefCollection m_refCollections;
  ExampleWithARelationDataContainer* m_data;
};

template<typename... Args>
ExampleWithARelation  ExampleWithARelationCollection::create(Args&&... args){
  if(m_isReadOnly) throw std::runtime_error("Collection has been set to read-only");
  int size = m_entries.size();
  auto obj = new ExampleWithARelationObj({size,m_collectionID},{args...});
  m_entries.push_back(obj);
  return ExampleWithARelation(obj);
}

template<size_t arraysize>
const std::array<float,arraysize> ExampleWithARelationCollection::number() const {
  std::array<float,arraysize> tmp;
  auto valid_size = std::min(arraysize,m_entries.size());
  for (unsigned i = 0; i<valid_size; ++i){
    tmp[i] = m_entries[i]->data.number;
 }
 return tmp;
}

} // namespace ex
#endif
