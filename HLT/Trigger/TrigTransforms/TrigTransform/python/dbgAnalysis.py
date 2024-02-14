#!/usr/bin/env python

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# @brief: Pre and Post debug_stream analysis operations for trigger transform
# @details: Code to carry out operations that are needed for running
#   the trigger debug_stream analysis in transform
# @author: Carlos Chavez

import os
import eformat
from TrigTransform.dbgEventInfo import dbgEventInfo
from TrigConfStorage.TriggerCoolUtil import TriggerCoolUtil
from TrigConfIO.L1TriggerConfigAccess import L1MenuAccess
from TrigConfIO.HLTTriggerConfigAccess import HLTMenuAccess

from ROOT import TFile, TH1F

import logging
msg = logging.getLogger("PyJobTransforms." + __name__)

# As an optional argument dictionary with config data extracted from arguments can be passed
def dbgPreRun(inputFileList, outputFileList, argdict = None):
    msg.info('Running debug_stream analysis PreRun operations on files :{0} '.format(inputFileList))
    msg.info('Running debug_stream analysis PreRun, histogram output in :{0} '.format(outputFileList))

    # Open root output file
    outFile = outputFileList[0]
    hfile = TFile(outFile, 'RECREATE')

    maxEvents = argdict.get('maxEvents')
    maxEvents = maxEvents.value.get('first') if maxEvents else -1

    skipEvents = argdict.get('skipEvents')
    skipEvents = skipEvents.value.get('first') if skipEvents else 0

    # Initialize dbgEventInfo, this is the main event analysis class
    eventInfo = dbgEventInfo('_Pre', inputFileList.value[0])
    data = []
    l1Info = []
    hltInfo = []
    configKeys = None
    for inputFile in inputFileList.value:
        bsfile = eformat.istream(inputFile)
        n = 0
        isFirstEvent = True

        bsfile = bsfile[skipEvents:skipEvents+maxEvents] if maxEvents > -1 else bsfile[skipEvents:]

        for event in bsfile:

            if isFirstEvent:
                runNumber = event.run_no() if event.run_no() else int(inputFile.split(".")[1])
                configKeys = getHLTConfigKeys(runNumber, argdict)

                if not argdict.get('useDB'):
                    msg.debug("Reading chains and items from database is skipped (missing --useDB=True)")
                    l1Info, hltInfo = ([], [])
                else:
                    l1Info, hltInfo = TriggerDBInfo(configKeys.get('DB'), configKeys.get('SMK'))

                isFirstEvent = False

            # Log the details of first 5 events
            n += 1
            if n < 5:
                data = [event.run_no(), event.lumi_block(), event.global_id(),
                        event.lvl1_id(), event.bc_time_seconds(), event.bc_time_nanoseconds()]
                msg.info('Event details :%s', data)
            
            # Run debug event analysis and fill output TTree
            eventInfo.eventCount(event)
            eventInfo.eventInfo(event, l1Info, hltInfo)
            eventInfo.eventConfig(configKeys, event)
            eventInfo.fillTree()

    # Close output TFile
    hfile.Write()
    hfile.Close()

    msg.info('Finished running debug_stream analysis PreRun operations')

    dbAlias = configKeys.get('DB')

    # Check release format, if relInfo is 'unknown' then print error
    if 'REL' in configKeys:
        # Returns the local asetupString from runs in input files and to be used by asetup
        return getAsetupString(configKeys['REL']), dbAlias
    else:
        msg.warn("Release not found in configuration - asetup string won't be created")

    # Without release and project asetup string is not available
    return None, dbAlias


def dbgPostRun(inputFile, outputFile, argdict = None, isSplitStream=False):
    msg.info('Running debug_stream analysis PostRun operations on files :{0} '.format(inputFile))
    msg.info('Running debug_stream analysis PostRun, histogram output in :{0} '.format(outputFile))

    # Open root output file
    hfile = TFile(outputFile, 'UPDATE')

    # Inicialize dbgEventInfo,  this is the main event analysis class
    if isSplitStream:
        eventInfo = dbgEventInfo('_Pos_Split', inputFile)
    else:
        eventInfo = dbgEventInfo('_Pos', inputFile)
    data = []
    l1Info = []
    hltInfo = []
    configKeys = None

    bsfile = eformat.istream(inputFile)
    n = 0
    isFirstEvent = True

    for event in bsfile:
        # If fist event get l1 and hlt counter and chain info from DB
        if isFirstEvent:
            configKeys = getHLTConfigKeys(event.run_no(), argdict)

            if not argdict.get('useDB'):
                msg.debug("Reading chains and items from database is skipped (missing --useDB=True)")
                l1Info, hltInfo = ([], [])
            else:
                l1Info, hltInfo = TriggerDBInfo(configKeys.get('DB'), configKeys.get('SMK'))

            isFirstEvent = False

        # Log the details of first 5 events
        n += 1
        if n < 5:
            data = [event.run_no(), event.lumi_block(), event.global_id(),
                    event.lvl1_id(), event.bc_time_seconds(), event.bc_time_nanoseconds()]
            msg.info('Event details :{0}'.format(data))

        # Run debug event analysis and fill output TTree
        eventInfo.eventCount(event)
        eventInfo.eventInfo(event, l1Info, hltInfo)
        eventInfo.eventConfig(configKeys, event)
        eventInfo.fillTree()

    # Close output TFile
    hfile.Write()
    hfile.Close()
    msg.info('Finished running debug_stream analysis PostRun operations')


def TriggerDBInfo(dbalias = None, smk = None):
    l1Info = []
    hltInfo = []

    if not dbalias or not smk:
        msg.warn("Reading chains and items from database is skipped (config keys are not available)") 
        return (l1Info, hltInfo)

    # L1 DB Info
    try:
        l1Cfg = L1MenuAccess(dbalias = dbalias, smkey = smk)

        # Create map id to item and then fill the array
        #   that on i spot has item with i ctpid
        l1Items = l1Cfg.items()
        l1ItemsMap = {}
        for item in l1Items:
            ctpid = l1Items[item]['ctpid']
            l1ItemsMap[ctpid] = item

        l1Info = (max(l1ItemsMap.keys()) + 1) * [0]

        for item in l1ItemsMap:
            l1Info[item] = l1ItemsMap[item]

    except RuntimeError:
        msg.info("Database for L1 Menu not available")

    # HLT DB Info
    try:
        hltCfg = HLTMenuAccess(dbalias = dbalias, smkey = smk)
        hltChains = hltCfg.chains()
        hltChainsMap = {}
        for chain in hltChains:
            counter = hltChains[chain]['counter']
            hltChainsMap[counter] = chain

        hltInfo = (max(hltChainsMap.keys()) + 1) * [0]

        for chain in hltChainsMap:
            hltInfo[chain] = hltChainsMap[chain]
    except RuntimeError:
        msg.info("Database for HLT Menu not available")


    return (l1Info, hltInfo)


def getAsetupString(release):

    asetupString = release

    # If TestArea is for tzero (tzero/software/patches/AtlasP1HLT-RELEASE),
    #   then returns tzero/software/patches/AtlasP1HLT-release where release is
    #   the parameter given to this function getAsetupString(release)
    if os.environ['TestArea']:
        TestArea = os.environ['TestArea']
        if TestArea.find('tzero/software/patches/AthenaP1-') > 0:
            testarea = TestArea.split('-')
            TestArea = testarea[0] + '-' + release
        asetupString += ' --testarea ' + TestArea

    return asetupString


def getHLTConfigKeys(runNumber = None, args = None):
    ''' 
    Returns map with config keys:
        DB - database alias
        REL - release
        SMK - Super Master Key
        HLTPSK - HLT Prescale key
    '''

    dbconn = TriggerCoolUtil.GetConnection("CONDBR2")
    configKeys = TriggerCoolUtil.getHLTConfigKeys(dbconn, [[runNumber, runNumber]])

    if configKeys and runNumber in configKeys.keys():
        configKeys = configKeys[runNumber]

        configKeys['HLTPSK'] = TriggerCoolUtil.getHLTPrescaleKeys(dbconn, [[runNumber, runNumber]])[runNumber]['HLTPSK2']

        msg.info("Found config keys %s", configKeys)
    else:
        msg.info("Config keys not found in COOL")
        configKeys = getHLTConfigFromArgs(args)

    if 'DB' in configKeys:
        if 'DBserver' in args and configKeys['DB'] != args['DBserver'].value:
            msg.warn("Different database read from config keys ({0}) and from args ({1})".format(configKeys['DB'], args['DBserver'].value))
            msg.warn("Database {0} from config keys will be used.".format(configKeys['DB']))   

    return configKeys


def getHLTConfigFromArgs(args):
    '''
    Prepare dictionary with data extracted from arguments if available. Contains:
        DB - database alias
        SMK - Super Master Key
        HLTPSC - HLT Prescale key
    '''

    configKeys = {}

    try:
        configKeys['DB'] = args['DBserver'].value
        configKeys['SMK'] = int(args['DBsmkey'].value)
        configKeys['HLTPSK'] = int(args['DBhltpskey'].value)

        msg.info("Found config keys in args %s", configKeys)
    
    except KeyError:
        msg.warn("Config keys not found in argdict")
        return {}

    return configKeys

def getHltDecision(accepted, rejected, outputFile):
    '''
       Add HLT_accepted_events and HLT_rejected_events to outputFile 
    ''' 

    # Open root output file
    hfile = TFile(outputFile, 'UPDATE')
    # Define a new histogram called HLT_rejected_events 
    HLT_rejected_events = TH1F("HLT_rejected_events", "HLT_rejected_events", 3, 0.0, 3.0)
    # Define a new histogram called HLT_accepted_events 
    HLT_accepted_events = TH1F("HLT_accepted_events", "HLT_accepted_events", 3, 0.0, 3.0)
    
    # Fill the histogram if events are rejected by the HLT 
    # HLT_rejected_events are assigned the value 1
    # If all events are accepted the Histogram is empty 
    HLT_rejected_events.Fill(1, rejected)
    
    # Fill the histogram if events are accepted by the HLT 
    # HLT_accepted_events are assigned the value 1
    # If all events are rejected the Histogram is empty 
    HLT_accepted_events.Fill(1, accepted)
    
    # Close output TFile
    hfile.Write("", TFile.kOverwrite)
    hfile.Close()
    return msg.info("Added HLT_accepted_events and HLT_rejeceted_events to %s", outputFile)

