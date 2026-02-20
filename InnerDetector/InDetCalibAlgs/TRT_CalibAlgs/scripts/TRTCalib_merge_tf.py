#!/usr/bin/env python

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

__doc__ = """JobTransform to run TRT R-t Calibration jobs"""


import sys
from PyJobTransforms.transform import transform
from PyJobTransforms.trfExe import athenaExecutor
from PyJobTransforms.trfArgs import addAthenaArguments, addDetectorArguments
import PyJobTransforms.trfArgClasses as trfArgClasses

if __name__ == '__main__':

    executorSet = set()
    executorSet.add(athenaExecutor(name = 'TRTCalibMerge',
                                   skeletonCA='TRT_CalibAlgs.TRTCalib_merge_Skeleton', inData = ['TAR'], outData = ['TAR_MERGED']))
    
    trf = transform(executor = executorSet)  
    addAthenaArguments(trf.parser)
    addDetectorArguments(trf.parser)

    # Use arggroup to get these arguments in their own sub-section (of --help)
    trf.parser.defineArgGroup('TRTCalib_merge_tf', 'TRT r-t calibration transform')
    
    # Input file! 
    trf.parser.add_argument('--inputTARFile', nargs='+',
                            type=trfArgClasses.argFactory(trfArgClasses.argBZ2File, io='input'),
                            help='Compressed input files', group='TRTCalib_merge_tf')
    
    # OutputFile name
    trf.parser.add_argument('--outputTAR_MERGEDFile',
                            type=trfArgClasses.argFactory(trfArgClasses.argBZ2File, io='output'),
                            help='Compressed output file', group='TRTCalib_merge_tf')
    
    trf.parseCmdLineArgs(sys.argv[1:])
    
    trf.execute()
    trf.generateReport()
    if trf.exitCode != 0:
        sys.exit(trf.exitCode)
