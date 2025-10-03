# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import argparse
from AnaAlgorithm.Logging import logging
from abc import ABC, abstractmethod
import os

class CPBaseRunner(ABC):
    def __init__(self):
        self.logger = logging.getLogger("CPBaseRunner")
        self._args = None
        self._inputList = None
        self.parser = self._defaultParseArguments()
        # parse the arguments here is a bad idea

    @property
    def args(self):
        if self._args is None:
            self._args = self.parser.parse_args()
        return self._args

    @property
    def inputList(self):
        if self._inputList is None:
            if self.args.input_list.endswith('.txt'):
                self._inputList = CPBaseRunner._parseInputFileList(self.args.input_list)
            elif ".root" in self.args.input_list:
                self._inputList = [self.args.input_list]
            else:
                raise FileNotFoundError(f'Input file list \"{self.args.input_list}\" is not supported!'
                                        'Please provide a text file with a list of input files or a single root file.')
            self.logger.info("Initialized input files: %s", self._inputList)
        return self._inputList
    
    @property
    def outputName(self):
        if self.args.output_name.endswith('.root'):
            return self.args.output_name[:-5]
        else:
            return self.args.output_name

    def printFlags(self):
        self.logger.info("="*73)
        self.logger.info("="*20 + "FLAG CONFIGURATION" + "="*20)
        self.logger.info("="*73)
        self.logger.info("    Input files:     %s", self.flags.Input.isMC)
        self.logger.info("    RunNumber:       %s", self.flags.Input.RunNumbers)
        self.logger.info("    MCCampaign:      %s", self.flags.Input.MCCampaign)
        self.logger.info("    GeneratorInfo:   %s", self.flags.Input.GeneratorsInfo)
        self.logger.info("    MaxEvents:       %s", self.flags.Exec.MaxEvents)
        self.logger.info("    SkipEvents:      %s", self.flags.Exec.SkipEvents)
        self.logger.info("="*73)

    @abstractmethod
    def addCustomArguments(self):
        pass

    @abstractmethod
    def makeAlgSequence(self):
        pass

    @abstractmethod
    def run(self):
        pass

    # The responsiblity of flag.lock will pass to the caller
    def _defaultFlagsInitialization(self):
        from AthenaConfiguration.AllConfigFlags import initConfigFlags
        flags = initConfigFlags()
        flags.Input.Files = self.inputList
        flags.Exec.MaxEvents = self.args.max_events
        flags.Exec.SkipEvents = self.args.skip_n_events
        return flags

    def _defaultParseArguments(self):
        parser = argparse.ArgumentParser(
            description='Runscript for CP Algorithm unit tests')
        baseGroup = parser.add_argument_group('Base Script Options')
        baseGroup.add_argument('-i', '--input-list', dest='input_list',
                            help='path to text file containing list of input files, or a single root file')
        baseGroup.add_argument('-o','--output-name', dest='output_name', default='output',
                            help='output name of the analysis root file')
        baseGroup.add_argument('-e', '--max-events', dest='max_events', type=int, default=-1,
                            help='Number of events to run')
        baseGroup.add_argument('-t', '--text-config', dest='text_config',
                            help='path to the YAML configuration file. Tips: use atlas_install_data(path/to/*.yaml) in CMakeLists.txt can help locating the config just by the config file name.')
        baseGroup.add_argument('--no-systematics', dest='no_systematics',
                            action='store_true', help='Disable systematics')
        baseGroup.add_argument('--skip-n-events', dest='skip_n_events', type=int, default=0,
                            help='Skip the first N events in the run, not first N events for each file. This is meant for debugging only. \nIn Eventloop, this option disable the cutbookkeeper algorithms due to technical reasons, and can only be ran in direct-driver.')
        return parser

    def _readYamlConfig(self):
        from AthenaCommon.Utils.unixtools import find_datafile
        yamlconfig = find_datafile(self.args.text_config)
        if not yamlconfig:
            raise FileNotFoundError(f'Failed to locate \"{self.args.text_config}\" config file!'
                                    'Check if you have a typo in -t/--text-config argument or missing file in the analysis configuration sub-directory.')
        self.logger.info("Setting up configuration based on YAML config:")
        from AnalysisAlgorithmsConfig.ConfigText import TextConfig
        config = TextConfig(yamlconfig)
        return config

    def _parseInputFileList(path):
        files = []
        with open(path, 'r') as inputText:
            for line in inputText.readlines():
                # Strip the line and skip comments and empty lines
                line = line.strip()
                if line.startswith('#') or not line:
                    continue
                if os.path.isdir(line):
                    if not os.listdir(line):
                        raise FileNotFoundError(f"The directory \"{path}\" is empty. Please provide a directory with .root files.")
                    for root_file in os.listdir(line):
                        if '.root' in root_file:
                            files.append(os.path.join(line, root_file))
                else:
                    files += line.split(',')
            # Remove leading/trailing whitespaces from file names
            files = [file.strip() for file in files]
        return files

    def setup(self):
        self.parser.parse_args()
        self.config = self._readYamlConfig()
        self.flags = self._defaultFlagsInitialization()

    def printAvailableArguments(self):
        self.parser.description = 'CPRunScript available arguments'
        self.parser.usage = argparse.SUPPRESS
        self.parser.print_help()
