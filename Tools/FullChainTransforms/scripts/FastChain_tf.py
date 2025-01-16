#! /usr/bin/env python

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#  FastChain_tf.py
#  One step transform to run SIM+DIGI as one job, then reco
#  to ESD/AOD output
#  Richard Hawkings, adapted from FullChain_tf.py by Graeme Stewart

import sys
import time

# Setup core logging here
from PyJobTransforms.trfLogger import msg
msg.info('logging set in %s', sys.argv[0])

from PyJobTransforms.transform import transform
from PyJobTransforms.trfExe import athenaExecutor
from PyJobTransforms.trfArgs import addAthenaArguments, addDetectorArguments, addTriggerArguments
from PyJobTransforms.trfDecorators import stdTrfExceptionHandler, sigUsrStackTrace
from RecJobTransforms.recTransformUtils import addRecoSubsteps, addAllRecoArgs
from SimuJobTransforms.simTrfArgs import addCommonSimTrfArgs, addBasicDigiArgs, addCommonSimDigTrfArgs, addTrackRecordArgs, addSim_tfArgs, addSimIOTrfArgs, addPileUpTrfArgs
from DerivationFrameworkConfiguration.DerivationTransformHelpers import addDerivationArguments, addDerivationSubstep, addPhysicsValidationArguments, addPhysicsValidationSubstep

from PyJobTransforms.trfArgClasses import argFactory, argList, argRDOFile

@stdTrfExceptionHandler
@sigUsrStackTrace
def main():

    msg.info('This is %s', sys.argv[0])

    trf = getTransform()
    trf.parseCmdLineArgs(sys.argv[1:])
    trf.execute()
    trf.generateReport()

    msg.info("%s stopped at %s, trf exit code %d", sys.argv[0], time.asctime(), trf.exitCode)
    sys.exit(trf.exitCode)

def getTransform():
    executorSet = set()

    addRecoSubsteps(executorSet)

    executorSet.add(athenaExecutor(name = 'EVNTtoRDO',
                                   skeletonCA = 'FullChainTransforms.FastChainSkeleton',
                                   substep = 'simdigi', tryDropAndReload = False, perfMonFile = 'ntuple.pmon.gz',
                                   inData=['NULL','EVNT', 'RDO_BKG', 'BS_SKIM'],
                                   outData=['RDO', 'HITS', 'NULL'] ))

    # Derivation
    addDerivationSubstep(executorSet)
    addPhysicsValidationSubstep(executorSet)

    trf = transform(executor = executorSet, description = 'Fast chain ATLAS transform with ISF simulation, digitisation'
                    ' and reconstruction. Inputs can be EVNT, with outputs of RDO, ESD, AOD or DPDs.'
                    ' See https://twiki.cern.ch/twiki/bin/viewauth/AtlasComputing/FastChainTf for more details.')

    # Common arguments
    addAthenaArguments(trf.parser)
    addDetectorArguments(trf.parser)
    addTriggerArguments(trf.parser)

    # Reconstruction arguments and outputs (use the factorised 'do it all' function)
    addAllRecoArgs(trf)

    # Simulation and digitisation options
    addCommonSimTrfArgs(trf.parser)
    addCommonSimDigTrfArgs(trf.parser)
    addBasicDigiArgs(trf.parser)
    addSim_tfArgs(trf.parser)
    addSimIOTrfArgs(trf.parser)
    # addForwardDetTrfArgs(trf.parser)
    addPileUpTrfArgs(trf.parser)
    addTrackRecordArgs(trf.parser)
    addFastChainTrfArgs(trf.parser)

    # Overlay arguments
    from OverlayConfiguration.OverlayTransformHelpers import addOverlayArguments
    addOverlayArguments(trf.parser)

    # Derivation agruments
    addDerivationArguments(trf.parser)
    addPhysicsValidationArguments(trf.parser)

    return trf

def addFastChainTrfArgs(parser):
    "Add transformation arguments for fast chain"
    parser.defineArgGroup('FastChain','Fast chain options')
    parser.add_argument('--preSimExec',type=argFactory(argList),nargs='+',
                        help='preExec before simulation step',
                        group='FastChain')
    parser.add_argument('--postSimExec',type=argFactory(argList),nargs='+',
                        help='postExec after simulation step',
                        group='FastChain')
    parser.add_argument('--preDigiExec',type=argFactory(argList),nargs='+',
                        help='preExec before digitisation step',
                        group='FastChain')
    parser.add_argument('--preSimInclude',type=argFactory(argList),nargs='+',
                        help='preInclude before simulation step',
                        group='FastChain')
    parser.add_argument('--postSimInclude',type=argFactory(argList),nargs='+',
                        help='postInclude after simulation step',
                        group='FastChain')
    parser.add_argument('--preDigiInclude',type=argFactory(argList),nargs='+',
                        help='preInclude before digitisation step',
                        group='FastChain')
    parser.defineArgGroup('MCOverlay', 'MC Overlay Options')
    parser.add_argument('--inputRDO_BKGFile', nargs='+',
                        type=argFactory(argRDOFile, io='input'),
                        help='Input background RDO for MC+MC overlay',
                        group='MCOverlay')


if __name__ == '__main__':
    main()
