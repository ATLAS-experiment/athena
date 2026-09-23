#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""EventGUIDLookup_Skeleton.py"""

import sys
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaCommon.Logging import logging

msglog = logging.getLogger('EventGUIDLookup_Skeleton')


def _getRunArg(runArgs, base):
    attrs = vars(runArgs)
    candidates = [a for a in attrs if a == base or a.startswith(base)]
    if not candidates:
        raise AttributeError('No runArgs attribute starting with %r (have: %s)'
                              % (base, sorted(attrs.keys())))
    candidates.sort(key=lambda a: (a != base, len(a)))
    return getattr(runArgs, candidates[0])


def _unwrap(a):
    if hasattr(a, 'value'):
        a = a.value
    return a if isinstance(a, list) else [a]


def _loadEventList(path):
    requested = set()
    with open(path) as fh:
        for lineNo, line in enumerate(fh, 1):
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            fields = line.split()
            if len(fields) < 2:
                msglog.warning('Ignoring malformed line %d in %s: %r',
                                lineNo, path, line)
                continue
            requested.add((int(fields[0]), int(fields[1])))
    return requested


def fromRunArgs(runArgs):
    msglog.info('***************** STARTING EventGUIDLookup *****************')

    dataType = runArgs.dataType
    inputDataType = runArgs.inputDataType
    eventListPath = runArgs.eventList
    inputFiles = [str(f) for f in _unwrap(_getRunArg(runArgs, 'inputFile'))]
    outputPath = _unwrap(_getRunArg(runArgs, 'outputTXTFile'))[0]

    requested = _loadEventList(eventListPath)
    msglog.info('Loaded %d requested (run,event) pairs', len(requested))

    flags = initConfigFlags()
    flags.Input.Files = inputFiles
    flags.Exec.MaxEvents = -1
    flags.Concurrency.NumThreads = 1
    flags.lock()

    cfg = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))
    # (xAODEventInfoCnvAlgDefault import removed -- not needed, and not
    # verified to exist at that path anyway)

    from EventGUIDLookup.EventGUIDLookupAlgConfig import EventGUIDLookupAlgCfg
    cfg.merge(EventGUIDLookupAlgCfg(flags, dataType=dataType,
                                     inputDataType=inputDataType,
                                     requestedEvents=requested,
                                     outputFile=outputPath))

    sc = cfg.run()
    sys.exit(0 if sc.isSuccess() else 1)
