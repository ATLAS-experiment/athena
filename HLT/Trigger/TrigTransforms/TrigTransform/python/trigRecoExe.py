#!/usr/bin/env python

# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# @brief: Trigger executor to call base transforms
# @details: Based on athenaExecutor with some modifications
# @author: Mark Stockton

import os
import fnmatch
import re
import subprocess
import six

from PyJobTransforms.trfExe import athenaExecutor

# imports for preExecute
from PyJobTransforms.trfUtils import asetupReport, cvmfsDBReleaseCheck, unpackDBRelease, setupDBRelease, lineByLine, asetupReleaseIsOlderThan
import PyJobTransforms.trfEnv as trfEnv
import PyJobTransforms.trfExceptions as trfExceptions
from PyJobTransforms.trfExitCodes import trfExit as trfExit
import TrigTransform.dbgAnalysis as dbgStream
from TrigTransform.trigTranslate import getTranslated as getTranslated
from TrigTransform.trigTranslate import translateToLegacy as translateToLegacy

# Setup logging here
import logging, eformat
msg = logging.getLogger("PyJobTransforms." + __name__)

# Trig_reco_tf.py executor for BS-BS step (aka running the trigger)
# used to setup input files/arguments and change output filenames
class trigRecoExecutor(athenaExecutor):

    # preExecute is based on athenaExecutor but with key changes:
    # - removed athenaMP detection
    # - removed environment so does not require the noimf notcmalloc flags
    # - added swap of argument name for runargs file so that athenaHLT reads it in
    def preExecute(self, input = set(), output = set()):
        msg.debug('Preparing for execution of {0} with inputs {1} and outputs {2}'.format(self.name, input, output))
        # Check we actually have events to process!
        if (self._inputEventTest and 'skipEvents' in self.conf.argdict and
            self.conf.argdict['skipEvents'].returnMyValue(name=self._name, substep=self._substep, first=self.conf.firstExecutor) is not None):
            msg.debug('Will test for events to process')
            for dataType in input:
                inputEvents = self.conf.dataDictionary[dataType].nentries
                msg.debug('Got {0} events for {1}'.format(inputEvents, dataType))
                if not isinstance(inputEvents, six.integer_types):
                    msg.warning('Are input events countable? Got nevents={0} so disabling event count check for this input'.format(inputEvents))
                elif self.conf.argdict['skipEvents'].returnMyValue(name=self._name, substep=self._substep, first=self.conf.firstExecutor) >= inputEvents:
                    raise trfExceptions.TransformExecutionException(trfExit.nameToCode('TRF_NOEVENTS'),
                                                                    'No events to process: {0} (skipEvents) >= {1} (inputEvents of {2}'.format(self.conf.argdict['skipEvents'].returnMyValue(name=self._name, substep=self._substep, first=self.conf.firstExecutor), inputEvents, dataType))

        ## Write the skeleton file and prep athena
        if self._skeleton is not None:
            inputFiles = dict()
            for dataType in input:
                inputFiles[dataType] = self.conf.dataDictionary[dataType]
            outputFiles = dict()
            for dataType in output:
                outputFiles[dataType] = self.conf.dataDictionary[dataType]

            # See if we have any 'extra' file arguments
            for dataType, dataArg in self.conf.dataDictionary.items():
                if dataArg.io == 'input' and self._name in dataArg.executor:
                    inputFiles[dataArg.subtype] = dataArg

            msg.info('Input Files: {0}; Output Files: {1}'.format(inputFiles, outputFiles))

            # Get the list of top options files that will be passed to athena (=runargs file + all skeletons)
            self._topOptionsFiles = self._jobOptionsTemplate.getTopOptions(input = inputFiles,
                                                                           output = outputFiles)

        ## Add input/output file information - this can't be done in __init__ as we don't know what our
        #  inputs and outputs will be then
        if len(input) > 0:
            self._extraMetadata['inputs'] = list(input)
        if len(output) > 0:
            self._extraMetadata['outputs'] = list(output)

        ## Do we need to run asetup first?
        asetupString = None
        if 'asetup' in self.conf.argdict:
            asetupString = self.conf.argdict['asetup'].returnMyValue(name=self._name, substep=self._substep, first=self.conf.firstExecutor)
        else:
            msg.info('Asetup report: {0}'.format(asetupReport()))

        ## DBRelease configuration
        dbrelease = dbsetup = None
        if 'DBRelease' in self.conf.argdict:
            dbrelease = self.conf.argdict['DBRelease'].returnMyValue(name=self._name, substep=self._substep, first=self.conf.firstExecutor)
            if dbrelease:
                # Classic tarball - filename format is DBRelease-X.Y.Z.tar.gz
                dbdMatch = re.match(r'DBRelease-([\d\.]+)\.tar\.gz', os.path.basename(dbrelease))
                if dbdMatch:
                    msg.debug('DBRelease setting {0} matches classic tarball file'.format(dbrelease))
                    if not os.access(dbrelease, os.R_OK):
                        msg.warning('Transform was given tarball DBRelease file {0}, but this is not there'.format(dbrelease))
                        msg.warning('I will now try to find DBRelease {0} in cvmfs'.format(dbdMatch.group(1)))
                        dbrelease = dbdMatch.group(1)
                        dbsetup = cvmfsDBReleaseCheck(dbrelease)
                    else:
                        # Check if the DBRelease is setup
                        unpacked, dbsetup = unpackDBRelease(tarball=dbrelease, dbversion=dbdMatch.group(1))
                        if unpacked:
                            # Now run the setup.py script to customise the paths to the current location...
                            setupDBRelease(dbsetup)
                # For cvmfs we want just the X.Y.Z release string (and also support 'current')
                else:
                    dbsetup = cvmfsDBReleaseCheck(dbrelease)

        # Look for environment updates and perpare the athena command line
        self._envUpdate = trfEnv.environmentUpdate()
        # above is needed by _prepAthenaCommandLine, but remove the setStandardEnvironment so doesn't include imf or tcmalloc
        # self._envUpdate.setStandardEnvironment(self.conf.argdict)
        self._prepAthenaCommandLine()

        # to get athenaHLT to read in the relevant parts from the runargs file we have to translate them
        if 'athenaHLT' in self._exe:
            self._cmd.remove('runargs.BSRDOtoRAW.py')
            # get list of translated arguments to be used by athenaHLT
            optionList = getTranslated(self.conf.argdict, name=self._name, substep=self._substep, first=self.conf.firstExecutor, output = outputFiles)
            self._cmd.extend(optionList)

            # Run preRun step debug stream analysis if output histogram are set
            if "outputHIST_DEBUGSTREAMMONFile" in self.conf.argdict:
                # Do debug stream preRun step and get asetup string from debug stream input files
                dbgAsetupString, dbAlias = dbgStream.dbgPreRun(self.conf.dataDictionary['BS_RDO'], self.conf.dataDictionary['HIST_DEBUGSTREAMMON'].value, self.conf.argdict)
                # Setup asetup from debug stream
                # if no --asetup r2b:string was given and is not running with tzero/software/patches as TestArea
                if asetupString is None and dbgAsetupString is not None:
                    asetupString = dbgAsetupString
                    msg.info('Will use asetup string for debug stream analysis %s', dbgAsetupString)

                # If legacy release, bring up centos7 container
                OSSetupString = None
                legacyOSRelease = asetupReleaseIsOlderThan(asetupString, 24)
                if asetupString is not None:
                    currentOS = os.environ['ALRB_USER_PLATFORM']
                if legacyOSRelease and "centos7" not in currentOS:
                    OSSetupString = "centos7"
                    msg.info('Legacy release required for the substep {}, will setup a container running {}'.format(self._substep, OSSetupString))

                # allow overriding the container OS using a flag
                if 'runInContainer' in self.conf.argdict:
                    OSSetupString = self.conf.argdict['runInContainer'].returnMyValue(name=self._name, substep=self._substep, first=self.conf.firstExecutor)
                    msg.info('The step {} will be performed in a container running {}, as explicitly requested'.format(self._substep, OSSetupString))
                if OSSetupString is not None and asetupString is None:
                    raise trfExceptions.TransformExecutionException(trfExit.nameToCode('TRF_EXEC_SETUP_FAIL'),
                                                                    '--asetup must be used for the substep which requires --runInContainer')

                # Set database in command line if it was missing
                if 'useDB' in self.conf.argdict and 'DBserver' not in self.conf.argdict and dbAlias:
                    msg.warn("Database alias will be set to %s", dbAlias)
                    self._cmd.append("--db-server " + dbAlias)
            else:
                msg.info("Flag outputHIST_DEBUGSTREAMMONFile not defined - debug stream analysis will not run.")

            # updates for CA
            if self._isCAEnabled():
                msg.info("Running in CA mode")
                # we don't use the runargs file with athenaHLT so add the JO and preExecs to the command line and remove the CA option
                self._cmd.remove('--CA')
                if not legacyOSRelease:
                    self._cmd.append(self._skeletonCA)
                    if 'preExec' in self.conf.argdict:
                        self._cmd.extend(self.conf.argdict['preExec'].returnMyValue(name=self._name, substep=self._substep, first=self.conf.firstExecutor))
                        msg.info('Command adjusted for CA to %s', self._cmd)
                else:
                    # if legacy release, use legacy config
                    self._cmd.append(self._skeleton[0])
                    if 'preExec' in self.conf.argdict:
                        preExecOptions = self.conf.argdict['preExec'].returnMyValue(name=self._name, substep=self._substep, first=self.conf.firstExecutor)
                        self._cmd.extend(translateToLegacy(preExecOptions))
                        msg.info('Original CA preExec adjusted for legacy to %s', self._cmd)
            else:
                msg.info("Running in legacy mode")

        # The following is needed to avoid conflicts in finding BS files prduced by running the HLT step
        # and those already existing in the working directory
        if 'BS' in self.conf.dataDictionary or 'DRAW_TRIGCOST' in self.conf.dataDictionary or 'HIST_DEBUGSTREAMMON' in self.conf.dataDictionary:
            # expected string based on knowing that the format will be of form: ####._HLTMPPy_####.data
            expectedOutputFileName = '*_HLTMPPy_*.data'
            # list of filenames of files matching expectedOutputFileName
            matchedOutputFileNames = self._findOutputFiles(expectedOutputFileName)
            # check there are no file matches
            if len(matchedOutputFileNames) > 0:
                msg.error(f'Directoy already contains files with expected output name format ({expectedOutputFileName}), please remove/rename these first: {matchedOutputFileNames}')
                raise trfExceptions.TransformExecutionException(trfExit.nameToCode('TRF_OUTPUT_FILE_ERROR'),
                    f'Directory already contains files with expected output name format {expectedOutputFileName}, please remove/rename these first: {matchedOutputFileNames}')

        # Call athenaExecutor parent as the above overrides what athenaExecutor would have done
        super(athenaExecutor, self).preExecute(input, output)

        # Now we always write a wrapper, because it's very convenient for re-running individual substeps
        # This will have asetup and/or DB release setups in it
        # Do this last in this preExecute as the _cmd needs to be finalised
        msg.info('Now writing wrapper for substep executor {0}'.format(self._name))
        self._writeAthenaWrapper(asetup=asetupString, dbsetup=dbsetup, ossetup=OSSetupString)
        msg.info('Athena will be executed in a subshell via {0}'.format(self._cmd))

    def _prepAthenaCommandLine(self):

        # When running from the DB athenaHLT needs no skeleton file
        msg.info("Before build command line check if reading from DB")

        # Check if expecting to run from DB
        removeSkeleton = False
        if 'useDB' in self.conf.argdict:
            removeSkeleton = True
        elif 'athenaopts' in self.conf.argdict:
            v = self.conf.argdict['athenaopts'].value
            if '--use-database' in v or '-b' in v:
                removeSkeleton = True

        if removeSkeleton and self._isCAEnabled():
            msg.error('Do not specify --CA when reading from the DB, CA config will already be contained in the SMK')
            raise trfExceptions.TransformExecutionException(trfExit.nameToCode('TRF_ARG_ERROR'),
                    'Do not specify --CA when reading from the DB, CA config will already be contained in the SMK')
            return 1

        # Due to athenaExecutor code don't remove skeleton, otherwise lose runargs too
        # instead remove skeleton from _topOptionsFiles
        if removeSkeleton and self._skeleton is not None:
            msg.info('Read from DB: remove skeleton {0} from command'.format(self._skeleton))
            self._topOptionsFiles.remove(self._skeleton[0])
        else:
            msg.info('Not reading from DB: keep skeleton in command')

        # Now build command line as in athenaExecutor
        super(trigRecoExecutor, self)._prepAthenaCommandLine()

    # Loop over current directory and find the output file matching input pattern
    def _findOutputFiles(self, pattern):
        # list to store the filenames of files matching pattern
        matchedOutputFileNames = []
        # list of input files that could be in the same folder and need ignoring
        ignoreInputFileNames = []
        for dataType, dataArg in self.conf.dataDictionary.items():
            if dataArg.io == 'input':
                ignoreInputFileNames.append(dataArg.value[0])
        # loop over all files in folder to find matching output files
        for file in os.listdir('.'):
            if fnmatch.fnmatch(file, pattern):
                if file in ignoreInputFileNames:
                    msg.info('Ignoring input file: %s', file)
                else:
                    matchedOutputFileNames.append(file)
        return matchedOutputFileNames

    # merge multiple BS files into a single BS file
    def _mergeBSfiles(self, inputFiles, outputFile):
        msg.info(f'Merging multiple BS files ({inputFiles}) into {outputFile}')
        # Write the list of input files to a text file, to use it as a input for file_merging
        mergeBSFileList = 'RAWFileMerge.list'
        try:
            with open(mergeBSFileList, 'w') as BSFileList:
                for fname in inputFiles:
                    BSFileList.write(f'{fname}\n')
        except (IOError, OSError) as e:
            raise trfExceptions.TransformExecutionException(trfExit.nameToCode('TRF_OUTPUT_FILE_ERROR'), 
                f'Got an error when writing list of BS files to {mergeBSFileList}: {e}')
        
        # The user should never need to use this directly, but check it just in case...
        if not outputFile.endswith('._0001.data'):
            raise trfExceptions.TransformExecutionException(trfExit.nameToCode('TRF_OUTPUT_FILE_ERROR'), 
                'Output merged BS filename must end in "._0001.data"')
        else:
            # We need to remove the suffix to pass the output as an argument to file_merging
            outputFile = outputFile.split('._0001.data')[0]
        
        mergeBSFailure = 0
        try:
            cmd = f'file_merging {mergeBSFileList} 0 {outputFile}'
            msg.info('running command for merging (in original asetup env): %s', cmd)
            mergeBSFailure = subprocess.call(cmd, shell=True)
            msg.debug('file_merging return code %s', mergeBSFailure)
        except OSError as e:
            raise trfExceptions.TransformExecutionException(trfExit.nameToCode('TRF_OUTPUT_FILE_ERROR'),
                f'Exception raised when merging BS files using file_merging: {e}')
        if mergeBSFailure != 0:
            msg.error('file_merging returned error (%s) no merged BS file created', mergeBSFailure)
            return 1
        return 0


    # run trigbs_extractStream.py to split a stream out of the BS file
    # renames the split file afterwards
    def _splitBSfile(self, streamsList, allStreamsFileName, splitFileName):
        # merge list of streams
        outputStreams = ','.join(str(stream) for stream in streamsList)
        msg.info('Splitting stream %s from BS file', outputStreams)
        splitStreamFailure = 0
        try:
            cmd = 'trigbs_extractStream.py -s ' + outputStreams + ' ' + allStreamsFileName
            msg.info('running command for splitting (in original asetup env): %s', cmd)
            splitStreamFailure = subprocess.call(cmd, shell=True)
            msg.debug('trigbs_extractStream.py splitting return code %s', splitStreamFailure)
        except OSError as e:
            raise trfExceptions.TransformExecutionException(trfExit.nameToCode('TRF_OUTPUT_FILE_ERROR'),
                'Exception raised when selecting stream with trigbs_extractStream.py in file {0}: {1}'.format(allStreamsFileName, e))
        if splitStreamFailure != 0:
            msg.error('trigbs_extractStream.py returned error (%s) no split BS file created', splitStreamFailure)
            return 1
        else:
            # know that the format will be of the form ####._athenaHLT.####.data
            expectedStreamFileName = '*_athenaHLT*.data'
            # list of filenames of files matching expectedStreamFileName
            matchedOutputFileName = self._findOutputFiles(expectedStreamFileName)
            if(len(matchedOutputFileName)):
                self._renamefile(matchedOutputFileName[0], splitFileName)
                return 0
            else:
                msg.error('trigbs_extractStream.py did not created expected file (%s)', expectedStreamFileName)
                return 1

    # rename a created file - used to overwrite filenames from athenaHLT into the requested argument name
    def _renamefile(self, currentFileName, newFileName):
        msg.info('Renaming file from %s to %s', currentFileName, newFileName)
        try:
            os.rename(currentFileName, newFileName)
        except OSError as e:
            msg.error('Exception raised when renaming {0} #to {1}: {2}'.format(currentFileName, newFileName, e))
            raise trfExceptions.TransformExecutionException(trfExit.nameToCode('TRF_OUTPUT_FILE_ERROR'),
                'Exception raised when renaming {0} #to {1}: {2}'.format(currentFileName, newFileName, e))

    def postExecute(self):

        # Adding check for HLTMPPU.*Child Issue in the log file
        # - Throws an error message if there so we catch that the child died
        # - Also sets the return code of the mother process to mark the job as failed
        # - Is based on trfValidation.scanLogFile
        log = self._logFileName
        msg.debug('Now scanning logfile {0} for HLTMPPU Child Issues'.format(log))
        # Using the generator so that lines can be grabbed by subroutines if needed for more reporting
       
        #Count the number of rejected events 
        rejected = 0 
        #Count the number of accepted events
        accepted = 0 
       
        try:
            myGen = lineByLine(log, substepName=self._substep)
        except IOError as e:
            msg.error('Failed to open transform logfile {0}: {1:s}'.format(log, e))
        for line, lineCounter in myGen:
            # Check to see if any of the hlt children had an issue
            if 'Child Issue' in line:
                try:
                    signal = int((re.search('signal ([0-9]*)', line)).group(1))
                except AttributeError:
                    # signal not found in message, so return 1 to highlight failure
                    signal = 1
                msg.error('Detected issue with HLTChild, setting mother return code to %s', signal)
                self._rc = signal

        # Merge child log files into parent log file
        # is needed to make sure all child log files are scanned
        # files are found by searching whole folder rather than relying on nprocs being defined
        try:
            # open original log file (log.BSRDOtoRAW) to merge child files into
            with open(self._logFileName, 'a') as merged_file:
                for file in os.listdir('.'):
                    # expected child log files should be of the format athenaHLT:XX.out and .err
                    if fnmatch.fnmatch(file, 'athenaHLT:*'):
                        msg.info('Merging child log file (%s) into %s', file, self._logFileName)
                        with open(file) as log_file:
                            # write header infomation ### Output from athenaHLT:XX.out/err ###
                            merged_file.write('### Output from {} ###\n'.format(file))
                            # write out file line by line
                            for line in log_file:
                                merged_file.write(line)
                                # Check for rejected events in log file
                                if 'rejected:' in line and int(line[14]) != 0:
                                    #Add the number of rejected events      
                                    rejected += int(line[14:])
                                # Check for accepted events in log file
                                if 'accepted:' in line and int(line[14]) != 0:
                                    #Add the number of accepted events      
                                    accepted += int(line[14:])
            
            if "HIST_DEBUGSTREAMMON" in self.conf.dataDictionary: 
                # Add the HLT_accepted_events and HLT_rejected_events histograms to the output file 
                dbgStream.getHltDecision(accepted, rejected, self.conf.argdict["outputHIST_DEBUGSTREAMMONFile"].value[0])

        except OSError as e:
            raise trfExceptions.TransformExecutionException(trfExit.nameToCode('TRF_OUTPUT_FILE_ERROR'),
                        'Exception raised when merging log files into {0}: {1}'.format(self._logFileName, e))

        msg.info("Check for expert-monitoring.root file")
        # the BS-BS step generates the files:
        # - expert-monitoring.root (from mother process)
        # - athenaHLT_workers/*/expert-monitoring.root (from child processes)
        # to save on panda it needs to be renamed via the outputHIST_HLTMONFile argument
        expectedFileName = 'expert-monitoring.root'

        # first check athenaHLT step actually completed
        if self._rc != 0:
            msg.info('HLT step failed (with status %s) so skip HIST_HLTMON filename check', self._rc)
        # next check argument is in dictionary as a requested output
        elif 'outputHIST_HLTMONFile' in self.conf.argdict:

            # rename the mother file
            expectedMotherFileName = 'expert-monitoring-mother.root'
            if(os.path.isfile(expectedFileName)):
                msg.info('Renaming %s to %s', expectedFileName, expectedMotherFileName)
                try:
                    os.rename(expectedFileName, expectedMotherFileName)
                except OSError as e:
                    raise trfExceptions.TransformExecutionException(trfExit.nameToCode('TRF_OUTPUT_FILE_ERROR'),
                        'Exception raised when renaming {0} to {1}: {2}'.format(expectedFileName, expectedMotherFileName, e))
            else:
                msg.error('HLTMON argument defined but mother %s not created', expectedFileName)

            # merge worker files
            expectedWorkerFileName = 'athenaHLT_workers/athenaHLT-01/' + expectedFileName
            if(os.path.isfile(expectedWorkerFileName) and os.path.isfile(expectedMotherFileName)):
                msg.info('Merging worker and mother %s files to %s', expectedFileName, self.conf.argdict['outputHIST_HLTMONFile'].value[0])
                try:
                    # have checked that at least one worker file exists
                    cmd = 'hadd ' + self.conf.argdict['outputHIST_HLTMONFile'].value[0] + ' athenaHLT_workers/*/expert-monitoring.root expert-monitoring-mother.root'
                    subprocess.call(cmd, shell=True)
                except OSError as e:
                    raise trfExceptions.TransformExecutionException(trfExit.nameToCode('TRF_OUTPUT_FILE_ERROR'),
                        'Exception raised when merging worker and mother {0} files to {1}: {2}'.format(expectedFileName, self.conf.argdict['outputHIST_HLTMONFile'].value[0], e))
            else:
                msg.error('HLTMON argument defined %s but worker %s not created', self.conf.argdict['outputHIST_HLTMONFile'].value[0], expectedFileName)

        else:
            msg.info('HLTMON argument not defined so skip %s check', expectedFileName)

        msg.info("Search for created BS files, and rename if single file found")
        # The following is needed to handle the BS file being written with a different name (or names)
        # base is from either the tmp value created by the transform or the value entered by the user
   
        argInDict = {}
        if self._rc != 0:
            msg.error('HLT step failed (with status %s) so skip BS filename check', self._rc)
        elif 'BS' in self.conf.dataDictionary or 'DRAW_TRIGCOST' in self.conf.dataDictionary or 'HIST_DEBUGSTREAMMON' in self.conf.dataDictionary:
            # expected string based on knowing that the format will be of form: ####._HLTMPPy_####.data
            expectedOutputFileName = '*_HLTMPPy_*.data'
            # list of filenames of files matching expectedOutputFileName
            matchedOutputFileNames = self._findOutputFiles(expectedOutputFileName)


            # check there are file matches and rename appropriately
            if len(matchedOutputFileNames) == 0:
                msg.error('No BS files created with expected name: %s - please check for earlier failures', expectedOutputFileName)
                raise trfExceptions.TransformExecutionException(trfExit.nameToCode('TRF_OUTPUT_FILE_ERROR'),
                    f'No BS files created with expected name: {expectedOutputFileName} - please check for earlier failures')
            else:
                # if only one BS file was created
                if len(matchedOutputFileNames) == 1:
                    msg.info('Single BS file found: will split (if requested) and rename the file')
                    BSFile = matchedOutputFileNames[0]

                # if more than one file BS was created, then merge them
                else:
                    msg.info('Multiple BS files found. A single BS file is required by the next transform steps, so they will be merged. Will split the merged file (if requested) and rename the file')
                    mergedBSFile = matchedOutputFileNames[0].split('._0001.data')[0] + '.mrg._0001.data' # a specific format is required to run file_merging

                    mergeFailed = self._mergeBSfiles(matchedOutputFileNames, mergedBSFile)
                    if(mergeFailed):
                        raise trfExceptions.TransformExecutionException(trfExit.nameToCode('TRF_OUTPUT_FILE_ERROR'),
                            'Did not produce a merged BS file with file_merging')
                    
                    BSFile = 'tmp.BS.mrg'
                    msg.info(f'Renaming temporary merged BS file to {BSFile}')
                    self._renamefile(mergedBSFile, BSFile)

                # First check if we want to produce the COST DRAW output
                if 'DRAW_TRIGCOST' in self.conf.dataDictionary:
                    splitFailed = self._splitBSfile(['CostMonitoring'], BSFile, self.conf.dataDictionary['DRAW_TRIGCOST'].value[0])
                    if(splitFailed):
                        raise trfExceptions.TransformExecutionException(trfExit.nameToCode('TRF_OUTPUT_FILE_ERROR'),
                            'Did not produce any BS file when selecting CostMonitoring stream with trigbs_extractStream.py in file')
                
                # Run debug step for all streams
                if "HIST_DEBUGSTREAMMON" in self.conf.dataDictionary:
                    self._postExecuteDebug(BSFile)

                # Rename BS file if requested
                if 'BS' in self.conf.dataDictionary:
                    argInDict = self.conf.dataDictionary['BS']
                    # If a stream (not All) is selected, then slim the orignal (many stream) BS output to the particular stream
                    if 'streamSelection' in self.conf.argdict and self.conf.argdict['streamSelection'].value[0] != "All":
                        splitEmpty = self._splitBSfile(self.conf.argdict['streamSelection'].value, BSFile, argInDict.value[0])
                        if(splitEmpty):
                            msg.info('Did not produce any BS file when selecting stream with trigbs_extractStream.py in file')
                            #If splitEmpty==1, the chosen streams contained no events 
                            #then run the command to produce an empty BS file and rename it to RAW.pool.root 
                            #this stops non-zero exit code for rejected events 
                            cmd_splitFailed = 'trigbs_failedStreamSelection.py ' + BSFile   
                            msg.info('running command for creating empty file: %s', cmd_splitFailed)
                            subprocess.call(cmd_splitFailed, shell=True)
                            #Rename the empty file to "RAW.pool.root" to prevent failure 
                            #expected filename will be of form: T0debug.runnumber.unknown_debug.unknown.RAW._lb0000._TRF._0001.data
                            runnumber = eformat.EventStorage.pickDataReader(BSFile).runNumber()
                            expectedOutputFileName = 'T0debug.00'+str(runnumber)+'.unknown_debug.unknown.RAW._lb0000._TRF._0001.data'
                            #rename the file to RAW.pool.root, this file will contain 0 events 
                            self._renamefile(expectedOutputFileName, argInDict.value[0])                     
                    else:
                        msg.info('Stream "All" requested, so not splitting BS file')
                        self._renamefile(BSFile, argInDict.value[0])
                else:
                    msg.info('BS output filetype not defined so skip renaming BS')
        else:
            msg.info('BS, DRAW_TRIGCOST or HIST_DEBUGSTREAMMON output filetypes not defined so skip BS post processing')

        msg.info('Now run athenaExecutor:postExecute')
        super(trigRecoExecutor, self).postExecute()

        if "HIST_DEBUGSTREAMMON" in self.conf.dataDictionary:
            # Do debug stream postRun step for BS file that contains events after the streamSelection
            fileNameDbg = self.conf.argdict["outputHIST_DEBUGSTREAMMONFile"].value
            dbgStream.dbgPostRun(argInDict.value[0], fileNameDbg[0], self.conf.argdict, isSplitStream=True)


    def _postExecuteDebug(self, outputBSFile):
        # Run postRun step debug stream analysis if output BS file and output histogram are set
        msg.info("debug stream analysis in postExecute")

        # Set file name for debug stream analysis output
        fileNameDbg = self.conf.argdict["outputHIST_DEBUGSTREAMMONFile"].value
        msg.info('outputHIST_DEBUGSTREAMMONFile argument is {0}'.format(fileNameDbg))

        if(os.path.isfile(fileNameDbg[0])):
            # Keep filename if not defined
            msg.info('Will use file created in PreRun step {0}'.format(fileNameDbg))
        else:
            msg.info('No file created  in PreRun step {0}'.format(fileNameDbg))
        
        # Do debug stream postRun step
        dbgStream.dbgPostRun(outputBSFile, fileNameDbg[0], self.conf.argdict)

