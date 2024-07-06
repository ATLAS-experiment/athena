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
    executorSet.add(athenaExecutor(name = 'TRTCalib',
                                   skeletonCA='TRT_CalibAlgs.TRTCalibSkeleton', inData = ['RAW','BINARY'], outData = ['NTUP_TRTCALIB']))
    
    trf = transform(executor = executorSet)  
    addAthenaArguments(trf.parser)
    addDetectorArguments(trf.parser)

    # Use arggroup to get these arguments in their own sub-section (of --help)
    trf.parser.defineArgGroup('TRTCalib_tf', 'TRT r-t calibration transform')
    
    # Input file! Always must be RAW data 
    trf.parser.add_argument('--inputRAWFile', nargs='+',
                            type=trfArgClasses.argFactory(trfArgClasses.argBSFile, io='input'),
                            help='Input bytestream file. RAW data', group='TRTCalib_tf')
    
    # OutputFile name
    trf.parser.add_argument('--outputNTUP_TRTCALIBFile', nargs='+',
                            type=trfArgClasses.argFactory(trfArgClasses.argNTUPFile, io='output'),
                            help='Output TRT calib file', group='TRTCalib_tf')
    
    # Running the calibrator
    trf.parser.add_argument('--doCalibrator', type=trfArgClasses.argFactory(trfArgClasses.argBool),
                            help='Runs (default: False)', default=trfArgClasses.argBool(False), group='TRTCalib_tf')
    
    # Input file for the calibrator. Only if there is a  - Must be a binary file!
    trf.parser.add_argument('--inputBINARYFile', type=trfArgClasses.argFactory(trfArgClasses.argString), 
                            help='Input binary file.',default=trfArgClasses.argString('') ,group='TRTCalib_tf')

    # Add here the Caltag!
    trf.parser.add_argument('--calTag', type=trfArgClasses.argFactory(trfArgClasses.argString), 
                            help='Detector part to be calibrated',default=trfArgClasses.argString('') ,group='TRTCalib_tf')
        
    trf.parseCmdLineArgs(sys.argv[1:])
    
    trf.execute()
    trf.generateReport()

    
