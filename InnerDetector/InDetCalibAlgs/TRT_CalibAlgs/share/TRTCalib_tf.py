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
                                   skeletonCA='TRT_CalibAlgs.TRTCalibSkeleton',
                                   substep = 'r2e', inData = ['RAW',], outData = ['NTUP_TRTCALIB']))
    
    trf = transform(executor = executorSet)  
    addAthenaArguments(trf.parser)
    addDetectorArguments(trf.parser)
    trf.parser.add_argument('--inputRAWFile', nargs='+',
                            type=trfArgClasses.argFactory(trfArgClasses.argBSFile, io='input'),
                            help='Input bytestream file')
    
    # default name for now: basic.root - To be updated (so used so far)
    trf.parser.add_argument('--outputNTUP_TRTCALIBFile', nargs='+',
                            type=trfArgClasses.argFactory(trfArgClasses.argNTUPFile, io='output', treeNames="events"),
                            help='Output TRT calib file')
    
    # TODO - Add the option to specify DoCalibrator ( default must be False )
    # Can also work writing another transform since we will need to change the Input (RAW and ntuple) and output names!
    
    trf.parseCmdLineArgs(sys.argv[1:])
    
    trf.execute()
    trf.generateReport()

    
