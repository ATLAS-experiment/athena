# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# Dev_HI_run3_v1.py menu for Run 3 development
#------------------------------------------------------------------------#

# This defines the input format of the chain and it's properties with the defaults set
# always required are: name, stream and groups
#['name', 'L1chainParts'=[], 'stream', 'groups', 'merging'=[], 'topoStartFrom'=False],


from ..Config.Utility.ChainDefInMenu import ChainProp

from .Physics_pp_run3_v1 import (
    #SingleMuonGroup,
    MinBiasGroup,
    #MultiMuonGroup,
    #SinglePhotonGroup,
    #SingleElectronGroup,
    #MultiElectronGroup,
    PrimaryLegGroup,
    PrimaryPhIGroup,
    #PrimaryL1MuGroup,
    SupportGroup,
    SupportLegGroup,
    SupportPhIGroup,
    SingleJetGroup,
    #SingleBjetGroup,
    #TagAndProbeGroup,
    #ZeroBiasGroup
)

from .SignatureDicts import ChainStore
from .PhysicsP1_HI_run3_v1 import HardProbesStream,MinBiasStream,UPCStream,MinBiasOverlayStream,UCCStream
from . import MC_HI_run3_v1 as mc_menu


def getDevHISignatures():

    chains = ChainStore()
    chains['Muon'] += [
     ]

    chains['Egamma'] += [
    ]

    chains['Jet'] += [
        ChainProp(name='HLT_j40_ion_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+SupportGroup),
        ChainProp(name='HLT_j50_ion_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+SupportGroup),

        #--- UPC jets
        #test items w/o ZDC
        ChainProp(name='HLT_j10a_L1VTE200',     l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j10a_L1TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j15a_L1TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j20a_L1TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream, 'express'], groups=SingleJetGroup+PrimaryLegGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j30a_L1TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j40a_L1TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),

        ChainProp(name='HLT_j10a_pf_jes_ftf_L1VTE200',     l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j10a_pf_jes_ftf_L1TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j15a_pf_jes_ftf_L1TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j20a_pf_jes_ftf_L1TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream, 'express'], groups=SingleJetGroup+PrimaryLegGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j30a_pf_jes_ftf_L1TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),

        #for testing of the new jet multiplicity hypo
        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10a_pf_jes_ftf_L1jTE5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j15a_pf_jes_ftf_L1jTE5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j20a_pf_jes_ftf_L1jTE5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j30a_pf_jes_ftf_L1jTE5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j40a_pf_jes_ftf_L1jTE5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j10a_pf_jes_ftf_L1jTE5_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j15a_pf_jes_ftf_L1jTE5_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j20a_pf_jes_ftf_L1jTE5_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j30a_pf_jes_ftf_L1jTE5_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j40a_pf_jes_ftf_L1jTE5_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10a_pf_jes_ftf_L1gTE5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
    ]


    chains['Combined'] += [

        #----------- UPC HMT phase-1
        #test chains w/o ZDC
        ChainProp(name='HLT_mb_sptrk_hi_FgapC5_L1VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sptrk_hi_FgapA5_L1VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        #test phase-1 chains

        #trk25
        ChainProp(name='HLT_mb_sp400_trk25_hmt_hi_FgapA5_L1VZDC_A_ZDC_C_gTE3_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_sp400_trk25_hmt_hi_FgapC5_L1ZDC_A_VZDC_C_gTE3_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),

        #trk35
        ChainProp(name='HLT_mb_sp700_trk35_hmt_hi_FgapA5_L1VZDC_A_ZDC_C_gTE5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_sp700_trk35_hmt_hi_FgapC5_L1ZDC_A_VZDC_C_gTE5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),
        #test chains with eTAU1 and jTAU1
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L1eTAU1', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L1jTAU1', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1eTAU1', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1jTAU1', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup),
        #ATR-29784
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L1DPHI-2eEM1', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L1DPHI-2eTAU1', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L1DPHI-2jTAU1', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup),
        #ATR-29025 - temporary chains seeded of legacy items - for HLT studies
        ChainProp(name='HLT_mb_sp_vpix35_hi_FgapAC5_L1TAU1_TE4_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp_vpix40_hi_FgapAC5_L1TAU1_TE4_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp_vpix45_hi_FgapAC5_L1TAU1_TE4_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp_vpix50_hi_FgapAC5_L1TAU1_TE4_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp_vpix55_hi_FgapAC5_L1TAU1_TE4_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_L1TAU1_TE4_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryLegGroup),
        #ATR-29025 - ditau chains
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1eTAU1_TRT_VjTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1jTAU1_TRT_VjTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1eEM1_TRT_VjTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1jTAU1_TRT_VjTE100_GAP_AANDC', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),
   
    ]


    chains['MinBias'] += [
        ChainProp(name='HLT_mb_sp_vetospmbts2in_L1TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_excl_1trk2_pt1_L1TRT_VTE20',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup+['PS:NoHLTRepro']),

        #----------- magnetic monopoles legacy

        ChainProp(name='HLT_mb_sp_pix20_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_pix50_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_pix100_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_pix200_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),

        ChainProp(name='HLT_mb_sp_nototpix20_q2_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_sp_nototpix30_q2_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_sp_nototpix50_q2_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_sp_nototpix70_q2_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),
        #to estimate the rate change when adding q2 requirment
        ChainProp(name='HLT_mb_sp_nototpix20_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_sp_nototpix30_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_sp_nototpix50_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),
        # above chains are for performance studies only, for physics proper L1 seeded chains will be added: ATR-29741
        ChainProp(name='HLT_mb_sp_nototpix70_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_sp_nototpix100_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_sp_nototpix200_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_sp_nototpix500_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),
        #for tests of the pixsptrk ATR-29849
        ChainProp(name='HLT_mb_sp_nototpix20_q2_L1VjTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_sptrk_nototpix20_q2_L1VjTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_pixsptrk_nototpix20_q2_L1VjTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_pixsptrk_nototpix20_q2_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),

        ChainProp(name='HLT_mb_pixsptrk_nototpix30_q2_L1TRT_ZDC_A_C_VjTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_pixsptrk_nototpix30_q2_L1TRT_ZDC_XOR_VjTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_pixsptrk_nototpix30_q2_L1TRT_ZDC_XOR_VjTE10_UNPAIRED_NONISO', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_pixsptrk_nototpix20_q2_L1TRT_ZDC_A_C_VjTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_pixsptrk_nototpix20_q2_L1TRT_ZDC_XOR_VjTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_pixsptrk_nototpix20_q2_L1TRT_ZDC_XOR_VjTE10_UNPAIRED_NONISO', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),

        #----------- mbts
        ChainProp(name="HLT_mb_mbts_L1MBTS_2_2", l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:NoHLTRepro']),
        ChainProp(name="HLT_mb_mbts_L1MBTS_3_3", l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:NoHLTRepro']),
        ChainProp(name="HLT_mb_mbts_L1MBTS_4_4", l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:NoHLTRepro']),

        #----------- UPC HMT -legacy
        ChainProp(name='HLT_mb_sp_L1VTE50',                    l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp_L1MBTS_1_VTE50',             l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_L1TE3_VTE50',                l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp15_trk15_hmt_L1MBTS_1_VTE50', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp300_trk20_hmt_L1TE3_VTE50',   l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),

        #----Physics streamer for 2022 Nov HI test run, ATR-26405
        ChainProp(name='HLT_mb_sptrk_L1MBTS_1_VTE5', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
    ]


    chains['HeavyIon'] += [
        #----------- UPC Fgap
        ChainProp(name='HLT_hi_FgapAC3_L1VTE50',                    l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_hi_FgapAC5_L1VTE50',                    l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_hi_FgapAC10_L1VTE50',                    l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_hi_FgapA10_L1VTE50',                    l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_hi_FgapC10_L1VTE50',                    l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup, monGroups=['mbMon:t0']),

    ]

    chains['Streaming'] += [


        #----Physics streamer for 2022 Nov HI test run, ATR-26405
        ChainProp(name='HLT_noalg_L1VTE5',           l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1MBTS_1_VTE5',    l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_mb_L1MBTS_1_VTE5', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1gTE3',        l1SeedThresholds=['FSNOSEED'], stream=[UPCStream]   , groups=['PS:NoBulkMCProd']+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1gTE5',        l1SeedThresholds=['FSNOSEED'], stream=[UPCStream]   , groups=['PS:NoBulkMCProd']+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1gTE10',        l1SeedThresholds=['FSNOSEED'], stream=[UPCStream]   , groups=['PS:NoBulkMCProd']+SupportPhIGroup),

        ChainProp(name='HLT_noalg_L1eTAU1',           l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jTAU1',           l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
    ]


    return chains

def setupMenu():

    from AthenaCommon.Logging import logging
    log = logging.getLogger( __name__ )

    chains = mc_menu.setupMenu()

    log.info('[setupMenu] going to add the Dev menu chains now')

    for sig,chainsInSig in getDevHISignatures().items():
        chains[sig] += chainsInSig

    return chains
