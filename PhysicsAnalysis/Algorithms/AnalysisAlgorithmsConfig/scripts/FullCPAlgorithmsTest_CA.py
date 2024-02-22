#! /bin/env python3
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# @author Tadej Novak
# @author Teng Jian Khoo

import os
from AnalysisAlgorithmsConfig.FullCPAlgorithmsTest import makeSequence
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from EventBookkeeperTools.EventBookkeeperToolsConfig import CutFlowSvcCfg

flags = initConfigFlags()
athArgsParser = flags.getArgumentParser()
athArgsParser.add_argument("--input-file", action="append", dest="input_file",
                           default=None,
                           help="Specify the input file")
athArgsParser.add_argument("--output-file", action="store", dest="output_file",
                           default=None,
                           help="Specify the output file")
athArgsParser.add_argument("--dump-config", action="store", dest="dump_config",
                           default=None,
                           help="Dump the config in a pickle file")
athArgsParser.add_argument("--data-type", action="store", dest="data_type",
                           default="data",
                           help="Type of input to run over. Valid options are 'data', 'mc', 'afii'")
athArgsParser.add_argument('--block-config', dest='block_config',
                           action='store_true', default=False,
                           help='Configure the job with block configuration')
athArgsParser.add_argument('--text-config', dest='text_config',
                           action='store', default='',
                           help='Configure the job with the provided text configuration')
athArgsParser.add_argument('--for-compare', dest='for_compare',
                           action='store_true', default=False,
                           help='Configure the job for comparison of sequences vs blocks')
athArgsParser.add_argument('--no-systematics', dest='no_systematics',
                           action='store_true', default=False,
                           help='Configure the job to with no systematics')
athArgsParser.add_argument('--physlite', dest='physlite',
                           action='store_true', default=False,
                           help='Configure the job for physlite')
athArgsParser.add_argument('--only-nominal-or', dest='onlyNominalOR',
                           action='store_true', default=False,
                           help='Only run overlap removal for nominal (skip systematics)')
athArgs = flags.fillFromArgs(parser=athArgsParser)

dataType = athArgs.data_type
blockConfig = athArgs.block_config
textConfig = athArgs.text_config
forCompare = athArgs.for_compare
isPhyslite = athArgs.physlite

if dataType not in ["data", "mc", "afii"]:
    raise Exception("invalid data type: " + dataType)

print("Running on data type: " + dataType)

if isPhyslite:
    inputfile = {"data": 'ASG_TEST_FILE_LITE_DATA',
                 "mc":   'ASG_TEST_FILE_LITE_MC',
                 "afii": 'ASG_TEST_FILE_LITE_MC_AFII'}
else:
    inputfile = {"data": 'ASG_TEST_FILE_DATA',
                 "mc":   'ASG_TEST_FILE_MC',
                 "afii": 'ASG_TEST_FILE_MC_AFII'}

# Set up the reading of the input file:
if athArgs.input_file:
    flags.Input.Files = athArgs.input_file[:]
else:
    testFile = os.getenv(inputfile[dataType])
    flags.Input.Files = [testFile]

flags.Exec.FPE = 500
flags.lock()

# Setup main services
cfg = MainServicesCfg(flags)
# Setup POOL reading
cfg.merge(PoolReadCfg(flags))
# Setup cutflow service
cfg.merge(CutFlowSvcCfg(flags))

# Setup the configuration
cp_cfg = makeSequence(dataType, blockConfig, textConfig, forCompare=forCompare,
                      noSystematics=athArgs.no_systematics,
                      isPhyslite=isPhyslite,
                      autoconfigFromFlags=flags, onlyNominalOR=athArgs.onlyNominalOR,
                      forceEGammaFullSimConfig=True)
# Add all algorithms from the sequence to the job.
cfg.merge(cp_cfg)

# Set up a histogram output file for the job:
if blockConfig:
    outputFile = "ANALYSIS DATAFILE='FullCPAlgorithmsConfigTest." + \
        dataType + ".hist.root' OPT='RECREATE'"
elif textConfig:
    outputFile = "ANALYSIS DATAFILE='FullCPAlgorithmsTextConfigTest." + \
        dataType + ".hist.root' OPT='RECREATE'"
else:
    outputFile = "ANALYSIS DATAFILE='FullCPAlgorithmsTest." + \
        dataType + ".hist.root' OPT='RECREATE'"
if athArgs.output_file:
    outputFile = "ANALYSIS DATAFILE='" + athArgs.output_file + "' OPT='RECREATE'"
cfg.addService(CompFactory.THistSvc(Output=[outputFile]))

# Set EventPrintoutInterval to 100 events
cfg.getService(cfg.getAppProps()['EventLoop']).EventPrintoutInterval = 100

cfg.printConfig()  # For debugging

# dump pickle
if athArgs.dump_config:
    with open(athArgs.dump_config, "wb") as f:
        cfg.store(f)

sc = cfg.run(500)
import sys
sys.exit(sc.isFailure())
