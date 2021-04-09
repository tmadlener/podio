#!/usr/bin/env python3

from typing import Callable, List
import logging

from reader import Reader
from collection import (
   CollectionBuffers, DataBuffer, RefCollBuffers, VecMemBuffers
)

class RootRawBuffers:
  """Root raw data buffers. Incidentally very similar to the CollectionBuffers
  already"""
  def __init__(self):
    """Initialize with some constant data for now"""
    self.data: DataBuffer = 1
    self.ref_colls: RefCollBuffers = [1, 2, 3]
    self.vec_mems: VecMemBuffers = [4, 5, 6]

class RootReader(Reader):
  """RootReader"""
  logger = logging.getLogger(f'{__name__}.RootReader')
  def __init__(self):
    super().__init__()

  def get_next_event(self) -> List[RootRawBuffers]:
    self.logger.info('get_next_event')
    return [RootRawBuffers(), RootRawBuffers(), RootRawBuffers()]

  def get_unpacking_function(self) -> Callable[[RootRawBuffers], CollectionBuffers]:
    self.logger.info('get_unpacking_function')
    return RootReader.unpack

  def open_file(self, fn: str) -> None:
    """Nothing to do here at the moment, since we do not really read data"""
    self.logger.info('Opening file "{fn}")')
    pass

  def get_id_table(self):
    self.logger.info('get_id_table')
    return {'ACollection': 0, 'BCollection': 1, 'CCollection': 2}

  @staticmethod
  def unpack(raw_buffers: RootRawBuffers) -> CollectionBuffers:
    RootReader.logger.info('RootReader.unpack')
    buffers = CollectionBuffers()
    buffers.data = raw_buffers.data
    # reference collections and vector members are not necessarily present, we
    # denote invalid CollectionBuffers with None in this case and valid but
    # empty with an empty list
    buffers.ref_colls = raw_buffers.ref_colls or []
    buffers.vec_mems = raw_buffers.vec_mems or []

    return buffers
