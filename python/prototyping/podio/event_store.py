#!/usr/bin/env python3

from typing import MutableSequence, Optional, Callable, Collection
import logging

from .reader import Reader
from .event import Event, EventRawData


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
    for reader in self.readers:
      reader_buffers.append(reader.get_next_event())

    evt = create_evt(evt) # create a new Event or re-use the existing one

    raw_evt = EventRawData(reader_buffers, self.id_table)
    evt.set_raw_data(raw_evt)

    return evt

  def _update_id_table(self, reader_id: int, entries) -> None:
    self.logger.debug(f'Updating id_table. Adding reader_id {reader_id} with {len(entries)} entries')
    # TODO: Do an overall dedupliation here and also check and (resolve?)
    # collissions, so that we can assume afterwards that the id -> name relation
    # is unique accross the whole event?
    self.id_table[reader_id] = entries

  def collection_names(self) -> Collection[str]:
    """Get the names of all the collections known to the EventStore (i.e. the ones
    who have been read and the ones who have been newly registered)"""
    # NOTE: Assuming here that the proper deduplication from reading and
    # registering new collection has already happened!
    names = []
    for reader_id_tables in self.id_table.values():
      for name in reader_id_tables.keys():
        names.append(name)

    return names