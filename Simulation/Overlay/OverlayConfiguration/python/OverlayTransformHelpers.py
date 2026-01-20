"""Main overlay transform configuration helpers

Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""

from PyJobTransforms.trfArgClasses import argBSFile, argFactory, argList, argRDOFile, argSubstepInt
from PyJobTransforms.trfExe import athenaExecutor


def addOverlayTrfArgs(parser):
    """Add common overlay command-line parser arguments."""
    parser.defineArgGroup('Overlay', 'Common Overlay Options')
    parser.add_argument('--detectors', nargs='+',
                        type=argFactory(argList),
                        help='Detectors autoconfiguration string',
                        group='Overlay')
    parser.add_argument('--skipSecondaryEvents', nargs='+',
                        type=argFactory(argSubstepInt, defaultSubstep='first'), 
                        help='Number of secondary input events to skip over in the first processing step (skipping substep can be overridden)',
                        group='Overlay')
    parser.add_argument('--inputRDO_BKGFile', nargs='+',
                        type=argFactory(argRDOFile, io='input'),
                        help='Input background RDO for MC+MC overlay',
                        group='MCOverlay')
    parser.add_argument('--outputRDO_SGNLFile', nargs='+',
                        type=argFactory(argRDOFile, io='output'),
                        help='The output RDO file of the MC signal alone',
                        group='Overlay')


def addDataOverlayBSTrfArgs(parser):
    """Add MC overlay command-line parser arguments."""
    parser.defineArgGroup('DataOverlayBS', 'Data overlay BS pre-processing')
    parser.add_argument('--inputBSFile', nargs='+',
                        type=argFactory(argBSFile, io='input'),
                        help='Input minimum-bias BS for data+MC overlay',
                        group='DataOverlayBS')
    parser.add_argument('--outputRDO_BKGFile', nargs='+',
                        type=argFactory(argRDOFile, io='output'),
                        help='Output background RDO for data+MC overlay',
                        group='DataOverlayBS')



def addOverlayArguments(parser, in_reco_chain=False):
    """Add all overlay command-line parser arguments."""
    # TODO: are forward detectors really needed?
    from SimuJobTransforms.simTrfArgs import addBasicDigiArgs  # , addForwardDetTrfArgs
    addBasicDigiArgs(parser)
    # addForwardDetTrfArgs(parser)
    addOverlayTrfArgs(parser)
    if not in_reco_chain:
        addDataOverlayBSTrfArgs(parser)


def addOverlaySubstep(executor_set, in_reco_chain=False):
    executor = athenaExecutor(name='Overlay',
                              skeletonCA='OverlayConfiguration.OverlaySkeleton',
                              substep='overlay',
                              tryDropAndReload=False,
                              perfMonFile='ntuple.pmon.gz',
                              inData=['RDO_BKG', 'HITS'],
                              outData=['RDO', 'RDO_SGNL'])

    if in_reco_chain:
        executor.inData = []
        executor.outData = []

    executor_set.add(executor)


def addBStoRDOSubstep(executor_set):
    executor = athenaExecutor(name='BStoRDO',
                              skeletonCA='OverlayConfiguration.BStoRDO_Skeleton',
                              substep='BStoRDO',
                              tryDropAndReload=False,
                              perfMonFile='ntuple.pmon.gz',
                              inData=['BS'],
                              outData=['RDO_BKG'])

    executor_set.add(executor)


def appendOverlaySubstep(trf, in_reco_chain=False):
    """Add overlay transform substep."""
    executor = set()
    addOverlaySubstep(executor, in_reco_chain)
    trf.appendToExecutorSet(executor)
