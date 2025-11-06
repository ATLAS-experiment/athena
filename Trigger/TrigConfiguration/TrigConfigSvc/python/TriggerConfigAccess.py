# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from typing import Any, Optional
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaCommon.Logging import logging
log = logging.getLogger( "TriggerConfigAccess.py" )

from .TrigConfigSvcCfg import getTrigConfigFromFlag, getL1MenuFileName, getHLTMenuFileName, getL1PrescalesSetFileName, getHLTPrescalesSetFileName, getBunchGroupSetFileName, getHLTJobOptionsFileName, getHLTMonitoringFileName

from TrigConfIO.L1TriggerConfigAccess import L1MenuAccess, L1PrescalesSetAccess, BunchGroupSetAccess
from TrigConfIO.HLTTriggerConfigAccess import HLTMenuAccess, HLTPrescalesSetAccess, HLTJobOptionsAccess, HLTMonitoringAccess

from AthenaConfiguration.AutoConfigFlags import GetFileMD
from AthenaConfiguration.Enums import Format

from functools import lru_cache

"""
Access to the trigger configuration in python is provided depending on
the trigger configuration source

The tc source is taken from the TriggerFlag triggerConfig

1) tc source is set to INFILE

This is usually the case when running on ESD, AOD, and dAOD files and
only in this case. An exception is RDO with trigger information in
MC. The menu and prescales are taken from the pool file, from the
in-file metadata.


2) tc source is set to FILE

This is the case when executing the trigger with the configuration
taken from file.  The filename is provided by the function
@getL1MenuFileName

3) tc source is set to DB

This is the case when executing the trigger from the DB. The DB
connection and keys are provided by the triggerConfig flag

4) tc source is COOL

This is the case when reconstructing the data. From COOL the
configuration keys and db alias are taken, the configurations
are then loaded from the DB.

"""

@lru_cache(maxsize=None)
def getKeysFromCool(runNr: int, lbNr: int = 0) -> dict[str,int]:
    """Return dictionary of trigger keys for given run and lumiblock number
    """
    from TrigConfStorage.TriggerCoolUtil import TriggerCoolUtil
    condb = "CONDBR2" if runNr > 236108 else "COMP200"
    db = TriggerCoolUtil.GetConnection(condb)
    run_range = [[runNr,runNr]]
    d = { k: TriggerCoolUtil.getHLTConfigKeys(db, run_range)[runNr][k] for k in ['SMK', 'DB'] }
    for ( key, lbfirst, lblast) in TriggerCoolUtil.getBunchGroupKey(db, run_range)[runNr]['BGKey']:
        if lbNr>=lbfirst and (lbNr<=lblast or lblast==-1):
            d['BGSK'] = key
            break
    for ( key, lbfirst, lblast) in TriggerCoolUtil.getL1ConfigKeys(db, run_range)[runNr]['LVL1PSK']:
        if lbNr>=lbfirst and (lbNr<=lblast or lblast==-1):
            d['L1PSK'] = key
            break
    for ( key, lbfirst, lblast) in TriggerCoolUtil.getHLTPrescaleKeys(db, run_range)[runNr]['HLTPSK2']:
        if lbNr>=lbfirst and (lbNr<=lblast or lblast==-1):
            d['HLTPSK'] = key
            break

    # dbalias mapping
    dbaliasMapping = { "TRIGGERDBR2R" : "TRIGGERDB",
                       "TRIGGERDBV2" : "TRIGGERDB_RUN1" }
    if d["DB"] in dbaliasMapping:
        d["DB"] = dbaliasMapping[ d["DB"] ]

    return d

def getDBKeysFromMetadata(flags) -> Optional[dict[str, Any]]:
    """Provides access to the database keys from the in-file metadata

    Gets the database keys from the in-file metadata which are stored together with the json representation
    If the keys are in the file, then usually SMK, L1PSK and HLTPSK are present.
    The bunchgroupset is not stored in the metadata, so 0 is returned for the BGS key for completeness.

    @returns: dictionary with the DB keys. Returns 'None' if information is not present.
    """
    metadata = GetFileMD(flags.Input.Files)
    keys = metadata.get("TriggerConfigInfo", None)
    if keys is None:
        return None
    return {
        'SMK': keys['HLT']['key'] if 'HLT' in keys else 0,
        'L1PSK': keys['L1PS']['key'] if 'L1PS' in keys else 0,
        'HLTPSK': keys['HLTPS']['key'] if 'HLTPSK' in keys else 0,
        'BGSK': 0
    }

"""
Returns a string-serialised JSON object from the metadata store.
Checks AOD syntax first, then fully-qualified ESD syntax
"""
def _getJSONFromMetadata(flags, key) -> Optional[dict[str,Any]]:
    if flags.Input.Format != Format.POOL:
        raise RuntimeError("Cannot read trigger configuration (%s) from input type %s", key, flags.Input.Format)
    from AthenaConfiguration.AutoConfigFlags import GetFileMD
    metadata = GetFileMD(flags.Input.Files)
    menu_json = metadata.get(key, None)
    if menu_json is None:
        menu_json = metadata.get('DataVector<xAOD::TriggerMenuJson_v1>_%s' % key, None)
    if menu_json is None:
        if key == 'TriggerMenuJson_HLTMonitoring':
            # currently the HLT Monitoring information is missing from many pool files' metadata
            log.info("Trigger metadata with key 'TriggerMenuJson_HLTMonitoring' is not available in this file. This feature is currently being introduced.")
            return None
        else:
            raise RuntimeError("Cannot read trigger configuration (%s) from input file metadata" % key)
    return menu_json


"""

L1 information

"""
@AccumulatorCache
def getL1MenuAccess( flags = None ) -> L1MenuAccess:
    tc = getTrigConfigFromFlag( flags )
    if tc["SOURCE"] == "FILE":
        cfg = L1MenuAccess( filename = getL1MenuFileName( flags ) )
    elif tc["SOURCE"] == "COOL":
        """This is the case when reconstructing the data."""
        if len(flags.Input.RunNumbers) == 0:
            raise RuntimeError("No run number available in input metadata")
        keysFromCool = getKeysFromCool( flags.Input.RunNumbers[0] )
        cfg = L1MenuAccess( dbalias = keysFromCool["DB"], smkey = keysFromCool['SMK'] )
    elif tc["SOURCE"] == "DB":
        cfg = L1MenuAccess( dbalias = tc["DBCONN"], smkey = tc["SMK"] )
    elif tc["SOURCE"] == "INFILE":
        cfg = L1MenuAccess(jsonString=_getJSONFromMetadata(flags, key='TriggerMenuJson_L1'))
    else:
        raise RuntimeError("Unknown source of trigger configuration: %s" % tc["SOURCE"])
    return cfg


@AccumulatorCache
def getL1PrescalesSetAccess( flags = None ) -> L1PrescalesSetAccess:
    tc = getTrigConfigFromFlag( flags )
    if tc["SOURCE"] == "FILE":
        cfg = L1PrescalesSetAccess( filename = getL1PrescalesSetFileName( flags ) )
    elif tc["SOURCE"] == "COOL":
        """This is the case when reconstructing the data."""
        if len(flags.Input.RunNumbers) == 0:
            raise RuntimeError("No run number available in input metadata")
        keysFromCool = getKeysFromCool( flags.Input.RunNumbers[0] )
        cfg = L1PrescalesSetAccess( dbalias = keysFromCool["DB"], l1pskey = keysFromCool['L1PSK'] )
    elif tc["SOURCE"] == "DB":
        cfg = L1PrescalesSetAccess( dbalias = tc["DBCONN"], l1pskey = tc["L1PSK"] )
    elif tc["SOURCE"] == "INFILE":
        cfg = L1PrescalesSetAccess(jsonString=_getJSONFromMetadata(flags, key='TriggerMenuJson_L1PS'))
    else:
        raise RuntimeError("Unknown source of trigger configuration: %s" % tc["SOURCE"])
    return cfg


@AccumulatorCache
def getBunchGroupSetAccess( flags = None ) -> BunchGroupSetAccess:
    tc = getTrigConfigFromFlag( flags )
    if tc["SOURCE"] == "FILE":
        cfg = BunchGroupSetAccess( filename = getBunchGroupSetFileName( flags ) )
    elif tc["SOURCE"] == "COOL":
        """This is the case when reconstructing the data."""
        if len(flags.Input.RunNumbers) == 0:
            raise RuntimeError("No run number available in input metadata")
        keysFromCool = getKeysFromCool( flags.Input.RunNumbers[0] )
        cfg = BunchGroupSetAccess( dbalias = keysFromCool["DB"], bgskey = keysFromCool['BGSK'] )
    elif tc["SOURCE"] == "DB":
        cfg = BunchGroupSetAccess( dbalias = tc["DBCONN"], bgskey = tc["BGSK"] )
    elif tc["SOURCE"] == "INFILE":
        if flags.Input.Format != Format.POOL:
            raise RuntimeError(f"Cannot read trigger configuration (Bunchgroup Set) from input type {flags.Input.Format}")
        raise NotImplementedError("Python access to the trigger configuration (Bunchgroup Set) from in-file metadata not yet implemented")
    else:
        raise RuntimeError("Unknown source of trigger configuration: %s" % tc["SOURCE"])
    return cfg


"""

HLT information

"""
@AccumulatorCache
def getHLTMenuAccess( flags = None ) -> HLTMenuAccess:
    tc = getTrigConfigFromFlag( flags )
    if tc["SOURCE"] == "FILE":
        cfg = HLTMenuAccess( filename = getHLTMenuFileName( flags ) )
    elif tc["SOURCE"] == "COOL":
        """This is the case when reconstructing the data."""
        if len(flags.Input.RunNumbers) == 0:
            raise RuntimeError("No run number available in input metadata")
        keysFromCool = getKeysFromCool( flags.Input.RunNumbers[0] )
        cfg = HLTMenuAccess( dbalias = keysFromCool["DB"], smkey = keysFromCool['SMK'] )
    elif tc["SOURCE"] == "DB":
        cfg = HLTMenuAccess( dbalias = tc["DBCONN"], smkey = tc["SMK"] )
    elif tc["SOURCE"] == "INFILE":
        cfg = HLTMenuAccess(jsonString=_getJSONFromMetadata(flags, key='TriggerMenuJson_HLT'))
    else:
        raise RuntimeError("Unknown source of trigger configuration: %s" % tc["SOURCE"])
    return cfg


@AccumulatorCache
def getHLTPrescalesSetAccess( flags = None ) -> HLTPrescalesSetAccess:
    tc = getTrigConfigFromFlag( flags )
    if tc["SOURCE"] == "FILE":
        cfg = HLTPrescalesSetAccess( filename = getHLTPrescalesSetFileName( flags ) )
    elif tc["SOURCE"] == "COOL":
        """This is the case when reconstructing the data."""
        if len(flags.Input.RunNumbers) == 0:
            raise RuntimeError("No run number available in input metadata")
        keysFromCool = getKeysFromCool( flags.Input.RunNumbers[0] )
        cfg = HLTPrescalesSetAccess( dbalias = keysFromCool["DB"], hltpskey = keysFromCool['HLTPSK'] )
    elif tc["SOURCE"] == "DB":
        cfg = HLTPrescalesSetAccess( dbalias = tc["DBCONN"], hltpskey = tc["HLTPSK"] )
    elif tc["SOURCE"] == "INFILE":
        cfg = HLTPrescalesSetAccess(jsonString=_getJSONFromMetadata(flags, key='TriggerMenuJson_HLTPS'))
    else:
        raise RuntimeError("Unknown source of trigger configuration: %s" % tc["SOURCE"])
    return cfg


@AccumulatorCache
def getHLTJobOptionsAccess( flags = None ) -> HLTJobOptionsAccess:
    tc = getTrigConfigFromFlag( flags )
    if tc["SOURCE"] == "FILE":
        cfg = HLTJobOptionsAccess( filename = getHLTJobOptionsFileName() )
    elif tc["SOURCE"] == "COOL":
        """This is the case when reconstructing the data."""
        if len(flags.Input.RunNumbers) == 0:
            raise RuntimeError("No run number available in input metadata")
        keysFromCool = getKeysFromCool( flags.Input.RunNumbers[0] )
        cfg = HLTJobOptionsAccess( dbalias = keysFromCool["DB"], smkey = keysFromCool['SMK'] )
    elif tc["SOURCE"] == "DB":
        cfg = HLTJobOptionsAccess( dbalias = tc["DBCONN"], smkey = tc["SMK"] )
    elif tc["SOURCE"] == "INFILE":
        raise NotImplementedError("Python access to the HLT Job Options configuration from in-file metadata is NOT SUPPORTED (this file is huge!)")
    else:
        raise RuntimeError("Unknown source of trigger configuration: %s" % tc["SOURCE"])
    return cfg


@AccumulatorCache
def getHLTMonitoringAccess( flags = None, filterOnActiveChains = False ) -> HLTMonitoringAccess:
    tc = getTrigConfigFromFlag( flags )
    if tc["SOURCE"] == "FILE":
        cfg = HLTMonitoringAccess( filename = getHLTMonitoringFileName( flags ) )
    elif tc["SOURCE"] == "COOL":
        """This is the case when reconstructing the data."""
        if len(flags.Input.RunNumbers) == 0:
            raise RuntimeError("No run number available in input metadata")
        keysFromCool = getKeysFromCool( flags.Input.RunNumbers[0] )
        cfg = HLTMonitoringAccess( dbalias = keysFromCool["DB"], smkey = keysFromCool['SMK'] )
    elif tc["SOURCE"] == "DB":
        cfg = HLTMonitoringAccess( dbalias = tc["DBCONN"], smkey = tc["SMK"] )
    elif tc["SOURCE"] == "INFILE":
        jsonHLTMon = _getJSONFromMetadata(flags, key='TriggerMenuJson_HLTMonitoring')
        if jsonHLTMon is not None:
            cfg = HLTMonitoringAccess(jsonString=jsonHLTMon)
        else:
            keysFromInfileMD = getDBKeysFromMetadata(flags)
            smkey = keysFromInfileMD['SMK'] if keysFromInfileMD is not None else 0
            if smkey < 3000 and not flags.Input.isMC:
                # Run 1/2 data or keys missing in metadata 
                log.info("Trigger metadata with key 'TriggerMenuJson_HLTMonitoring' is not available for Run 2 data. Returning empty dummy.")
                jsonHLTMon = '{"filetype": "hltmonitoringsummary","name": "EmptyDefault", "signatures": {}}'
                cfg = HLTMonitoringAccess(jsonString=jsonHLTMon)
            else:
                # Run 3 data or MC
                try:
                    log.info("Falling back on reading the HLTMonitoring from the TRIGGERDB_RUN3 for SMK %i.", smkey)
                    cfg = HLTMonitoringAccess( dbalias = "TRIGGERDB_RUN3", smkey = smkey )
                except KeyError:
                    # SMK for this run has no HLT monitoring (earlier 2022 data) => providing dummy configuration
                    log.info("Trigger HLTMonitoring is not available for SMK %i. Returning empty dummy.", smkey)
                    jsonHLTMon = '{"filetype": "hltmonitoringsummary","name": "EmptyDefault", "signatures": {}}'
                    cfg = HLTMonitoringAccess(jsonString=jsonHLTMon)

    else:
        raise RuntimeError("Unknown source of trigger configuration: %s" % tc["SOURCE"])

    # Optional deletion of chains from the monitoring JSON which were not enabled,
    # based on the first HLT PS set in the file or the HLT PS set for the RAW file's LB.
    if filterOnActiveChains:
        hltPs = getHLTPrescalesSetAccess(flags)
        if hltPs:
            for sig in cfg["signatures"]:
                for chain in list(cfg["signatures"][sig]): # Make a copy as we might be deleting
                    if chain in hltPs.chainNames() and not hltPs.enabled(chain):
                        del cfg["signatures"][sig][chain]

    return cfg
