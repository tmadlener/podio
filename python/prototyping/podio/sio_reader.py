#!/usr/bin/env python3

from typing import Callable, NewType, Tuple, List
import logging

from .reader import Reader
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


class SioUnpacker:
  """Stateful unpacking is necessary for SIO"""
  logger = logging.getLogger(f'{__name__}.SioReader.SioUnpacker')
  def __init__(self, type_infos: List[str]):
    self.logger.debug('__init__')
    self.uncompressed_data: List[Tuple[int, List[int], List[int]]] = []
    # Sio needs type information to construct the SioBlocks
    self.type_infos: List[str] = type_infos

  def __call__(self, raw_data: SioCompressedEvent, local_id: int) -> CollectionBuffers:
    self.logger.info(f'__call__(local_id={local_id})')
    if not self.uncompressed_data:
      self.uncompressed_data = SioUnpacker._uncompress(raw_data)

    # TODO: Can we free the space that is taken by the compressed data somehow
    # after we have decompressed it in the first call? It is not owned by us!

    buffers = CollectionBuffers()
    data, refs, vecs = self.uncompressed_data[local_id]
    buffers.data = DataBuffer(data)
    buffers.ref_colls = [RefCollBuffer(r) for r in refs]
    buffers.vec_mems = [VecMemBuffer(r) for r in vecs]

    return buffers

  @staticmethod
  def _uncompress(raw_data: SioCompressedEvent) -> List[Tuple[int, List[int], List[int]]]:
    """Just here to explictly mention this step. In reality this would of course do
      more, but it can also return an arbitrary type since this is not visible
      from the outside
    """
    SioUnpacker.logger.debug('_uncompress in SioUnpacker.unpacker')
    # Here we have nothing because of the structure of the compressed data
    return raw_data.compressed_data


class SioReader(Reader):
  """SioReader"""
  logger = logging.getLogger(f'{__name__}.SioReader')
  def __init__(self):
    super().__init__()

  def get_next_event(self) -> SioCompressedEvent:
    self.logger.info('get_next_event')
    return SioCompressedEvent()

  def open_file(self, fn: str) -> None:
    """Nothing to do here at the moment"""
    self.logger.info(f'Opening file "{fn}")')
    pass

  def get_unpacking_function(self) -> Callable[[SioCompressedEvent, int], CollectionBuffers]:
    self.logger.info('get_unpacking_function')
    return SioUnpacker(self.get_id_table())

  def get_id_table(self):
    self.logger.info('get_id_table')
    # really just a dummy return here
    return {'DummyCollection': 0, 'OtherCollection': 1}
