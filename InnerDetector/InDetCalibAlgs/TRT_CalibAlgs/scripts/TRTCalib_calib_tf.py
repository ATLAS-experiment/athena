#!/usr/bin/env python

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

__doc__ = """JobTransform to run TRT R-t Calibration jobs"""


import sys
from PyJobTransforms.transform import transform
from PyJobTransforms.trfExe import athenaExecutor
from PyJobTransforms.trfArgs import addAthenaArguments, addDetectorArguments
import PyJobTransforms.trfArgClasses as trfArgClasses

if __name__ == '__main__':

    executorSet = set()
    executorSet.add(athenaExecutor(name = 'TRTCalibCalib',
                                   skeletonCA='TRT_CalibAlgs.TRTCalib_calib_Skeleton',inData = ['TAR'], outData = ['TAR_CALIB']))
    
    trf = transform(executor = executorSet)  
    addAthenaArguments(trf.parser)
    addDetectorArguments(trf.parser)

    # Use arggroup to get these arguments in their own sub-section (of --help)
    trf.parser.defineArgGroup('TRTCalib_calib_tf', 'TRT r-t calibrator transform')
    
    # Input file!
    trf.parser.add_argument('--inputTARFile',
                            type=trfArgClasses.argFactory(trfArgClasses.argBZ2File, io='input'),
                            help='Compressed input data from TRTCalibratorMgr', group='TRTCalib_calib_tf')
    
    # OutputFile name
    trf.parser.add_argument('--outputTAR_CALIBFile',
                            type=trfArgClasses.argFactory(trfArgClasses.argBZ2File, io='output'),
                            help='Output TRT calib file', group='TRTCalib_calib_tf')

    # Add here the Caltag/piecetoken!
    trf.parser.add_argument('--piecetoken', type=trfArgClasses.argFactory(trfArgClasses.argString), 
                            help='Detector part to be calibrated',default=trfArgClasses.argString('') ,group='TRTCalib_calib_tf')

    trf.parser.add_argument('--project', type=trfArgClasses.argFactory(trfArgClasses.argString), 
                            help='project name. E.g.: data24_13p6TeV',default=trfArgClasses.argString('') ,group='TRTCalib_calib_tf')

    trf.parser.add_argument('--runnr', type=trfArgClasses.argFactory(trfArgClasses.argString), 
                            help='run number. E.g.: 00480407',default=trfArgClasses.argString('') ,group='TRTCalib_calib_tf')

    trf.parser.add_argument('--stream', type=trfArgClasses.argFactory(trfArgClasses.argString), 
                            help='stream. E.g.: express_express',default=trfArgClasses.argString('') ,group='TRTCalib_calib_tf')

    trf.parser.add_argument('--step', type=trfArgClasses.argFactory(trfArgClasses.argString), 
                            help='stream step. E.g.: daq or merge',default=trfArgClasses.argString('*.RAW/') ,group='TRTCalib_calib_tf')

    trf.parser.add_argument('--rawfile', type=trfArgClasses.argFactory(trfArgClasses.argString), 
                            help='rawfile only used for testing purposes',default=trfArgClasses.argString('') ,group='TRTCalib_calib_tf')
        
    trf.parseCmdLineArgs(sys.argv[1:])
    
    trf.execute()
    trf.generateReport()
    sys.exit(trf.exitCode)

    
