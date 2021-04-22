#!/usr/bin/env python3

from typing import Collection, Mapping, List
import logging

from .writer import Writer
from .event import Event
from .collection import CollectionBuffers, CollectionBase
from .sio_reader import SioCompressedEvent

def sio_write(*args):
  """Simple no-op func for exemplary usage below (and silencing unused
  warnings)"""
  pass


class SioWriter(Writer):
  """SioWriter"""
  logger = logging.getLogger(f'{__name__}.SioWriter')
  def __init__(self, fn: str):
    super().__init__(fn)
    # In reality open a file to write to, but here we don't write anything, so
    # no need to open one
    self.filename = fn
    self.collections: List[str] = []

  def register_for_write(self, coll: CollectionBase, name: str):
    self.logger.info(f'register_for_write(id={id(coll)}, name={name})')
    # TODO: check for duplicates
    self.collections.append(name)
    # NOTE: For sio we don't have to do anything here, since we can always
    # extract the necessary type information when writing the event. However, we
    # can cache it here as well so that we do not have to do the look up for
    # every event

  def write_event(self, event: Event):
    self.logger.info(f'write_event(event={id(event)}) storing {len(self.collections)} collections')
    if not self.collections:
      return

    buffers = event.collections_for_write(self.collections)
    cmp_buffers = SioWriter.compress(buffers)
    sio_write(cmp_buffers)

  @staticmethod
  def compress(buffers: Collection[CollectionBuffers]) -> SioCompressedEvent:
    """Compress the passed list into something that can be written by the sio
    writer

    NOTE: It doesn't really matter whether this is what is actually happening,
    or whether the exact type in c++ is really the same as what is returned by
    the SioReader. Here we just want to briefly 'approximate' what is happening
    in the real world.
    """
    SioWriter.logger.debug(f'Compressing {len(buffers)} buffers')
    cmp_event = SioCompressedEvent()
    for b in buffers:
      cmp_event.compressed_data.append([b.data, b.ref_colls, b.vec_mems])
    return cmp_event

  def write_id_table(self, id_table):
    self.logger.info('Writing collection id table')
    sio_write(id_table)

  def write_collection_metadata(self, metadata: Mapping[str, str]):
    self.logger.info('Writing collection metadata')
    sio_write(metadata)
