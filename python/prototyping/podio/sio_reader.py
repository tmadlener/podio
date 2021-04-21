#!/usr/bin/env python3

from typing import NewType, Tuple, List
import logging

from .reader import Reader, ReaderRawData
from .collection import (
    CollectionBuffers, DataBuffer, RefCollBuffer, VecMemBuffer
)

CompressedBuffer = NewType('CompressedBuffer', list)

class SioCompressedEvent:
  """Sio raw data buffer is the compressed complete event block"""
  def __init__(self):
    """Initialize to something that has to be 'uncompressed' and comprises more than
    one collection, because all collection blocks are compressed as one event in
    sio
    """
    self.compressed_data: CompressedBuffer = [
      [1, [1, 2, 3], [4, 5, 6]],
      [2, [3, 4, 5], [7, 8, 9]]
    ]


class SioRawData(ReaderRawData):
  """Stateful unpacking is necessary for SIO"""
  logger = logging.getLogger(f'{__name__}.SioRawData')
  def __init__(self, raw_data: SioCompressedEvent, type_infos: List[str]):
    self.logger.debug('__init__')
    self.raw_data = raw_data
    self.uncompressed_data: List[Tuple[int, List[int], List[int]]] = []
    # Sio needs type information to construct the SioBlocks
    self.type_infos: List[str] = type_infos

  def get_buffer(self, local_id: int) -> CollectionBuffers:
    self.logger.info(f'get_buffer(local_id={local_id})')
    if not self.uncompressed_data:
      self.uncompressed_data = self._uncompress(self.raw_data)
      # We no longer need this, since we now have the uncompressed data
      del self.raw_data

    buffers = CollectionBuffers()
    data, refs, vecs = self.uncompressed_data[local_id]
    buffers.data = DataBuffer(data)
    buffers.ref_colls = [RefCollBuffer(r) for r in refs]
    buffers.vec_mems = [VecMemBuffer(r) for r in vecs]

    return buffers

  def _uncompress(self, raw_data: SioCompressedEvent) -> List[Tuple[int, List[int], List[int]]]:
    """Just here to explictly mention this step. In reality this would of course do
      more, but it can also return an arbitrary type since this is not visible
      from the outside
    """
    self.logger.debug('_uncompress in SioUnpacker.unpacker')
    # Here we have nothing because of the structure of the compressed data In
    # reality we would use the type_info hear to create all the necessary
    # SioBlocks
    return raw_data.compressed_data


class SioReader(Reader):
  """SioReader"""
  logger = logging.getLogger(f'{__name__}.SioReader')
  def __init__(self):
    super().__init__()

  def get_next_event(self) -> SioRawData:
    self.logger.info('get_next_event')
    return SioRawData(SioCompressedEvent(), self.get_id_table())

  def open_file(self, fn: str) -> None:
    """Nothing to do here at the moment"""
    self.logger.info(f'Opening file "{fn}")')
    pass

  def get_id_table(self):
    self.logger.info('get_id_table')
    # really just a dummy return here
    return {'DummyCollection': 0, 'OtherCollection': 1}
