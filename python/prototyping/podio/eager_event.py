#!/usr/bin/env python3

from typing import MutableMapping, Optional, Collection, List
import logging

from .collection import CollectionBase, CollectionBuffers
from .event import EventRawData, Event

class EagerEvent(Event):
  """Event that does all the unpacking up-front and only retrieves the unpacked
  collections from cache when asked to"""
  logger = logging.getLogger(f'{__name__}.EagerEvent')
  def __init__(self, raw_data: EventRawData=None):
    super().__init__()
    self.unpacked: MutableMapping[str, CollectionBase] = {}
    self.raw_data: Optional[EventRawData] = raw_data

  def set_raw_data(self, raw_data: EventRawData) -> None:
    self.logger.info(f'set_raw_data, id: {id(self)}')
    self.raw_data = raw_data

  def get(self, coll_name: str) -> Optional[CollectionBase]:
    # At this point we need the data. We cannot unpack in the constructor nor in
    # the set_raw_data call, since that could potentially block the EventStore
    # and we want the unpacking to happen in the "event thread" (if there is
    # one)
    self.logger.info(f'get({coll_name}), id: {id(self)}')
    if not self.unpacked:
      self.unpacked = self._try_unpack()

    self.logger.debug(f'Collection present: {coll_name in self.unpacked}')
    return self.unpacked.get(coll_name, None)

  def put(self, coll: CollectionBase, name: str):
    # TODO: Check if already present
    self.logger.info(f'put({id(coll)}, "{name}"), id: {id(self)}')
    self.unpacked[name] = coll

  def collections_for_write(self, collections: Collection[str]=[]) -> List[CollectionBuffers]:
    self.logger.info(f'collections_for_write({len(collections)} collections), id: {id(self)}')
    # TODO: Handle case for non-present collections?
    if not collections:
      self.logger.debug('No collections passed, getting all stored collection names')
      collections = self.unpacked.keys()

    buffers = []
    for name in collections:
      coll = self.unpacked.get(name, None)
      self.logger.debug(f'Processing collection "{name}" (id={id(coll)})')
      if coll:
        coll.prepare_for_write()
        buffers.append(coll.buffers)

    return buffers

  def _try_unpack(self) -> MutableMapping[str, CollectionBase]:
    """Try to unpack all collections in the raw data"""
    self.logger.info('_try_unpack')
    if not self.raw_data:
      raise ValueError('No raw data to unpack')

    colls = {}
    for name, buffers in self.raw_data.get_all_buffers():
      self.logger.debug(f'creating collection "{name}"')
      coll = CollectionBase.from_buffers(buffers)
      colls[name] = coll

    # At this point we no longer need the raw data
    del self.raw_data
    self.logger.info(f'unpacked {len(colls)} collections in total')
    return colls

  @staticmethod
  def reuse(evt: 'EagerEvent') -> 'EagerEvent':
    """Re-use an existing event"""
    EagerEvent.logger.info(f'Re-using event: {id(evt)}')
    evt.unpacked = {}
    evt.raw_data = None
    return evt
