#!/usr/bin/env python3

from typing import Tuple
import logging

from .collection import CollectionBuffers

class ReaderRawData:
  """Abstract type that holds raw data and also knows how to unpack it on
  demand"""
  def get_buffer(self, coll_id: int) -> Tuple[str, CollectionBuffers]:
    """Get the buffer with the internal coll_id (might be different from the
    external/global id)"""
    raise NotImplementedError


class Reader:
  """Base class for reading"""
  logger = logging.getLogger(f'{__name__}.Reader')
  def __init__(self):
    self.logger.info(f'({self.__class__.__name__}) __init__')

  def get_next_event(self) -> ReaderRawData:
    """Get the next event (raw data) return a collection of arbitrary type
    containing all the necessary buffers to construct a CollectionBuffers
    """
    raise NotImplementedError

  def open_file(self, fn: str) -> None:
    """Open a file for reading"""
    raise NotImplementedError

  def get_id_table(self):
    """Get the collection id table from this reader"""
    raise NotImplementedError
