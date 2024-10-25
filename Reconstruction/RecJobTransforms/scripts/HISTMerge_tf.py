#! /usr/bin/env python

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

## @brief Small transform which only runs HIST merging

import sys
import time

# Setup core logging here
from PyJobTransforms.trfLogger import msg
msg.info('logging set in %s' % sys.argv[0])

from PyJobTransforms.transform import transform
from PyJobTransforms.trfExe import DQMergeExecutor
from PyJobTransforms.trfDecorators import stdTrfExceptionHandler, sigUsrStackTrace

import PyJobTransforms.trfArgClasses as trfArgClasses

ListOfDefaultPositionalKeys=['--amiConfig', '--amiMetadataTag', '--checkEventCount', '--env', '--execOnly', '--ignoreErrors', '--inputHIST_File', '--noimf', '--notcmalloc', '--outputHISTFile', '--reportName', '--showGraph', '--showPath', '--showSteps', '--skipFileValidation', '--skipInputFileValidation', '--skipOutputFileValidation','--excludeDir','--excludeHist','--run_post_processing','--is_incremental_merge']

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
    executorSet = DQMergeExecutor(name = 'HistogramMerge', inData = ['HIST'], outData = ['HIST_MRG'])
    
    trf = transform(executor = executorSet)
    
    addMyArgs(trf.parser)
    return trf


def addMyArgs(parser):
    # Use arggroup to get these arguments in their own sub-section (of --help)
    parser.defineArgGroup('DQHistMerge_tf', 'DQ merge specific options')
    parser.add_argument('--inputHISTFile', nargs='+', 
                        type=trfArgClasses.argFactory(trfArgClasses.argHISTFile, io='input',
                                                      countable=False), 
                        help='Input DQ histogram file', group='DQHistMerge_tf')
    parser.add_argument('--outputHIST_MRGFile', '--outputHISTFile',nargs='+', 
                        type=trfArgClasses.argFactory(trfArgClasses.argHISTFile, io='output', countable=False), 
                        help='Output DQ monitoring file', group='DQHistMerge_tf')
    
    parser.add_argument('--excludeDir', help='Regex pattern for directories to exclude from merge',group='DQHistMerge_tf')
    parser.add_argument('--excludeHist', help='Regex pattern for histogram names to exclude from merge\n'
                                            'Note that this is just the name - paths cannot be specified',group='DQHistMerge_tf')
    
    parser.add_argument('--run_post_processing', default=False, action="store_true",  help='False/True/0/1 default=0',group='DQHistMerge_tf')
    parser.add_argument('--is_incremental_merge', default=False, action="store_true", help='False/True/0/1 default=0',group='DQHistMerge_tf')
    

if __name__ == '__main__':
    main()
