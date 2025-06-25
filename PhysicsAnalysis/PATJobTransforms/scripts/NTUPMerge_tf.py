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
    
    trf = getTransform()
    trf.parseCmdLineArgs(sys.argv[1:])
    trf.execute()
    trf.generateReport()

    msg.info("%s stopped at %s, tf exit code %d" % (sys.argv[0], time.asctime(), trf.exitCode))
    sys.exit(trf.exitCode)

def getTransform():
    msg.debug("in getTransform...")

    # get the default executor list
    executorSet = set()
    # instantiate a transform with no steps
    trf = transform(executor = executorSet, description = 'ATLAS NTUPLE merge and post-processing transform')
    
    # add custom merge and post-processing 
    # steering parameters and get the 'args'
    addPhysValidationMergeFiles(trf.parser)
    args = trf.parser.parse_args()
    msg.debug("args:", args)

    # get the modified executor
    mergeStepSet = set()

    # Check the user's optional parameters
    # NOTE: we need to first check if the arg is present, 
    # then we get the value. When not specified, in fact, 
    # the optional args are not present in the list of
    # args. Also, if we only check its existance, 
    # we don't get its value when set with set()
    skipPP = False
    if 'skipPostProcessing' in args:
        skipPP = args.skipPostProcessing

    # add to the transform the merge and 
    # post-processing steps conditionally 
    # based on user's input
    addNTUPMergeSubsteps(mergeStepSet, skip_post_processing = skipPP)
    trf.appendToExecutorSet(list(mergeStepSet))

    # additional setup
    addExtraDPDTypes(trf.parser, transform=trf, NTUPMergerArgs = True)
    return trf

if __name__ == '__main__':
    main()

