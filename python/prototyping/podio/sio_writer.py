#!/usr/bin/env python3

from typing import Collection, Mapping
import logging

from .writer import Writer
from .event import Event
from .collection import CollectionBuffers
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

  def write_event(self, event: Event, collections: Collection[str]=[]):
    self.logger.info(f'write_event(event={id(event)}) storing {len(collections)} collections (0==all)')
    buffers = event.collections_for_write(collections)
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
