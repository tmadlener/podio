#!/usr/bin/env python3

from typing import Callable, NewType, Tuple, List
import logging

from .reader import Reader
from .collection import (
    CollectionBuffers, DataBuffer, RefCollBuffer, VecMemBuffer
)

CompressedBuffer = NewType('CompressedBuffer', list)

class SioRawBuffers:
  """Sio raw data buffer is the compressed complete event block"""
  def __init__(self):
    """Initialize to something that has to be 'uncompressed'"""
    self.data: CompressedBuffer = [1, [1, 2, 3], [4, 5, 6]]

class SioReader(Reader):
  """SioReader"""
  logger = logging.getLogger(f'{__name__}.SioReader')
  def __init__(self):
    super().__init__()

  def get_next_event(self) -> List[SioRawBuffers]:
    self.logger.info('get_next_event')
    return [SioRawBuffers(), SioRawBuffers()]

  def open_file(self, fn: str) -> None:
    """Nothing to do here at the moment"""
    self.logger.info(f'Opening file "{fn}")')
    pass

  def get_unpacking_function(self) -> Callable[[SioRawBuffers], CollectionBuffers]:
    self.logger.info('get_unpacking_function')
    return SioReader.unpack

  def get_id_table(self):
    self.logger.info('get_id_table')
    # really just a dummy return here
    return {'DummyCollection': 0, 'OtherCollection': 1}

  @staticmethod
  def unpack(raw_buffers: SioRawBuffers) -> CollectionBuffers:
    SioReader.logger.info('SioReader.unpack')
    def _uncompress(raw_buffers: SioRawBuffers) -> Tuple[int, List[int], List[int]]:
      """Just here to explictly mention this step. In reality this would of course do
      more, but it can also return an arbitrary type since this is not visible
      from the outside"""
      SioReader.logger.debug('_uncompress in SioReader.unpack')
      return (raw_buffers.data[0], raw_buffers.data[1], raw_buffers.data[2])

    data, refs, vecs = _uncompress(raw_buffers)
    buffers = CollectionBuffers()
    # In this case we do some more "involved unpacking"
    buffers.data = DataBuffer(data)
    buffers.ref_colls = [RefCollBuffer(r) for r in refs]
    buffers.vec_mems = [VecMemBuffer(v) for v in vecs]
    return buffers
