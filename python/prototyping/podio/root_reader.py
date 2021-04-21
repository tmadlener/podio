#!/usr/bin/env python3

from typing import Callable, List
import logging

from .reader import Reader
from .collection import (
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

  def get_unpacking_function(self) -> Callable[[List[RootRawBuffers], int], CollectionBuffers]:
    self.logger.info('get_unpacking_function')
    return RootReader.unpack

  def open_file(self, fn: str) -> None:
    """Nothing to do here at the moment, since we do not really read data"""
    self.logger.info(f'Opening file "{fn}")')
    pass

  def get_id_table(self):
    self.logger.info('get_id_table')
    return {'ACollection': 0, 'BCollection': 1, 'CCollection': 2}

  @staticmethod
  def unpack(raw_data: List[RootRawBuffers], local_id: int) -> CollectionBuffers:
    """ROOT can in principle do without stateful unpacking because it basically does
    everything for us"""
    RootReader.logger.info(f'RootReader.unpack(local_id={local_id})')
    buffers = CollectionBuffers()

    root_buffer = raw_data[local_id]
    buffers.data = root_buffer.data
    # reference collections and vector members are not necessarily present, we
    # denote invalid CollectionBuffers with None in this case and valid but
    # empty with an empty list
    buffers.ref_colls = root_buffer.ref_colls or []
    buffers.vec_mems = root_buffer.vec_mems or []

    return buffers
