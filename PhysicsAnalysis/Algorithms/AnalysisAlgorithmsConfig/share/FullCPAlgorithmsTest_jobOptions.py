# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

# User options, which can be set from command line after a "-" character
# athena FullCPAlgorithmsTest_jobOptions.py - --myOption ...
from AthenaCommon.AthArgumentParser import AthArgumentParser
athArgsParser = AthArgumentParser()
athArgsParser.add_argument("--force-input", action = "store", dest = "force_input",
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
athArgs = athArgsParser.parse_args()

dataType = athArgs.data_type
blockConfig = athArgs.block_config
textConfig = athArgs.text_config
forCompare = athArgs.for_compare
isPhyslite = athArgs.physlite
noPhysliteBroken = athArgs.no_physlite_broken
if not dataType in ["data", "mc", "afii"] :
    raise Exception ("invalid data type: " + dataType)

print("Running on data type: " + dataType)

# No R24 FastSim recommendations for EGamma yet
forceEGammaFullSimConfig = True

if isPhyslite :
    inputfile = {"data": 'ASG_TEST_FILE_LITE_DATA',
                 "mc":   'ASG_TEST_FILE_LITE_MC',
                 "afii": 'ASG_TEST_FILE_LITE_MC_AFII'}
else :
    inputfile = {"data": 'ASG_TEST_FILE_DATA',
                 "mc":   'ASG_TEST_FILE_MC',
                 "afii": 'ASG_TEST_FILE_MC_AFII'}

# Set up the reading of the input file:
import AthenaPoolCnvSvc.ReadAthenaPool
theApp.EvtMax = 500
testFile = os.getenv ( inputfile[dataType] )
if athArgs.force_input :
    testFile = athArgs.force_input
svcMgr.EventSelector.InputCollections = [testFile]

from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()
flags.Input.Files = [testFile]
flags.lock()


from AnalysisAlgorithmsConfig.FullCPAlgorithmsTest import makeSequence
algSeq = makeSequence (dataType, blockConfig, textConfig, forCompare=forCompare,
                       noSystematics = athArgs.no_systematics,
                       isPhyslite=isPhyslite, noPhysliteBroken=noPhysliteBroken,
                       autoconfigFromFlags=flags, onlyNominalOR=athArgs.onlyNominalOR,
                       forceEGammaFullSimConfig=forceEGammaFullSimConfig)

# Need to explicitly instantiate the CutFlowSvc in Athena to allow
# the event filters to run.  In AnalysisBase that is (currently)
# not necessary, but we may need a CutFlowSvc instance to report
# cutflows to the user.
from EventBookkeeperTools.CutFlowHelpers import CreateCutFlowSvc
CreateCutFlowSvc( seq=algSeq )

print (algSeq) # For debugging

# Add all algorithms from the sequence to the job.
athAlgSeq += algSeq

# Set up a histogram output file for the job:
ServiceMgr += CfgMgr.THistSvc()
if not blockConfig :
    outputFile = "ANALYSIS DATAFILE='FullCPAlgorithmsTest." + dataType + ".hist.root' OPT='RECREATE'"
else :
    outputFile = "ANALYSIS DATAFILE='FullCPAlgorithmsConfigTest." + dataType + ".hist.root' OPT='RECREATE'"
if athArgs.force_output :
    outputFile = "ANALYSIS DATAFILE='" + athArgs.force_output + "' OPT='RECREATE'"
ServiceMgr.THistSvc.Output += [ outputFile ]

# Reduce the printout from Athena:
include( "AthAnalysisBaseComps/SuppressLogging.py" )
