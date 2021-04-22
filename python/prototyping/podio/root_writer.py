#!/usr/bin/env python3

from typing import Mapping, MutableMapping

import logging

from .writer import Writer
from .event import Event
from .collection import CollectionBase

class RootWriter(Writer):
  """RootWriter"""
  logger = logging.getLogger(f'{__name__}.RootWriter')
  def __init__(self, fn: str):
    super().__init__(fn)
    # In reality open a file to write to, but here we don't write anything, so
    # no need to open one
    self.filename = fn
    self.tree: MutableMapping[str, str] = {} # dummy tree here just to do some basic checks

  def register_for_write(self, coll: CollectionBase, name: str):
    self.logger.info(f'register_for_write(id={id(coll)}, name={name})')
    # TODO: Check for duplicatss
    #
    # Just storing some dummy info here, ususally we would set up all the
    # necessary branches for writing this collection
    self.tree[name] = str(id(coll))

  def write_event(self, event: Event):
    self.logger.info(f'write_event(event={id(event)}) storing {len(self.tree)} collections')
    if not self.tree:
      return

    buffers = event.collections_for_write(self.tree.keys())
    for name, buff in zip(self.tree.keys(), buffers):
      self.logger.debug(f'Setting branch addresses for {name}')
      # Set branch addresses should be trivial because the buffers ofer
      # everything that is needed

  def write_id_table(self, id_table):
    self.logger.info('Writing collection id table')

  def write_collection_metadata(self, metadata: Mapping[str, str]):
    self.logger.info('Writing collection metadata')
