# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
log = logging.getLogger( __name__ )

'''
This file defines the Data Scouting chain identifiers which serve also
as EDM target identifiers and their mapping to HLT Result module IDs.

When adding new identifiers, please follow the naming convention SomeNameDS,
where SomeName is generally camel-case unless it's an acronym like LAr or MET.

Possible examples:
CostMonDS, PhysicsTLA
'''


# Data scouting identifiers and the corresponding HLT result ROBFragment module IDs.
# If you add new entries also add a corresponding entry to EventBuildingInfo.py.
# WARNING: Never change the module IDs during data taking!
# WARNING: ID=0 is reserved for full HLT result
_DataScoutingIdentifiers = {
    'CostMonDS': 1,
    'MuonDS' : 2,
    'PhysicsTLA': 5,
    'DarkJetPEBTLA': 6,
    'FTagPEBTLA' : 7,
    'EgammaPEBTLA' : 8,
}

# Each stream should correspond to exactly one event building type
_DataScoutingStreams = {
    'calibration_CostMonitoring': 'CostMonDS',
    'calibration_MuonDSCalib':'MuonDS',
    'physics_TLA': 'PhysicsTLA',
    'physics_DarkJetPEBTLA': 'DarkJetPEBTLA',
    'physics_FTagPEBTLA': 'FTagPEBTLA',
    'physics_EgammaPEBTLA': 'EgammaPEBTLA',
}

# Truncation thresholds (in bytes) for each HLT result type
TruncationThresholds = {
    0: 7*(1024**2),  # Main: 7 MB (increased from 5MB after ATR-29142)
    1: 2*(1024**2),  # CostMonDS: 2 MB
    2: 1*(1024**2),  # MuonDS: 1 MB
    5: 1*(1024**2),  # PhysicsTLA: 1 MB
    6: 1*(1024**2),  # DarkJetPEBTLA: 1 MB
    7: 1*(1024**2),  # FTagPEBTLA 1 MB
    8: 1*(1024**2),  # EgammaPEBTLA 1 MB
    
}


def getDataScoutingResultID(name):
    if name in _DataScoutingIdentifiers:
        return _DataScoutingIdentifiers[name]
    else:
        log.error('Unknown name %s, cannot assign result ID', name)


def getDataScoutingStreams():
    return list(_DataScoutingStreams.keys())


def getDataScoutingTypeFromStream(streamname):
    if streamname in _DataScoutingStreams:
        return _DataScoutingStreams[streamname]
    else:
        log.error('Unknown name %s, not a data scouting stream', streamname)


def getAllDataScoutingResultIDs():
    return _DataScoutingIdentifiers.values()


def getFullHLTResultID():
    # WARNING: Don't change this, it should always be zero.
    # It's defined here to avoid using a magic number in other places
    return 0


def getAllDataScoutingIdentifiers():
    return list(_DataScoutingIdentifiers.keys())
