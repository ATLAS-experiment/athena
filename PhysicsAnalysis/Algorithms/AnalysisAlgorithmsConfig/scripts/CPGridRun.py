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
        self._isFirstRun = True
        self._tarballRecreated = False
        self._inputList = None 
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
        ioGroup.add_argument('-i','--input-list', dest='input_list', help='Path to the text file containing list of containers on the panda grid. Each container will be passed to prun as --inDS and is run individually')
        ioGroup.add_argument('--output-files', dest='output_files', nargs='+', default=['output.root'],
                             help='The output files of the grid job. Example: --output-files A.root B.txt B.root results in A/A.root, B/B.txt, B/B.root in the output directory. No need to specify if using CPRun.py')
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
                                    'Run CPRun.py with preset behavior including streamlined file i/o. E.g, "CPRun.py -t config.yaml --no-systematics".\n'
                                    'Run custom script: "customRun.py -i inputs -o output --text-config config.yaml --flagA --flagB"\n'
                                    )

        submissionGroup = parser.add_argument_group('Submission configuration')
        submissionGroup.add_argument('--noSubmit', dest='noSubmit', action='store_true', help='Do not submit the job to the grid (PanDA). Useful to inspect the prun command')
        submissionGroup.add_argument('--testRun', dest='testRun', action='store_true', help='Will submit job to the grid but greatly limit the number of files per job (10) and number of events (300)')
        submissionGroup.add_argument('--recreateTar', dest='recreateTar', action='store_true', help='Re-compress the source code. Source code are compressed by default in submission, this is useful when the source code is updated')
        submissionGroup.add_argument('-y', '--agreeAll', dest='agreeAll', action='store_true', help='Agree to all the submission details without asking for confirmation. Use with caution!')
        self.args = parser.parse_args()
        self.outputFilesParsing()
        return parser

    @property
    def inputList(self):
        if self._inputList is None:
            if self.args.input_list.endswith('.txt'):
                self._inputList = CPGridRun._parseInputFileList(self.args.input_list)
            elif self.args.input_list.endswith('.json'):
                raise NotImplementedError('JSON input list parsing is not implemented')
            elif CPGridRun.isAtlasProductionFormat(self.args.input_list):
                self._inputList = [self.args.input_list]
            else:
                raise ValueError(
                    'use --input-list to specify input containers')
        return self._inputList

    def outputFilesParsing(self):
        output_files = []
        for output in self.args.output_files:
            if ',' in output:
                output_files.extend(output.split(','))
            else:
                output_files.append(output)
        self.output_files = output_files

    def printHelp(self):
        self.gridParser.print_help()
        logCPGridRun.info("\033[92m\n If you are using CPRun.py, the following flags are for the CPRun.py in this framework\033[0m")
        self._runscript.parser.usage = argparse.SUPPRESS
        self._runscript.parser.print_help()

    def getParser(self):
        return self.gridParser

    # This function do all the checking, cleaning and preparing the command to be submitted to the grid
    # separated for client to be able to change the behavior
    def configureSumbission(self):
        for input in self.inputList:
            cmd = self.configureSubmissionSingleSample(input)
            self.cmd[input] = cmd
            self._isFirstRun = False

    def configureSubmissionSingleSample(self, input):
        config = {
            'inDS': input,
            'outDS': self.args.outDS if self.args.outDS else self.outputDSFormatter(input) ,
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

        if not self._tarballRecreated and (self.args.recreateTar or not os.path.exists(self._tarfile) or self._filesChanged()):
            config['outTarBall'] = self._tarfile
            self._tarballRecreated = True
        elif os.path.exists(self._tarfile) or self._tarballRecreated:
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
            logCPGridRun.info("\n"
                f"Input: {key}\n" +
                "\n".join([f"  {k.replace('_', ' ').title()}: {v}" for k, v in parsed_name.items()]))
            logCPGridRun.info(f"Command: \n{cmd}")
            print("-" * 70)
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

    def _filesChanged(self):
        tarball_mtime = os.path.getmtime(self._tarfile) if os.path.exists(self._tarfile) else 0
        buildDir = self._buildDir()
        sourceDir = self._sourceDir()

        # Check for changes in buildDir
        for root, _, files in os.walk(buildDir):
            for file in files:
                file_path = os.path.join(root, file)
                try:
                    if os.path.getmtime(file_path) > tarball_mtime:
                        logCPGridRun.info(f"File {file_path} is newer than the tarball.")
                        return True
                except FileNotFoundError:
                    continue

        # Check for changes in sourceDir
        if sourceDir is None:
            logCPGridRun.warning("Source directory is not detected, auto-compression is not performed. Use --recreateTar to update the submission")
            return False
        for root, _, files in os.walk(sourceDir):
            for file in files:
                file_path = os.path.join(root, file)
                try:
                    if os.path.getmtime(file_path) > tarball_mtime:
                        logCPGridRun.info(f"File {file_path} is newer than the tarball.")
                        return True
                except FileNotFoundError:
                    continue
        return False

    def _buildDir(self):
        buildDir = os.environ["CMAKE_PREFIX_PATH"]
        buildDir = os.path.dirname(buildDir.split(":")[0])
        return buildDir

    def _sourceDir(self):
        cmakeCachePath = os.path.join(self._buildDir(), 'CMakeCache.txt')
        sourceDir = None
        if not os.path.exists(cmakeCachePath):
            return sourceDir
        with open(cmakeCachePath, 'r') as cmakeCache:
            for line in cmakeCache:
                if '_SOURCE_DIR:STATIC=' in line:
                    sourceDir = line.split('=')[1].strip()
                    break
        return sourceDir

    def execFormatter(self):
        # Check if the execution command starts with 'CPRun.py' or '-'
        isCPRunDefault = self.args.exec.startswith('-') or self.args.exec.startswith('CPRun.py')
        formatingClause = {
            'input_list': 'in.txt',
            'merge_output_files': True,
        }
        if not isCPRunDefault:
            if self._isFirstRun: logCPGridRun.warning("Non-CPRun.py is detected, please ensure the exec string is formatted correctly. Exec string will not be automatically formatted.")
            return f'"{self.args.exec}"'
        
        # Parse the exec string using the parser to validate and extract known arguments
        self._runscript = self._initRunscript()
        runscriptArgs, unknownArgs = self._runscript.parser.parse_known_args(self.args.exec.split(' '))
        
        # Throw error if unknownArgs contains any --args
        unknown_flags = [arg for arg in unknownArgs if arg.startswith('--')]
        if unknown_flags:
            logCPGridRun.error(f"Unknown flags detected in the exec string: {unknown_flags}. Please check the exec string.")
            raise ValueError(f"Unknown arguments detected: {unknown_flags}")

        # Only override if value is None or the parser default
        for key, value in formatingClause.items():
            if hasattr(runscriptArgs, key):
                old_value = getattr(runscriptArgs, key)
                if old_value is None or old_value == self._runscript.parser.get_default(key):
                    setattr(runscriptArgs, key, value)
                    if self._isFirstRun: logCPGridRun.info(f"Setting '{key}' to '{value}' (CPRun.py default is: '{old_value}')")
                else:
                    if self._isFirstRun: logCPGridRun.warning(f"Preserving user-defined '{key}': '{old_value}', default formatting '{value}' will not be applied.")
            else:
                logCPGridRun.error(f"Formatting clause '{key}' is not recognized in the CPRun.py script. Check CPGridRun.py")
                raise ValueError(f"Formatting clause '{key}' is not recognized in the CPRun.py script. Check CPGridRun.py")
            
        # Return the formatted arguments as a string
        arg_string = ' '.join(
            f'--{k.replace("_", "-")}' if isinstance(v, bool) and v else
            f'--{k.replace("_", "-")} {v}' for k, v in vars(runscriptArgs).items() if v not in [None, False]
        )
        return f'"CPRun.py {arg_string}"'


    def outputsFormatter(self):
        outputs = [f'{output.split(".")[0]}:{output}' for output in self.args.output_files]
        return ','.join(outputs)

    def submit(self):
        import subprocess
        import shutil
        prun_path = shutil.which("prun")
        if prun_path is None:
            logCPGridRun.error("The 'prun' command is not found. If you use are on lxplus, please run `setupATLAS` and `lsetup panda`")
            return
        for key, cmd in self.cmd.items():
            process = subprocess.Popen(cmd, shell=True, stdout=sys.stdout, stderr=sys.stderr)
            process.communicate()

    @staticmethod
    def isAtlasProductionFormat(name):
        if name.startswith('mc') or name.startswith('data'):
            return True
        logCPGridRun.warning("Name is not in the Atlas production format, assuming it is a user production")
        return False

    @staticmethod
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

    @staticmethod
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

    @staticmethod
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

    def _askSubmission(self):
        if self.args.agreeAll:
            logCPGridRun.info("You have agreed to all the submission details, jobs will be submitted without confirmation.")
            self.submit()
            return
        answer = input("Please confirm ALL the submission details are correct to submit [y/n]: ")
        if answer.lower() == 'y':
            self.submit()
        elif answer.lower() == 'n':
            logCPGridRun.info("Feel free to report any unexpected behavior to CPAlgorithms team!")
        else:
            logCPGridRun.error("Invalid input. Please enter 'y' or 'n'. Jobs are not submitted")

if __name__ == '__main__':
    cpgrid = CPGridRun()
    cpgrid.configureSumbission()
    cpgrid.printInputDetails()
    cpgrid._askSubmission()
