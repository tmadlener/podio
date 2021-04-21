#!/usr/bin/env python3

from typing import MutableMapping, Optional, Collection, List
import logging

from .collection import CollectionBase, CollectionBuffers
from .event import EventRawData, Event

class LazyEvent(Event):
  """Event storing raw data, unpacking it on demand and giving access to it"""
  logger = logging.getLogger(f'{__name__}.LazyEvent')
  def __init__(self, raw_data: EventRawData=None):
    """Construct need at least a table on how to identify collections via name"""
    super().__init__()
    self.unpacked: MutableMapping[str, CollectionBase] = {}
    self.raw_data: Optional[EventRawData] = raw_data

  def set_raw_data(self, raw_data: EventRawData) -> None:
    self.logger.info(f'set_raw_data, id: {id(self)}')
    self.raw_data = raw_data

  def get(self, coll_name: str) -> Optional[CollectionBase]:
    """Get a collection by name."""
    # First check if we have already unpacked it
    self.logger.info(f'get({coll_name}), id: {id(self)}')
    coll = self.unpacked.get(coll_name, None)
    if coll:
      self.logger.debug(f'Collection {coll_name} already unpacked previously')
      return coll

    # By this point we need to have raw_data otherwise we fail
    if self.raw_data:
      buffers = self.raw_data.get_buffers(coll_name)
      if buffers:
        # Construct and fully unpack the collection
        coll = CollectionBase.from_buffers(buffers)

        # cache it now that we have everything
        self.unpacked[coll_name] = coll
        return coll

    return None

  def put(self, coll: CollectionBase, name: str):
    # TODO: Check if already present
    self.logger.info(f'put({id(coll)}, "{name}"), id: {id(self)}')
    self.unpacked[name] = coll

  def collections_for_write(self, collections: Collection[str]=[]) -> List[CollectionBuffers]:
    self.logger.info(f'collections_for_write({len(collections)} collections), id: {id(self)}')
    # TODO: Handle case for non-present collections?
    if not collections:
      # We want to write all collections including the ones that are potentially
      # still unpacked.
      # For this to work we need raw data (which should be here by now).
      collections = set() # Set for easy deduplication in this toy
      if self.raw_data:
        self.logger.debug('Getting all collection names from raw_data')
        collections.update(self.raw_data.get_all_collection_names())

      # In any case we want the ones that are unpacked (i.e. also the ones that
      # have been put into the event after it has been read)
      self.logger.debug('Using all unpacked collections')
      collections.update(self.unpacked.keys())

    buffers = []
    for name in collections:
      coll = self.unpacked.get(name, None)
      self.logger.debug(f'Processing collection "{name}" (id={id(coll)})')
      if coll:
        coll.prepare_for_write()
        buffers.append(coll.buffers)
      else:
        # Check is here to silence the mypy warning
        if self.raw_data:
          buff = self.raw_data.get_buffers(name)
          if buff:
            buffers.append(buff)

    return buffers

  @staticmethod
  def reuse(evt: 'LazyEvent') -> 'LazyEvent':
    """Re-use an existing event"""
    LazyEvent.logger.info(f'Re-using event: {id(evt)}')
    evt.unpacked = {}
    evt.raw_data = None
    return evt
