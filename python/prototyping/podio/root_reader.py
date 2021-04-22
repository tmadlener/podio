#!/usr/bin/env python3

from typing import List, Tuple
import logging

from .reader import Reader, ReaderRawData
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


class RootRawData(ReaderRawData):
  """ROOT would not necessarily need a stateful unpacking mechanism, but we need
  a common interface"""
  logger = logging.getLogger(f'{__name__}.RootRawData')
  def __init__(self, raw_data: List[RootRawBuffers],
               type_info: List[str],
               schema_version: int):
    self.logger.debug('__init__')
    self.raw_data = raw_data
    self.schema_version = schema_version
    # Need type information also for root for succesfull schema evolution and
    # also for easier construction of collections ouside
    self.type_info: List[str] = type_info

  def get_buffer(self, local_id: int) -> Tuple[str, CollectionBuffers]:
    root_buffer = self.raw_data[local_id]

    buffers = CollectionBuffers()
    buffers.data = root_buffer.data
    # reference collections and vector members are not necessarily present, we
    # denote invalid CollectionBuffers with None in this case and valid but
    # empty with an empty list
    buffers.ref_colls = root_buffer.ref_colls or []
    buffers.vec_mems = root_buffer.vec_mems or []
    buffers.schema_version = self.schema_version

    return self.type_info[local_id], buffers

class RootReader(Reader):
  """RootReader"""
  logger = logging.getLogger(f'{__name__}.RootReader')
  def __init__(self):
    super().__init__()
    # Again the cached type info is of rather arbitrary type here (in comparison
    # to reality) because we just need it for the interfaces in this toy. In
    # reality root is very flexible here and we probably do not have too many
    # restrictions. These would probably also be (re-)populated when a file is
    # opened
    self.type_info = ['X', 'X', 'Z']

  def get_next_event(self) -> RootRawData:
    self.logger.info('get_next_event')
    return RootRawData([RootRawBuffers(), RootRawBuffers(), RootRawBuffers()],
                       self.type_info,
                       schema_version=1)

  def open_file(self, fn: str) -> None:
    """Nothing to do here at the moment, since we do not really read data"""
    self.logger.info(f'Opening file "{fn}")')

  def get_id_table(self):
    self.logger.info('get_id_table')
    return {'ACollection': 0, 'BCollection': 1, 'CCollection': 2}
