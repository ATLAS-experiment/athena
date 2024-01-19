#! /bin/env python3
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# @author Teng Jian Khoo

# User options, which can be set from command line after a "-" character
# athena FullCPAlgorithmsTest_jobOptions.py - --myOption ...
from AthenaConfiguration.AllConfigFlags import initConfigFlags
import os

flags = initConfigFlags()
athArgsParser = flags.getArgumentParser()
athArgsParser.add_argument("--force-input", action = "append", dest = "force_input",
                           default = None,
                           help = "Force the given input file")
athArgsParser.add_argument("--force-output", action = "store", dest = "force_output",
                           default = None,
                           help = "Force the given output file")
athArgsParser.add_argument("--data-type", action = "store", dest = "data_type",
                           default = "data",
                           help = "Type of input to run over. Valid options are 'data', 'mc', 'afii'")
athArgsParser.add_argument( '--block-config', dest='block_config',
                            action = 'store_true', default = False,
                            help = 'Configure the job with block configuration' )
athArgsParser.add_argument( '--text-config', dest='text_config',
                            action = 'store', default = '',
                            help = 'Configure the job with the provided text configuration' )
athArgsParser.add_argument( '--for-compare', dest='for_compare',
                            action = 'store_true', default = False,
                            help = 'Configure the job for comparison of sequences vs blocks' )
athArgsParser.add_argument( '--no-systematics', dest='no_systematics',
                            action = 'store_true', default = False,
                            help = 'Configure the job to with no systematics' )
athArgsParser.add_argument( '--physlite', dest='physlite',
                            action = 'store_true', default = False,
                            help = 'Configure the job for physlite' )
athArgsParser.add_argument( '--no-physlite-broken', dest='no_physlite_broken',
                            action = 'store_true', default = False,
                            help = 'Configure the job to skip algorithms that fail on physlite test file' )
athArgsParser.add_argument( '--only-nominal-or', dest='onlyNominalOR',
                            action = 'store_true', default = False,
                            help = 'Only run overlap removal for nominal (skip systematics)')
athArgs = flags.fillFromArgs(parser=athArgsParser)

dataType = athArgs.data_type
blockConfig = athArgs.block_config
textConfig = athArgs.text_config
forCompare = athArgs.for_compare
isPhyslite = athArgs.physlite
noPhysliteBroken = athArgs.no_physlite_broken
if not dataType in ["data", "mc", "afii"] :
    raise Exception ("invalid data type: " + dataType)

print("Running on data type: " + dataType)

if isPhyslite :
    inputfile = {"data": 'ASG_TEST_FILE_LITE_DATA',
                 "mc":   'ASG_TEST_FILE_LITE_MC',
                 "afii": 'ASG_TEST_FILE_LITE_MC_AFII'}
else :
    inputfile = {"data": 'ASG_TEST_FILE_DATA',
                 "mc":   'ASG_TEST_FILE_MC',
                 "afii": 'ASG_TEST_FILE_MC_AFII'}

# Set up the reading of the input file:
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
if athArgs.force_input :
    flags.Input.Files = []
    for file_idx in range(len(athArgs.force_input)):
        testFile = athArgs.force_input[file_idx]
        flags.Input.Files.append(testFile)
else:
    testFile = os.getenv ( inputfile[dataType] )
    flags.Input.Files = [testFile]

flags.lock()

from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AnalysisAlgorithmsConfig.FullCPAlgorithmsTest import makeSequence
cfg = MainServicesCfg(flags)
cfg.merge(PoolReadCfg(flags))

from EventBookkeeperTools.EventBookkeeperToolsConfig import CutFlowSvcCfg
cfg.merge(CutFlowSvcCfg(flags))

cp_cfg = makeSequence (dataType, blockConfig, textConfig, forCompare=forCompare,
                       noSystematics = athArgs.no_systematics,
                       isPhyslite=isPhyslite, noPhysliteBroken=noPhysliteBroken,
                       autoconfigFromFlags=flags, onlyNominalOR=athArgs.onlyNominalOR)
# Add all algorithms from the sequence to the job.
cfg.merge(cp_cfg)

# Set up a histogram output file for the job:
if blockConfig :
    outputFile = "ANALYSIS DATAFILE='FullCPAlgorithmsConfigTest." + dataType + ".hist.root' OPT='RECREATE'"
elif textConfig :
    outputFile = "ANALYSIS DATAFILE='FullCPAlgorithmsTextConfigTest." + dataType + ".hist.root' OPT='RECREATE'"
else :
    outputFile = "ANALYSIS DATAFILE='FullCPAlgorithmsTest." + dataType + ".hist.root' OPT='RECREATE'"
if athArgs.force_output :
    outputFile = "ANALYSIS DATAFILE='" + athArgs.force_output + "' OPT='RECREATE'"
from AthenaConfiguration.ComponentFactory import CompFactory
cfg.addService(CompFactory.THistSvc(Output=[ outputFile ]))

cfg.printConfig() # For debugging

cfg.run(500)
