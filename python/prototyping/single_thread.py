#!/usr/bin/env python3

import logging

logger = logging.getLogger()
logger.setLevel(logging.DEBUG)

stream_handler = logging.StreamHandler()
formatter = logging.Formatter('[%(levelname)8s | %(name)35s] | %(message)s',
                              datefmt='%d.%m.%Y %H:%M:%S')
stream_handler.setFormatter(formatter)

logger.addHandler(stream_handler)

from utils import (
  setup_store, process, get_create_evt, setup_writers, initialize
)

def main(args):
  """Main. This corresponds roughly to what the FW would do"""
  store = setup_store(args.sio_reader, args.root_reader)
  event, create_ev_f = get_create_evt(args.evt_type, args.reuse)

  writers = setup_writers(args.sio_writer, args.root_writer)

  initialize(store, writers)

  for w in writers:
    w.write_id_table(store.id_table)

  logger.info('-------------------- END OF SETUP -------------------------------')

  for i in range(args.nevents):
    event = store.get_next_event(create_ev_f, event)
    process(event, i)

    for w in writers:
      w.write_event(event)

  for w in writers:
    w.write_collection_metadata({'someColl': 'someExampleMetaData'})


if __name__ == '__main__':
  import argparse

  parser = argparse.ArgumentParser(description='Small main program that exhibits the possibilities of the prototype')
  parser.add_argument('--sio-reader', help='Add an SIO example reader', action='store_true',
                      default=False)
  parser.add_argument('--root-reader', help='Add a ROOT example reader', action='store_true',
                      default=False)
  parser.add_argument('--sio-writer', help='Add an SIO example writer', action='store_true',
                      default=False)
  parser.add_argument('--root-writer', help='Add an ROOT example writer', action='store_true',
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
