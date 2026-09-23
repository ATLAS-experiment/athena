# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# @brief Utilities for handling MT and MP Athena jobs
# 

import os
import os.path as path

from xml.etree import ElementTree

import logging
msg = logging.getLogger(__name__)

from PyJobTransforms.trfExeStepTools import commonExecutorStepName
from PyJobTransforms.trfExitCodes import trfExit

import PyJobTransforms.trfExceptions as trfExceptions


## @brief Get athenaopts for step
def _athenaopts(argdict={}, currentName=''):
    # argument not present
    if 'athenaopts' not in argdict:
        return []

    # argument present but non -> not expected
    if argdict['athenaopts'] is None:
        raise ValueError("athenaopts argument is None")

    # argument present and not None -> return value
    return argdict['athenaopts'].returnMyValue(name='all'), argdict['athenaopts'].returnMyValue(name=currentName, withoutAll=True)


## @brief Get threads per process for step
def _threadsPerProcess(argdict={}, currentName=''):
    # argument not present
    if 'threadsPerProcess' not in argdict:
        return 0

    # argument present but non -> not expected
    if argdict['threadsPerProcess'] is None:
        raise ValueError("threadsPerProcess argument is None")

    # argument present and not None -> return value
    value = argdict['threadsPerProcess'].returnMyValue(name=currentName)
    if value is None:
        return 0
    return value


## @brief Detect how many threads and processes have been requested for Athena
#  @param argdict Argument dictionary, used to access athenaopts for the job
#  @param currentName Name of the current step
#  @param legacyThreadingRelease If true, then MP is used unconditionally
#  @return Three integers with the number of threads, number of processes and number of concurrent events, N.B. 0 means non-MT serial mode
def detectAthenaThreadsProcesses(argdict={}, currentName='', legacyThreadingRelease=False):
    athenaThreads = 0
    athenaConcurrentEvents = 0
    athenaProcs = 0
    currentName = commonExecutorStepName(currentName)

    # Try and detect if any AthenaMT has been enabled 
    try:
        athenaOptsList = _athenaopts(argdict, currentName)
        for i in range(len(athenaOptsList)):
            athenaOpts = athenaOptsList[i]
            if not athenaOpts:
                continue
            procArg = [opt.replace("--nprocs=", "") for opt in athenaOpts if '--nprocs' in opt]
            if not procArg:
                athenaProcs = 0
            elif len(procArg) == 1:
                if 'multiprocess' in argdict and i == 0:
                    raise ValueError("Detected conflicting methods to configure AthenaMP: --multiprocess and --nprocs=N (via athenaopts). Only one method must be used")
                athenaProcs = int(procArg[0])
                if athenaProcs < -1:
                    raise ValueError("--nprocs was set to a value less than -1")
            else:
                raise ValueError("--nprocs was set more than once in 'athenaopts'")

            threadArg = [opt.replace("--threads=", "") for opt in athenaOpts if '--threads' in opt]
            if not threadArg:
                athenaThreads = 0
            elif len(threadArg) == 1:
                if 'multithreaded' in argdict and i == 0:
                    raise ValueError("Detected conflicting methods to configure AthenaMT: --multithreaded and --threads=N (via athenaopts). Only one method must be used")
                athenaThreads = int(threadArg[0])
                if athenaThreads < -1:
                    raise ValueError("--threads was set to a value less than -1")
            else:
                raise ValueError("--threads was set more than once in 'athenaopts'")

            concurrentEventsArg = [opt.replace("--concurrent-events=", "") for opt in athenaOpts if '--concurrent-events' in opt]
            if len(concurrentEventsArg) == 1:
                athenaConcurrentEvents = int(concurrentEventsArg[0])
                if athenaConcurrentEvents < -1:
                    raise ValueError("--concurrent-events was set to a value less than -1")
                
            else:
                athenaConcurrentEvents = athenaThreads

        if athenaProcs > 0 or athenaThreads > 0 or 'ATHENA_CORE_NUMBER' not in os.environ:
            if athenaProcs > 0:
                msg.info('AthenaMP detected from "nprocs" setting with {0} workers for step {1}'.format(athenaProcs, currentName))
            if athenaThreads > 0:
                msg.info('AthenaMT detected from "threads" setting with {0} threads for step {1}'.format(athenaThreads, currentName))
            if athenaConcurrentEvents != athenaThreads:
                msg.info('AthenaMT detected from "concurrent-events" setting with {0} concurrent events for step {1}'.format(athenaConcurrentEvents, currentName))

            return athenaThreads, athenaConcurrentEvents, athenaProcs

        threadsPerProcess = _threadsPerProcess(argdict, currentName)
        if ('multiprocess' in argdict and argdict['multiprocess'].value) or legacyThreadingRelease:
            if 'threadsPerProcess' in argdict and threadsPerProcess:
                raise ValueError("Detected conflicting methods to configure AthenaMP: --multiprocess and --threadsPerProcess=N. Only one method must be used")
            athenaProcs = int(os.environ['ATHENA_CORE_NUMBER'])
            if athenaProcs < -1:
                raise ValueError("ATHENA_CORE_NUMBER value was less than -1")
            msg.info('AthenaMP detected from ATHENA_CORE_NUMBER with {0} workers'.format(athenaProcs))

        elif 'multithreaded' in argdict and argdict['multithreaded'].value:
            athenaThreads = int(os.environ['ATHENA_CORE_NUMBER'])
            if athenaThreads < -1:
                raise ValueError("ATHENA_CORE_NUMBER value was less than -1")
            if 'threadsPerProcess' in argdict and threadsPerProcess:
                athenaProcs = athenaThreads // threadsPerProcess
                athenaThreads = threadsPerProcess
                msg.info('Hybrid Athena MT+MP detected from ATHENA_CORE_NUMBER with {0} threads per process and {1} processes for substep {2}'.format(athenaThreads, athenaProcs, currentName))
            else:
                msg.info('AthenaMT detected from ATHENA_CORE_NUMBER with {0} threads'.format(athenaThreads))
            if athenaConcurrentEvents > 0 and athenaConcurrentEvents != athenaThreads:
                msg.info('AthenaMT detected from "concurrent-events" setting with {0} concurrent events for step {1}'.format(athenaConcurrentEvents, currentName))
            else:
                athenaConcurrentEvents = athenaThreads
    except ValueError as errMsg:
        myError = 'Problem discovering Athena threading setup: {0}'.format(errMsg)
        raise trfExceptions.TransformExecutionException(trfExit.nameToCode('TRF_EXEC_SETUP_FAIL'), myError)

    return athenaThreads, athenaConcurrentEvents, athenaProcs


## @brief Handle AthenaMP outputs, updating argFile instances to real 
#  @param athenaMPFileReport XML file with outputs that AthenaMP knew about
#  @param athenaMPWorkerTopDir Subdirectory with AthenaMP worker run directories
#  @param dataDictionary This substep's data dictionary, allowing all files to be
#  updated to the appropriate AthenaMP worker files
#  @param athenaMPworkers Number of AthenaMP workers
#  @param skipFileChecks Switches off checks on output files
#  @return @c None; side effect is the update of the @c dataDictionary
def athenaMPOutputHandler(athenaMPFileReport, athenaMPWorkerTopDir, dataDictionary, athenaMPworkers, skipFileChecks = False, argdict = {}):
    msg.debug("MP output handler called for report {0} and workers in {1}, data types {2}".format(athenaMPFileReport, athenaMPWorkerTopDir, list(dataDictionary)))
    outputHasBeenHandled = dict([ (dataType, False) for dataType in dataDictionary if dataDictionary[dataType] ])

    # if sharedWriter mode is active ignore athenaMPFileReport
    sharedWriter=False
    if 'sharedWriter' in argdict and argdict['sharedWriter'].value:
        sharedWriter=True
        skipFileChecks=True

    if not sharedWriter:
        # First, see what AthenaMP told us
        mpOutputs = ElementTree.ElementTree()
        try:
            mpOutputs.parse(athenaMPFileReport)
        except IOError:
            raise trfExceptions.TransformExecutionException(trfExit.nameToCode("TRF_OUTPUT_FILE_ERROR"), "Missing AthenaMP outputs file {0} (probably athena crashed)".format(athenaMPFileReport))
        for filesElement in mpOutputs.getroot().iter(tag='Files'):
            msg.debug('Examining element {0} with attributes {1}'.format(filesElement, filesElement.attrib))
            originalArg = None 
            startName = filesElement.attrib['OriginalName']
            for dataType, fileArg in dataDictionary.items():
                if fileArg.value[0] == startName:
                    originalArg = fileArg
                    outputHasBeenHandled[dataType] = True
                    break
            if originalArg is None:
                msg.warning('Found AthenaMP output with name {0}, but no matching transform argument'.format(startName))
                continue
        
            msg.debug('Found matching argument {0}'.format(originalArg))
            fileNameList = []
            for fileElement in filesElement.iter(tag='File'):
                msg.debug('Examining element {0} with attributes {1}'.format(fileElement, fileElement.attrib))
                fileNameList.append(path.relpath(fileElement.attrib['name']))

            athenaMPoutputsLinkAndUpdate(fileNameList, fileArg)

    # Now look for additional outputs that have not yet been handled
    if len([ dataType for dataType in outputHasBeenHandled if outputHasBeenHandled[dataType] is False]):
        # OK, we have something we need to search for; cache the dirwalk here
        MPdirWalk = [ dirEntry for dirEntry in os.walk(athenaMPWorkerTopDir) ]

        for dataType, fileArg in dataDictionary.items():
            if outputHasBeenHandled[dataType]:
                continue
            if fileArg.io == "input":
                continue
            msg.info("Searching MP worker directories for {0}".format(dataType))
            startName = fileArg.value[0]
            fileNameList = []
            for entry in MPdirWalk:
                if "evt_count" in entry[0]:
                    continue
                if "range_scatterer" in entry[0]:
                    continue
                # N.B. AthenaMP may have made the output name unique for us, so 
                # we need to treat the original name as a prefix
                possibleOutputs = [ fname for fname in entry[2] if fname.startswith(startName) ]
                if len(possibleOutputs) == 0:
                    continue
                elif len(possibleOutputs) == 1:
                    fileNameList.append(path.join(entry[0], possibleOutputs[0]))
                elif skipFileChecks:
                    pass
                else:
                    raise trfExceptions.TransformExecutionException(trfExit.nameToCode("TRF_OUTPUT_FILE_ERROR"), "Found multiple matching outputs for datatype {0} in {1}: {2}".format(dataType, entry[0], possibleOutputs))
            if skipFileChecks:
                pass
            elif len(fileNameList) != athenaMPworkers:
                raise trfExceptions.TransformExecutionException(trfExit.nameToCode("TRF_OUTPUT_FILE_ERROR"), "Found {0} output files for {1}, expected {2} (found: {3})".format(len(fileNameList), dataType, athenaMPworkers, fileNameList))

            # Found expected number of files - good!
            athenaMPoutputsLinkAndUpdate(fileNameList, fileArg) 


def athenaMPoutputsLinkAndUpdate(newFullFilenames, fileArg):
    # Any files we link are numbered from 1, because we always set
    # the filename given to athena has _000 as a suffix so that the
    # mother process' file can be used without linking
    fileIndex = 1
    linkedNameList = []
    newFilenameValue = []
    for fname in newFullFilenames:
        if path.dirname(fname) == "":
            linkedNameList.append(None)
            newFilenameValue.append(fname)
        else:
            linkName = "{0}{1:03d}".format(path.basename(fname).rstrip('0'), fileIndex)
            linkedNameList.append(linkName)
            newFilenameValue.append(linkName)
            fileIndex += 1
            
    for linkname, fname in zip(linkedNameList, newFullFilenames):
        if linkname:
            if len(newFullFilenames) == 1:
                if path.exists(fname):
                    try:
                        os.rename(fname, fileArg.originalName)
                    except OSError as e:
                        raise trfExceptions.TransformExecutionException(trfExit.nameToCode("TRF_OUTPUT_FILE_ERROR"), "Failed to move {0} to {1}: {2}".format(fname, fileArg.originalName, e))
                elif not path.exists(fileArg.originalName):
                    raise trfExceptions.TransformExecutionException(trfExit.nameToCode("TRF_OUTPUT_FILE_ERROR"), "Neither {0} nor {1} exists".format(fname, fileArg.originalName))
                newFilenameValue[0] = fileArg.originalName
            else:
                 try:
                     if path.lexists(linkname):
                         os.unlink(linkname)
                     os.symlink(fname, linkname)
                 except OSError as e:  
                     raise trfExceptions.TransformExecutionException(trfExit.nameToCode("TRF_OUTPUT_FILE_ERROR"), "Failed to link {0} to {1}: {2}".format(fname, linkname, e))

    fileArg.multipleOK = True
    fileArg.value = newFilenameValue
    msg.debug('MP output argument updated to {0}'.format(fileArg))
