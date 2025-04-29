#! /bin/env python3
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# @author Tadej Novak
# @author Teng Jian Khoo

import sys
import os
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
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
                           help="Type of input to run over. Valid options are 'data', 'fullsim', 'fastsim'")
athArgsParser.add_argument('--text-config', dest='text_config',
                           action='store', default=None,
                           help='Configure the job with the provided text configuration')
athArgsParser.add_argument('--no-systematics', dest='no_systematics',
                           action='store_true', default=False,
                           help='Configure the job to with no systematics')
athArgsParser.add_argument('--physlite', dest='physlite',
                           action='store_true', default=False,
                           help='Run the job on physlite')
athArgsParser.add_argument('--run', action='store', dest='run',
                           default=2, type=int,
                           help='Run number for the inputs')
athArgsParser.add_argument('--only-nominal-or', dest='onlyNominalOR',
                           action='store_true', default=False,
                           help='Only run overlap removal for nominal (skip systematics)')
athArgsParser.add_argument('--bleeding-edge', dest='bleeding_edge',
                           action='store_true', default=False,
                           help='Run on the latest bleeding edge input (usually from the CI output)')
athArgs = flags.fillFromArgs(parser=athArgsParser)

dataType = DataType(athArgs.data_type)
textConfig = athArgs.text_config

if textConfig:
    from PathResolver import PathResolver
    textConfig = PathResolver.FindCalibFile(textConfig)

print(f"Running on data type: {dataType.value}")

if athArgs.physlite:
    if athArgs.run==3:
        inputfile = {DataType.Data:    'ASG_TEST_FILE_LITE_RUN3_DATA',
                     DataType.FullSim: 'ASG_TEST_FILE_LITE_RUN3_MC',
                     DataType.FastSim: 'ASG_TEST_FILE_LITE_RUN3_MC_FASTSIM'}
    elif athArgs.run==2:
        inputfile = {DataType.Data:    'ASG_TEST_FILE_LITE_DATA',
                     DataType.FullSim: 'ASG_TEST_FILE_LITE_MC',
                     DataType.FastSim: 'ASG_TEST_FILE_LITE_MC_FASTSIM'}
else:
    if athArgs.run==3:
        inputfile = {DataType.Data:    'ASG_TEST_FILE_RUN3_DATA',
                     DataType.FullSim: 'ASG_TEST_FILE_RUN3_MC',
                     DataType.FastSim: 'ASG_TEST_FILE_RUN3_MC_FASTSIM'}
    elif athArgs.run==2:
        inputfile = {DataType.Data:    'ASG_TEST_FILE_DATA',
                     DataType.FullSim: 'ASG_TEST_FILE_MC',
                     DataType.FastSim: 'ASG_TEST_FILE_MC_FASTSIM'}

# Set up the reading of the input file:
if athArgs.input_file:
    flags.Input.Files = athArgs.input_file[:]
else:
    testFile = os.getenv(inputfile[dataType])
    flags.Input.Files = [testFile]

flags.Exec.FPE = 500
flags.Exec.EventPrintoutInterval = 100
if flags.PerfMon.doFullMonMT or flags.PerfMon.doFastMonMT:
        flags.PerfMon.OutputJSON="perfmonmt_CPAnalysis.json"
flags.lock()

# Setup main services
cfg = MainServicesCfg(flags)
# Setup POOL reading
cfg.merge(PoolReadCfg(flags))
# Setup cutflow service
cfg.merge(CutFlowSvcCfg(flags))

# Setup the configuration
cp_cfg = makeSequence(dataType, yamlPath=textConfig,
                      noSystematics=athArgs.no_systematics,
                      isPhyslite=athArgs.physlite,
                      autoconfigFromFlags=flags, onlyNominalOR=athArgs.onlyNominalOR,
                      forceEGammaFullSimConfig=True,
                      bleedingEdge=athArgs.bleeding_edge)
# Add all algorithms from the sequence to the job.
cfg.merge(cp_cfg)

# Set up a histogram output file for the job:
if textConfig:
    outputFile = f"ANALYSIS DATAFILE='FullCPAlgorithmsTextConfigTest.{dataType.value}.hist.root' OPT='RECREATE'"
else:
    outputFile = f"ANALYSIS DATAFILE='FullCPAlgorithmsConfigTest.{dataType.value}.hist.root' OPT='RECREATE'"
if athArgs.output_file:
    outputFile = f"ANALYSIS DATAFILE='{athArgs.output_file}' OPT='RECREATE'"
cfg.addService(CompFactory.THistSvc(Output=[outputFile]))

cfg.printConfig()  # For debugging

# dump pickle
if athArgs.dump_config:
    with open(athArgs.dump_config, "wb") as f:
        cfg.store(f)
if flags.Exec.MaxEvents>0:
  sc=cfg.run(flags.Exec.MaxEvents)
else:
  sc = cfg.run(500)
sys.exit(sc.isFailure())
