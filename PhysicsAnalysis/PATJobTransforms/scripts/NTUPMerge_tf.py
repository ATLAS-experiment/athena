#! /usr/bin/env python

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

## MergeNTUP_tf.py - NTUPLE merger

import sys
import time

# Setup core logging here
from PyJobTransforms.trfLogger import msg
msg.info('logging set in %s' % sys.argv[0])

from PyJobTransforms.transform import transform
from PyJobTransforms.trfArgs import addExtraDPDTypes
from PyJobTransforms.trfDecorators import stdTrfExceptionHandler, sigUsrStackTrace
from PATJobTransforms.PATTransformUtils import addNTUPMergeSubsteps, addPhysValidationMergeFiles

@stdTrfExceptionHandler
@sigUsrStackTrace
def main():
    
    msg.info('This is %s' % sys.argv[0])
    if sys.argv[1:] == []:
        msg.info("%s stopped at %s, no input parameters given" % (sys.argv[0], time.asctime()))
    
    # It is a bit of a hack to look for the skip post-processing argument in the command line arguments.
    skip_post_processing = '--skipPostProcessing' in sys.argv
    trf = getTransform(skip_post_processing)
    trf.parseCmdLineArgs(sys.argv[1:])
    trf.execute()
    trf.generateReport()

    msg.info("%s stopped at %s, tf exit code %d" % (sys.argv[0], time.asctime(), trf.exitCode))
    sys.exit(trf.exitCode)

def getTransform(skip_post_processing=False):
    msg.debug("in getTransform...")

    # get the default executor list
    executorSet = set()
    addNTUPMergeSubsteps(executorSet, skip_post_processing)
    trf = transform(executor = executorSet, description = 'ATLAS NTUPLE merge and post-processing transform')
    addPhysValidationMergeFiles(trf.parser)

    # additional setup
    addExtraDPDTypes(trf.parser, transform=trf, NTUPMergerArgs = True)
    return trf

if __name__ == '__main__':
    main()

