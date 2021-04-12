#!/usr/bin/env python3

import logging

logger = logging.getLogger()
logger.setLevel(logging.DEBUG)

stream_handler = logging.StreamHandler()
formatter = logging.Formatter('[%(levelname)8s | %(name)35s] | %(message)s',
                              datefmt='%d.%m.%Y %H:%M:%S')
stream_handler.setFormatter(formatter)

logger.addHandler(stream_handler)

import sys

from podio.event_store import EventStore
from podio.lazy_event import LazyEvent
from podio.eager_event import EagerEvent
from podio.sio_reader import SioReader
from podio.root_reader import RootReader

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

def process(event):
  """Do some things with the event this is basically in 'user-land'"""
  colls_to_get = [
      'DummyCollection', 'OtherCollection', # present in sio reader
      'ACollection', 'BCollection', 'CCollection', # present in root reader
      'NotPresent' # To check that this is also handled gracefully
  ]

  for name in colls_to_get:
    coll = event.get(name)
    logger.debug(f'Collection "{name}" is valid and has set refs: {coll and coll.valid and coll.resolved}')


def main(args):
  """Main. This corresponds roughly to what the FW would do"""

  store = setup_store(args.sio_reader, args.root_reader)
  event, create_ev_f = get_create_evt(args.evt_type, args.reuse)
  logger.info('-------------------- END OF SETUP -------------------------------')
   

  for i in range(args.nevents):
    logger.info(f'-------------------- PROCESSING EVENT {i} -------------------------')
    event = store.get_next_event(create_ev_f, event)
    process(event)


if __name__ == '__main__':
  import argparse

  parser = argparse.ArgumentParser(description='Small main program that exhibits the possibilities of the prototype')
  parser.add_argument('--sio-reader', help='Add an SIO example reader', action='store_true',
                      default=False)
  parser.add_argument('--root-reader', help='Add a ROOT example reader', action='store_true',
                      default=False)
  parser.add_argument('-n', '--nevents', default=1, help='How many events to run', type=int)

  evt_type = parser.add_mutually_exclusive_group()
  evt_type.add_argument('--lazy-event', help='Choose the lazy event',
                        action='store_const', dest='evt_type', const='lazy')
  evt_type.add_argument('--eager-event', help='Choose the eager event',
                        action='store_const', dest='evt_type', const='eager')
  parser.set_defaults(evt_type='eager')

  parser.add_argument('--reuse', help='Re-use the events instead of creating '
                      'them from scratch for every event', action='store_true',
                      default=False)

  clargs = parser.parse_args()
  main(clargs)
