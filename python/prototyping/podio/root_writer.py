#!/usr/bin/env python3

from typing import Collection, Mapping, MutableMapping

import logging

from .writer import Writer
from .event import Event

class RootWriter(Writer):
  """RootWriter"""
  logger = logging.getLogger(f'{__name__}.RootWriter')
  def __init__(self, fn: str):
    super().__init__(fn)
    # In reality open a file to write to, but here we don't write anything, so
    # no need to open one
    self.filename = fn
    self.tree: MutableMapping[str, str] = {} # dummy tree here just to do some basic checks

  def write_event(self, event: Event, collections: Collection[str]=[]):
    self.logger.info(f'write_event(event={id(event)}) storing {len(collections)} collections (0==all)')
    # The ROOT writer can keep track of the collections it can write so here we
    # do a "pre-filtering" of the collections to make sure that all the passed
    # collection names can be stored
    if not collections:
      self.logger.debug('Collecting collection names from tree')
      collections = self.tree.keys()
    else:
      self.logger.debug('Verifying collection names against internal tree')
      collections = [c for c in collections if c in self.tree]

    buffers = event.collections_for_write(collections)
    for name, buff in zip(collections, buffers):
      self.logger.debug(f'Setting branch addresses for {name}')
      # Set branch addresses should be trivial because the buffers ofer
      # everything that is needed

  def write_id_table(self, id_table):
    self.logger.info('Writing collection id table')
    for ireader, reader_ids in id_table.items():
      for name in reader_ids.keys():
        self.tree[name] = 'some root tree'

  def write_collection_metadata(self, metadata: Mapping[str, str]):
    self.logger.info('Writing collection metadata')
