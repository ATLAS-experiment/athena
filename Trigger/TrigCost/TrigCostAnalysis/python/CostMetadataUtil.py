#!/usr/bin/env python
#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

'''
@file CostMetadataUtil.py
@brief Helper functions to create cost metadata json file based on input ntuple
        and histogram under/overflows
'''
import xml.etree.ElementTree as ET

import datetime

from AthenaCommon.Logging import logging
log = logging.getLogger('CostAnalysisPostProcessing')


def saveMetadata(inputFile, argsMetadata={}, processingWarnings=[], doTRPDetails=False, loglevel=3, maxRanges=5):
    ''' @brief Save metadata from ntuple to json file
    '''
    import json

    log.level = loglevel

    metatree = inputFile.Get("metadata")
    if metatree is None:
        return None

    metatree.GetEntry(0)
    metadata = []

    metadata.append({'runNumber' : metatree.runNumber})
    metadata.append({'Details' : argsMetadata["userDetails"]})
    metadata.append({'JIRA' : argsMetadata["jira"]})
    metadata.append({'AMITag' : argsMetadata["amiTag"]})
    if "ProcessedRanges" in metadata:
        metadata.append({'ProcessedRanges' : str(metatree.ProcessedRanges)})

    if argsMetadata["amiTag"]:
        metadata += readHLTConfigKeysFromAMI(argsMetadata["amiTag"])
    else:
        metadata += readHLTConfigKeysFromCOOL(metatree.runNumber)

    if metatree.hostname and argsMetadata["readOKSDetails"]:
        metadata.append({'OKS configuration' : addOKSDetails(str(metatree.hostname), metatree.runNumber, argsMetadata["partition"])})
    elif metatree.hostname:
        metadata.append({'Hostnames' : str(metatree.hostname)})

    metadata.append({'AtlasCostProcessingProject' : str(metatree.AtlasProject)})
    metadata.append({'AtlasCostProcessingVersion' : str(metatree.AtlasVersion)})

    metadata.append({'ChainMonitor' : metatree.ChainMonitor})
    metadata.append({'AlgorithmMonitor' : metatree.AlgorithmMonitor})
    metadata.append({'AlgorithmClassMonitor' : metatree.AlgorithmClassMonitor})
    metadata.append({'ROSMonitor' : metatree.ROSMonitor})
    metadata.append({'GlobalsMonitor' : metatree.GlobalsMonitor})
    metadata.append({'ThreadMonitor' : metatree.ThreadMonitor})

    metadata.append({'AdditionalHashMap' : str(metatree.AdditionalHashMap)})
    metadata.append({'DoEBWeighting' : metatree.DoEBWeighting})
    metadata.append({'BaseEventWeight' : metatree.BaseEventWeight})

    if doTRPDetails:
        # First run with new physics deadtime item https://gitlab.cern.ch/atlas-tdaq-oks/p1/tdaq-10-00-00/-/commit/31c7c6e6b9f3c796c97cf4a61e76818d7da410df
        if metatree.runNumber >= 452028:
            dtItem = "L1_eEM26M--enabled"
        else:
            dtItem = "L1_TAU8--enabled"
        detailsPerLb = readDetailsFromTRP(inputFile, metatree.runNumber, maxRanges, dtItem)
        if detailsPerLb:
            for detail in detailsPerLb["Global"]:
                metadata.append({detail : detailsPerLb["Global"][detail]})
            metadata.append({"LumiblockDetails" : detailsPerLb["PerLb"]})
            if metadata[1]['Details']:
                metadata[1]['Details'] += " "
            else:
                metadata[1]['Details'] = ""
            metadata[1]['Details'] += "Monitored time {4}h: {0} - {1} max <mu> {2} deadtime {3}".format(
                detailsPerLb["Global"]["DataRangeStart"], detailsPerLb["Global"]["DataRangeEnd"], 
                detailsPerLb["Global"]["GlobalMaxPileup"], detailsPerLb["Global"]["GlobalMeanDeadtime"],
                detailsPerLb["Global"]["DataRangeDuration"])
        else:
            log.error("Reading lumiblock details for TRP failed!")

    metadata.append({'Histogram under/overflows' : processingWarnings})

    metadata.append({'HLTMenu' : json.loads(str(metatree.HLTMenu))})

    with open('metadata.json', 'w') as outMetaFile:
        metafile = {}
        metafile['text'] = 'metadata'
        metafile['children'] = metadata
        json.dump(obj=metafile, fp=outMetaFile, indent=2, sort_keys=True)


def createOverflowSummary(warnings):
    ''' @brief Create summery of under/overflows based on passed warnings
    '''
    histogramStats = {}
    log.debug("Received %s warnings", len(warnings))
    for entry in warnings:
        histFullName = entry.split(" ")[-1]
        histType = histFullName.split("_")[-2] + "_" + histFullName.split("_")[-1]
        summary = entry.split(" ")[-1].split("HLT")[0] + "HLT"

        if "LumiBlock" in summary:
            # format LumiBlock_000XX_SummaryName...
            summary = summary.split('_', 1)[1]
            summary = summary.split('_', 1)[1]
        elif "All" in summary:
            # format All_SummaryName...
            summary = summary.split('_', 1)[1]

        entryName = summary + "_" + histType
        if entryName in histogramStats:
            histogramStats[entryName] += 1
        else:
            histogramStats[entryName] = 1

    histogramStatsStr = []
    for name, value in histogramStats.items():
        histogramStatsStr.append("{0}: {1} histograms with over/underflows".format(name, value))

    return {"Summary": histogramStatsStr}


def ignoreUnderflow(histogramName):
    ''' @brief Filter out the histograms to ignore in underflow check
    '''

    # Ignore time histograms for filters
    if "FStep" in histogramName and "Time" in histogramName:
        log.debug("Filter %s underflow will be ignored", histogramName)
        return True

    return False


def addOKSDetails(hostname, runNumber, partition):
    ''' @brief Retrieve additional run metadata from oks repository
    '''
    oksMetadata = []

    # Clone TDAQ repository
    oksTag = "r{0}@{1}".format(runNumber, partition)
    log.info("Cloning tdaq-09-04-00 Tag " + oksTag)
    import os
    os.system("git clone https://gitlab.cern.ch/atlas-tdaq-oks/p1/tdaq-09-04-00.git --branch " + oksTag + " --single-branch")

    # Browse OKS    
    try:
        if partition == "TDAQ":
            partitionRoot = ET.parse('tdaq-09-04-00/daq/partitions/TDAQ.data.xml').getroot()
            hltRoot = ET.parse('tdaq-09-04-00/daq/segments/HLT/HLT-TDAQ.data.xml').getroot()
        elif partition == "ATLAS":
            partitionRoot = ET.parse('tdaq-09-04-00/combined/partitions/ATLAS.data.xml').getroot()
            hltRoot = ET.parse('tdaq-09-04-00/daq/segments/HLT/HLT-internal.data.xml').getroot()
    except FileNotFoundError as e:
        log.warning("OKS files not available: {0}".format(e))
        return []


    # Read F/T/S
    racksToComp = dict()
    if 'pc' in hostname:
        # Convert computer names to set of racks
        for computerName in hostname.split(","):
            rackName = findRackForComputer(computerName)
            if rackName not in racksToComp:
                racksToComp[rackName] = list()
            racksToComp[rackName].append(computerName)

        hostname = ",".join(racksToComp.keys())

    for rack in hostname.split(","):
        hltApplication = findHLTApplication(partitionRoot, hltRoot, rack, partition)

        metadataDict = [{'Hostname' : rack},                     
                        {'Forks' : hltRoot.findall("./*[@id='{0}']/*[@name='numForks']".format(hltApplication))[0].get("val")},
                        {'Threads' : hltRoot.findall("./*[@id='{0}']/*[@name='numberOfAthenaMTThreads']".format(hltApplication))[0].get("val")},
                        {'Slots' : hltRoot.findall("./*[@id='{0}']/*[@name='numberOfEventSlots']".format(hltApplication))[0].get("val")}]

        if rack in racksToComp:
            metadataDict.append({'Computers' : str(racksToComp[rack])})

        oksMetadata.append(metadataDict)

    # Cleanup cloned repository
    os.system("rm -rf tdaq-09-04-00")
    return oksMetadata


def findHLTApplication(partitionRoot, hltRoot, hostname, partitionName="ATLAS"):
    ''' @brief Find HLT application based on hostname and disabled segments
    '''
    segments = []

    if hostname:
        # Find segment based on hostname
        for segment in hltRoot.findall("./*[@class='TemplateSegment']/*[@name='Racks']/*[@id='{0}'].../...".format(hostname)):
            segments.append(segment.get("id"))
    else:
        # Get available segments
        segments = []
        for segment in hltRoot.findall("./*[@class='Segment']/*[@name='Segments']/*[@class='TemplateSegment']"):
            segments.append(segment.get("id"))

    log.debug("Found segments {0}".format(segments))

    # disabled segments
    for segment in partitionRoot.findall("./*[@class='Partition']/*[@name='Disabled']/*[@class='TemplateSegment']"):
        if segment.get("id") in segments:
            segments.remove(segment.get("id"))

    if len(segments) > 1:
        log.warning("Found more than one enabled segment, will use {0}".format(segments[0]))

    return hltRoot.findall("./*[@id='{0}']/*[@name='Applications']/*[@class='HLTRCApplication']".format(segments[0]))[0].get("id")


def findRackForComputer(computerName):
    ''' Find rack for computer name '''

    import re
    m = re.search(r'pc-tdq-tpu-(.*?)\.cern\.ch', computerName)
    if m:
        computerNum = m.group(1)
        rackNumber = computerNum[0:2]
        return "tpu-rack-{0}".format(rackNumber)

    log.warning("Cannot retrieve rack number from {0}".format(computerName))
    return ""


def readHLTConfigKeysFromCOOL(runNumber):
    ''' 
    Returns list of config keys read from COOL:
        DB - database alias
        Release - release
        SMK - Super Master Key
        HLTPSK - HLT Prescale keys
        LVL1PSK - L1 Prescale keys
    '''

    configMetadata = []

    from TrigConfStorage.TriggerCoolUtil import TriggerCoolUtil
    dbconn = TriggerCoolUtil.GetConnection("CONDBR2")
    configKeys = TriggerCoolUtil.getHLTConfigKeys(dbconn, [[runNumber, runNumber]])

    if configKeys and runNumber in configKeys.keys():
        configKeys = configKeys[runNumber]

        configMetadata.append({'DB' : configKeys['DB']})
        configMetadata.append({'Release' : configKeys['REL']})
        configMetadata.append({'SMK' : configKeys['SMK']})
        
        configMetadata.append({'HLTPSK' : str(TriggerCoolUtil.getHLTPrescaleKeys(dbconn, [[runNumber, runNumber]])[runNumber]['HLTPSK2'])})
        configMetadata.append({'LVL1PSK' : str(TriggerCoolUtil.getL1ConfigKeys(dbconn, [[runNumber, runNumber]])[runNumber]['LVL1PSK'])})

    else:
        log.warning("Config keys not found in COOL")

    return configMetadata



def readHLTConfigKeysFromAMI(amiTag):
    ''' 
    Returns list of config keys read from AMI Tag:
        DB - database alias
        Release - release
        SMK - Super Master Key
        HLTPSK - HLT Prescale keys
        LVL1PSK - L1 Prescale keys
    '''

    configMetadata = []

    try:
        import pyAMI.client
        import pyAMI.atlas.api as AtlasAPI
    except ModuleNotFoundError:
        log.warning("Unable to import AMIClient from pyAMI. Maybe you didn't do localSetupPyAMI?")
        return configMetadata

    amiclient = pyAMI.client.Client('atlas')
    AtlasAPI.init()

    command = [ 'AMIGetAMITagInfo', '-amiTag="%s"' % amiTag, '-cached' ]
    amiTagInfo = amiclient.execute(command, format = 'dict_object').get_rows('amiTagInfo')[0]

    configMetadata.append({'Release' : amiTagInfo['SWReleaseCache']})
    configMetadata.append({'SMK' : amiTagInfo['DBsmkey'] if "DBsmkey" in amiTagInfo else None}) 
    configMetadata.append({'DB' : amiTagInfo['DBserver'] if "DBserver" in amiTagInfo else None})
    configMetadata.append({'HLTPSK' : amiTagInfo['DBhltpskey'] if "DBhltpskey" in amiTagInfo else None})
    configMetadata.append({'LVL1PSK' : amiTagInfo['DBl1pskey'] if "DBl1pskey" in amiTagInfo else None})

    return configMetadata


def readDetailsFromTRP(inputFile, runNumber, maxRanges, itemName="L1_eEM26M--enabled", server="https://atlasop.cern.ch"):
    log.info("Reading run details from TRP")

    import ROOT

    lumiBlockDict = {} # Mapping of range to lumiblocks in the range

    for timeRange in inputFile.GetListOfKeys():
        rangeObj = timeRange.ReadObj()
        if not rangeObj.IsA().InheritsFrom(ROOT.TDirectory.Class()): continue # Skip metadata TTree
        rangeName = rangeObj.GetName()

        for table in rangeObj.GetListOfKeys():
            tableObj = table.ReadObj()
            if "Global" not in tableObj.GetName(): continue # Find Global summary

            dirKey = set(key.ReadObj().GetName() for key in tableObj.GetListOfKeys() if key.ReadObj().GetName().startswith('LumiBlock'))
            lumiBlockDict[rangeName] = sorted(dirKey)

    if not lumiBlockDict:
        log.error("No lumiblocks were found in the input file")
        return {}

    # Read start and stop timestamps for lumiblock ranges
    from DQUtils.sugar import RunLumi
    from time import ctime
    from PyCool import cool
    from TrigConfStorage.TriggerCoolUtil import TriggerCoolUtil
    dbconn = TriggerCoolUtil.GetConnection("CONDBR2")

    lbRangeTsDict = {} # Timestamps for lumiblock range

    f = dbconn.getFolder( "/TRIGGER/LUMI/LBLB" )
    for lbRange in lumiBlockDict:
        startLb = int(min(lumiBlockDict[lbRange]).replace('LumiBlock_', ''))
        endLb = int(max(lumiBlockDict[lbRange]).replace('LumiBlock_', ''))
        log.debug("For range {0} first lumiblock is {1} and last {2}".format(lbRange, startLb, endLb))

        since = RunLumi(runNumber, startLb)
        until = RunLumi(runNumber, endLb)

        objs = f.browseObjects(since, until, cool.ChannelSelection(0))
        objs.goToNext()
        objCurrRef = objs.currentRef()
        startTime = int(objCurrRef.payload()["StartTime"]/1000)

        while objs.goToNext():
            objCurrRef = objs.currentRef()

        endTime = int(objCurrRef.payload()["EndTime"]/1000)

        lbRangeTsDict[lbRange] = {"start": startTime, "end" : endTime}

        log.debug("Read start and end of range {0} from COOL: {1} - {2}".format(lbRange, ctime(startTime/1E6).replace(' ','_'), ctime(endTime/1E6).replace(' ','_')))


    # Read details from PBeast
    lbRangeDetailsDict = {}
    physicsDeadtimeGlobal = []
    pileupGlobal = []
    try:
        import libpbeastpy
        pbeast = libpbeastpy.ServerProxy(server)

        for lbRange in lbRangeTsDict:
            lbStart = lbRangeTsDict[lbRange]["start"]
            lbEnd = lbRangeTsDict[lbRange]["end"]

            # Deadtime
            physicsDeadtimeTRP = pbeast.get_data('ATLAS', 'L1_Rate', 'DT', 'ISS_TRP.' + itemName, False, lbStart, lbEnd, 0, True)
            
            if len(physicsDeadtimeTRP) == 0:
                log.error("Deadtime not found for item {0} for range {1}".format(itemName, lbRange))
                physicsDeadtimeAvg = -1
            else: 
                physicsDeadtimeTRP = physicsDeadtimeTRP[0].data['ISS_TRP.' + itemName]
                physicsDeadtimeArray = []
                for entry in physicsDeadtimeTRP:
                    # Read only values between timestamps - pbeast returns one timestamp earlier and one later
                    if entry.ts < lbStart or entry.ts > lbEnd:
                        continue
                    if type(entry.value) is not float: # None type
                        continue

                    physicsDeadtimeArray.append(entry.value)
                    physicsDeadtimeGlobal.append(entry.value)
                        
                physicsDeadtimeAvg = sum(physicsDeadtimeArray)/len(physicsDeadtimeArray) if len(physicsDeadtimeArray) > 0 else 1.

            # Pileup
            pileupPbeast = pbeast.get_data('OLC', 'OCLumi', 'Mu', 'OLC.OLCApp/ATLAS_PREFERRED_LBAv_PHYS', False, lbStart, lbEnd)[0].data['OLC.OLCApp/ATLAS_PREFERRED_LBAv_PHYS']
            pileupArr = []
            for entry in pileupPbeast:
                if entry.ts < lbStart or entry.ts > lbEnd:
                    continue
                if type(entry.value) is not float: # None type
                    continue

                pileupArr.append(entry.value)
                pileupGlobal.append(entry.value)

            pileupAvg = sum(pileupArr)/len(pileupArr) if len(pileupArr) > 0 else -1
            lbRangeDetailsDict[lbRange] = {"avgPileup" : round(pileupAvg, 3), "minPileup" : round(min(pileupArr), 3), 
                                           "maxPileup" : round(max(pileupArr), 3), "deadtime" : round(physicsDeadtimeAvg, 3)}

    except ImportError as e:
        log.error("The pbeast python library was not found! Remember to setup tdaq release!")
        log.debug(e)
        return {}
    except RuntimeError as e:
        if "Sign in to your account" in str(e):
            log.error("PBeast authentication failed! Remember to export pbeast server sso: export PBEAST_SERVER_SSO_SETUP_TYPE=AutoUpdateKerberos")
        elif "cannot create CERN SSO cookie" in str(e):
            log.error("PBeast authentication requires the cookies, please setup")
        else:
            log.error("Error when reading from Pbeast! ")
        log.debug(e)
        return {}

    log.debug("The final lumiblock dictionary is {0}".format(lbRangeDetailsDict))

    physicsDeadtimeGlobalAvg = sum(physicsDeadtimeGlobal)/len(physicsDeadtimeGlobal) if len(physicsDeadtimeGlobal) > 0 else 1.
    pileupGlobalAvg = sum(pileupGlobal)/len(pileupGlobal) if len(pileupGlobal) > 0 else 1.

    startTs = lbRangeTsDict[min(lbRangeTsDict.keys())]["start"]/1E6
    endTs = lbRangeTsDict[max(lbRangeTsDict.keys())]["end"]/1E6
    monitoredTime = datetime.timedelta(seconds=(int(endTs - startTs)))
    additionalDetails = {
        "DataRangeStart" : ctime(startTs),
        "DataRangeEnd" : ctime(endTs),
        "DataRangeDuration" : "{0}:{1}".format(int(monitoredTime.total_seconds()//3600), int((monitoredTime.total_seconds()%3600)//60)),
        "GlobalMeanPileup" : round(pileupGlobalAvg, 3),
        "GlobalMinPileup" : round(min(pileupGlobal), 3),
        "GlobalMaxPileup" : round(max(pileupGlobal), 3),
        "GlobalMeanDeadtime" : round(physicsDeadtimeGlobalAvg, 3)
    }

    return {"Global" : additionalDetails, "PerLb" : lbRangeDetailsDict}
