#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""
EventGUIDLookup_tf.py

Given an ATLAS data type (RAW, ESD, AOD, DAOD_PHYS, DAOD_PHYSLITE, RDO,
HITS, EVNT) and a text file listing (run number, event number) pairs
-- one pair per line, space separated -- resolve, for every requested
event actually present in the input file(s), the provenance GUID for
that data type by decoding the "Stream references" POOL writes into the
CollectionTree, exactly as
  Database/EventIndex/EventIndexProducer/python/POOL2EI_Lib.py
does when building the full Event Index -- except here only the
requested (run,event,dataType) GUIDs are resolved and written out as
plain text, one "runNumber eventNumber GUID" triplet per line.
"""

import sys

from PyJobTransforms.trfLogger import msg
from PyJobTransforms.transform import transform
from PyJobTransforms.trfExe import athenaExecutor
from PyJobTransforms.trfArgs import addAthenaArguments
from PyJobTransforms.trfDecorators import stdTrfExceptionHandler, sigUsrStackTrace
import PyJobTransforms.trfArgClasses as trfArgClasses


SUPPORTED_TYPES = ('RAW', 'ESD', 'AOD', 'DAOD_PHYS', 'DAOD_PHYSLITE',
                    'RDO', 'HITS', 'EVNT')


@stdTrfExceptionHandler
@sigUsrStackTrace

def addEventGUIDLookupArgs(parser):
    parser.defineArgGroup('EventGUIDLookup', 'EventGUIDLookup Options')

    parser.add_argument('--dataType',
                         type=trfArgClasses.argFactory(trfArgClasses.argString,
                                                        runarg=True),
                         required=True,
                         group='EventGUIDLookup',
                         help='Data type whose provenance GUID to resolve: '
                              'one of %s' % ', '.join(SUPPORTED_TYPES))

    parser.add_argument('--inputFile', nargs='+', required=True,
                         type=trfArgClasses.argFactory(
                             trfArgClasses.argAthenaFile, io='input',
                             runarg=True, type='input'),
                         group='EventGUIDLookup',
                         help='Input file(s) to search (any POOL format, '
                              'or RAW if --dataType RAW)')

    parser.add_argument('--inputDataType',
                         type=trfArgClasses.argFactory(trfArgClasses.argString,
                                                        runarg=True),
                         required=True,
                         group='EventGUIDLookup',
                         help='Actual data type of --inputFile (always a '
                              'POOL format): one of %s'
                              % ', '.join(SUPPORTED_TYPES))

    parser.add_argument('--eventList', required=True,
                         type=trfArgClasses.argFactory(trfArgClasses.argString,
                                                        runarg=True),
                         group='EventGUIDLookup',
                         help='Text file, one "runNumber eventNumber" pair '
                              'per line (space separated)')

    parser.add_argument('--outputTXTFile', required=True,
                         type=trfArgClasses.argFactory(trfArgClasses.argFile,
                                                        io='output', runarg=True,
                                                        type='eventGUIDOutput'),
                         group='EventGUIDLookup',
                         help='Output text file: "runNumber eventNumber GUID" '
                              'triplets, one per line')

def getTransform():
    executorSet = {athenaExecutor(
        name='EventGUIDLookup',
        skeletonCA='EventGUIDLookup.EventGUIDLookup_Skeleton',
        substep='eglkp',
        inData=set(),
        outData=set())}

    trf = transform(executor=executorSet,
                     description='Resolve provenance GUIDs, for a given '
                                  'data type, of a list of (run,event) '
                                  'pairs -- via the same Stream-references '
                                  'mechanism used by POOL2EI_Lib.py.')
    addAthenaArguments(trf.parser)
    addEventGUIDLookupArgs(trf.parser)
    return trf

def main():
    msg.info('This is %s', sys.argv[0])

    trf = getTransform()
    trf.parseCmdLineArgs(sys.argv[1:])

    dataType = trf.argdict['dataType'].value
    if dataType not in SUPPORTED_TYPES:
        msg.error('Unsupported --dataType %r (must be one of: %s)',
                   dataType, ', '.join(SUPPORTED_TYPES))
        sys.exit(1)

    trf.execute()
    trf.generateReport()

    msg.info('%s stopped gracefully', sys.argv[0])
    sys.exit(0)

if __name__ == '__main__':
    main()
