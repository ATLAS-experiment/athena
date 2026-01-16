# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
log = logging.getLogger( __name__ )

'''
This file defines Event Building identifiers which can be used in chain names.

When adding new identifiers, please follow the naming convention
    SomeNamePEBVariant for Partial Event Building (here) or
    SomeNameDSVariant for Data Scouting (in DataScoutingInfo),
where SomeName and Variant are generally camel-case unless they're acronyms
like LAr or RPC. Variant is normally empty unless there are several variants
like RPCPEB.

Possible examples:
LArPEB, LumiPEB, RPCPEB, TrkPEB, PhysicsTLA
'''

# Dictionary with PEB identifiers and if RoI-based PEB is used:
_PartialEventBuildingIdentifiers = {
    'BeamSpotPEB' : False,
    'MuonTrkPEB' : True,
    'IDCalibPEB' : True,
    'LArPEB' : True,
    'LArPEBCalib' : False,
    'LArPEBHLT' : True,
    'LArPEBNoise' : True,
    'LATOMEPEB' : False,
    'SCTPEB' : False,
    'TilePEB' : False,
    'ZDCPEB' : False,
    'AFPPEB' : False,
    'LumiPEB' : False,
    'IDScanPEB' : False,
    'Lvl1CaloPEB' : False,
    'Lvl1CaloLatomePEB' : False,
    # DataScouting identifiers from TrigEDMConfig.DataScoutingInfo:
    'CostMonDS': False,
    'MuonDS': False,
    'PhysicsTLA': False,
    'DarkJetPEBTLA' : True,
    'FTagPEBTLA' : True,
    'EgammaPEBTLA' : True,
}


def getAllEventBuildingIdentifiers():
    return _PartialEventBuildingIdentifiers.keys()


def isRoIBasedPEB(eventBuildType):
    """Helper function to determine if eventBuildType corresponds to RoI-based PEB"""
    try:
        return _PartialEventBuildingIdentifiers[eventBuildType]
    except KeyError:
        log.error("'%s' is not a known event building identifier", eventBuildType)
        raise
