#!/usr/bin/env python
#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

# Read the submission directory as a command line argument. You can
# extend the list of arguments with your private ones later on.
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
import json
import optparse
parser = optparse.OptionParser()
parser.add_option( '-d', '--data-type', dest = 'data_type',
                   action = 'store', type = 'string', default = 'data',
                   help="Type of input to run over. Valid options are 'data', 'fullsim', 'fastsim'")
parser.add_option( '-s', '--submission-dir', dest = 'submission_dir',
                   action = 'store', type = 'string', default = 'submitDir',
                   help = 'Submission directory for EventLoop' )
parser.add_option( "--input-file", action = "append", dest = "input_file",
                   default = None,
                   help = "Specify the input file")
parser.add_option( '-u', '--unit-test', dest='unit_test',
                   action = 'store_true', default = False,
                   help = 'Run the job in "unit test mode"' )
parser.add_option( '--direct-driver', dest='direct_driver',
                   action = 'store_true', default = False,
                   help = 'Run the job with the direct driver' )
parser.add_option( '--exec-driver', dest='exec_driver',
                   action = 'store_true', default = False,
                   help = 'Run the job with the exec driver' )
parser.add_option( '--max-events', dest = 'max_events',
                   action = 'store', type = 'int', default = 500,
                   help = 'Number of events to run' )
parser.add_option( '--algorithm-timer', dest='algorithm_timer',
                   action = 'store_true', default = False,
                   help = 'Run the job with a timer for each algorithm' )
parser.add_option( '--algorithm-memory', dest='algorithm_memory',
                   action = 'store_true', default = False,
                   help = 'Run the job with a memory monitor for each algorithm' )
parser.add_option( '--factory-preload', dest='factory_preload',
                   action = 'store', type = 'str', default = '',
                   help = 'Factory preloader(s) to run at the beginning of the job' )
parser.add_option( '--no-systematics', dest='no_systematics',
                   action = 'store_true', default = False,
                   help = 'Configure the job to with no systematics' )
parser.add_option( '--block-config', dest='block_config',
                   action = 'store_true', default = False,
                   help = 'Configure the job with block configuration' )
parser.add_option( '--text-config', dest='text_config',
                   action = 'store', default = '',
                   help = 'Configure the job with the provided text configuration' )
parser.add_option( '--physlite', dest='physlite',
                   action = 'store_true', default = False,
                   help = 'Configure the job for physlite' )
parser.add_option('--run', action='store', dest='run',
                    default=2, type=int,
                    help='Run number for the inputs')
parser.add_option( '--force-mc', dest='forceMC',
                   action = 'store_true', default = False,
                   help = 'Force the job to treat input as MC' )
parser.add_option( '--only-nominal-or', dest='onlyNominalOR',
                   action = 'store_true', default = False,
                   help = 'Only run overlap removal for nominal (skip systematics)')
parser.add_option('--seq-output-file', dest='seq_out_filename',
                   action='store',type='str',default='',
                   help = 'Save the sequence configuration output to the provided file')
( options, args ) = parser.parse_args()

# Set up (Py)ROOT.
import ROOT
ROOT.xAOD.Init().ignore()

# Force-load some xAOD dictionaries. To avoid issues from ROOT-10940.
ROOT.xAOD.TauJetContainer()

# ideally we'd run over all of them, but we don't have a mechanism to
# configure per-sample right now

dataType = DataType(options.data_type)
blockConfig = options.block_config
textConfig = options.text_config

if textConfig:
    from PathResolver import PathResolver
    textConfig = PathResolver.FindCalibFile(textConfig)

print(f"Running on data type: {dataType.value}")

if options.physlite:
    if options.run==3:
        inputfile = {DataType.Data:    'ASG_TEST_FILE_LITE_RUN3_DATA',
                     DataType.FullSim: 'ASG_TEST_FILE_LITE_RUN3_MC',
                     DataType.FastSim: 'ASG_TEST_FILE_LITE_RUN3_MC_FASTSIM'}
    elif options.run==2:
        inputfile = {DataType.Data:    'ASG_TEST_FILE_LITE_DATA',
                     DataType.FullSim: 'ASG_TEST_FILE_LITE_MC',
                     DataType.FastSim: 'ASG_TEST_FILE_LITE_MC_FASTSIM'}
else:
    if options.run==3:
        inputfile = {DataType.Data:    'ASG_TEST_FILE_RUN3_DATA',
                     DataType.FullSim: 'ASG_TEST_FILE_RUN3_MC',
                     DataType.FastSim: 'ASG_TEST_FILE_RUN3_MC_FASTSIM'}
    elif options.run==2:
        inputfile = {DataType.Data:    'ASG_TEST_FILE_DATA',
                     DataType.FullSim: 'ASG_TEST_FILE_MC',
                     DataType.FastSim: 'ASG_TEST_FILE_MC_FASTSIM'}

# No R24 FastSim recommendations for EGamma yet
forceEGammaFullSimConfig = True

# Set up the sample handler object. See comments from the C++ macro
# for the details about these lines.
import os
sh = ROOT.SH.SampleHandler()
sh.setMetaString( 'nc_tree', 'CollectionTree' )
sample = ROOT.SH.SampleLocal (dataType.value)
if options.input_file:
    for file_idx in range(len(options.input_file)):
        testFile = options.input_file[file_idx]
        sample.add (testFile)
else:
    testFile = os.getenv (inputfile[dataType])
    sample.add(testFile)
sh.add (sample)
sh.printContent()

from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()
if options.input_file:
    flags.Input.Files = options.input_file[:]
else:
    flags.Input.Files = [testFile]
if options.forceMC :
    flags.Input.isMC = True
flags.lock()

# Create an EventLoop job.
job = ROOT.EL.Job()
job.sampleHandler( sh )
if options.max_events > 0:
    job.options().setDouble( ROOT.EL.Job.optMaxEvents, options.max_events )
if options.algorithm_timer :
    job.options().setBool( ROOT.EL.Job.optAlgorithmTimer, True )
if options.algorithm_memory :
    job.options().setBool( ROOT.EL.Job.optAlgorithmMemoryMonitor, True )
if options.factory_preload != '' :
    job.options().setString( ROOT.EL.Job.optFactoryPreload, options.factory_preload )


from AnalysisAlgorithmsConfig.FullCPAlgorithmsTest import makeSequence, printSequenceAlgs
algSeq = makeSequence (dataType, yamlPath=textConfig,
                       noSystematics = options.no_systematics,
                       isPhyslite=options.physlite,
                       autoconfigFromFlags=flags, onlyNominalOR=options.onlyNominalOR,
                       forceEGammaFullSimConfig=forceEGammaFullSimConfig)

if options.seq_out_filename:
    from AnalysisAlgorithmsConfig.SaveConfigUtils import save_algs_from_sequence_ELjob
    with(open(options.seq_out_filename, 'w', encoding='utf-8')) as seq_out_file:
        output_dict = {}
        save_algs_from_sequence_ELjob(algSeq, output_dict) # For debugging
        json.dump(output_dict, seq_out_file, ensure_ascii=False, indent=4)
else:
    printSequenceAlgs( algSeq ) # For debugging
    
algSeq.addSelfToJob( job )

# Make sure that both the ntuple and the xAOD dumper have a stream to write to.
job.outputAdd( ROOT.EL.OutputStream( 'ANALYSIS' ) )

# Find the right output directory:
submitDir = options.submission_dir
if options.unit_test:
    job.options().setString (ROOT.EL.Job.optSubmitDirMode, 'unique')
else :
    job.options().setString (ROOT.EL.Job.optSubmitDirMode, 'unique-link')


# Run the job using the local driver.  This is intentionally the local
# driver, unlike most other tests that use the direct driver.  That
# way it tests whether the code works correctly with that driver,
# which is a lot more similar to the way the batch/grid drivers work.
driver = ROOT.EL.LocalDriver()

if options.direct_driver :
    # this is for testing purposes, as only the direct driver respects
    # the limit on the number of events.
    driver = ROOT.EL.DirectDriver()
if options.exec_driver :
    # this is for limiting the memory usage, as it will end the current process
    # releasing all memory before starting the event loop
    driver = ROOT.EL.ExecDriver()

print ("submitting job now", flush=True)
driver.submit( job, submitDir )

if options.seq_out_filename:
    from AnalysisAlgorithmsConfig.SaveConfigUtils import combine_tools_and_algorithms_ELjob
    _ = combine_tools_and_algorithms_ELjob(True, text_file = 'tool_config.txt',
                                           alg_file = options.seq_out_filename,
                                           output_file = 'my_analysis_config.json')

