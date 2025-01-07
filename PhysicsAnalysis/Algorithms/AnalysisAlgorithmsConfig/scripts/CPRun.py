#! /usr/bin/env python

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
import argparse
import logging
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AnalysisAlgorithmsConfig.ConfigText import TextConfig
import ROOT
from ROOT import PathResolver
from AnaAlgorithm.AlgSequence import AlgSequence
from AnalysisAlgorithmsConfig.ConfigAccumulator import ConfigAccumulator

logger = logging.getLogger(__name__)

def parseArguments():
    parser = argparse.ArgumentParser(
        description='Runscript for CP Algorithm unit tests')
    parser.add_argument('--input-list', dest='input_list',
                        help='path to text file containing list of input files')
    parser.add_argument('--work-dir', dest='work_dir', default='workDir',
                        help='path to work directory, containing output and intermediate files')
    parser.add_argument('-e', '--max-events', dest='max_events',
                        type=int, default=-1, help='Number of events to run')
    parser.add_argument('-t', '--text-config', dest='text_config',
                        help='path to the YAML configuration file')
    parser.add_argument('--no-systematics', dest='no_systematics',
                        action='store_true', help='Disable systematics')
    parser.add_argument('--direct-driver', dest='direct_driver',
                        action='store_true', help='Run the job with the direct driver')
    parser.add_argument('--input-file', dest='input_file',
                        help='path to a single input file')
    args = parser.parse_args()
    return args

def parseInputFileList(path):
    files = []
    with open(path, 'r') as inputText:
        for line in inputText.readlines():
            # skip comments and empty lines
            if line.startswith('#') or not line.strip():
                continue
            files += line.split(',')
        # remove leading/trailing whitespaces, and \n
        files = [file.strip() for file in files]
    return files

def makeJob(sampleHandler, args):
    job = ROOT.EL.Job()
    job.sampleHandler(sampleHandler)    
    job.options().setDouble(ROOT.EL.Job.optMaxEvents, args.max_events)
    job.options().setString(ROOT.EL.Job.optSubmitDirMode, 'unique-link')
    return job

# copied from makeTextAlgSequence


def readYamlConfig(yaml_path):
    yamlconfig = PathResolver.find_file(
        yaml_path, "CALIBPATH", PathResolver.RecursiveSearch)
    if not yamlconfig:
        raise FileNotFoundError(f'PathResolver failed to locate \"{yaml_path}\" config file!'
                                'Check if you have a typo in -t/--text-config argument or missing file in the analysis configuration sub-directory.')
    logger.info("Setting up configuration based on YAML config:")
    config = TextConfig(yamlconfig)
    return config


def makeAlgSequence(config, args, flags):
    algSeq = AlgSequence()
    logger.info("Configuring algorithms based on YAML file")
    configSeq = config.configure()
    logger.info("Configuring common services")
    configAccumulator = ConfigAccumulator(autoconfigFromFlags=flags,
                                          algSeq=algSeq,
                                          noSystematics=args.no_systematics)
    logger.info("Configuring algorithms")
    configSeq.fullConfigure(configAccumulator)
    return algSeq

#copy from TopCP toolkit
def prettyPrint(flags):
    """
    Print all the relevant flags we have set up, both from the
    metadata and from our fall-back options.
    """
    logger.info("="*73)
    logger.info("="*20 + "FLAG CONFIGURATION" + "="*20)
    logger.info("="*73)
    logger.info("   Input files:     %s", flags.Input.isMC)
    logger.info("   RunNumber:       %s", flags.Input.RunNumbers)
    logger.info("   MCCampaign:      %s", flags.Input.MCCampaign)
    logger.info("   GeneratorInfo:   %s", flags.Input.GeneratorsInfo)
    logger.info("="*73)

def main():
    args = parseArguments()
    inputList = parseInputFileList(args.input_list) if args.input_list else [args.input_file]

    ROOT.xAOD.Init().ignore()
    sampleHandler = ROOT.SH.SampleHandler()

    sampleFiles = ROOT.SH.SampleLocal()
    logger.info("Adding files to the sample handler")
    for file in inputList:
        sampleFiles.add(file)
    sampleHandler.add(sampleFiles)

    logger.info("Reading file metadata")
    flags = initConfigFlags()
    flags.Input.Files = inputList
    flags.lock()
    prettyPrint(flags)

    config = readYamlConfig(args.text_config)
    algSeq = makeAlgSequence(config, args, flags)
    logger.info("Alg Sequence: %s", algSeq)

    job = makeJob(sampleHandler, args)
    for alg in algSeq:
        job.algsAdd(alg)
    job.outputAdd(ROOT.EL.OutputStream('ANALYSIS'))

    driver = ROOT.EL.DirectDriver() if args.direct_driver else ROOT.EL.ExecDriver()
    driver.submit(job, args.work_dir)

    return
# test


if __name__ == '__main__':
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
        datefmt="%Y-%m-%d %H:%M:%S"
    )
    main()
