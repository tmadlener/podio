#!/usr/bin/env python3

from typing import MutableSequence, Optional, Callable
import logging

from reader import Reader
from event import Event, EventRawData


class EventStore:
  """EventStore class that manages several readers and that hands out 'complete
  events' (plus potential capabilities to unpack the raw buffers)"""
  logger = logging.getLogger(f'{__name__}.EventStore')
  def __init__(self):
    self.logger.info('Creating EventStore')
    self.readers: MutableSequence[Reader] = []
    self.id_table = {}

  def add_reader(self, reader: Reader) -> None:
    """Add a reader. No new readers after first call to get_next_event?"""
    self.logger.info(f'Adding {reader.__class__.__name__} to readers')
    self.readers.append(reader)
    self.logger.debug(f'Stored readers now: {[r.__class__.__name__ for r in self.readers]}')
    self._update_id_table(len(self.readers) - 1, reader.get_id_table())

  def get_next_event(self,
                     create_evt: Callable[[Optional[Event]], Event],
                     evt: Optional[Event]=None) -> Event:
    """Get all the raw buffers from all readers and return an Event for further
    usage. Allow for customization of which concrete event type is actually
    returned and also for how to 'create' such an event. This would allow for
    re-using existing event slots if an existing event instance is passed
    here.
    """
    self.logger.info('Getting next event')
    reader_buffers = []
    unpacker_funcs = []
    for reader in self.readers:
      reader_buffers.append(reader.get_next_event())
      unpacker_funcs.append(reader.get_unpacking_function())

    evt = create_evt(evt) # create a new Event or re-use the existing one

    raw_evt = EventRawData(reader_buffers, unpacker_funcs, self.id_table)
    evt.set_raw_data(raw_evt)

    return evt

  def _update_id_table(self, reader_id: int, entries) -> None:
    self.logger.debug(f'Updating id_table. Adding reader_id {reader_id} with {len(entries)} entries')
    self.id_table[reader_id] = entries
