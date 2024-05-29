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
from .PhysicsP1_HI_run3_v1 import HardProbesStream,MinBiasStream,UPCStream
from . import MC_HI_run3_v1 as mc_menu


def getDevHISignatures():

    chains = ChainStore()
    chains['Muon'] += [
     ]

    chains['Egamma'] += [
    ]

    chains['Jet'] += [
        ChainProp(name='HLT_j40_ion_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j50_ion_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+SupportLegGroup),

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

    ]


    chains['Combined'] += [

        #----------- UPC HMT phase-1
        #test chains w/o ZDC
        ChainProp(name='HLT_mb_sptrk_hi_FgapC5_L1VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sptrk_hi_FgapA5_L1VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),

        #test chains with eTAU1 and jTAU1
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L1eTAU1', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L1jTAU1', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1eTAU1', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1jTAU1', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup),
   
    ]


    chains['MinBias'] += [
        ChainProp(name='HLT_mb_sp_vetospmbts2in_L1TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_excl_1trk2_pt1_L1TRT_VTE20',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup+['PS:NoHLTRepro']),

        #----------- magnetic monopoles legacy

        ChainProp(name='HLT_mb_sp_pix20_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp_pix50_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp_pix100_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp_pix200_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup+['PS:NoHLTRepro']),

        ChainProp(name='HLT_mb_sp_nototpix70_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp_nototpix100_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp_nototpix200_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp_nototpix500_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup+['PS:NoHLTRepro']),

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
        ChainProp(name='HLT_hi_FgapAC3_L1VTE50',                    l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_hi_FgapAC5_L1VTE50',                    l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_hi_FgapAC10_L1VTE50',                    l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_hi_FgapA10_L1VTE50',                    l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_hi_FgapC10_L1VTE50',                    l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),

    ]

    chains['Streaming'] += [


        #----Physics streamer for 2022 Nov HI test run, ATR-26405
        ChainProp(name='HLT_noalg_L1VTE5',           l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1MBTS_1_VTE5',    l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_mb_L1MBTS_1_VTE5', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),

        ChainProp(name='HLT_noalg_L1eTAU1',           l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jTAU1',           l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
    ]


    return chains

def setupMenu(menu_name):

    from AthenaCommon.Logging import logging
    log = logging.getLogger( __name__ )

    chains = mc_menu.setupMenu(menu_name)

    log.info('[setupMenu] going to add the Dev menu chains now')

    for sig,chainsInSig in getDevHISignatures().items():
        chains[sig] += chainsInSig

    return chains
