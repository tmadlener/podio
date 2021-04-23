#!/usr/bin/env python3

from typing import Mapping
import logging

from .event import Event
from .collection import CollectionBase

class Writer:
  """Base class for writers"""
  logger = logging.getLogger(f'{__name__}.Writer')
  def __init__(self, fn: str):
    self.logger.info(f'({self.__class__.__name__}) __init__(file={fn})')

  def register_for_write(self, coll: CollectionBase, name: str):
    """Register this collection for writing"""
    raise NotImplementedError

  def write_event(self, event: Event):
    """Write the registered collections to file"""
    raise NotImplementedError

  def write_id_table(self, id_table):
    """Write the collection id table"""
    raise NotImplementedError

  def write_collection_metadata(self, metadata: Mapping[str, str]):
    """Write the collection meta data information, currently assumed to consist only
    of strings for easier python type annotations, but not a real limitation
    """
    raise NotImplementedError

  # TODO: Run metadata, should work similarly to collection metadata
  # TODO: Event metadata, but I think that is a different kind of "metadata" and
  # should be treated more like "generic" event data without a pre-defined data
  # type. Hence, it is considered to be handled by write_event here (although it
  # is of course not currently)
