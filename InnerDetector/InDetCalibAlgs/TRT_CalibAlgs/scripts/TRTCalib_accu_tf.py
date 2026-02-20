#!/usr/bin/env python

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

__doc__ = """JobTransform to run TRT R-t Calibration jobs"""


import sys
from PyJobTransforms.transform import transform
from PyJobTransforms.trfExe import athenaExecutor
from PyJobTransforms.trfArgs import addAthenaArguments, addDetectorArguments
import PyJobTransforms.trfArgClasses as trfArgClasses
from AthenaConfiguration.TestDefaults import defaultTestFiles

if __name__ == '__main__':

    executorSet = set()
    executorSet.add(athenaExecutor(name = 'TRTCalibAccu',
                                   skeletonCA='TRT_CalibAlgs.TRTCalib_accu_Skeleton', inData = ['RAW'], outData = ['TAR']))
    
    trf = transform(executor = executorSet)  
    addAthenaArguments(trf.parser)
    addDetectorArguments(trf.parser)

    # Use arggroup to get these arguments in their own sub-section (of --help)
    trf.parser.defineArgGroup('TRTCalib_tf', 'TRT r-t calibration transform')
    
    # Input file! Always must be RAW data 
    trf.parser.add_argument('--inputRAWFile', nargs='+',
                            type=trfArgClasses.argFactory(trfArgClasses.argBSFile, io='input'),
                            help='Input bytestream file name. RAW data',  default=trfArgClasses.argBSFile(defaultTestFiles.RAW_RUN3), group='TRTCalib_tf')
    
    # OutputFile name
    trf.parser.add_argument('--outputTARFile',
                            type=trfArgClasses.argFactory(trfArgClasses.argBZ2File, io='output'),
                            help='Output TRT calib file name.', group='TRTCalib_tf')
    
    trf.parser.add_argument('--calibrate', type=trfArgClasses.argFactory(trfArgClasses.argBool), 
                            help='Calibration constants file.',default=trfArgClasses.argBool(False) ,group='TRTCalib_tf')
    
    trf.parser.add_argument('--calibconstants', type=trfArgClasses.argFactory(trfArgClasses.argString), 
                            help='Calibration constants file.',default=trfArgClasses.argString('') ,group='TRTCalib_tf')
    
    trf.parseCmdLineArgs(sys.argv[1:])
    
    trf.execute()
    trf.generateReport()
    if trf.exitCode != 0:
        sys.exit(trf.exitCode)
