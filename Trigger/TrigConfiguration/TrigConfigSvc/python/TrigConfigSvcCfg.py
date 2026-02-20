# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from typing import Any, cast
from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from TrigConfStorage.TriggerCrestUtil import TriggerCrestUtil

from functools import cache
import json

log = logging.getLogger('TrigConfigSvcCfg')

# To avoid accidental overrwrite of the L1 JSON menu, only allow one L1 menu generation.
# Either via JSON conversion from Run-1&2 or native Run-3 (see ATR-24531).
def l1menu_generated():
    try:
        return l1menu_generated._hasRun # type: ignore
    except AttributeError:
        l1menu_generated._hasRun = True # type: ignore
        return False


def getTrigConfFromConditions(runNumber, lumiBlock, flags) -> dict[str, int | str]:
    if flags.Trigger.useCrest:
        return getTrigConfFromCrest(runNumber, lumiBlock, flags.Trigger.crestServer)
    else:
        return getTrigConfFromCool(runNumber, lumiBlock)

@cache
def getTrigConfFromCrest(runNumber, lumiBlock, crestServer) -> dict[str, int | str]:
    trigConf = TriggerCrestUtil.getTrigConfKeys(runNumber, lumiBlock, server=crestServer)
    log.info("Extracted the following info for run %d and lumi block %d from CREST: %r",
             runNumber, lumiBlock, trigConf)
    for key, value in trigConf.items():
        if value is None:
            msg: str = f"Did not find {key} for run {runNumber} and lumi block {lumiBlock}"
            log.error(msg)
            raise RuntimeError(msg)
    return trigConf

@cache
def getTrigConfFromCool(runNumber, lumiBlock) -> dict[str, int | str]:
    from TrigConfStorage.TriggerCoolUtil import TriggerCoolUtil
    trigConf = TriggerCoolUtil.getTrigConfKeys(runNumber, lumiBlock)
    log.info("Extracted the following info for run %d and lumi block %d from COOL: %r",
             runNumber, lumiBlock, trigConf)
    for key, value in trigConf.items():
        if value is None:
            msg: str = f"Did not find {key} for run {runNumber} and lumi block {lumiBlock}"
            log.error(msg)
            raise RuntimeError(msg)
    return trigConf

def createJsonMenuFiles(run, lb, flags):
    crestServer: str | None = flags.Trigger.crestServer if flags.Trigger.useCrest else None
    return _createJsonMenuFiles(run, lb, crestServer)

@cache
def _createJsonMenuFiles(run, lb, crestServer: str | None = None) -> dict[str, int | str]:
    """Retrieve Run-2 trigger configuration from the DB and save as Run3 .JSON files
    returns the trigger DB keys used, or None if the L1 menu has already been generated
    """
    import subprocess

    if l1menu_generated():
        log.error("L1 menu has already been generated")
        return None  # type: ignore

    log.info("Configuring Run-1&2 to Run-3 configuration metadata conversion")
    if crestServer is not None:
        triggerDBKeys = getTrigConfFromCrest(run, lb, crestServer)
    else:
        triggerDBKeys = getTrigConfFromCool(run, lb)
    triggerDBKeys['DB'] = 'TRIGGERDB' if run > 230000 else 'TRIGGERDB_RUN1'

    cmd = "TrigConfReadWrite -i {DB} {SMK},{LVL1PSK},{HLTPSK},{BGSK} -o r3json > Run3ConfigFetchJSONFiles.log".format(**triggerDBKeys)
    log.info("Running command '%s'", cmd)
    filesFetchStatus = subprocess.run(cmd, shell=True)
    assert filesFetchStatus.returncode == 0, "TrigConfReadWrite failed to fetch JSON files"
    return triggerDBKeys


# This interprets the Trigger.triggerConfig flag according to
# https://twiki.cern.ch/twiki/bin/view/Atlas/TriggerConfigFlag#triggerConfig_in_Run_3
def getTrigConfigFromFlag( flags ):
    flags.dump("Input", evaluate=True)
    # run and lb are only needed if source is DB
    run: int = flags.Input.RunNumbers[0] if flags.Input.RunNumbers else -1
    lb: int = flags.Input.LumiBlockNumbers[0] if flags.Input.LumiBlockNumbers else 0
    return _getTrigConfigFromFlag(triggerConfig=flags.Trigger.triggerConfig, run=run, lb=lb,
                                  useCrest=flags.Trigger.useCrest, crestServer=flags.Trigger.crestServer)

@cache
def _getTrigConfigFromFlag(*, triggerConfig, run, lb, useCrest, crestServer) -> dict[str, Any]:
    log.info("Parsing trigger configuration from flag Trigger.triggerConfig='%s' for run=%d and lb=%d", 
             triggerConfig, run, lb)
    log.info("Crest usage flags are: Trigger.useCrest=%s, Trigger.crestServer=%s", useCrest, crestServer)
    # Pad the triggerConfig value and extract available fields:
    dbconn: str
    source, dbconn, keys = (triggerConfig+":::").split(":")[:3]
    smk,l1psk,hltpsk,bgsk = (keys+",,,").split(",")[:4]
    # Convert to int or None:
    smk, l1psk, hltpsk, bgsk = (int(k) if k!="" else None for k in (smk, l1psk, hltpsk, bgsk))
    source: str = source.upper()

    if source == "DB":
        if run < 0:
            msg: str = "Run number is required to extract trigger conditions"
            log.error(msg)
            raise RuntimeError(msg)
        # If any of the keys or DB connection is missing, retrieve from conditions:
        if useCrest:
            trigConf: dict[str, int | str] = getTrigConfFromCrest(run, lb, crestServer)
        else:
            trigConf: dict[str, int | str] = getTrigConfFromCool(run, lb)
        if dbconn == "":
            dbconn = cast(str, trigConf["DB"])
            
        if dbconn in ["TRIGGERDB_RUN3", "TRIGGERDBDEV1_I8", "TRIGGERDBDEV1", "TRIGGERDBDEV2"]:
            if smk is None:
                smk = trigConf["SMK"]
            if l1psk is None:
                l1psk = trigConf['LVL1PSK']
            if hltpsk is None:
                hltpsk = trigConf['HLTPSK']
            if bgsk is None:
                bgsk = trigConf['BGSK']

        if useCrest:
            # need to modify the DB connection alias (e.g. TRIGGERDB_RUN3) to the corresponding CREST server URL
            crestConn = TriggerCrestUtil.getCrestConnection(dbconn)
            if crestConn is None:
                msg: str = f"Could not find CREST triggerdb connection from DB connection alias '{dbconn}'"
                log.error(msg)
                raise RuntimeError(msg)
            dbconn = f"{crestServer}/{crestConn}"

    tcdict = {
        "SOURCE" : source,  # DB, FILE, COOL
        "DBCONN" : dbconn, # db connection (if origin==DB or COOL) or "JOSVC" if connection is to be taken from TrigConf::IJobOptionsSvc 
        "SMK"    : smk,
        "LVL1PSK": l1psk,
        "HLTPSK" : hltpsk,
        "BGSK"   : bgsk
    }
    return tcdict


def getL1PrescaleFolderName():
    return "/TRIGGER/LVL1/Lvl1ConfigKey <tag>HEAD</tag>"


def getHLTPrescaleFolderName():
    return "/TRIGGER/HLT/PrescaleKey <tag>HEAD</tag>"


def _doMenuConversion(flags):
    """Do JSON menu conversion for Run-1&2 data"""
    return flags.Input.Files and flags.Trigger.EDMVersion in [1, 2] and not flags.Input.isMC


def _getMenuFileName(flags):
    """Return base name for menu files"""
    if not _doMenuConversion(flags):  # menu created in this release
        from PyUtils.Helpers import release_metadata
        return '_'+flags.Trigger.triggerMenuSetup+'_'+release_metadata()['release']
    else:  # menu files created via JSON conversion
        return ''

# L1 Json file name 
def getL1MenuFileName(flags):
    return 'L1Menu'+_getMenuFileName(flags)+'.json'

# HLT Json file name 
def getHLTMenuFileName( flags ):
    return 'HLTMenu'+_getMenuFileName(flags)+'.json'

# HLT Monitoring set json file name
def getHLTMonitoringFileName( flags ):
    return 'HLTMonitoring'+_getMenuFileName(flags)+'.json'

# L1 Prescales set json file name
def getL1PrescalesSetFileName( flags ):
    return 'L1PrescalesSet'+_getMenuFileName(flags)+'.json'

# HLT Prescales set json file name
def getHLTPrescalesSetFileName( flags ):
    return 'HLTPrescalesSet'+_getMenuFileName(flags)+'.json'

# L1 Bunchgroups set json file name
def getBunchGroupSetFileName( flags ):
    return 'BunchGroupSet'+_getMenuFileName(flags)+'.json'

# HLT Job options json file name
def getHLTJobOptionsFileName( ):
    return 'HLTJobOptions.json'

# Creates an L1 Prescale file from the menu
def createL1PrescalesFileFromMenu(flags, prescales: dict[str, float] | None = None):
    from TriggerMenuMT.L1.Base.PrescaleHelper import getCutFromPrescale

    menuFN = getL1MenuFileName(flags)
    with open(menuFN,'r') as fh:
        data = json.load(fh)
        pso = { 'filetype': 'l1prescale',
                'name': data['name'],
                'cutValues': {} }
        ps = pso['cutValues']
        for name in sorted(data['items'].keys()):
            ps = prescales[name] if prescales and name in prescales else 1
            pso['cutValues'][name] = {
                'cut': getCutFromPrescale(ps),
                'enabled': ps > 0,
                'info': f'prescale: {ps}'
            }

    psFN = getL1PrescalesSetFileName( flags )
    with open(psFN, 'w') as outfile:
        json.dump(pso, outfile, indent = 4)
        log.info("Generated default L1 prescale set %s", outfile.name)

# L1 menu generation
def generateL1Menu( flags ):
    if l1menu_generated():
        log.error("L1 menu has already been generated")
        return

    log.info("Generating L1 menu %s", flags.Trigger.triggerMenuSetup)
    from TriggerMenuMT.L1.L1MenuConfig import L1MenuConfig
    l1cfg = L1MenuConfig(flags)
    l1cfg.writeJSON(outputFile    = getL1MenuFileName(flags),
                    bgsOutputFile = getBunchGroupSetFileName(flags))


# provide L1 config service in new JO
@AccumulatorCache
def L1ConfigSvcCfg( flags ):
    log.info( "Setting up LVL1ConfigSvc" )
    acc = ComponentAccumulator()

    cfg = getTrigConfigFromFlag( flags )

    # configure config svc
    l1ConfigSvc = CompFactory.getComp("TrigConf::LVL1ConfigSvc")("LVL1ConfigSvc")  # type: ignore

    if cfg["SOURCE"] == "FILE":
        if _doMenuConversion(flags):
            # Save the menu in JSON format
            dbKeys = createJsonMenuFiles(run = flags.Input.RunNumbers[0],
                                         lb = flags.Input.LumiBlockNumbers[0], flags=flags)
            l1ConfigSvc.SMK = dbKeys['SMK']

        l1ConfigSvc.InputType = "FILE"
        l1ConfigSvc.L1JsonFileName = getL1MenuFileName(flags)
        l1ConfigSvc.HLTJsonFileName = getHLTMenuFileName(flags)
        log.info( "Configured LVL1ConfigSvc with InputType='FILE', L1JsonFileName=%s (and HLT, used to compute SMK:%s) ", l1ConfigSvc.L1JsonFileName, l1ConfigSvc.HLTJsonFileName )
    elif cfg["SOURCE"] == "DB":
        l1ConfigSvc.InputType = "DB"
        l1ConfigSvc.L1JsonFileName = ""
        l1ConfigSvc.HLTJsonFileName = ""
        l1ConfigSvc.TriggerDB = cfg["DBCONN"]
        l1ConfigSvc.SMK = cfg["SMK"]
        log.info( "Configured LVL1ConfigSvc with InputType='DB', TriggerDB='%s' and SMK %d", l1ConfigSvc.TriggerDB, cfg['SMK'] )

    acc.addService( l1ConfigSvc, create=True )
    return acc

# provide HLT config service in new JO
@AccumulatorCache
def HLTConfigSvcCfg( flags ):
    log.info( "Setting up HLTConfigSvc" )
    acc = ComponentAccumulator()
    cfg = getTrigConfigFromFlag( flags )

    hltConfigSvc = CompFactory.getComp("TrigConf::HLTConfigSvc")("HLTConfigSvc")  # type: ignore

    if cfg["SOURCE"] == "FILE":
        if _doMenuConversion(flags):
            # Save the menu in JSON format
            dbKeys = createJsonMenuFiles(run = flags.Input.RunNumbers[0],
                                         lb = flags.Input.LumiBlockNumbers[0], flags=flags)
            hltConfigSvc.SMK = dbKeys['SMK']

        hltConfigSvc.InputType = "FILE"
        hltConfigSvc.L1JsonFileName = getL1MenuFileName( flags )
        hltConfigSvc.HLTJsonFileName = getHLTMenuFileName( flags )
        hltConfigSvc.MonitoringJsonFileName = getHLTMonitoringFileName( flags )
        log.info("Configured HLTConfigSvc with InputType='FILE', HLTJsonFileName=%s and MonitoringJsonFileName=%s (and L1, used to compute MC-SMK:%s)",
                 hltConfigSvc.HLTJsonFileName, hltConfigSvc.MonitoringJsonFileName, hltConfigSvc.L1JsonFileName)
    elif cfg["SOURCE"] == "DB":
        hltConfigSvc.InputType = "DB"
        hltConfigSvc.L1JsonFileName = ""
        hltConfigSvc.HLTJsonFileName = ""
        hltConfigSvc.MonitoringJsonFileName = ""
        hltConfigSvc.TriggerDB = cfg["DBCONN"]
        hltConfigSvc.SMK = cfg["SMK"]
        log.info("Configured HLTConfigSvc with InputType='DB', TriggerDB='%s' and SMK %d", hltConfigSvc.TriggerDB, cfg['SMK'])
    acc.addService( hltConfigSvc, create=True )
    return acc

# provide both services in new JO
def TrigConfigSvcCfg( flags ):
    acc = ComponentAccumulator()
    acc.merge( BunchGroupCondAlgCfg( flags ) )
    acc.merge( L1ConfigSvcCfg( flags ) )
    acc.merge( HLTConfigSvcCfg( flags ) )
    acc.merge( L1PrescaleCondAlgCfg( flags ) )
    acc.merge( HLTPrescaleCondAlgCfg( flags ) )
    return acc

@AccumulatorCache
def L1PrescaleCondAlgCfg( flags ):
    log.info("Setting up L1PrescaleCondAlg")
    acc = ComponentAccumulator()
    TrigConf__L1PrescaleCondAlg = CompFactory.getComp("TrigConf::L1PrescaleCondAlg")
    l1PrescaleCondAlg = TrigConf__L1PrescaleCondAlg("L1PrescaleCondAlg")  # type: ignore

    tc = getTrigConfigFromFlag( flags )
    l1PrescaleCondAlg.Source = tc["SOURCE"]
    if flags.Common.isOnline:
        from IOVDbSvc.IOVDbSvcConfig import addFolders
        acc.merge(addFolders(flags, getL1PrescaleFolderName(), "TRIGGER_ONL", className="AthenaAttributeList"))
        log.info("Adding folder %s to CompAcc", getL1PrescaleFolderName() )
    if tc["SOURCE"] == "COOL":
        l1PrescaleCondAlg.TriggerDB = tc["DBCONN"]
    elif tc["SOURCE"] == "DB":
        l1PrescaleCondAlg.TriggerDB = tc["DBCONN"]
        l1PrescaleCondAlg.L1Psk    = tc["LVL1PSK"]
        log.info("Configured L1PrescaleCondAlg with InputType='DB', TriggerDB='%s' and L1Psk %d", 
                 l1PrescaleCondAlg.TriggerDB, l1PrescaleCondAlg.L1Psk)
    elif tc["SOURCE"] == "FILE":
        l1PrescaleCondAlg.Filename = getL1PrescalesSetFileName( flags )
        if _doMenuConversion(flags):
            # Save the menu in JSON format
            dbKeys = createJsonMenuFiles(run = flags.Input.RunNumbers[0],
                                         lb = flags.Input.LumiBlockNumbers[0], flags=flags)
            l1PrescaleCondAlg.L1Psk = dbKeys['LVL1PSK']
    else:
        raise RuntimeError("trigger configuration flag 'trigConfig' starts with %s, which is not understood" % tc["SOURCE"])
    acc.addCondAlgo(l1PrescaleCondAlg)
    return acc

@AccumulatorCache
def BunchGroupCondAlgCfg( flags ):
    log.info("Setting up BunchGroupCondAlg")
    acc = ComponentAccumulator()
    TrigConf__BunchGroupCondAlg = CompFactory.getComp("TrigConf::BunchGroupCondAlg")
    bunchGroupCondAlg = TrigConf__BunchGroupCondAlg("TrigConf__BunchGroupCondAlg")  # type: ignore

    tc = getTrigConfigFromFlag( flags )
    bunchGroupCondAlg.Source = tc["SOURCE"]
    if tc["SOURCE"] == "COOL":
        bunchGroupCondAlg.TriggerDB = tc["DBCONN"]
    elif tc["SOURCE"] == "DB":
        bunchGroupCondAlg.TriggerDB = tc["DBCONN"]
        bunchGroupCondAlg.BGSK    = tc["BGSK"]
        log.info("Configured BunchGroupCondAlg with InputType='DB', TriggerDB='%s' and BGSK %d", 
                 bunchGroupCondAlg.TriggerDB, bunchGroupCondAlg.BGSK)
    elif tc["SOURCE"] == "FILE":
        bunchGroupCondAlg.Filename = getBunchGroupSetFileName( flags )
        if _doMenuConversion(flags):
            # Save the menu in JSON format
            dbKeys = createJsonMenuFiles(run = flags.Input.RunNumbers[0],
                                         lb = flags.Input.LumiBlockNumbers[0], flags=flags)
            bunchGroupCondAlg.BGSK = dbKeys['BGSK']
    else:
        raise RuntimeError("trigger configuration flag 'trigConfig' starts with %s, which is not understood" % tc["SOURCE"])
    acc.addCondAlgo(bunchGroupCondAlg)
    return acc

@AccumulatorCache
def HLTPrescaleCondAlgCfg( flags ):
    log.info("Setting up HLTPrescaleCondAlg")
    acc = ComponentAccumulator()
    hltPrescaleCondAlg = CompFactory.getComp("TrigConf::HLTPrescaleCondAlg")("HLTPrescaleCondAlg")  # type: ignore

    tc = getTrigConfigFromFlag( flags )
    hltPrescaleCondAlg.Source = tc["SOURCE"]
    if flags.Common.isOnline or tc["SOURCE"]=="COOL":
        from IOVDbSvc.IOVDbSvcConfig import addFolders
        acc.merge(addFolders(flags, getHLTPrescaleFolderName(), "TRIGGER_ONL",
                             className = "AthenaAttributeList",
                             extensible = flags.Trigger.Online.isPartition))
        log.info("Adding folder %s to CompAcc", getHLTPrescaleFolderName() )
    if tc["SOURCE"] == "COOL":
        hltPrescaleCondAlg.TriggerDB = tc["DBCONN"]
    elif tc["SOURCE"] == "DB":
        hltPrescaleCondAlg.TriggerDB = tc["DBCONN"]
        hltPrescaleCondAlg.HLTPsk    = tc["HLTPSK"]
        log.info("Configured HLTPrescaleCondAlg with InputType='DB', TriggerDB='%s' and HLTPsk %d", 
                 hltPrescaleCondAlg.TriggerDB, hltPrescaleCondAlg.HLTPsk)
    elif tc["SOURCE"] == "FILE":
        hltPrescaleCondAlg.Filename = getHLTPrescalesSetFileName( flags )
        if _doMenuConversion(flags):
            # Save the menu in JSON format
            dbKeys = createJsonMenuFiles(run = flags.Input.RunNumbers[0],
                                         lb = flags.Input.LumiBlockNumbers[0], flags=flags)
            hltPrescaleCondAlg.HLTPsk = dbKeys['HLTPSK']
    else:
        raise RuntimeError("trigger configuration flag 'trigConfig' starts with %s, which is not understood" % tc["SOURCE"])
    acc.addCondAlgo(hltPrescaleCondAlg)
    return acc


if __name__ == "__main__":
    import unittest

    class Tests(unittest.TestCase):

        def setUp(self):
            # Allow multiple L1 menu generations for these tests
            l1menu_generated._hasRun = False # type: ignore

        def test_currentMenu(self):
            from AthenaConfiguration.AllConfigFlags import initConfigFlags
            flags = initConfigFlags()
            flags.Trigger.EDMVersion = 3
            from AthenaConfiguration.TestDefaults import defaultTestFiles
            flags.Input.Files = defaultTestFiles.RAW_RUN2
            flags.lock()
            TrigConfigSvcCfg( flags )

        def test_legacyMenu(self):
            from AthenaConfiguration.AllConfigFlags import initConfigFlags
            flags = initConfigFlags()
            from AthenaConfiguration.TestDefaults import defaultTestFiles
            flags.Input.Files = defaultTestFiles.RAW_RUN2
            flags.lock()
            TrigConfigSvcCfg( flags )

        def test_jsonConverter(self):
            keys = _createJsonMenuFiles(run=360026, lb=151, crestServer=None)
            assert keys is not None, "No keys returned"
            for k,v in {"SMK" : 2749, "LVL1PSK" : 23557, "HLTPSK" : 17824, "BGSK" : 2181}.items():
                assert  k in keys, "Missing key {}".format(k)
                assert v == keys[k], "Wrong value {}".format(v)

    unittest.main(verbosity=2)
