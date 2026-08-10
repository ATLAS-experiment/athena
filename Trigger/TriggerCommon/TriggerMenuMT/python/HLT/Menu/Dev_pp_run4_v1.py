# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# Dev_pp_run4_v1.py menu for Phase-II developments 
#------------------------------------------------------------------------#

# This defines the input format of the chain and it's properties with the defaults set
# always required are: name, stream and groups
# ['name', 'L1chainParts'=[], 'stream', 'groups', 'merging'=[], 'topoStartFrom'=False],

import TriggerMenuMT.HLT.Menu.MC_pp_run4_v1 as mc_menu
from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp
from TriggerMenuMT.HLT.Menu.Physics_pp_run4_v1 import (MultiJetGroup,
                                                       MultiBjetGroup)


DevGroup = ['Development']

def setupMenu():

    chains = mc_menu.setupMenu()

    from AthenaCommon.Logging import logging
    log = logging.getLogger( __name__ )
    log.info('[setupMenu] going to add the Dev menu chains now')

    chains['Muon'] += []
    chains['Egamma'] += []
    chains['Tau'] += []
    # Hit-based per-jet z regression (HitZ) preselection. Working points from a
    # likelihood scan on the MDNv01e86 outputs; larger number = looser cut.
    # The preselection-only chains measure efficiency and rate, the last one
    # replaces the calorimeter preselection of an existing multi-b chain.
    chains['Jet'] += [
        ChainProp(name='HLT_j0_pf_ftf_preselHZ84XX4c20_L13jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_j0_pf_ftf_preselHZ120XX4c20_L13jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_j0_pf_ftf_preselHZ160XX4c20_L13jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_j0_pf_ftf_preselHZ120MAXMULT5cXX4c20_L13jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),
        # the likelihood discriminates much better at high jet multiplicity
        ChainProp(name='HLT_j0_pf_ftf_preselHZ60XX6c20_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_j0_pf_ftf_preselHZ84XX6c20_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_j0_pf_ftf_preselHZ120XX6c20_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),
    ]
    chains['Bjet'] += [
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn282_pf_ftf_preselHZ120XX2c20XX2c20bgtwo85_L13jJ40', l1SeedThresholds=['FSNOSEED']*5, groups=MultiBjetGroup+DevGroup),
    ]
    chains['MET'] += []
    chains['Bphysics'] += []
    chains['UnconventionalTracking'] += []
    chains['Combined'] += []
    chains['Beamspot'] += []
    chains['MinBias'] += []
    chains['Calib'] += []
    chains['Streaming'] += []
    chains['Monitor'] += [
        ChainProp(name='HLT_timeburnerprocessing_L1All', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoHLTRepro','RATE:Monitoring','BW:Other']+DevGroup),
    ]
    return chains
