#!/usr/bin/env python3

from typing import Optional, Collection, Tuple, TypeVar, List, Callable, Sequence
import logging

from .collection import CollectionBase, CollectionBuffers

T = TypeVar('T')

class EventRawData:
  """Event raw data potentially comprised of inputs from multiple distinct
  readers"""
  logger = logging.getLogger(f'{__name__}.EventRawData')
  def __init__(self,
               reader_buffers: List[Sequence[T]],
               unpacking_funcs: List[Callable[[T], CollectionBuffers]],
               id_table):
    """Take ownership of the reader_buffers. Take a (const) reference to the
    id_table from the EventStore. Maybe a shared_ptr, to make it possible for
    changing id_table in the EventStore without immediately invalidating all
    references that events in flight might still have?"""
    self.logger.info(f'__init__(buffers from {len(reader_buffers)} readers)')
    self.id_table = id_table
    self.raw_buffers: Sequence[Sequence[T]] = reader_buffers
    self.unpack_funcs: Sequence[Callable[[T], CollectionBuffers]] = unpacking_funcs
    self.logger.debug(f'{sum(len(b) for b in reader_buffers)} raw buffers total')

  def get_buffers(self, name: str) -> Optional[CollectionBuffers]:
    """Check and see if the raw data for the collection of this name is present"""
    self.logger.info(f'get_buffer({name})')
    for index, table in self.id_table.items():
      coll_idx = table.get(name, -1)
      if coll_idx >= 0:
        return self.unpack_funcs[index](self.raw_buffers[index][coll_idx])

    self.logger.debug(f'No collection with name "{name}" found')
    return None

  def get_all_buffers(self) -> Collection[Tuple[str, CollectionBuffers]]:
    """Get all the available buffers"""
    self.logger.info('get_all_buffers')
    colls: List[Tuple[str, CollectionBuffers]] = []
    for ireader, (reader_buffers, unpack_func) in enumerate(zip(self.raw_buffers, self.unpack_funcs)):
      ids = self.id_table[ireader]
      for name, index in ids.items():
        colls.append(
            (name, unpack_func(reader_buffers[index]))
        )

    self.logger.debug(f'Unpacked {len(colls)} collections in total')
    return colls

  def get_all_collection_names(self) -> Collection[str]:
    """Get all the names of the collections in this raw data"""
    names = []
    for _, id_table in self.id_table.items():
      for name in id_table.keys():
        names.append(name)

    return names


class Event:
  """Abstract event class (type-erased?)"""
  logger = logging.getLogger(f'{__name__}.Event')
  def __init__(self, raw_data: EventRawData=None):
    self.logger.info(f'({self.__class__.__name__}) __init__(raw_data={raw_data}), id: {id(self)}')

  def set_raw_data(self, raw_data: EventRawData) -> None:
    """Set the raw data for this event. Necessary for re-using of event slots"""
    raise NotImplementedError

  def get(self, coll_name: str) -> Optional[CollectionBase]:
    """Get a collection stored under a name (if it exists)"""
    raise NotImplementedError

  def collections_for_write(self, collections: Collection[str]=[]) -> Collection[CollectionBuffers]:
    """Get the collection buffers to write for the desired collections (default all)"""
    raise NotImplementedError

  def put(self, coll: CollectionBase, name: str):
    """Put the given collection into the event"""
    raise NotImplementedError
