# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# Dev_pp_lowMu_run3_v1.py menu for Run 3 development
#------------------------------------------------------------------------#

# All chains are represented as ChainProp objects in a ChainStore
from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp
from .SignatureDicts import ChainStore

import TriggerMenuMT.HLT.Menu.PhysicsP1_pp_lowMu_run3_v1 as physics_menu
from .Physics_pp_run3_v1 import (
    SingleMuonGroup,
    SingleElectronGroup,
    MinBiasGroup,
    SupportGroup,
    SupportPhIGroup,
    PrimaryPhIGroup,
    SinglePhotonGroup,
    SingleJetGroup,
)
from .PhysicsP1_pp_lowMu_run3_v1 import (LowMuGroup, LowMuGroupPhI)


def getDevLowMuSignatures():
    chains = ChainStore()

    chains['Muon'] += [
        # ATR-30691/ATR-30692: Oxygen runs
        ChainProp(name='HLT_mu3_L1MU3V', stream=['MinBias', 'express'], groups=SingleMuonGroup+SupportGroup, monGroups=['muonMon:shifter','muonMon:online']),
    ]

    chains['Egamma'] += [
        # ATR-30691/ATR-30692: Oxygen runs
        ChainProp(name='HLT_e6_etcut_L1eEM5',    stream=['MinBias'], groups=SingleElectronGroup+SupportPhIGroup),
        ChainProp(name='HLT_e6_nopid_L1eEM5',    stream=['MinBias'], groups=SingleElectronGroup+SupportPhIGroup,  monGroups=['egammaMon:online','egammaMon:shifter','caloMon:t0']),
        ChainProp(name='HLT_e6_loose_L1eEM5',    stream=['MinBias'], groups=SingleElectronGroup+SupportPhIGroup,  monGroups=['egammaMon:online','egammaMon:shifter','caloMon:t0']),
        ChainProp(name='HLT_e6_lhloose_L1eEM5',  stream=['MinBias'], groups=SingleElectronGroup+SupportPhIGroup),
        ChainProp(name='HLT_e6_medium_L1eEM5',   stream=['MinBias'], groups=SingleElectronGroup+SupportPhIGroup),
        ChainProp(name='HLT_e6_lhmedium_L1eEM5', stream=['MinBias'], groups=SingleElectronGroup+SupportPhIGroup),

        ChainProp(name='HLT_e10_etcut_L1eEM9',    stream=['MinBias'], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e10_nopid_L1eEM9',    stream=['MinBias'], groups=SingleElectronGroup+PrimaryPhIGroup,  monGroups=['egammaMon:online','egammaMon:shifter','caloMon:t0']),
        ChainProp(name='HLT_e10_loose_L1eEM9',    stream=['MinBias', 'express'], groups=SingleElectronGroup+PrimaryPhIGroup,  monGroups=['egammaMon:online','egammaMon:shifter','caloMon:t0']),
        ChainProp(name='HLT_e10_lhloose_L1eEM9',  stream=['MinBias'], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e10_medium_L1eEM9',   stream=['MinBias'], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e10_lhmedium_L1eEM9', stream=['MinBias'], groups=SingleElectronGroup+PrimaryPhIGroup),

        # ATR-30691/ATR-30692: Oxygen runs
        ChainProp(name='HLT_g6_etcut_L1eEM5',  stream=['MinBias'], groups=SinglePhotonGroup+SupportPhIGroup),
        ChainProp(name='HLT_g6_nopid_L1eEM5',  stream=['MinBias'], groups=SinglePhotonGroup+SupportPhIGroup,  monGroups=['egammaMon:online','egammaMon:shifter','caloMon:t0']),
        ChainProp(name='HLT_g6_loose_L1eEM5',  stream=['MinBias'], groups=SinglePhotonGroup+SupportPhIGroup,  monGroups=['egammaMon:online','egammaMon:shifter','caloMon:t0']),
        ChainProp(name='HLT_g6_medium_L1eEM5', stream=['MinBias'], groups=SinglePhotonGroup+SupportPhIGroup),

        ChainProp(name='HLT_g10_etcut_L1eEM9',  stream=['MinBias'], groups=SinglePhotonGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_g10_nopid_L1eEM9',  stream=['MinBias'], groups=SinglePhotonGroup+PrimaryPhIGroup,  monGroups=['egammaMon:online','egammaMon:shifter','caloMon:t0']),
        ChainProp(name='HLT_g10_loose_L1eEM9',  stream=['MinBias', 'express'], groups=SinglePhotonGroup+PrimaryPhIGroup,  monGroups=['egammaMon:online','egammaMon:shifter','caloMon:t0']),
        ChainProp(name='HLT_g10_medium_L1eEM9', stream=['MinBias'], groups=SinglePhotonGroup+PrimaryPhIGroup),
    ]

    chains['Jet'] += [
        # ATR-30691/ATR-30692: Oxygen runs
        # Central and eta inclusive EMTopo jets
        ChainProp(name='HLT_j20_L1jJ10',   l1SeedThresholds=['FSNOSEED'], stream=['MinBias','express'], groups=SingleJetGroup+LowMuGroupPhI,   monGroups=['jetMon:t0','jetMon:online','jetMon:expert']),
        ChainProp(name='HLT_j30_L1jJ10',   l1SeedThresholds=['FSNOSEED'], stream=['MinBias'],           groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j40_L1jJ20',   l1SeedThresholds=['FSNOSEED'], stream=['MinBias','express'], groups=SingleJetGroup+LowMuGroupPhI,   monGroups=['jetMon:t0','jetMon:online','jetMon:expert','jetMon:shifter']),
        ChainProp(name='HLT_j50_L1jJ30',   l1SeedThresholds=['FSNOSEED'], stream=['MinBias'],           groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j60_L1jJ40',   l1SeedThresholds=['FSNOSEED'], stream=['MinBias'],           groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j20a_L1jTE10', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'],           groups=SingleJetGroup+LowMuGroupPhI,),
        ChainProp(name='HLT_j20a_L1jTE20', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'],           groups=SingleJetGroup+LowMuGroupPhI),

        # Forward EMTopo jets
        ChainProp(name='HLT_j25f_L1jJ10p30ETA49', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=SingleJetGroup+LowMuGroupPhI,  monGroups=['jetMon:t0','jetMon:online','jetMon:expert']),
        ChainProp(name='HLT_j35f_L1jJ10p30ETA49', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=SingleJetGroup+LowMuGroupPhI),

        # Central and eta inclusive HIP jets
        ChainProp(name='HLT_j20_ionp_L1jJ10',   l1SeedThresholds=['FSNOSEED'], stream=['MinBias','express'], groups=SingleJetGroup+LowMuGroupPhI,  monGroups=['jetMon:t0','jetMon:online','jetMon:expert']),
        ChainProp(name='HLT_j20a_ionp_L1jTE10', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'],           groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j20a_ionp_L1jTE20', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'],           groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j30_ionp_L1jJ10',   l1SeedThresholds=['FSNOSEED'], stream=['MinBias'],           groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j30a_ionp_L1jTE20', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'],           groups=SingleJetGroup+LowMuGroupPhI,  monGroups=['jetMon:t0','jetMon:online','jetMon:expert']),
        ChainProp(name='HLT_j40_ionp_L1jJ20',   l1SeedThresholds=['FSNOSEED'], stream=['MinBias','express'], groups=SingleJetGroup+LowMuGroupPhI,  monGroups=['jetMon:t0','jetMon:online','jetMon:expert','jetMon:shifter']),
        ChainProp(name='HLT_j40a_ionp_L1jTE20', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'],           groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j50_ionp_L1jJ30',   l1SeedThresholds=['FSNOSEED'], stream=['MinBias'],           groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j60_ionp_L1jJ40',   l1SeedThresholds=['FSNOSEED'], stream=['MinBias'],           groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j75_ionp_L1jJ50',   l1SeedThresholds=['FSNOSEED'], stream=['MinBias'],           groups=SingleJetGroup+LowMuGroupPhI),

        # Forward HIP jets
        ChainProp(name='HLT_j25f_ionp_L1jJ10p30ETA49', l1SeedThresholds=['FSNOSEED'], stream=['MinBias','express'], groups=SingleJetGroup+LowMuGroupPhI,  monGroups=['jetMon:t0','jetMon:online','jetMon:expert']),
        ChainProp(name='HLT_j35f_ionp_L1jJ10p30ETA49', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'],           groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j45f_ionp_L1jJ40p30ETA49', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'],           groups=SingleJetGroup+LowMuGroupPhI),

        # Central and eta inclusive pFlow jets
        ChainProp(name='HLT_j20_pf_ftf_L1jJ10',   l1SeedThresholds=['FSNOSEED'], stream=['MinBias','express'], groups=SingleJetGroup+LowMuGroupPhI,  monGroups=['jetMon:t0','jetMon:online','jetMon:expert']),
        ChainProp(name='HLT_j20a_pf_ftf_L1jTE10', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'],           groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j20a_pf_ftf_L1jTE20', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'],           groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j30_pf_ftf_L1jJ10',   l1SeedThresholds=['FSNOSEED'], stream=['MinBias'],           groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j30a_pf_ftf_L1jTE20', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'],           groups=SingleJetGroup+LowMuGroupPhI,  monGroups=['jetMon:t0','jetMon:online','jetMon:expert']),
        ChainProp(name='HLT_j40_pf_ftf_L1jJ20',   l1SeedThresholds=['FSNOSEED'], stream=['MinBias','express'], groups=SingleJetGroup+LowMuGroupPhI,  monGroups=['jetMon:t0','jetMon:online','jetMon:expert','jetMon:shifter']),
        ChainProp(name='HLT_j40a_pf_ftf_L1jTE20', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'],           groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j50_pf_ftf_L1jJ30',   l1SeedThresholds=['FSNOSEED'], stream=['MinBias'],           groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j60_pf_ftf_L1jJ40',   l1SeedThresholds=['FSNOSEED'], stream=['MinBias'],           groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j75_pf_ftf_L1jJ50',   l1SeedThresholds=['FSNOSEED'], stream=['MinBias'],           groups=SingleJetGroup+LowMuGroupPhI),
    ]

    chains['Combined'] += [
    ]

    chains['MinBias'] += [
        # ATR-30691/ATR-30692: Oxygen runs
        ChainProp(name='HLT_mb_sptrk_L1MBTS_1_1_VjTE50', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+SupportGroup),
        # ChainProp(name='HLT_mb_sptrk_L1ZDC_OR_VjTE50',   l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_A_C_VjTE50',  l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+SupportPhIGroup),
        # ChainProp(name='HLT_mb_sptrk_L1ZDC_XOR_VjTE50',  l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_sptrk_L1TRT_VjTE50',         l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+SupportPhIGroup),
        # ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_OR_VjTE50',  l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        # ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_A_C_VjTE50', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+SupportPhIGroup),
        # ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_XOR_VjTE50', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_sptrk_L1TRT_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['MinBias', 'express'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sptrk_L1jTE5',       l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1jTE10',      l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1jTE20',      l1SeedThresholds=['FSNOSEED'], stream=['MinBias', 'express'], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sptrk_L1jTE50',      l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+SupportPhIGroup),

        # ChainProp(name='HLT_mb_sptrk_L1ZDC_XNXN',   l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_mb_sptrk_L1ZDC_XNYN',   l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_mb_sptrk_L1ZDC_XNZN',   l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_mb_sptrk_L1ZDC_XN_XOR', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_mb_sptrk_L1ZDC_YN_XOR', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_mb_sptrk_L1ZDC_ZN_XOR', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_mb_sptrk_L1ZDC_YN',     l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_mb_sptrk_L1ZDC_ZN',     l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_mb_sptrk_L1ZDC_LOR',    l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_mb_sptrk_L1ZDC_YNYN',   l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_mb_sptrk_L1ZDC_A',      l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_mb_sptrk_L1ZDC_C',      l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_mb_sptrk_L1ZDC_A_C',    l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),

        # ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_OR',     l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:t0']),
        # ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_XNXN',   l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_XNYN',   l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_XNZN',   l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_XN_XOR', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_YN_XOR', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_ZN_XOR', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_YN',     l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_ZN',     l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_LOR',    l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_YNYN',   l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_A',      l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_C',      l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_A_C',    l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),

        ChainProp(name='HLT_mb_sp1500_trk100_hmt_L1TRT_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:t0']),
        # ChainProp(name='HLT_mb_sp1500_trk100_hmt_L1TRT_ZDC_OR', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp1500_trk100_hmt_L1ZDC_OR',     l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),

        ChainProp(name='HLT_mb_sp3000_trk200_hmt_L1TRT_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:t0']),
        # ChainProp(name='HLT_mb_sp3000_trk200_hmt_L1TRT_ZDC_OR', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp3000_trk200_hmt_L1ZDC_OR',     l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),

        ChainProp(name='HLT_mb_sp5000_trk290_hmt_L1TRT_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:t0']),
        # ChainProp(name='HLT_mb_sp5000_trk290_hmt_L1TRT_ZDC_OR', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp5000_trk290_hmt_L1ZDC_OR',     l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),

        ChainProp(name='HLT_mb_sp1500_pusup40_trk100_hmt_L1TRT_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:t0']),
        # ChainProp(name='HLT_mb_sp1500_pusup40_trk100_hmt_L1TRT_ZDC_OR', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp1500_pusup40_trk100_hmt_L1ZDC_OR',     l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),

        ChainProp(name='HLT_mb_sp3000_pusup100_trk200_hmt_L1TRT_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:t0']),
        # ChainProp(name='HLT_mb_sp3000_pusup100_trk200_hmt_L1TRT_ZDC_OR', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp3000_pusup100_trk200_hmt_L1ZDC_OR',     l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),

        ChainProp(name='HLT_mb_sp5000_pusup250_trk290_hmt_L1TRT_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:t0']),
        # ChainProp(name='HLT_mb_sp5000_pusup250_trk290_hmt_L1TRT_ZDC_OR', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp5000_pusup250_trk290_hmt_L1ZDC_OR',     l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),

        ChainProp(name='HLT_mb_sptrk_L1AFP_A_AND_C', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_L1AFP_A_OR_C',  l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_mbts_L1AFP_A_AND_C',  l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_mbts_L1AFP_A_OR_C',   l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),

        ChainProp(name='HLT_mb_sptrk_L1ZDC_A_C_VjTE50_OVERLAY', l1SeedThresholds=['FSNOSEED'], stream=['MinBiasOverlay'], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jTE50_OVERLAY',             l1SeedThresholds=['FSNOSEED'], stream=['MinBiasOverlay'], groups=MinBiasGroup+SupportPhIGroup),
    ]

    chains['HeavyIon'] += [
    ]

    chains['Streaming'] += [
        # ATR-30691/ATR-30692: Oxygen runs
        ChainProp(name='HLT_noalg_L1jTE5',           l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_noalg_L1jTE20',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_OR',         l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_XNXN',       l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_XNYN',       l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_XNZN',       l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_XN_XOR',     l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_YN_XOR',     l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_ZN_XOR',     l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_YN',         l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_ZN',         l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_LOR',        l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_YNYN',       l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_A',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_C',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_A_C',        l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_noalg_L1TRT_ZDC_OR',     l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_noalg_L1TRT_ZDC_XNXN',   l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_noalg_L1TRT_ZDC_XNYN',   l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_noalg_L1TRT_ZDC_XNZN',   l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_noalg_L1TRT_ZDC_XN_XOR', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_noalg_L1TRT_ZDC_YN_XOR', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_noalg_L1TRT_ZDC_ZN_XOR', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_noalg_L1TRT_ZDC_YN',     l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_noalg_L1TRT_ZDC_ZN',     l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_noalg_L1TRT_ZDC_LOR',    l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_noalg_L1TRT_ZDC_YNYN',   l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_noalg_L1TRT_ZDC_A',      l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_noalg_L1TRT_ZDC_C',      l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_noalg_L1TRT_ZDC_A_C',    l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),

        # ChainProp(name='HLT_noalg_L1ZDC_OR_EMPTY',            l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_LOR_EMPTY',           l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_OR_UNPAIRED_NONISO',  l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_LOR_UNPAIRED_NONISO', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),

        # ATR-31426
        ChainProp(name='HLT_noalg_L1LHCF',              l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_noalg_L1LHCF_EMPTY',        l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_noalg_L1LHCF_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
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
