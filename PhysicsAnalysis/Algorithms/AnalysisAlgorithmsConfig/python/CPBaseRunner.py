# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import argparse
import logging
from ROOT import PathResolver
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AnalysisAlgorithmsConfig.ConfigText import TextConfig

from abc import ABC, abstractmethod

class CPBaseRunner(ABC):
    def __init__(self):
        self.logger = logging.getLogger("CPBaseRunner")
        self._inputList = None
        self.parser = self._defaultParseArguments()
        self.config = self._readYamlConfig()
        self.flags = self._defaultFlagsInitialization()

    @property
    def args(self):
        return self.parser.parse_args()
    
    @property
    def inputList(self):
        if self.args.input_list and self.args.input_file:
            raise ValueError('use either --input-list or --input-file, not both')
        if self._inputList is None:
            if self.args.input_list:
                self._inputList = self._parseInputFileList(self.args.input_list)
            elif self.args.input_file:
                self._inputList = [self.args.input_file]
            else:
                raise ValueError('use --input-list or --input-file to specify input files')
        return self._inputList
    
    def printFlags(self):
        self.logger.info("="*73)
        self.logger.info("="*20 + "FLAG CONFIGURATION" + "="*20)
        self.logger.info("="*73)
        self.logger.info("    Input files:     %s", self.flags.Input.isMC)
        self.logger.info("    RunNumber:       %s", self.flags.Input.RunNumbers)
        self.logger.info("    MCCampaign:      %s", self.flags.Input.MCCampaign)
        self.logger.info("    GeneratorInfo:   %s", self.flags.Input.GeneratorsInfo)
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
        flags = initConfigFlags()
        flags.Input.Files = self.inputList
        flags.Exec.MaxEvents = self.args.max_events
        return flags
        
    def _defaultParseArguments(self):
        parser = argparse.ArgumentParser(
            description='Runscript for CP Algorithm unit tests')
        parser.add_argument('--input-list', dest='input_list',
                            help='path to text file containing list of input files')
        parser.add_argument('--work-dir', dest='work_dir', default='workDir',
                            help='path to work directory, containing output and intermediate files')
        parser.add_argument('-e', '--max-events', dest='max_events', type=int, default=-1,
                            help='Number of events to run')
        parser.add_argument('-t', '--text-config', dest='text_config',
                            help='path to the YAML configuration file')
        parser.add_argument('--no-systematics', dest='no_systematics',
                            action='store_true', help='Disable systematics')
        parser.add_argument('--input-file', dest='input_file',
                            help='path to a single input file')
        return parser
    
    def _readYamlConfig(self):
        yamlconfig = PathResolver.find_file(
            self.args.text_config, "CALIBPATH", PathResolver.RecursiveSearch)
        if not yamlconfig:
            raise FileNotFoundError(f'PathResolver failed to locate \"{self.args.text_config}\" config file!'
                                    'Check if you have a typo in -t/--text-config argument or missing file in the analysis configuration sub-directory.')
        self.logger.info("Setting up configuration based on YAML config:")
        config = TextConfig(yamlconfig)
        return config
    
    def _parseInputFileList(path):
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