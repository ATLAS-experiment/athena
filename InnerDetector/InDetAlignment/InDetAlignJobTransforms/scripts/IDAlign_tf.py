#!/usr/bin/env python3

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# File: InDetAlignJobTransforms/scripts/IDAlign_tf.py
# Author: David Brunner (david.brunner@cern.ch)

import os
import sys
import time

# Setup core logging here
from PyJobTransforms.trfLogger import msg
msg.info('logging set in %s' % sys.argv[0])

from PyJobTransforms.transform import transform
from PyJobTransforms.trfArgs import addAthenaArguments
from PyJobTransforms.trfDecorators import stdTrfExceptionHandler, sigUsrStackTrace
from PyJobTransforms.trfExe import athenaExecutor
from InDetAlignJobTransforms.IDAlignTransformUtils import addIDAlignArguments

@stdTrfExceptionHandler
@sigUsrStackTrace
def main():
    trf = getTransform()
    trf.parseCmdLineArgs(sys.argv[1:])

    # Just add a note here that this is the place to insert extra checks or manipulations
    # after the arguments are known, but before the transform tries to trace the graph
    # path or actually execute (e.g., one can add some steering based on defined arguments) 

    trf.execute()
    trf.generateReport()

    msg.info("%s stopped at %s, trf exit code %d", sys.argv[0], time.asctime(), trf.exitCode)
    sys.exit(trf.exitCode)


def getTransform():
    executorSet = set()
    executorSet.add(athenaExecutor(name = 'IDAlign',
                                   skeletonCA = 'InDetAlignJobTransforms.IDAlign_Skeleton'
                                   ))

    trf = transform(executor = executorSet, description = 'Running the ID alignment using refitted tracks reconstructed from RAW files. Either accumulation step (track refitting, calculating derivates) or solve step (matrix inversion + constant update) can be run.')
    
    addAthenaArguments(trf.parser)
    addIDAlignArguments(trf.parser)

    return trf


if __name__ == '__main__':
    main()
