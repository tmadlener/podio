#!/usr/bin/env python3

import logging

logger = logging.getLogger()
logger.setLevel(logging.DEBUG)

stream_handler = logging.StreamHandler()
formatter = logging.Formatter('[%(levelname)8s | %(threadName)10s | %(name)35s] | %(message)s',
                              datefmt='%d.%m.%Y %H:%M:%S')
stream_handler.setFormatter(formatter)

logger.addHandler(stream_handler)

import time
import itertools
import random

from multiprocessing.dummy import Pool

from utils import setup_store, process, get_create_evt

def process_event(store, ievent, evt_create_f):
  """Create an event and process it. Wrapping this for easier 'threading'"""
  event = store.get_next_event(evt_create_f)
  process(event, ievent)
  time.sleep(random.randint(1, 10) * 0.1) # sleep random amounts of time on threads


def main(args):
  """Main: This corresponds roughly to what the FW would do"""
  store = setup_store(args.sio_reader, args.root_reader)
  # Not allowing for re-using of events here, since that requires a lot of
  # additional setup for synchronizing which events can be re-used
  _, evt_create_f = get_create_evt(args.evt_type, False)

  logger.debug(f'Setting up a thread pool with {args.threads} threads')

  logger.info('-------------------- END OF SETUP -------------------------------')
  with Pool(processes=args.threads) as event_pool:
      event_pool.starmap(process_event,
                         itertools.product([store], range(args.nevents), [evt_create_f]))


if __name__ == '__main__':
  import argparse

  parser = argparse.ArgumentParser(description='Small main program that exhibits the possibilities of the prototype')
  parser.add_argument('--sio-reader', help='Add an SIO example reader', action='store_true',
                      default=False)
  parser.add_argument('--root-reader', help='Add a ROOT example reader', action='store_true',
                      default=False)
  parser.add_argument('-n', '--nevents', default=10, help='How many events to run', type=int)
  parser.add_argument('-t', '--threads', default=2, help='How many threads to use', type=int)

  evt_type = parser.add_mutually_exclusive_group()
  evt_type.add_argument('--lazy-event', help='Choose the lazy event',
                        action='store_const', dest='evt_type', const='lazy')
  evt_type.add_argument('--eager-event', help='Choose the eager event',
                        action='store_const', dest='evt_type', const='eager')
  parser.set_defaults(evt_type='eager')

  clargs = parser.parse_args()
  main(clargs)
