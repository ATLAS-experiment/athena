# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# Dev_pp_lowMu_run3_v1.py menu for Run 3 development
#------------------------------------------------------------------------#

# All chains are represented as ChainProp objects in a ChainStore
from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp
from .SignatureDicts import ChainStore

from .Physics_pp_run3_v1 import (
    LowMuGroup,
    MinBiasGroup,
    PrimaryPhIGroup,
    SingleMuonGroup,
    SupportGroup,
    SupportPhIGroup,
    SingleJetGroup,
    SingleElectronGroup,
    SinglePhotonGroup
)
from .PhysicsP1_HI_run3_v1 import MinBiasStream
import TriggerMenuMT.HLT.Menu.PhysicsP1_pp_lowMu_run3_v1 as physics_menu


def getDevLowMuSignatures():
    chains = ChainStore()

    chains['Muon'] += [
        ChainProp(name='HLT_mu3_L1MU3V', stream=[MinBiasStream], groups=SingleMuonGroup+SupportGroup, monGroups=['muonMon:shifter','muonMon:online']),
    ]

    chains['Egamma'] += [
        ChainProp(name='HLT_e6_etcut_L1eEM5',  stream=[MinBiasStream], groups=SingleElectronGroup+SupportPhIGroup),
        ChainProp(name='HLT_e6_nopid_L1eEM5',  stream=[MinBiasStream], groups=SingleElectronGroup+SupportPhIGroup, monGroups=['egammaMon:online','egammaMon:shifter','egammaMon:val','caloMon:t0']),
        ChainProp(name='HLT_e6_loose_L1eEM5',  stream=[MinBiasStream], groups=SingleElectronGroup+SupportPhIGroup, monGroups=['egammaMon:online','egammaMon:shifter','egammaMon:val','caloMon:t0']),
        ChainProp(name='HLT_e6_lhloose_L1eEM5',  stream=[MinBiasStream], groups=SingleElectronGroup+SupportPhIGroup),
        ChainProp(name='HLT_e6_medium_L1eEM5',  stream=[MinBiasStream], groups=SingleElectronGroup+SupportPhIGroup),
        ChainProp(name='HLT_e6_lhmedium_L1eEM5',  stream=[MinBiasStream], groups=SingleElectronGroup+SupportPhIGroup),

        ChainProp(name='HLT_e10_etcut_L1eEM9', stream=[MinBiasStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e10_nopid_L1eEM9', stream=[MinBiasStream], groups=SingleElectronGroup+PrimaryPhIGroup, monGroups=['egammaMon:online','egammaMon:shifter','egammaMon:val','caloMon:t0']),
        ChainProp(name='HLT_e10_loose_L1eEM9', stream=[MinBiasStream], groups=SingleElectronGroup+PrimaryPhIGroup, monGroups=['egammaMon:online','egammaMon:shifter','egammaMon:val','caloMon:t0']),
        ChainProp(name='HLT_e10_lhloose_L1eEM9', stream=[MinBiasStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e10_medium_L1eEM9', stream=[MinBiasStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e10_lhmedium_L1eEM9', stream=[MinBiasStream], groups=SingleElectronGroup+PrimaryPhIGroup),

        ChainProp(name='HLT_g6_etcut_L1eEM5',  stream=[MinBiasStream], groups=SinglePhotonGroup+SupportPhIGroup),
        ChainProp(name='HLT_g6_nopid_L1eEM5',  stream=[MinBiasStream], groups=SinglePhotonGroup+SupportPhIGroup,   monGroups=['egammaMon:online','egammaMon:shifter','egammaMon:val','caloMon:t0']),
        ChainProp(name='HLT_g6_loose_L1eEM5',  stream=[MinBiasStream], groups=SinglePhotonGroup+SupportPhIGroup,   monGroups=['egammaMon:online','egammaMon:shifter','egammaMon:val','caloMon:t0']),
        ChainProp(name='HLT_g6_medium_L1eEM5',  stream=[MinBiasStream], groups=SinglePhotonGroup+SupportPhIGroup),

        ChainProp(name='HLT_g10_etcut_L1eEM9', stream=[MinBiasStream], groups=SinglePhotonGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_g10_nopid_L1eEM9', stream=[MinBiasStream], groups=SinglePhotonGroup+PrimaryPhIGroup,   monGroups=['egammaMon:online','egammaMon:shifter','egammaMon:val','caloMon:t0']),
        ChainProp(name='HLT_g10_loose_L1eEM9', stream=[MinBiasStream], groups=SinglePhotonGroup+PrimaryPhIGroup,   monGroups=['egammaMon:online','egammaMon:shifter','egammaMon:val','caloMon:t0']),
        ChainProp(name='HLT_g10_medium_L1eEM9', stream=[MinBiasStream], groups=SinglePhotonGroup+PrimaryPhIGroup),
    ]

    chains['Jet'] += [
        ChainProp(name='HLT_j10_L1jJ10',      l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=SingleJetGroup+SupportPhIGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j10f_L1jJ10',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=SingleJetGroup+SupportPhIGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j20f_L1jJ20',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j10_ion_L1jJ10',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=SingleJetGroup+SupportPhIGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j20_ion_L1jJ20',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j10f_ion_L1jJ10', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=SingleJetGroup+SupportPhIGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j20f_ion_L1jJ20', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:t0','jetMon:online']),
    ]

    chains['Combined'] += [
    ]

    chains['MinBias'] += [
        ChainProp(name='HLT_mb_sptrk_L1MBTS_1_1_VjTE50', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_OR_VjTE50',   l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_A_C_VjTE50',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_XOR_VjTE50',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_sptrk_L1TRT_VjTE50',         l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_OR_VjTE50',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_A_C_VjTE50', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_XOR_VjTE50', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_sptrk_L1TRT_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sptrk_L1jTE5',       l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1jTE10',      l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1jTE20',      l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sptrk_L1jTE50',      l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_sptrk_L1ZDC_XNXN', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_XNYN', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_XNZN', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_XN_XOR', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_YN_XOR',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_ZN_XOR', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_YN', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_ZN', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_LOR', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_YNYN', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_A', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_C', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_A_C', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_OR', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_XNXN', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_XNYN', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_XNZN', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_XN_XOR', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_YN_XOR',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_ZN_XOR', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_YN', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_ZN', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_LOR', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_YNYN', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_A', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_C', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_A_C', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_sp1500_trk100_hmt_L1TRT_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp1500_trk100_hmt_L1TRT_ZDC_OR', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp1500_trk100_hmt_L1ZDC_OR', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_sp3000_trk200_hmt_L1TRT_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp3000_trk200_hmt_L1TRT_ZDC_OR', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp3000_trk200_hmt_L1ZDC_OR', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_sp5000_trk290_hmt_L1TRT_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp5000_trk290_hmt_L1TRT_ZDC_OR', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp5000_trk290_hmt_L1ZDC_OR', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_sp1500_pusup40_trk100_hmt_L1TRT_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp1500_pusup40_trk100_hmt_L1TRT_ZDC_OR', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp1500_pusup40_trk100_hmt_L1ZDC_OR', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_sp3000_pusup100_trk200_hmt_L1TRT_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp3000_pusup100_trk200_hmt_L1TRT_ZDC_OR', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp3000_pusup100_trk200_hmt_L1ZDC_OR', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_sp5000_pusup250_trk290_hmt_L1TRT_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp5000_pusup250_trk290_hmt_L1TRT_ZDC_OR', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp5000_pusup250_trk290_hmt_L1ZDC_OR', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_sptrk_L1AFP_A_AND_C', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1AFP_A_OR_C',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_mbts_L1AFP_A_AND_C',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_mbts_L1AFP_A_OR_C',   l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
    ]

    chains['HeavyIon'] += [
    ]

    chains['Streaming'] += [
        ChainProp(name='HLT_noalg_L1jTE5',          l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_noalg_L1jTE20',         l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_noalg_L1ZDC_OR',        l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_noalg_L1ZDC_XNXN', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_noalg_L1ZDC_XNYN', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_noalg_L1ZDC_XNZN', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_noalg_L1ZDC_XN_XOR', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_noalg_L1ZDC_YN_XOR', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_noalg_L1ZDC_ZN_XOR',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_noalg_L1ZDC_YN', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_noalg_L1ZDC_ZN', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_noalg_L1ZDC_LOR', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_noalg_L1ZDC_YNYN', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_noalg_L1ZDC_A',        l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_noalg_L1ZDC_C',        l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_noalg_L1ZDC_A_C',        l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_noalg_L1TRT_ZDC_OR',        l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_noalg_L1TRT_ZDC_XNXN', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_noalg_L1TRT_ZDC_XNYN', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_noalg_L1TRT_ZDC_XNZN', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_noalg_L1TRT_ZDC_XN_XOR', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_noalg_L1TRT_ZDC_YN_XOR', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_noalg_L1TRT_ZDC_ZN_XOR',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_noalg_L1TRT_ZDC_YN', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_noalg_L1TRT_ZDC_ZN', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_noalg_L1TRT_ZDC_LOR', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_noalg_L1TRT_ZDC_YNYN', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_noalg_L1TRT_ZDC_A',        l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_noalg_L1TRT_ZDC_C',        l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_noalg_L1TRT_ZDC_A_C',        l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),

        ChainProp(name='HLT_noalg_L1ZDC_OR_EMPTY',        l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_noalg_L1ZDC_LOR_EMPTY',        l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_noalg_L1ZDC_OR_UNPAIRED_NONISO',        l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_noalg_L1ZDC_LOR_UNPAIRED_NONISO',        l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup),
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
