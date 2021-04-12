#!/usr/bin/env python3

from typing import TypeVar, Callable, Generic, Sequence
import logging

from .collection import CollectionBuffers

T = TypeVar('T')

class Reader(Generic[T]):
  """Base class for reading"""
  logger = logging.getLogger(f'{__name__}.Reader')
  def __init__(self):
    self.logger.info(f'({self.__class__.__name__}) __init__')

  def get_next_event(self) -> Sequence[T]:
    """Get the next event (raw data) return a collection of arbitrary type
    containing all the necessary buffers to construct a CollectionBuffers
    """
    raise NotImplementedError

  def get_unpacking_function(self) -> Callable[[T], CollectionBuffers]:
    """Get the thread-safe function that is able to unpack the raw buffers returned
    by read_next_events into a CollectionBuffers"""
    raise NotImplementedError

  def open_file(self, fn: str) -> None:
    """Open a file for reading"""
    raise NotImplementedError

  def get_id_table(self):
    """Get the collection id table from this reade"""
    raise NotImplementedError
