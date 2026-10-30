# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# Dev_pp_run4_v1.py menu for Phase-II developments 
#------------------------------------------------------------------------#

# This defines the input format of the chain and its properties with the defaults set
# always required are: name, stream and groups
# ['name', 'L1chainParts'=[], 'stream', 'groups', 'merging'=[], 'topoStartFrom'=False],

# All chains are represented as ChainProp objects in a ChainStore
from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp
from .SignatureDicts import ChainStore

import TriggerMenuMT.HLT.Menu.MC_pp_run4_v1 as mc_menu

from .Physics_pp_run4_v1 import ( SingleTauGroup,
                                  SupportPhIGroup,
                                )

DevGroup = ['Development']

def getDevSignatures():
    chains = ChainStore()

    chains['Muon'] = []
    
    chains['Egamma'] = []

    chains['Tau'] = [
        # Chains with EMTopo seed
        ChainProp(name='EF_tau0_ptonly_em_L1eTAU12', groups=SupportPhIGroup+SingleTauGroup+DevGroup, monGroups=['tauMon:online', 'tauMon:shifter']),
        ChainProp(name='EF_tau20_idperf_em_L1eTAU12', groups=SupportPhIGroup+SingleTauGroup+DevGroup, monGroups=['tauMon:online', 'tauMon:shifter']),
        ChainProp(name='EF_tau20_perf_em_L1eTAU12', groups=SupportPhIGroup+SingleTauGroup+DevGroup, monGroups=['tauMon:online', 'tauMon:shifter']),
        ChainProp(name='EF_tau20_mediumGNTau_em_L1eTAU12', groups=SupportPhIGroup+SingleTauGroup+DevGroup, monGroups=['tauMon:online', 'tauMon:shifter']), 
    ]
    
    chains['Jet'] = []
    
    chains['Bjet'] = []
    
    chains['MET'] = []
    
    chains['Bphysics'] = []
    
    chains['UnconventionalTracking'] = []
    
    chains['Combined'] = []
    
    chains['Beamspot'] += [
        ChainProp(name='EF_beamspot_trkFS_trkfast_BeamSpotPEB_L1All', l1SeedThresholds=['FSNOSEED'], stream=['BeamSpot'], groups=['RATE:BeamSpot','BW:BeamSpot']+DevGroup),
    ]
    
    chains['MinBias'] = []
    
    chains['Calib'] = []
    
    chains['Streaming'] = []
    
    chains['Monitor'] = [
        ChainProp(name='EF_timeburnerprocessing_L1All', l1SeedThresholds=['FSNOSEED'], stream=['DISCARD'], groups=['PS:NoHLTRepro','RATE:DISCARD','BW:DISCARD']+DevGroup),
    ]

    return chains


def setupMenu():

    chains = mc_menu.setupMenu()

    from AthenaCommon.Logging import logging
    log = logging.getLogger( __name__ )
    log.info('[setupMenu] going to add the Dev menu chains now')

    for sig,chainsInSig in getDevSignatures().items():
        chains[sig] += chainsInSig
    
    return chains
