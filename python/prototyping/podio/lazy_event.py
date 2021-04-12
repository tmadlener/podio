#!/usr/bin/env python3

from typing import MutableMapping, Optional
import logging

from .collection import CollectionBase
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
    self.logger.info('set_raw_data')
    self.raw_data = raw_data

  def get(self, coll_name: str) -> Optional[CollectionBase]:
    """Get a collection by name."""
    # First check if we have already unpacked it
    self.logger.info(f'get({coll_name})')
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

  @staticmethod
  def reuse(evt: 'LazyEvent') -> 'LazyEvent':
    """Re-use an existing event"""
    LazyEvent.logger.info(f'Re-using event: {id(evt)}')
    evt.unpacked = {}
    evt.raw_data = None
    return evt
