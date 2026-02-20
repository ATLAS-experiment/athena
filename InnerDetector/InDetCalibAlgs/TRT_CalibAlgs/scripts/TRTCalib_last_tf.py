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
    executorSet.add(athenaExecutor(name = 'TRTCalibLast',
                                   skeletonCA='TRT_CalibAlgs.TRTCalib_last_Skeleton', inData = ['TAR'], outData = ['TAR_MERGED']))
    
    trf = transform(executor = executorSet)  
    addAthenaArguments(trf.parser)
    addDetectorArguments(trf.parser)

    # Use arggroup to get these arguments in their own sub-section (of --help)
    trf.parser.defineArgGroup('TRTCalib_last_tf', 'TRT r-t calibration transform')
    
    # Input file! 
    trf.parser.add_argument('--inputTARFile', nargs='+',
                            type=trfArgClasses.argFactory(trfArgClasses.argBZ2File, io='input'),
                            help='Compressed input files', group='TRTCalib_last_tf')
    
    # OutputFile name
    trf.parser.add_argument('--outputTAR_MERGEDFile',
                            type=trfArgClasses.argFactory(trfArgClasses.argBZ2File, io='output'),
                            help='Compressed output file', group='TRTCalib_last_tf')
    
    # Directory for the attrtcal account web display
    trf.parser.add_argument('--attrtcal_dir',
                            type=trfArgClasses.argFactory(trfArgClasses.argString),
                            help='Saving output for web display', default=trfArgClasses.argString('/afs/cern.ch/user/a/attrtcal/Tier0') , group='TRTCalib_last_tf')
    
    # Notify people 
    trf.parser.add_argument('--sendNotification',
                            type=trfArgClasses.argFactory(trfArgClasses.argBool),
                            help='Notify the offline TRT calibration team', default=trfArgClasses.argBool(True) , group='TRTCalib_last_tf')
    
    # Email list 
    trf.parser.add_argument('--emailList', nargs='+',
                            type=trfArgClasses.argFactory(trfArgClasses.argList),
                            help='Notify the offline TRT calibration team', default=trfArgClasses.argList(["atlas-trt-offline-shifts@cern.ch"]) , group='TRTCalib_last_tf')
    
    trf.parseCmdLineArgs(sys.argv[1:])
    
    trf.execute()
    trf.generateReport()
    if trf.exitCode != 0:
        sys.exit(trf.exitCode)
