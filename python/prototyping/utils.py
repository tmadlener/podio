#!/usr/bin/env python3

from typing import List

import logging
logger = logging.getLogger('root')

import sys

from podio.event_store import EventStore
from podio.sio_reader import SioReader
from podio.root_reader import RootReader
from podio.lazy_event import LazyEvent
from podio.eager_event import EagerEvent
from podio.collection import CollectionBase
from podio.writer import Writer
from podio.sio_writer import SioWriter
from podio.root_writer import RootWriter

def setup_store(use_sio: bool, use_root: bool) -> EventStore:
  """Setup up an event store"""
  if not (use_sio or use_root):
    print('Need to specify either a ROOT or SIO reader')
    sys.exit(1)

  logger.debug(f'Setting up reader: use_sio={use_sio}, use_root={use_root}')
  store = EventStore()

  if use_sio:
    # Usually we would first set the reader up and then add it, but here we
    # really do not have to do any setup
    reader = SioReader()
    reader.open_file('dummy_input_file.sio')
    store.add_reader(reader)

  if use_root:
    reader = RootReader()
    reader.open_file('dummy_input_file.root')
    store.add_reader(reader)

  return store


def setup_writers(use_sio: bool, use_root: bool) -> List[Writer]:
  """Setup writers depending on the arguments"""
  writers = []
  if use_sio:
    writers.append(SioWriter('dummy_output_file.sio'))
  if use_root:
    writers.append(RootWriter('dummy_output_file.root'))

  return writers


def _register_collections(writer: Writer, collections: List[str]):
  """Register the collections to write for the given writer"""
  for name in collections:
    # NOTE: In this toy we can will arbitrary collections into existence, since
    # they are not used for anything behind the scenes. In reality we need the
    # following things from the framework:
    # - An instance of the correct collection at initialization that we can pass
    #   here. IMPORTANT: We only rely on type information that we can obtain
    #   from the instance, we do not require that the exact same instance is
    #   also used for event processing!
    # - A layer of abstraction that we can hide behind, to first collect all the
    #   possible collections that can be written and a possibility to filter
    #   them if we do not want to write all of them
    # - Additionally, we need to update the collection id table, currently
    #   stored in the EventStore BEFORE we write it IF the collection we intend
    #   to write is not already present
    #
    # We partially have all this below in initialize, but a full implementation
    # of all this goes beyond the scope of this toy
    writer.register_for_write(CollectionBase(), name)

# The collections that are newly created in process
NEW_COLLECTIONS = [
  'NewCollection', 'AnotherNewCollection'
]

# The collections that will be written. NOTE: contains collections that might
# not be present (i.e. not read)
WRITE_COLLECTIONS = [
  'AnotherNewCollection', 'DummyCollection', 'BCollection'
]

def initialize(store: EventStore, writers: List[Writer]):
  """Initialize: As in Gaudi. In this toy mainly registering the collections that
  should be written"""
  logger.info('---------------------- INITIALIZE ----------------------------------')
  # First we register the new collections. We use reader_id == -1 here
  id_table = {n: i for i, n in enumerate(NEW_COLLECTIONS)}
  store._update_id_table(-1, id_table)

  all_collections = store.collection_names()
  collections_to_write = [c for c in WRITE_COLLECTIONS if c in all_collections]
  if len(collections_to_write) != WRITE_COLLECTIONS:
    miss_colls = [c for c in WRITE_COLLECTIONS if c not in all_collections]
    logger.debug(f'Not writing collections: {miss_colls} because they are not available in the EventStore')

  for writer in writers:
    _register_collections(writer, collections_to_write)


def process(event, ievent):
  """Do some things with the event this is basically in 'user-land'"""
  logger.info(f'-------------------- PROCESSING EVENT {ievent} -------------------------')
  # In reality this list might be filtered much earlier, because we have the
  # necessary tools to declare the inputs and outputs for all algorithms, and
  # hence we know which collections will be present. Here we just check all
  # possibilities to highlight, that in principle the Event class design can
  # handle such non-present collections as well
  colls_to_get = [
      'DummyCollection', 'OtherCollection', # present in sio reader
      'ACollection', 'BCollection', 'CCollection', # present in root reader
      'NotPresent' # To check that this is also handled gracefully
  ]

  for name in colls_to_get:
    coll = event.get(name)
    logger.debug(f'Collection "{name}" is valid and has set refs: {coll and coll.valid and coll.resolved}')

  for name in NEW_COLLECTIONS:
    coll = CollectionBase()
    event.put(coll, name)


def get_create_evt(evt_type: str, re_use: bool):
  """Get the prototype event and the function that creates events. If we want to
  re-use events we need an empty event, which we construct here. If we want to
  create new events everytime we read, we simply do not create one here. (Using
  None as return-type mainly, but it doesn't really matter, just need one that
  can be unpacked ath the calling site)

  In this case this function is mainly here to handle the command line
  arguments. In a real world use case, we would need to default to the
  re-creation probably and optionally make the FW handle all the setup steps in
  case of an arena based approach"""
  logger.debug(f'Creating event creation func for evt_type={evt_type}')
  if not re_use:
    if evt_type == 'lazy':
      return None, lambda _: LazyEvent() # ignore the optional event that is passed
    if evt_type == 'eager':
      return None, lambda _: EagerEvent() # ignore the optional event that is passed
  else:
    if evt_type == 'lazy':
      return LazyEvent(), LazyEvent.reuse
    if evt_type == 'eager':
      return EagerEvent(), EagerEvent.reuse
