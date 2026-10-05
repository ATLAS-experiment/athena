#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Algorithm + CA config for EventGUIDLookup_Skeleton.py."""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaPython import PyAthena
from AthenaPython.PyAthena import StatusCode
from GaudiKernel.Constants import INFO


class EventGUIDLookupAlg(PyAthena.Alg):

    def __init__(self, name='EventGUIDLookupAlg', **kwargs):
        self.dataType = kwargs.pop('dataType')
        self.inputDataType = kwargs.pop('inputDataType')
        self.requestedEvents = kwargs.pop('requestedEvents', set())
        self.outputFile = kwargs.pop('outputFile', 'eventGUIDLookup.txt')
        super(EventGUIDLookupAlg, self).__init__(name=name, **kwargs)
        self._found = set()
        self._outFH = None

    def initialize(self):
        return StatusCode.Success

    def _matchRun(self, ctx_run, evt):
        """Return the run-like number used for matching and output:
        the MC channel number (DSID) for simulation, the run number for data.
        """
        try:
            ei = self.evtStore['EventInfo']
            if ei.eventType(ei.IS_SIMULATION):              # isMC
                dsid = ei.mcChannelNumber()
                if dsid != 0:
                    return dsid
                # Some old/private productions have no channel number set
                self.msg.debug('run=%d event=%d: MC with mcChannelNumber=0; '
                               'using run number', ctx_run, evt)
        except Exception as e:
            self.msg.warning('run=%d event=%d: cannot read EventInfo (%s); '
                             'using run number', ctx_run, evt, e)
        return ctx_run

    def execute(self):
        from EventGUIDLookup.EventGUIDLookupLib import resolveProvenanceGuid

        # Event number and (data) run number come from the EventContext,
        # which is format-agnostic. For MC the run number is replaced by
        # the MC channel number (DSID) read from EventInfo.

        eid = self.getContext().eventID()
        ctx_run, evt = eid.run_number(), eid.event_number()

        run = self._matchRun(ctx_run, evt)   # DSID for MC, run number for data
        key = (run, evt)

        if key in self.requestedEvents and key not in self._found:
            guid = resolveProvenanceGuid(self.evtStore, self.dataType,
                                         self.inputDataType)
            if guid is not None:
                if self._outFH is None:
                    self._outFH = open(self.outputFile, 'w')
                self._outFH.write('%d %d %s\n' % (run, evt, guid))
                self._outFH.flush()
                self._found.add(key)
            else:
                self.msg.warning('run/dsid=%d event=%d has no %s provenance '
                                 'reference', run, evt, self.dataType)
        return StatusCode.Success
    def finalize(self):
        if self._outFH:
            self._outFH.close()
        missing = self.requestedEvents - self._found
        if missing:
            self.msg.warning('%d of %d requested events had no %s GUID',
                              len(missing), len(self.requestedEvents),
                              self.dataType)
            for run, evt in sorted(missing):
                self.msg.warning('  missing: run=%d event=%d', run, evt)
        else:
            self.msg.info('Resolved GUIDs for all %d requested events',
                           len(self.requestedEvents))
        return StatusCode.Success


def EventGUIDLookupAlgCfg(flags, dataType, inputDataType, requestedEvents,
                           outputFile, name='EventGUIDLookupAlg', **kwargs):
    acc = ComponentAccumulator()
    acc.addEventAlgo(EventGUIDLookupAlg(
        name=name,
        dataType=dataType,
        inputDataType=inputDataType,
        requestedEvents=requestedEvents,
        outputFile=outputFile,
        OutputLevel=kwargs.pop('OutputLevel', INFO)))
    return acc
