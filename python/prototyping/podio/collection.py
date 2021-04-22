#!/usr/bin/env python3

from typing import NewType, Collection
import logging

# Define a few helper types to string things together
# DataBuffers, i.e. vector<Data>, but for type reasons maybe void*?
DataBuffer = NewType('DataBuffer', int)
# RefCollBuffers, i.e vector<ObjectID>, resp. vectors of those
RefCollBuffer = NewType('RefCollBuffer', int)
RefCollBuffers = Collection[RefCollBuffer]
# VecMemBuffers, i.e. pair<string, void*>, resp. vectors of those
VecMemBuffer = NewType('VecMemBuffer', int)
VecMemBuffers = Collection[VecMemBuffer]

class CollectionBuffers:
  """Class holding all (unpacked) buffers from which collections can be
  constructed"""
  def __init__(self):
    """Initialize everything to none to indicate the 'invalid' state"""
    self.data: DataBuffer = None
    self.ref_colls: RefCollBuffers = None
    self.vec_mems: VecMemBuffers = None
    self.schema_version: int = -1

  def valid_buffers(self) -> bool:
    """Are all buffers valid, i.e. read correctly"""
    return (
      self.data is not None and
      self.ref_colls is not None and
      self.vec_mems is not None and
      self.schema_version >= 0
      )

class CollectionBase:
  """Simple class that resembles the c++ collection"""
  logger = logging.getLogger(f'{__name__}.CollectionBase')
  def __init__(self) -> None:
    """By default all collections are 'invalid'"""
    self.logger.info(f'__init__, id: {id(self)}')
    self.buffers: CollectionBuffers = CollectionBuffers()
    self.valid = False
    self.resolved = False
    self.prepared = False

  def set_buffer(self, data: DataBuffer) -> None:
    """Set the data buffer"""
    self.logger.info(f'set_buffer, id: {id(self)}')
    self.buffers.data = data

  def set_ref_colls(self, ref_colls: RefCollBuffers=[]) -> None:
    """Set the ref coll buffer"""
    self.logger.info(f'set_ref_colls, id: {id(self)}')
    self.buffers.ref_colls = ref_colls

  def set_vec_mems(self, vec_mems: VecMemBuffers=[]) -> None:
    """Set the vector members"""
    self.logger.info(f'set_vec_mems, id: {id(self)}')
    self.buffers.vec_mems = vec_mems

  def prepare_after_read(self) -> None:
    """Prepare after read"""
    self.valid = self.buffers.valid_buffers()
    self.logger.info(f'prepare_after_read (valid={self.valid}), id: {id(self)}')

  def prepare_for_write(self) -> None:
    """Prepare for write"""
    self.logger.info(f'prepare_for_write, id: {id(self)}')
    if self.prepared:
      self.logger.debug('Already prepared')
      return

    self.prepared = True
    # To differentiate them from the unset default init simply fill them with
    # some values or set to the empty list at least
    self.buffers.data = DataBuffer(42)
    self.buffers.ref_colls = [RefCollBuffer(3), RefCollBuffer(14), RefCollBuffer(15)]
    self.buffers.vec_mems = []

  def set_references(self) -> None:
    """Set the references"""
    if self.valid:
      self.resolved = True
    self.logger.info(f'set_references (resolved={self.resolved})')

  @staticmethod
  def from_buffers(buffers: CollectionBuffers) -> 'CollectionBase':
    """Do the full unpacking from collection buffers"""
    coll = CollectionBase()
    coll.set_buffer(buffers.data)
    coll.set_ref_colls(buffers.ref_colls)
    coll.set_vec_mems(buffers.vec_mems)
    coll.buffers.schema_version = buffers.schema_version
    coll.prepare_after_read()
    coll.set_references()

    return coll
