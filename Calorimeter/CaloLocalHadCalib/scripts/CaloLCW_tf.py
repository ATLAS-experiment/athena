#!/usr/bin/env python

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

__doc__ = """JobTransform to run Calo LCW computation job"""


import sys
from PyJobTransforms.transform import transform
from PyJobTransforms.trfExe import athenaExecutor
from PyJobTransforms.trfArgs import addAthenaArguments, addDetectorArguments
import PyJobTransforms.trfArgClasses as trfArgClasses

if __name__ == '__main__':

    executorSet = set()
    executorSet.add(athenaExecutor(name = 'CaloLCW', skeletonCA='CaloLocalHadCalib.CaloLCWSkeleton',
                                   substep = 'e2d', inData = ['ESD',], outData = ['HIST_CLASS','HIST_OOC','HIST_WEIGHTS','NTUP_DM']))

    trf = transform(executor = executorSet)
    addAthenaArguments(trf.parser)
    addDetectorArguments(trf.parser)
    # Use arggroup to get these arguments in their own sub-section (of --help)
    trf.parser.defineArgGroup('CaloLCW_tf', 'LCW specific options')
    # input output files
    trf.parser.add_argument('--inputESDFile', nargs='+',
                            type=trfArgClasses.argFactory(trfArgClasses.argPOOLFile, io='input'),
                            help='Input ESD file', group='CaloLCW_tf')

    trf.parser.add_argument('--outputHIST_CLASSFile', nargs='+',
                            type=trfArgClasses.argFactory(trfArgClasses.argHISTFile, io='output', type='hist', countable=False),
                            help='Output LCW classification file (default: classify.roo)', group='CaloLCW_tf',  
                            default='classify.root')

    trf.parser.add_argument('--outputHIST_OOCFile', nargs='+',
                            type=trfArgClasses.argFactory(trfArgClasses.argHISTFile, io='output', type='hist', countable=False),
                            help='Output LCW OOC file (default: ooc.root)', group='CaloLCW_tf',  
                            default='ooc.root')

    trf.parser.add_argument('--outputHIST_WEIGHTSFile', nargs='+',
                            type=trfArgClasses.argFactory(trfArgClasses.argHISTFile, io='output', type='hist', countable=False),
                            help='Output LCW Weights file (default: weights.root)', group='CaloLCW_tf',  
                            default='weights.root')

    trf.parser.add_argument('--outputNTUP_DMFile', nargs='+',
                            type=trfArgClasses.argFactory(trfArgClasses.argNTUPFile, io='output', treeNames="DeadMaterialTree"),
                            help='Output LCW Dead material tree file (default: dmc.root)', group='CaloLCW_tf', 
                            default='dmc.root')

    # boolean switches
    trf.parser.add_argument('--doClassification', type=trfArgClasses.argFactory(trfArgClasses.argBool),
                                                  help='LCW runs classification (default: True)', group='CaloLCW_tf', 
                                                  default=trfArgClasses.argBool(True))

    trf.parser.add_argument('--doWeighting', type=trfArgClasses.argFactory(trfArgClasses.argBool),
                                              help='LCW runs weighting (default: True)', group='CaloLCW_tf', 
                                                  default=trfArgClasses.argBool(True))

    trf.parser.add_argument('--doOOC', type=trfArgClasses.argFactory(trfArgClasses.argBool),
                                              help='LCW runs OOC (default: True)', group='CaloLCW_tf', 
                                              default=trfArgClasses.argBool(True))

    trf.parser.add_argument('--doDeadM', type=trfArgClasses.argFactory(trfArgClasses.argBool),
                                              help='LCW runs Dead Material (default: True)', group='CaloLCW_tf', 
                                              default=trfArgClasses.argBool(True))

    trf.parser.add_argument('--doLocalCalib', type=trfArgClasses.argFactory(trfArgClasses.argBool),
                                              help='reclustering runs calibration (default: False)', group='CaloLCW_tf', 
                                              default=trfArgClasses.argBool(False))

    trf.parser.add_argument('--doCellWeight', type=trfArgClasses.argFactory(trfArgClasses.argBool),
                                              help='reclustering runs cell weights (default: False)', group='CaloLCW_tf', 
                                              default=trfArgClasses.argBool(False))

    trf.parser.add_argument('--doHitMoments', type=trfArgClasses.argFactory(trfArgClasses.argBool),
                                              help='reclustering runs calib. hit moments (default: True)', group='CaloLCW_tf', 
                                              default=trfArgClasses.argBool(True))


    # string parameters
    trf.parser.add_argument('--ClusKey', type=trfArgClasses.argFactory(trfArgClasses.argString),
                                         help='key for copied clusters (default: CopyCaloTopoCluster)', group='CaloLCW_tf',
                                         default='CopyCaloTopoCluster')

    trf.parseCmdLineArgs(sys.argv[1:])
    trf.execute()
    trf.generateReport()
