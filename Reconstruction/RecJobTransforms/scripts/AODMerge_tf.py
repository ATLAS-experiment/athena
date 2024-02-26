#! /usr/bin/env python

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

## AODMerge_tf.py - AOD merger with optional TAG and DPD outputs
#  N.B. Do need clarification as to if AODtoDPD is ever run in parallel with AOD merging 
# @version $Id: AODMerge_tf.py 670203 2015-05-27 13:19:01Z graemes $ 

import sys
import time

# Setup core logging here
from PyJobTransforms.trfLogger import msg
msg.info('logging set in %s' % sys.argv[0])

from PyJobTransforms.transform import transform
from PyJobTransforms.trfExe import hybridPOOLMergeExecutor
from PyJobTransforms.trfArgs import addAthenaArguments, addDetectorArguments
from PyJobTransforms.trfDecorators import stdTrfExceptionHandler, sigUsrStackTrace
from RecJobTransforms.recTransformUtils import addCommonRecTrfArgs

import PyJobTransforms.trfArgClasses as trfArgClasses

ListOfDefaultPositionalKeys=['--DBRelease', '--amiConfig', '--amiMetadataTag', '--asetup', '--athena', '--athenaopts', '--autoConfiguration', '--beamType', '--checkEventCount', '--command', '--conditionsTag', '--env', '--eventAcceptanceEfficiency', '--execOnly', '--geometryVersion', '--ignoreErrors', '--ignoreFilters', '--ignorePatterns', '--inputAODFile', '--maxEvents', '--noimf', '--notcmalloc', '--outputAOD_MRGFile', '--postExec', '--postInclude', '--preExec', '--preInclude', '--reportName', '--runNumber', '--showGraph', '--showPath', '--showSteps', '--skipEvents', '--skipFileValidation', '--skipInputFileValidation', '--skipOutputFileValidation']

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
    executorSet.add(hybridPOOLMergeExecutor(name = 'AODMerge', inData = ['AOD'], outData = ['AOD_MRG']))

    trf = transform(executor = executorSet)
    
    addAthenaArguments(trf.parser)
    addDetectorArguments(trf.parser)
    addCommonRecTrfArgs(trf.parser)
    addMyArgs(trf.parser)
    return trf


def addMyArgs(parser):
    # Use arggroup to get these arguments in their own sub-section (of --help)
    parser.defineArgGroup('AODMerge_tf', 'AOD merge job specific options')
    parser.add_argument('--inputAODFile', nargs='+',
                        type=trfArgClasses.argFactory(trfArgClasses.argPOOLFile, io='input', type='aod'),
                        help='Input AOD file', group='AODMerge_tf')
    parser.add_argument('--inputAOD_MRGFile', nargs='+',
                        type=trfArgClasses.argFactory(trfArgClasses.argPOOLFile, io='input', type='aod'),
                        help='Input (premarged) AOD file', group='AODMerge_tf')
    parser.add_argument('--outputAOD_MRGFile', '--outputAODFile', 
                        type=trfArgClasses.argFactory(trfArgClasses.argPOOLFile, io='output', type='aod'),
                        help='Output merged AOD file', group='AODMerge_tf')

if __name__ == '__main__':
    main()
