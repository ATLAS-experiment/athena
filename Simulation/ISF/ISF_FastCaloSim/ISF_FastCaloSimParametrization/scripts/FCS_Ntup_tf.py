#! /usr/bin/env python

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

"""
Run HITS file and produce histograms.
"""

import sys
import time

# Setup core logging here
from PyJobTransforms.trfLogger import msg
msg.info('logging set in %s' % sys.argv[0])
from PyJobTransforms.transform import transform
from PyJobTransforms.trfExe import athenaExecutor
from PyJobTransforms.trfArgs import addAthenaArguments, addDetectorArguments
from PyJobTransforms.trfDecorators import stdTrfExceptionHandler, sigUsrStackTrace
from ISF_FastCaloSimParametrization.fcsTrfArgs import addFCS_NtupArgs

@stdTrfExceptionHandler
@sigUsrStackTrace
def main():

    msg.info('This is %s' % sys.argv[0])

    trf = getTransform()
    trf.parseCmdLineArgs(sys.argv[1:])
    trf.execute()
    trf.generateReport()

    msg.info("%s stopped at %s, trf exit code %d" % (sys.argv[0], time.asctime(), trf.exitCode))
    sys.exit(trf.exitCode)

def getTransform():
    executorSet = set()
    from SimuJobTransforms.SimTransformUtils import addSimulationArguments
    executorSet.add(athenaExecutor(name = 'FCS_Ntup',
                                   skeletonCA = 'ISF_FastCaloSimParametrization.ESDtoNTUP_FCS_Skeleton',
                                   inData = ['ESD'], outData = ['NTUP_FCS'],))
    trf = transform(executor = executorSet, description = 'FastCaloSim V2 Parametrization ntuple transform. Inputs must be ESD. Outputs must be ntuple files.')
    addAthenaArguments(trf.parser)
    addDetectorArguments(trf.parser)
    addSimulationArguments(trf.parser)
    addFCS_NtupArgs(trf.parser)
    return trf


if __name__ == '__main__':
    main()
