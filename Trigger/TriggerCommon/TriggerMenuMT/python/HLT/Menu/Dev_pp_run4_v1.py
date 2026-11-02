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
                                  MultiJetGroup,
                                  MultiBjetGroup,
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
    
    # Hit-based per-jet z regression (HitZ) preselection. Working points give
    # 90/85/80% signal on the HitZV01 outputs at |eta|<2.4, scored as the hypo
    # does (best n-subset of the passing jets); larger number = looser cut.
    # The preselection-only chains measure efficiency and rate, the last one
    # replaces the calorimeter preselection of an existing multi-b chain.
    # MAXMULT keeps only the leading 30 (20 for six jets) central jets so the
    # hypo's combinations stay bounded at mu=200.
    chains['Jet'] = [
        ChainProp(name='EF_j0_pf_ftf_preselHZ128MAXMULT30cXX4c20_L13jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='EF_j0_pf_ftf_preselHZ104MAXMULT30cXX4c20_L13jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='EF_j0_pf_ftf_preselHZ91MAXMULT30cXX4c20_L13jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),
        # the likelihood discriminates much better at high jet multiplicity
        ChainProp(name='EF_j0_pf_ftf_preselHZ208MAXMULT20cXX6c20_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='EF_j0_pf_ftf_preselHZ177MAXMULT20cXX6c20_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='EF_j0_pf_ftf_preselHZ152MAXMULT20cXX6c20_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),
    ]
    
    chains['Bjet'] = [
        ChainProp(name='EF_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn282_pf_ftf_preselHZ104MAXMULT30cXX2c20XX2c20_L13jJ40', l1SeedThresholds=['FSNOSEED']*5, groups=MultiBjetGroup+DevGroup),
    ]
    
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
        ChainProp(name='EF_timeburnerprocessing_L1All', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoHLTRepro','RATE:Monitoring','BW:Other']+DevGroup),
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
