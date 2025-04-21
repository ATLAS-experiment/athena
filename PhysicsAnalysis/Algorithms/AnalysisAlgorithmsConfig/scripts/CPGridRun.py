#! /usr/bin/env python

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AnaAlgorithm.DualUseConfig import isAthena
from AnaAlgorithm.Logging import logging
import argparse
import sys 
import os

logCPGridRun = logging.getLogger('CPGridRun')
class CPGridRun:
    def __init__(self): 
        self.gridParser = self._parseGridArguments()
        if self.args.help:
            self._runscript = self._initRunscript()
            self.printHelp()
            sys.exit(0)
        self._tarfile = 'cpgrid.tar.gz'
        self._clean_tarball()
        self._inputList = None # list of name?
        self.cmd = {} # sample name -> command
        
    def _initRunscript(self):
        if isAthena:
            from AnalysisAlgorithmsConfig.AthenaCPRunScript import AthenaCPRunScript
            runscript = AthenaCPRunScript()
        else:
            from AnalysisAlgorithmsConfig.EventLoopCPRunScript import EventLoopCPRunScript
            runscript = EventLoopCPRunScript()
        return runscript
    
    def _parseGridArguments(self):
        parser = argparse.ArgumentParser(description='CPGrid runscript to submit CPRun.py jobs to the grid. '
                                         'This script will submit a job to the grid using files in the input text one by one.'
                                         'CPRun.py can handle multiple sources of input and create one output; but not this script', 
                                         add_help=False,
                                         formatter_class=argparse.RawTextHelpFormatter)
        parser.add_argument('-h', '--help', dest='help', action='store_true', help='Show this help message and continue')
        
        ioGroup = parser.add_argument_group('Input/Output file configuration')
        ioGroup.add_argument('--input-list', dest='input_list', help='Path to the text file containing list of containers on the panda grid. Each container will be passed to prun as --inDS and is run individually')
        ioGroup.add_argument('--output-files', dest='output_files', default='output.root',
                             help='The output files of the grid job. Example: --output-files "A.root,B.txt,B.root" results in A/A.root, B/B.txt, B/B.root in the output directory. No need to specify if using CPRun.py')
        ioGroup.add_argument('--destSE', dest='destSE', default='', type=str, help='Destination storage element (PanDA)')
        ioGroup.add_argument('--mergeType', dest='mergeType', default='Default', type=str, help='Output merging type, [None, Default, xAOD]')
        
        pandaGroup = parser.add_argument_group('Input/Output naming configuration')
        pandaGroup.add_argument('--gridUsername', dest='gridUsername', default=os.getenv('USER', ''), type=str, help='Grid username, or the groupname. Default is the current user. Only affect file naming')
        pandaGroup.add_argument('--prefix', dest='prefix', default='', type=str, help='Prefix for the output directory. Dynamically set with input container if not provided')
        pandaGroup.add_argument('--suffix', dest='suffix', default='',type=str, help='Suffix for the output directory')
        pandaGroup.add_argument('--outDS', dest='outDS', default='', type=str, 
                                help='Name of an output dataset. outDS will contain all output files (PanDA). If not provided, support dynamic naming if input name is in the Atlas production format or typical user production format')
        
        cpgridGroup = parser.add_argument_group('CPGrid configuration')
        cpgridGroup.add_argument('--groupProduction', dest='groupProduction', action='store_true', help='Only use for official production')
        
        cpgridGroup.add_argument('--exec', dest='exec', type=str, 
                                    help='Executable line for the CPRun.py or custom script to run on the grid encapsulated in a double quote (PanDA)\n'
                                    'Run CPRun.py with preset behavior including streamlined file i/o. E.g, "-t config.yaml --no-systematics".\n'
                                    'CPRun.py but overriding the preset behavior （for future and experts): "CPRun.py --input-list ourExpert.txt -t config.yaml --flagB"\n'
                                    'Run custom script: "customRun.py -i inputs -o output --text-config config.yaml --flagA --flagB"\n'
                                    ) 
    
        submissionGroup = parser.add_argument_group('Submission configuration')
        submissionGroup.add_argument('--noSubmit', dest='noSubmit', action='store_true', help='Do not submit the job to the grid (PanDA). Useful to inspect the prun command')
        submissionGroup.add_argument('--testRun', dest='testRun', action='store_true', help='Will submit job to the grid but greatly limit the number of files per job (10) and number of events (300)')
        submissionGroup.add_argument('--recreateTar', dest='recreate', action='store_true', help='Re-compress the source code. Source code are compressed by default in submission, this is useful when the source code is updated')
        
        self.args = parser.parse_args()
        return parser
    
    @property
    def inputList(self):
        if self._inputList is None:
            if self.args.input_list:
                self._inputList = CPGridRun._parseInputFileList(self.args.input_list)
            else:
                raise ValueError(
                    'use --input-list to specify input containers')
        return self._inputList
    
    def printHelp(self):
        self.gridParser.print_help()
        print("\033[92m\n If you are using CPRun.py, the following flags are for the CPRun.py in this framework\033[0m")
        self._runscript.parser.usage = argparse.SUPPRESS
        self._runscript.parser.print_help()
      
    def getParser(self):
        return self.gridParser
    
    # This function do all the checking, cleaning and preparing the command to be submitted to the grid
    # separated for client to be able to change the behavior
    def configureSumbission(self):
        #check for prun?
        #check for Merge type?
        self._clean_tarball()
        for input in self.inputList:            
            cmd = self.configureSubmissionSingleSample(input)
            self.cmd[input] = cmd
    
    def configureSubmissionSingleSample(self, input):
        config = {
            'inDS': input,
            'outDS': self.args.outDS if self.args.outDS else self.outputDSFormatter(input) ,
            # 'outDS': self.outputDSFormatter(input) if CPGridRun.isAtlasProductionFormat(input) else self.customOutputDSFormatter(input),
            'useAthenaPackages': True,
            'cmtConfig': os.environ["CMTCONFIG"],
            'writeInputToTxt': 'IN:in.txt',
            'outputs': self.outputsFormatter(),
            'exec': self.execFormatter(),
            'memory': "2000", # MB
            'addNthFieldOfInDSToLFN': '2,3,6',
        }
        if self.args.noSubmit:
            config['noSubmit'] = True
            
        if self.args.mergeType == 'xAOD':
            config['mergeScript'] = 'xAODMerge %OUT `echo %IN | sed \'s/,/ /g\'`'
            
        if self.args.mergeType != 'None':
            config['mergeOutput'] = True
            
        if self.args.recreate or not os.path.exists(self._tarfile):
            config['outTarBall'] = self._tarfile
        elif os.path.exists(self._tarfile):
            config['inTarBall'] = self._tarfile
            
        if self.args.groupProduction:
            config['official'] = True
            config['voms'] = f'atlas:/atlas/{self.args.gridUsername}/Role=production'
        
        if self.args.destSE:
            config['destSE'] = self.args.destSE
            
        if self.args.testRun:
            config['nEventsPerFile'] = 300
            config['nFiles'] = 10
            
        cmd = 'prun \\\n'
        for k, v in config.items():
            if isinstance(v, bool) and v:
                cmd += f'--{k} \\\n'
            elif v is not None and v != '':
                cmd += f'--{k} {v} \\\n'
        return cmd.rstrip(' \\\n')
    
    def printInputDetails(self):
        for key, cmd in self.cmd.items():
            parsed_name = CPGridRun.atlasProductionNameParser(key)
            print(f"Input: {key}")
            for k, v in parsed_name.items():
                print(f"  {k.replace('_', ' ').title()}: {v}")
            print(f"Command: \n{cmd}")
            print("-" * 40)
        # Add your submission logic here

    def outputDSFormatter(self, name):
        if CPGridRun.isAtlasProductionFormat(name):
            return self._outputDSFormatter(name)
        else:
            return self._customOutputDSFormatter(name)

    def _outputDSFormatter(self, name): 
        '''
        {group/user}.{username}.{prefix}.{DSID}.{format}.{tags}.{suffix}
        '''
        nameParser = CPGridRun.atlasProductionNameParser(name)
        base = 'group' if self.args.groupProduction else 'user'
        username = self.args.gridUsername
        dsid = nameParser['DSID']
        tags = '_'.join(nameParser['tags'])
        fileFormat = nameParser['format']
        base = 'group' if self.args.groupProduction else 'user'
        prefix = self.args.prefix if self.args.prefix else nameParser['main'].split('_')[0] # Dynamically set the prefix, likely to be something like PhPy8Eg
        suffix = self._suffixFormatter()
        
        result = [base, username, prefix, dsid, fileFormat, tags, suffix]
        return ".".join(filter(None, result))
    
    def _customOutputDSFormatter(self, name):
        '''
        {group/user}.{username}.{main}.outputDS.{suffix}
        '''
        parts = name.split('.')
        base = 'group' if self.args.groupProduction else 'user'
        username = self.args.gridUsername
        main = parts[2]
        outputDS = 'outputDS'
        suffix = parts[-1]
        
        result  = [base, username,main, outputDS, suffix]
        return ".".join(filter(None, result))


    def _suffixFormatter(self):
        if self.args.suffix:
            return self.args.suffix
        if self.args.testRun:
            import uuid
            return f"test_{uuid.uuid4().hex[:6]}"
        else:
            ''
        
    
    
    def _clean_tarball(self):
        """
        Remove existing tarball if recreation is requested.
        """
        if not self.args.recreate:
            return
        try:
            os.remove(self._tarfile)
        except FileNotFoundError:
            pass  # File doesn't exist, nothing to do
        
    def execFormatter(self):
        # Check if the execution command starts with 'CPRun.py' or '-'
        isCPRunDefault = self.args.exec.startswith('-')
        if isCPRunDefault:
            clause = self.args.exec.split(' ')
            base = ['CPRun.py']
            inputClause = ['--input-list in.txt']
            strip = ['--strip'] if not isAthena else []
            return ' '.join(base + inputClause + strip + clause)
        else:
            return self.args.exec            
    
    def outputsFormatter(self):
        outputs = self.args.output_files.split(',')
        outputs = [f'{output.split(".")[0]}:{output}' for output in outputs]
        return ','.join(outputs)    
    
    
    def submit(self):
        pass
    
    def isAtlasProductionFormat(name):
        if name.startswith('mc') or name.startswith('data'):
            return True
        print(f"Name is not in the Atlas production format, assuming it is a user production")
        return False
    
    def rucioCustomNameParser(filename):
        '''
        The custom name has many variations, but most of them follow user/group.username.datasetname.suffix
        '''
        result = {}
        parts = filename.split('.')
        result['userType'] = parts[0]
        result['username'] = parts[1]
        result['main'] = parts[2]
        result['suffix'] = parts[-1]
        return result
    
    def atlasProductionNameParser(filename):
        '''
        Parsing file name into a dictionary, an example is given here
        mc20_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.deriv.DAOD_PHYS.e6337_s3681_r13167_p5855/DAOD_PHYS.34865530._000740.pool.root.1
        For the first part
        datasetName: mc20_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.deriv.DAOD_PHYS.e6337_s3681_r13167_p5855
        projectName: mc20_13TeV
        campaign: mc20
        energy: 13 #(TeV)
        DSID: 410470
        main: PhPy8EG_A14_ttbar_hdamp258p75_nonallhad
        TODO  generator: PhPy8Eg
        TODO  tune: A14 # For Pythia8
        TODO  process: ttbar
        TODO  hdamp: 258p75 # For Powheg 
        TODO  decayType: nonallhad
        step: deriv
        format: DAOD_PHYS
        tags: e###_s###_r###_p###_a###_t###_b#
        etag: e6337 # EVNT (EVGEN) production and merging
        stag: s3681 # Geant4 simulation to produce HITS and merging!
        rtag: r13167 # Digitisation and reconstruction, as well as AOD merging
        ptag: p5855 # Production of NTUP_PILEUP format and merging
        atag: aXXX: atlfast configuration (both simulation and digit/recon)
        ttag: tXXX: tag production configuration
        btag: bXXX: bytestream production configuration
        
        For the second part
        JeditaskID: 34865530
        fileNumber: 000740
        version: 1
        
        '''
        result = {}
        #split the / in case
        # mc20_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.deriv.DAOD_PHYS.e6337_s3681_r13167_p5855
        # /DAOD_PHYS.34865530._000740.pool.root.1
        if '/' in filename:
            datasetPart, filePart = filename.split('/')
        else:
            datasetPart = filename
            filePart = None
        
        # Split the dataset part by dots
        datasetParts = datasetPart.split('.')
        result['datasetName'] = datasetPart
        # Extract the first part
        result['projectName'] = datasetParts[0] # is positional
        # Extract the campaign and energy
        campaign_energy = result['projectName'].split('_')
        result['campaign'] = campaign_energy[0]
        result['energy'] = campaign_energy[1]
        
        # Extract the DSID, positional
        result['DSID'] = datasetParts[1]       
        result['main'] = datasetParts[2]
        result['step'] = datasetParts[3]
        result['format'] = datasetParts[4]
        
        # Extract the tags (etag, stag, rtag, ptag)
        tags = datasetParts[5].split('_')
        result['tags'] = tags
        for tag in tags:
            if tag.startswith('e'):
                result['etag'] = tag
            elif tag.startswith('s'):
                result['stag'] = tag
            elif tag.startswith('r'):
                result['rtag'] = tag
            elif tag.startswith('p'):
                result['ptag'] = tag
            elif tag.startswith('a'):
                result['atag'] = tag
            elif tag.startswith('t'):
                result['ttag'] = tag
            elif tag.startswith('b'):
                result['btag'] = tag
        
        # Extract the file part if it exists
        if filePart:
            fileParts = filePart.split('.')
            result['jediTaskID'] = fileParts[1]
            result['fileNumber'] = fileParts[2]
            result['version'] = fileParts[-1]
        return result

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
 
        
if __name__ == '__main__':
    cprun = CPGridRun()
    cprun.configureSumbission()
    cprun.printInputDetails()