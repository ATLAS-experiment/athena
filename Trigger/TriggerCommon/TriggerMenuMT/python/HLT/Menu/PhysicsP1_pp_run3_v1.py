# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# PhysicsP1_pp_run3_v1.py menu
#
# Combines physics triggers with P1 monitoring chains
#------------------------------------------------------------------------#

# All chains are represented as ChainProp objects in a ChainStore
from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp
from .SignatureDicts import ChainStore


from . import Physics_pp_run3_v1 as physics_menu 
from . import P1_run3_v1

from .Physics_pp_run3_v1 import (
    SingleMuonGroup,
    SingleJetGroup,
    MinBiasGroup,
    JetPhaseIStreamersGroup,
    TauPhaseIStreamersGroup,
    EgammaPhaseIStreamersGroup,
    MuonXStreamersGroup,
    SupportGroup,
    SupportPhIGroup,
)


from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

def addPhysicsP1Chains(chains):
    chainsP1 = ChainStore()

    chainsP1['Muon'] = [
        # ATR-25219, 1mu, for alignment run
        # L1 item is not in MC menu
        ChainProp(name='HLT_mu5_mucombTag_L1MU20VFC',groups=SingleMuonGroup+SupportGroup, monGroups=['muonMon:online']),
        ChainProp(name='HLT_mu8_mucombTag_L1MU20VFC',groups=SingleMuonGroup+SupportGroup, monGroups=['muonMon:online']),
        ChainProp(name='HLT_mu10_mucombTag_L1MU20VFC',groups=SingleMuonGroup+SupportGroup, monGroups=['muonMon:online']),
        ChainProp(name='HLT_mu15_mucombTag_L1MU20VFC',groups=SingleMuonGroup+SupportGroup, monGroups=['muonMon:online']),
        ChainProp(name='HLT_mu20_mucombTag_L1MU20VFC',groups=SingleMuonGroup+SupportGroup, monGroups=['muonMon:online']),
        ChainProp(name='HLT_mu25_mucombTag_L1MU20VFC',groups=SingleMuonGroup+SupportGroup, monGroups=['muonMon:online']),
        ChainProp(name='HLT_mu30_mucombTag_L1MU20VFC',groups=SingleMuonGroup+SupportGroup, monGroups=['muonMon:online']),
        ChainProp(name='HLT_mu50_mucombTag_L1MU20VFC',groups=SingleMuonGroup+SupportGroup, monGroups=['muonMon:online']),
    ]

    chainsP1['Jet'] = [
        # L1 item is not in MC menu
        ChainProp(name='HLT_j0_perf_L1jJ30_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SingleJetGroup+SupportPhIGroup, monGroups=['jetMon:online']),
        # ATR-31286 Higher Threshold Duplicate
        ChainProp(name='HLT_j0_perf_L1jJ60_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SingleJetGroup+SupportPhIGroup, monGroups=['jetMon:online']),

    ]

    # Streamers with L1 items removed from MC menu
    chainsP1['Streaming'] = [
        ChainProp(name='HLT_noalg_L1MU3VC',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU3EOF',    l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU4BO',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU8FC',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU8FH',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU8EOF',    l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU9VF',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU9VFC',    l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU12FCH',   l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU14FCHR',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU14EOF',   l1SeedThresholds=['FSNOSEED'], stream=['Main','express'], groups=SingleMuonGroup+SupportGroup,monGroups=['detMon'] ),
        ChainProp(name='HLT_noalg_L1MU15VFCH',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU15VFCHR', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU20VFC',   l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SingleMuonGroup+SupportGroup),

        ChainProp(name='HLT_noalg_L1eTAU35',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SupportPhIGroup+TauPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eTAU40HM',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SupportPhIGroup+TauPhaseIStreamersGroup),

        ChainProp(name='HLT_noalg_L1eEM7',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SupportPhIGroup+EgammaPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eEM10L',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SupportPhIGroup+EgammaPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eEM15',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SupportPhIGroup+EgammaPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eEM18',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SupportPhIGroup+EgammaPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eEM22M',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SupportPhIGroup+EgammaPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eEM24VM',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SupportPhIGroup+EgammaPhaseIStreamersGroup),

        ChainProp(name='HLT_noalg_L1jEM25',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SupportPhIGroup+EgammaPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jEM20M',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SupportPhIGroup+EgammaPhaseIStreamersGroup),

        ChainProp(name='HLT_noalg_L1jJ30p0ETA25',   l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ40p0ETA25',   l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SupportPhIGroup+JetPhaseIStreamersGroup),

        ChainProp(name='HLT_noalg_L1jJ55',          l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ55p0ETA23',   l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ70p0ETA23',   l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ80',          l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SupportPhIGroup+JetPhaseIStreamersGroup, monGroups=['jetMon:online']),
        ChainProp(name='HLT_noalg_L1jJ80p0ETA25',   l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ85p0ETA21',   l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ140',         l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SupportPhIGroup+JetPhaseIStreamersGroup, monGroups=['jetMon:online']),
        ChainProp(name='HLT_noalg_L1jJ180',         l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SupportPhIGroup+JetPhaseIStreamersGroup),


        # Exotics support streamers
        ChainProp(name='HLT_noalg_L1MU14FCH_EMPTY',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SupportGroup+MuonXStreamersGroup),
        ChainProp(name='HLT_noalg_L1MU14FCH_UNPAIRED_ISO',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SupportGroup+MuonXStreamersGroup),

        # AFP calibration
        ChainProp(name='HLT_noalg_L1AFP_FSA_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSC_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=MinBiasGroup+SupportGroup),
        # all mu
        ChainProp(name='HLT_noalg_L1AFP_FSA_TOF_T0_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSA_TOF_T1_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSC_TOF_T0_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSC_TOF_T1_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=MinBiasGroup+SupportGroup),
    ]

    for sig,chainsInSig in chainsP1.items():
        for chain in chainsInSig:
            for group in chain.groups:
                if 'Primary' in group:
                    log.error("chain %s in PhysicsP1 menu [%s] with Primary tag. Please move this to Physics menu file", chain.name, sig)
                    raise RuntimeError("Move %s chain to Physics menu file",chain.name)

    for sig,chainsInSig in chainsP1.items():
        chains[sig] += chainsInSig

def setupMenu():
    log.info('setupMenu ...')

    # Add physics chains (data + MC)
    chains = physics_menu.setupMenu()

    addPhysicsP1Chains(chains)

    # Add calibration and monitoring chains (not in Main)
    P1_run3_v1.addCommonP1Signatures(chains)
    P1_run3_v1.addHighMuP1Signatures(chains)

    return chains

