#!/usr/bin/env python

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

__doc__ = """JobTransform to run LAr SuperCells reco and dumping jobs"""


import sys
from PyJobTransforms.transform import transform
from PyJobTransforms.trfExe import athenaExecutor
from PyJobTransforms.trfArgs import addAthenaArguments, addDetectorArguments
import PyJobTransforms.trfArgClasses as trfArgClasses

if __name__ == '__main__':

    executorSet = set()
    executorSet.add(athenaExecutor(name = 'LArSCRecoTest',
                                   skeletonCA='LArCafJobs.LArSCRecoTestSkeleton',
                                   substep = 'r2e', inData = ['BS',], outData = ['NTUP_SCReco']))
    
    trf = transform(executor = executorSet)  
    addAthenaArguments(trf.parser)
    addDetectorArguments(trf.parser)
    trf.parser.add_argument('--inputBSFile', nargs='+',
                            type=trfArgClasses.argFactory(trfArgClasses.argBSFile, io='input'),
                            help='Input bytestream file', group='Reco Files')
    
    trf.parser.add_argument('--outputNTUP_SCRecoFile', nargs='+',
                            type=trfArgClasses.argFactory(trfArgClasses.argNTUPFile, io='output', treeNames="LARSC"),
                            help='Output LAr SuperCells file', group='Ntuple Files')

    trf.parser.add_argument('--startSampleShift', 
                            default=trfArgClasses.argInt(0),
                            type=trfArgClasses.argFactory(trfArgClasses.argInt),
                            help='shift of start sample for reco...')
    
    trf.parseCmdLineArgs(sys.argv[1:])
    trf.execute()
    trf.generateReport()

    
