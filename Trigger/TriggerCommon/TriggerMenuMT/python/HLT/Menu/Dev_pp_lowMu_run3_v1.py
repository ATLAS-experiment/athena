# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# Dev_pp_lowMu_run3_v1.py menu for Run 3 development
#------------------------------------------------------------------------#

# All chains are represented as ChainProp objects in a ChainStore
from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp
from .SignatureDicts import ChainStore

from .Physics_pp_run3_v1 import (
    MinBiasGroup,
    PrimaryPhIGroup,
    SupportGroup,
    SupportPhIGroup,
    SingleJetGroup,
    SingleElectronGroup,
    MultiElectronGroup,
    SinglePhotonGroup
)
from .PhysicsP1_pp_lowMu_run3_v1 import PhysicsStream
from .PhysicsP1_HI_run3_v1 import MinBiasStream
import TriggerMenuMT.HLT.Menu.PhysicsP1_pp_lowMu_run3_v1 as physics_menu


def getDevLowMuSignatures():
    chains = ChainStore()

    chains['Muon'] += [
    ]

    chains['Egamma'] += [
        ChainProp(name='HLT_e15_lhloose_nogsf_ion_L1eEM15',  stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryPhIGroup,  monGroups=['egammaMon:t0_tp','egammaMon:shifter','caloMon:t0']),
        ChainProp(name='HLT_e15_loose_nogsf_ion_L1eEM15',    stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryPhIGroup,  monGroups=['egammaMon:t0_tp','egammaMon:shifter']),
        ChainProp(name='HLT_e15_lhmedium_nogsf_ion_L1eEM15', stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryPhIGroup,  monGroups=['caloMon:t0']),
        ChainProp(name='HLT_e15_medium_nogsf_ion_L1eEM15',   stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryPhIGroup),

        ChainProp(name='HLT_e20_lhloose_nogsf_ion_L1eEM18',  stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e20_lhmedium_nogsf_ion_L1eEM18', stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e20_loose_nogsf_ion_L1eEM18',    stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e20_loose_nogsf_ion_L1eEM18L',   stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e20_medium_nogsf_ion_L1eEM18',   stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_2e20_loose_nogsf_ion_L12eEM18',  stream=[PhysicsStream], groups=MultiElectronGroup+PrimaryPhIGroup,   monGroups=['egammaMon:online','egammaMon:shifter_tag','egammaMon:shifter']),

        ChainProp(name='HLT_g15_loose_ion_L1eEM12',   stream=[PhysicsStream], groups=SinglePhotonGroup+SupportPhIGroup,  monGroups=['egammaMon:online','egammaMon:shifter','egammaMon:val','caloMon:t0']),
        ChainProp(name='HLT_g15_loose_ion_L1eEM15',   stream=[PhysicsStream], groups=SinglePhotonGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_g20_loose_ion_L1eEM15',   stream=[PhysicsStream], groups=SinglePhotonGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_g20_loose_ion_L1eEM18',   stream=[PhysicsStream], groups=SinglePhotonGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_g30_loose_ion_L1eEM18',   stream=[PhysicsStream], groups=SinglePhotonGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_g50_loose_ion_L1eEM26',   stream=[PhysicsStream], groups=SinglePhotonGroup+PrimaryPhIGroup,  monGroups=['egammaMon:online','egammaMon:shifter','egammaMon:val','caloMon:t0']),
        ChainProp(name='HLT_2g15_loose_ion_L12eEM12', stream=[PhysicsStream], groups=SinglePhotonGroup+PrimaryPhIGroup),
    ]

    chains['Jet'] += [
        ChainProp(name='HLT_j60_ion_L1jJ40',  l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+SupportPhIGroup,  monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j75_ion_L1jJ50',  l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j75_ion_L1jJ60',  l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+PrimaryPhIGroup,  monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j85_ion_L1jJ50',  l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j85_ion_L1jJ60',  l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+PrimaryPhIGroup,  monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j100_ion_L1jJ60', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j120_ion_L1jJ60', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j150_ion_L1jJ90', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+PrimaryPhIGroup,  monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j200_ion_L1jJ90', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+PrimaryPhIGroup,  monGroups=['jetMon:t0','jetMon:online']),

        ChainProp(name='HLT_j50f_ion_L1jJ40p30ETA49', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j60f_ion_L1jJ40p30ETA49', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j70f_ion_L1jJ60p30ETA49', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j80f_ion_L1jJ60p30ETA49', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j90f_ion_L1jJ90p30ETA49', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+PrimaryPhIGroup),
    ]

    chains['Combined'] += [
    ]

    chains['MinBias'] += [
        ChainProp(name='HLT_mb_sptrk_L1MBTS_1_1_VjTE50', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_OR_VjTE50',   l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_A_C_VjTE50',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_XOR_VjTE50',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),

        ChainProp(name='HLT_mb_sptrk_L1TRT_VjTE50',         l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_OR_VjTE50',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_A_C_VjTE50', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_XOR_VjTE50', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
    ]

    chains['HeavyIon'] += [
    ]

    chains['Streaming'] += [
    ]


    return chains

def setupMenu():
    chains = physics_menu.setupMenu()

    from AthenaCommon.Logging import logging
    log = logging.getLogger( __name__ )
    log.info('[setupMenu] going to add the Dev menu chains now')

    for sig,chainsInSig in getDevLowMuSignatures().items():
        chains[sig] += chainsInSig

    return chains
