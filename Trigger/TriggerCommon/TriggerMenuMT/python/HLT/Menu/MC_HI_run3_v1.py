
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# MC_HI_run3_v1.py menu for the obsoleted Phase-0 L1Calo based chains
#------------------------------------------------------------------------#

# This defines the input format of the chain and it's properties with the defaults set
# always required are: name, stream and groups
#['name', 'L1chainParts'=[], 'stream', 'groups', 'merging'=[], 'topoStartFrom'=False],
from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp
from .SignatureDicts import ChainStore

import TriggerMenuMT.HLT.Menu.PhysicsP1_HI_run3_v1 as physics_menu
from .Physics_pp_run3_v1 import (
        SingleElectronGroup,
        SinglePhotonGroup,
        SingleJetGroup,
        SingleBjetGroup,
        SingleMuonGroup,
        PrimaryL1MuGroup,
        MultiMuonGroup,
        MultiElectronGroup,
        MinBiasGroup,
        SupportLegGroup,
        PrimaryLegGroup,
        PrimaryPhIGroup,
        SupportPhIGroup,
        SupportGroup,
)

HardProbesStream="HardProbes"
MinBiasStream="MinBias"
UPCStream="UPC"
MinBiasOverlayStream="MinBiasOverlay"
PCStream="PC"
CCStream="CC"

from AthenaCommon.Logging import logging
log = logging.getLogger( __name__ )

def getMCSignatures():
    chains = ChainStore()


    chains['Muon'] = [
        #-- UPC
        ChainProp(name='HLT_mu3_L1MU3V_VTE50',        stream=[UPCStream], groups=SingleMuonGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu4_L1MU3V_VTE50',        stream=[UPCStream, 'express'], groups=SingleMuonGroup+PrimaryL1MuGroup, monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu4_L1MU3V_VTE200',        stream=[UPCStream], groups=SingleMuonGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu6_L1MU3V_VTE50',        stream=[UPCStream], groups=SingleMuonGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu8_L1MU5VF_VTE50',       stream=[UPCStream], groups=SingleMuonGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_2mu4_L12MU3V_VTE50',      stream=[UPCStream], groups=MultiMuonGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu4_mu4noL1_L1MU3V_VTE50',stream=[UPCStream], l1SeedThresholds=['MU3V','FSNOSEED'], groups=MultiMuonGroup+PrimaryL1MuGroup),
    ]

    chains['Egamma'] = [

        # ElectronChains----------
        #--------- legacy supporting electron chains
        ChainProp(name='HLT_e15_etcut_ion_L1EM12', stream=[HardProbesStream, 'express'] ,groups=SingleElectronGroup+SupportLegGroup, monGroups=['egammaMon:online','egammaMon:shifter_tag','egammaMon:shifter']),
        ChainProp(name='HLT_e18_etcut_ion_L1EM15', stream=[HardProbesStream] ,groups=SingleElectronGroup+SupportLegGroup),
        ChainProp(name='HLT_e20_etcut_ion_L1EM15', stream=[HardProbesStream, 'express'] ,groups=SingleElectronGroup+SupportLegGroup, monGroups=['egammaMon:online','egammaMon:shifter_tag','egammaMon:shifter']),
        ChainProp(name='HLT_e30_etcut_ion_L1EM15', stream=[HardProbesStream, 'express'] ,groups=SingleElectronGroup+SupportLegGroup, monGroups=['egammaMon:online','egammaMon:shifter_tag','egammaMon:shifter']),
        ChainProp(name='HLT_e50_etcut_ion_L1EM15', stream=[HardProbesStream] ,groups=SingleElectronGroup+SupportLegGroup),
        ChainProp(name='HLT_e20_idperf_loose_nogsf_ion_L1EM15', stream=[HardProbesStream, 'express'], groups=SingleElectronGroup+SupportLegGroup, monGroups=['idMon:t0']),


        #--------- legacy physics electon chains
        ChainProp(name='HLT_e15_lhloose_nogsf_ion_L1EM12',  stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryLegGroup, monGroups=['egammaMon:online','egammaMon:shifter_tag','egammaMon:shifter']),
        ChainProp(name='HLT_e15_loose_nogsf_ion_L1EM12',    stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryLegGroup, monGroups=['egammaMon:online','egammaMon:shifter_tag','egammaMon:shifter']),
        ChainProp(name='HLT_e15_lhmedium_nogsf_ion_L1EM12', stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryLegGroup),
        ChainProp(name='HLT_e15_medium_nogsf_ion_L1EM12',   stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryLegGroup),

        ChainProp(name='HLT_e18_lhloose_nogsf_ion_L1EM15',  stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryLegGroup, monGroups=['egammaMon:shifter_tp', 'egammaMon:shifter']),
        ChainProp(name='HLT_e18_loose_nogsf_ion_L1EM15',    stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryLegGroup, monGroups=['egammaMon:shifter_tp', 'egammaMon:shifter']),
        ChainProp(name='HLT_e18_lhmedium_nogsf_ion_L1EM15', stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryLegGroup),
        ChainProp(name='HLT_e18_medium_nogsf_ion_L1EM15',   stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryLegGroup),

        ChainProp(name='HLT_e20_lhloose_nogsf_ion_L1EM15',  stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryLegGroup),
        ChainProp(name='HLT_e20_lhmedium_nogsf_ion_L1EM15', stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryLegGroup),
        ChainProp(name='HLT_e20_loose_nogsf_ion_L1EM15',    stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryLegGroup),
        ChainProp(name='HLT_e20_medium_nogsf_ion_L1EM15',   stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryLegGroup),

        ChainProp(name='HLT_2e20_loose_nogsf_ion_L12EM15',  stream=[HardProbesStream, 'express'], groups=MultiElectronGroup+PrimaryLegGroup, monGroups=['egammaMon:online','egammaMon:shifter_tag','egammaMon:shifter']),

        # UPC electron chains
        #phase-1
        ChainProp(name='HLT_e10_lhloose_L1eEM9_VTE200',  stream=[UPCStream], groups=SingleElectronGroup+PrimaryPhIGroup, monGroups=['caloMon:t0']),
        ChainProp(name='HLT_e10_loose_L1eEM9_VTE200',  stream=[UPCStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e10_lhmedium_L1eEM9_VTE200',  stream=[UPCStream], groups=SingleElectronGroup+PrimaryPhIGroup, monGroups=['caloMon:t0']),
        ChainProp(name='HLT_e10_medium_L1eEM9_VTE200',  stream=[UPCStream], groups=SingleElectronGroup+PrimaryPhIGroup),

        # PhotonChains----------
        #----------- legacy support photon chains
        ChainProp(name='HLT_g13_etcut_ion_L1EM10', stream=[HardProbesStream], groups=SinglePhotonGroup+SupportLegGroup),
        ChainProp(name='HLT_g18_etcut_ion_L1EM10', stream=[HardProbesStream], groups=SinglePhotonGroup+SupportLegGroup),
        ChainProp(name='HLT_g28_etcut_ion_L1EM15', stream=[HardProbesStream], groups=SinglePhotonGroup+SupportLegGroup),
        #g15_etcut and g20_etcut have high thresholds, not sure they are needed, to be followed up
        ChainProp(name='HLT_g15_etcut_ion_L1EM12', stream=[HardProbesStream], groups=SinglePhotonGroup+SupportLegGroup),
        ChainProp(name='HLT_g18_etcut_ion_L1EM12', stream=[HardProbesStream], groups=SinglePhotonGroup+SupportLegGroup),
        ChainProp(name='HLT_g20_etcut_ion_L1EM12', stream=[HardProbesStream, 'express'], groups=SinglePhotonGroup+SupportLegGroup, monGroups=['egammaMon:online','egammaMon:shifter','egammaMon:shifter']),
        ChainProp(name='HLT_g18_etcut_L1EM10',     stream=[HardProbesStream], groups=SinglePhotonGroup+SupportLegGroup),
        ChainProp(name='HLT_g20_loose_L1EM12',     stream=[HardProbesStream], groups=SinglePhotonGroup+SupportLegGroup),

        #----------- legacy primary photon chains
        ChainProp(name='HLT_g15_loose_ion_L1EM10',  stream=[HardProbesStream], groups=SinglePhotonGroup+PrimaryLegGroup, monGroups=['egammaMon:online','egammaMon:shifter','egammaMon:val']),
        ChainProp(name='HLT_g15_loose_ion_L1EM12',  stream=[HardProbesStream], groups=SinglePhotonGroup+PrimaryLegGroup),
        ChainProp(name='HLT_g20_loose_ion_L1EM12',  stream=[HardProbesStream], groups=SinglePhotonGroup+PrimaryLegGroup, monGroups=['egammaMon:online','egammaMon:shifter','egammaMon:val']),
        ChainProp(name='HLT_g20_loose_ion_L1EM15',  stream=[HardProbesStream], groups=SinglePhotonGroup+PrimaryLegGroup, monGroups=['egammaMon:online','egammaMon:shifter','egammaMon:val']),
        ChainProp(name='HLT_g30_loose_ion_L1EM15',  stream=[HardProbesStream], groups=SinglePhotonGroup+PrimaryLegGroup),
        ChainProp(name='HLT_g50_loose_ion_L1EM15',  stream=[HardProbesStream, 'express'], groups=SinglePhotonGroup+PrimaryLegGroup, monGroups=['egammaMon:online','egammaMon:shifter','egammaMon:val']),

        # UPC photon chains
        #phase-1
        ChainProp(name='HLT_g10_loose_L1eEM9_VTE200',  stream=[UPCStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_g10_medium_L1eEM9_VTE200',  stream=[UPCStream], groups=SingleElectronGroup+PrimaryPhIGroup),

    ]

    chains['Jet'] = [
        #HI jets
        ChainProp(name='HLT_j50_ion_L1TE50', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j60_ion_L1TE50', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j30f_ion_L1TE20', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j50f_ion_L1TE50', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j50f_ion_L1J15p31ETA49', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream, 'express'], groups=SingleJetGroup+PrimaryLegGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j60f_ion_L1J15p31ETA49', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryLegGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j70f_ion_L1J30p31ETA49', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryLegGroup),

        ChainProp(name='HLT_j60_ion_L1J15', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream, 'express'], groups=SingleJetGroup+PrimaryLegGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j75_ion_L1J20', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j75_ion_L1J30', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream, 'express'], groups=SingleJetGroup+PrimaryLegGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_j85_ion_L1J15', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j85_ion_L1J30', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream, 'express'], groups=SingleJetGroup+PrimaryLegGroup,monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j100_ion_L1J30',l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j120_ion_L1J30',l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j150_ion_L1J50',l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryLegGroup,monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j200_ion_L1J50',l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream, 'express'], groups=SingleJetGroup+PrimaryLegGroup,monGroups=['jetMon:t0','jetMon:online']),

        # UPC jets
        #primary
        ChainProp(name='HLT_j10a_L1VZDC_A_VZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j10a_L1ZDC_XOR_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j10a_L11ZDC_NZDC_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j15a_L1VZDC_A_VZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j15a_L1ZDC_XOR_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j15a_L11ZDC_NZDC_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j20a_L1VZDC_A_VZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream, 'express'], groups=SingleJetGroup+PrimaryLegGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j20a_L1ZDC_XOR_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream, 'express'], groups=SingleJetGroup+PrimaryLegGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j20a_L11ZDC_NZDC_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream, 'express'], groups=SingleJetGroup+PrimaryLegGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j30a_L1VZDC_A_VZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j30a_L1ZDC_XOR_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j30a_L11ZDC_NZDC_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j40a_L1VZDC_A_VZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j40a_L1ZDC_XOR_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j40a_L11ZDC_NZDC_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),

        #supporting
        ChainProp(name='HLT_j10a_L1VZDC_A_VZDC_C_TE5_VTE200_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j10a_L1ZDC_XOR_TE5_VTE200_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j15a_L1VZDC_A_VZDC_C_TE5_VTE200_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j15a_L1ZDC_XOR_TE5_VTE200_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j10a_L15ZDC_A_5ZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j15a_L15ZDC_A_5ZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j20a_L15ZDC_A_5ZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j30a_L15ZDC_A_5ZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j40a_L15ZDC_A_5ZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j10a_L1VZDC_A_VZDC_C_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j10a_L1ZDC_XOR_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j40a_L1VZDC_A_VZDC_C_TE5', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j40a_L1ZDC_XOR_TE5', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),

        #Pflow UPC jets
        #primary
        ChainProp(name='HLT_j10a_pf_jes_ftf_L1VZDC_A_VZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j10a_pf_jes_ftf_L1ZDC_XOR_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j10a_pf_jes_ftf_L11ZDC_NZDC_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j15a_pf_jes_ftf_L1VZDC_A_VZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j15a_pf_jes_ftf_L1ZDC_XOR_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j15a_pf_jes_ftf_L11ZDC_NZDC_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j20a_pf_jes_ftf_L1VZDC_A_VZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream, 'express'], groups=SingleJetGroup+PrimaryLegGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j20a_pf_jes_ftf_L1ZDC_XOR_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream, 'express'], groups=SingleJetGroup+PrimaryLegGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j20a_pf_jes_ftf_L11ZDC_NZDC_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream, 'express'], groups=SingleJetGroup+PrimaryLegGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j30a_pf_jes_ftf_L1VZDC_A_VZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j30a_pf_jes_ftf_L1ZDC_XOR_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j30a_pf_jes_ftf_L11ZDC_NZDC_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j40a_pf_jes_ftf_L1VZDC_A_VZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j40a_pf_jes_ftf_L1ZDC_XOR_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j40a_pf_jes_ftf_L11ZDC_NZDC_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),

        #supporting
        ChainProp(name='HLT_j10a_pf_jes_ftf_L1VZDC_A_VZDC_C_TE5_VTE200_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j10a_pf_jes_ftf_L1ZDC_XOR_TE5_VTE200_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j15a_pf_jes_ftf_L1VZDC_A_VZDC_C_TE5_VTE200_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j15a_pf_jes_ftf_L1ZDC_XOR_TE5_VTE200_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j10a_pf_jes_ftf_L15ZDC_A_5ZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j15a_pf_jes_ftf_L15ZDC_A_5ZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j20a_pf_jes_ftf_L15ZDC_A_5ZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j30a_pf_jes_ftf_L15ZDC_A_5ZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j40a_pf_jes_ftf_L15ZDC_A_5ZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j10a_pf_jes_ftf_L1VZDC_A_VZDC_C_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j10a_pf_jes_ftf_L1ZDC_XOR_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j40a_pf_jes_ftf_L1VZDC_A_VZDC_C_TE5', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j40a_pf_jes_ftf_L1ZDC_XOR_TE5', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),

        #Pflow UPC jets, OOTP cleaning
        #primary
        ChainProp(name='HLT_j0_HT300XX15ptXX0eta490XXveto_j10a_pf_jes_ftf_L1VZDC_A_VZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j0_HT300XX15ptXX0eta490XXveto_j10a_pf_jes_ftf_L1ZDC_XOR_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j0_HT300XX15ptXX0eta490XXveto_j10a_pf_jes_ftf_L11ZDC_NZDC_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j0_HT300XX10ptXX0eta490XXveto_j10a_pf_jes_ftf_L1VZDC_A_VZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j0_HT300XX10ptXX0eta490XXveto_j10a_pf_jes_ftf_L1ZDC_XOR_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j0_HT300XX10ptXX0eta490XXveto_j10a_pf_jes_ftf_L11ZDC_NZDC_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j0_HT300XX15ptXX0eta490XXveto_j15a_pf_jes_ftf_L1VZDC_A_VZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j0_HT300XX15ptXX0eta490XXveto_j15a_pf_jes_ftf_L1ZDC_XOR_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j0_HT300XX15ptXX0eta490XXveto_j15a_pf_jes_ftf_L11ZDC_NZDC_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j0_HT300XX15ptXX0eta490XXveto_j20a_pf_jes_ftf_L1VZDC_A_VZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream, 'express'], groups=SingleJetGroup+PrimaryLegGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j0_HT300XX15ptXX0eta490XXveto_j20a_pf_jes_ftf_L1ZDC_XOR_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream, 'express'], groups=SingleJetGroup+PrimaryLegGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j0_HT300XX15ptXX0eta490XXveto_j20a_pf_jes_ftf_L11ZDC_NZDC_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream, 'express'], groups=SingleJetGroup+PrimaryLegGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j0_HT300XX15ptXX0eta490XXveto_j30a_pf_jes_ftf_L1VZDC_A_VZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j0_HT300XX15ptXX0eta490XXveto_j30a_pf_jes_ftf_L1ZDC_XOR_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j0_HT300XX15ptXX0eta490XXveto_j30a_pf_jes_ftf_L11ZDC_NZDC_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j0_HT300XX15ptXX0eta490XXveto_j40a_pf_jes_ftf_L1VZDC_A_VZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j0_HT300XX15ptXX0eta490XXveto_j40a_pf_jes_ftf_L1ZDC_XOR_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j0_HT300XX15ptXX0eta490XXveto_j40a_pf_jes_ftf_L11ZDC_NZDC_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),

        #supporting
        ChainProp(name='HLT_j0_HT300XX15ptXX0eta490XXveto_j10a_pf_jes_ftf_L1VZDC_A_VZDC_C_TE5_VTE200_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j0_HT300XX15ptXX0eta490XXveto_j10a_pf_jes_ftf_L1ZDC_XOR_TE5_VTE200_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j0_HT300XX10ptXX0eta490XXveto_j10a_pf_jes_ftf_L1VZDC_A_VZDC_C_TE5_VTE200_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j0_HT300XX10ptXX0eta490XXveto_j10a_pf_jes_ftf_L1ZDC_XOR_TE5_VTE200_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j0_HT300XX15ptXX0eta490XXveto_j15a_pf_jes_ftf_L1VZDC_A_VZDC_C_TE5_VTE200_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j0_HT300XX15ptXX0eta490XXveto_j15a_pf_jes_ftf_L1ZDC_XOR_TE5_VTE200_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j0_HT300XX15ptXX0eta490XXveto_j10a_pf_jes_ftf_L15ZDC_A_5ZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j0_HT300XX10ptXX0eta490XXveto_j10a_pf_jes_ftf_L15ZDC_A_5ZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j0_HT300XX15ptXX0eta490XXveto_j15a_pf_jes_ftf_L15ZDC_A_5ZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j0_HT300XX15ptXX0eta490XXveto_j20a_pf_jes_ftf_L15ZDC_A_5ZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j0_HT300XX15ptXX0eta490XXveto_j30a_pf_jes_ftf_L15ZDC_A_5ZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j0_HT300XX15ptXX0eta490XXveto_j40a_pf_jes_ftf_L15ZDC_A_5ZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j0_HT300XX15ptXX0eta490XXveto_j10a_pf_jes_ftf_L1VZDC_A_VZDC_C_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j0_HT300XX15ptXX0eta490XXveto_j10a_pf_jes_ftf_L1ZDC_XOR_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j0_HT300XX10ptXX0eta490XXveto_j10a_pf_jes_ftf_L1VZDC_A_VZDC_C_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j0_HT300XX10ptXX0eta490XXveto_j10a_pf_jes_ftf_L1ZDC_XOR_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),

        #higher L1 TE seed
        #pflow primary
        ChainProp(name='HLT_j20a_pf_jes_ftf_L1VZDC_A_VZDC_C_TE10_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream, 'express'], groups=SingleJetGroup+PrimaryLegGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j30a_pf_jes_ftf_L1VZDC_A_VZDC_C_TE10_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j40a_pf_jes_ftf_L1VZDC_A_VZDC_C_TE10_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),

        ChainProp(name='HLT_j0_HT300XX15ptXX0eta490XXveto_j20a_pf_jes_ftf_L1VZDC_A_VZDC_C_TE10_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream, 'express'], groups=SingleJetGroup+PrimaryLegGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j0_HT300XX15ptXX0eta490XXveto_j30a_pf_jes_ftf_L1VZDC_A_VZDC_C_TE10_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j0_HT300XX15ptXX0eta490XXveto_j40a_pf_jes_ftf_L1VZDC_A_VZDC_C_TE10_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
    ]

    chains['Combined'] = [
        #----------- mu+j
        ChainProp(name='HLT_mu4_j50_ion_dRAB05_L1MU3V_J12', l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=PrimaryLegGroup+SingleBjetGroup, monGroups=['muonMon:online','bJetMon:online']),
        ChainProp(name='HLT_mu4_j20_L1MU3V_J12',            l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=SupportLegGroup+SingleBjetGroup),

        #----------- mu + UPC Fgap
        ChainProp(name='HLT_mu3_hi_FgapAC5_L1MU3V_VTE50', l1SeedThresholds=['MU3V','FSNOSEED'], stream=[UPCStream], groups=SingleMuonGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mu4_hi_FgapAC5_L1MU3V_VTE50', l1SeedThresholds=['MU3V','FSNOSEED'], stream=[UPCStream], groups=SingleMuonGroup+PrimaryLegGroup),

        #----------- UPC HMT - legacy
        #primary
        ChainProp(name='HLT_mb_sp50_trk15_hmt_hi_FgapA5_L1MBTS_1_VZDC_A_ZDC_C_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream, 'express'], groups=MinBiasGroup+PrimaryLegGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp50_trk15_hmt_hi_FgapA5_L1MBTS_1_1ZDC_A_1ZDC_C_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup, monGroups=['mbMon:shifter']),
        ChainProp(name='HLT_mb_sp50_trk15_hmt_hi_FgapA5_L1MBTS_1_ZDC_1XOR5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp50_trk15_hmt_hi_FgapC5_L1MBTS_1_ZDC_A_VZDC_C_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream, 'express'], groups=MinBiasGroup+PrimaryLegGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp50_trk15_hmt_hi_FgapC5_L1MBTS_1_1ZDC_A_1ZDC_C_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup, monGroups=['mbMon:shifter']),
        ChainProp(name='HLT_mb_sp50_trk15_hmt_hi_FgapC5_L1MBTS_1_ZDC_1XOR5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup, monGroups=['mbMon:t0']),

        ChainProp(name='HLT_mb_sp400_trk25_hmt_hi_FgapA5_L1VZDC_A_ZDC_C_TE3_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp400_trk25_hmt_hi_FgapA5_L11ZDC_A_1ZDC_C_TE3_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp400_trk25_hmt_hi_FgapA5_L1ZDC_1XOR5_TE3_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp400_trk25_hmt_hi_FgapC5_L1ZDC_A_VZDC_C_TE3_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp400_trk25_hmt_hi_FgapC5_L11ZDC_A_1ZDC_C_TE3_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp400_trk25_hmt_hi_FgapC5_L1ZDC_1XOR5_TE3_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),

        ChainProp(name='HLT_mb_sp700_trk35_hmt_hi_FgapA5_L1VZDC_A_ZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp700_trk35_hmt_hi_FgapA5_L11ZDC_A_1ZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp700_trk35_hmt_hi_FgapA5_L1ZDC_1XOR5_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp700_trk35_hmt_hi_FgapC5_L1ZDC_A_VZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp700_trk35_hmt_hi_FgapC5_L11ZDC_A_1ZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp700_trk35_hmt_hi_FgapC5_L1ZDC_1XOR5_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),

        #supporting
        ChainProp(name='HLT_mb_sp50_trk15_hmt_hi_FgapA5_L1MBTS_2_VZDC_A_ZDC_C_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_sp50_trk15_hmt_hi_FgapA5_L1MBTS_2_1ZDC_A_1ZDC_C_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_sp50_trk15_hmt_hi_FgapA5_L1MBTS_2_ZDC_1XOR5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_sp50_trk15_hmt_hi_FgapC5_L1MBTS_2_ZDC_A_VZDC_C_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_sp50_trk15_hmt_hi_FgapC5_L1MBTS_2_1ZDC_A_1ZDC_C_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_sp50_trk15_hmt_hi_FgapC5_L1MBTS_2_ZDC_1XOR5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),

        ChainProp(name='HLT_mb_sp400_trk25_hmt_hi_FgapA5_L1VZDC_A_ZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_sp400_trk25_hmt_hi_FgapA5_L11ZDC_A_1ZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_sp400_trk25_hmt_hi_FgapA5_L1ZDC_1XOR5_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_sp400_trk25_hmt_hi_FgapC5_L1ZDC_A_VZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_sp400_trk25_hmt_hi_FgapC5_L11ZDC_A_1ZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_sp400_trk25_hmt_hi_FgapC5_L1ZDC_1XOR5_TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),

        ChainProp(name='HLT_mb_sptrk_hi_FgapA5_L1VZDC_A_ZDC_C_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sptrk_hi_FgapA5_L11ZDC_A_1ZDC_C_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup, monGroups=['mbMon:shifter']),
        ChainProp(name='HLT_mb_sptrk_hi_FgapA5_L1ZDC_1XOR5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sptrk_hi_FgapC5_L1ZDC_A_VZDC_C_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sptrk_hi_FgapC5_L11ZDC_A_1ZDC_C_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup, monGroups=['mbMon:shifter']),
        ChainProp(name='HLT_mb_sptrk_hi_FgapC5_L1ZDC_1XOR5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup, monGroups=['mbMon:t0']),

        #----------- UPC diphotons/dielectrons - legacy
        #primary
        ChainProp(name='HLT_mb_sp_vpix15_hi_FgapAC5_L1TAU1_TE4_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp_vpix15_hi_FgapAC5_L1TAU2_TE4_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp_vpix15_hi_FgapAC5_L12TAU1_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp_vpix15_hi_FgapAC5_L1TAU8_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L1TAU1_TE4_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream, 'express'],groups=MinBiasGroup+PrimaryLegGroup, monGroups=['mbMon:shifter']),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L1TAU2_TE4_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream, 'express'],groups=MinBiasGroup+PrimaryLegGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L12TAU1_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream, 'express'],groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L1TAU8_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L12TAU1_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1TAU1_TE4_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryLegGroup, monGroups=['mbMon:shifter']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1TAU2_TE4_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1TAU8_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryLegGroup),

        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_2g0_etcut_L12TAU1_VTE200', l1SeedThresholds=['FSNOSEED']*2+['TAU1'],stream=[UPCStream, 'express'],groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_2g0_etcut_25dphiCC_L12TAU1_VTE200', l1SeedThresholds=['FSNOSEED']*2+['TAU1'],stream=[UPCStream, 'express'],groups=MinBiasGroup+PrimaryLegGroup),

        #supporting
        ChainProp(name='HLT_mb_sp_vpix15_hi_FgapAC5_L1TAU1_TE4_VTE200_EMPTY', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L1TAU1_TE4_VTE200_EMPTY', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L12TAU1_VTE200_EMPTY', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L12TAU1_VTE200_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L12TAU1_VTE200_UNPAIRED_NONISO', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L1TAU8_VTE200_EMPTY', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1TAU1_TE4_VTE200_EMPTY', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L11ZDC_A_1ZDC_C_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1VZDC_A_VZDC_C_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1TRT_VZDC_A_VZDC_C_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1ZDC_XOR4_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt10_hi_FgapAC5_L11ZDC_A_1ZDC_C_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_sptrk_hi_FgapAC5_L12TAU1_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_sptrk_hi_FgapAC5_L1TAU8_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_sptrk_hi_FgapAC5_L1VTE50', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream, 'express'],groups=MinBiasGroup+SupportLegGroup),

        #for tests
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_2g0_etcut_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED']*2+['TAU1'],stream=[UPCStream],groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_2g0_etcut_25dphiCC_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED']*2+['TAU1'],stream=[UPCStream],groups=MinBiasGroup+PrimaryLegGroup),

        #----------- UPC diphotons/dielectrons - mixed
        #primary
        ChainProp(name='HLT_mb_sp_vpix15_hi_FgapAC5_L1eEM1_TE4_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp_vpix15_hi_FgapAC5_L1eEM2_TE4_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L1eEM1_TE4_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream, 'express'],groups=MinBiasGroup+PrimaryPhIGroup+['PS:NoHLTRepro'], monGroups=['mbMon:shifter']),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L1eEM2_TE4_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream, 'express'],groups=MinBiasGroup+PrimaryPhIGroup+['PS:NoHLTRepro'], monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L12eEM1_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream, 'express'],groups=MinBiasGroup+PrimaryPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L12eEM2_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L12eEM1_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L12eEM2_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1eEM1_TE4_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1eEM2_TE4_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup+['PS:NoHLTRepro']),
        #supporting
        ChainProp(name='HLT_mb_sp_vpix15_hi_FgapAC5_L1eEM1_TE4_VTE200_EMPTY', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L1eEM1_TE4_VTE200_EMPTY', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1eEM1_TE4_VTE200_EMPTY', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        
        #----------- UPC ditaus - legacy
        #primary
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1TAU1_VZDC_A_VZDC_C_VTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream, 'express'], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1TAU1_ZDC_XOR4_VTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream, 'express'], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt2_hi_FgapAC5_L1TAU1_VZDC_A_VZDC_C_VTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt2_hi_FgapAC5_L1TAU1_ZDC_XOR4_VTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1TAU2_VZDC_A_VZDC_C_VTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream, 'express'], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1TAU2_ZDC_XOR4_VTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream, 'express'], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt2_hi_FgapAC5_L1TAU2_VZDC_A_VZDC_C_VTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt2_hi_FgapAC5_L1TAU2_ZDC_XOR4_VTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1TAU1_TRT_VZDC_A_VZDC_C_VTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1TAU1_TRT_ZDC_XOR4_VTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt2_hi_FgapAC5_L1TAU1_TRT_VZDC_A_VZDC_C_VTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt2_hi_FgapAC5_L1TAU1_TRT_ZDC_XOR4_VTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1TAU1_TRT_VTE50', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup, monGroups=['mbMon:shifter']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1TRT_VTE50', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream, 'express'], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1TRT_VTE20', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1TRT_VZDC_A_VZDC_C_VTE50', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1TRT_VZDC_A_VZDC_C_VTE20', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),

        #----------- UPC ditaus - mixed
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1eEM1_VZDC_A_VZDC_C_VTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream, 'express'], groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1eEM1_ZDC_XOR4_VTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt2_hi_FgapAC5_L1eEM1_VZDC_A_VZDC_C_VTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt2_hi_FgapAC5_L1eEM1_ZDC_XOR4_VTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1eEM2_VZDC_A_VZDC_C_VTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream, 'express'], groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1eEM2_ZDC_XOR4_VTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt2_hi_FgapAC5_L1eEM2_VZDC_A_VZDC_C_VTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt2_hi_FgapAC5_L1eEM2_ZDC_XOR4_VTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),

    ]

    chains['MinBias'] = [
        #----------- magnetic monopoles legacy

        ChainProp(name='HLT_mb_sp_pix20_L1ZDC_A_C_VTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp_pix50_L1ZDC_A_C_VTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp_pix100_L1ZDC_A_C_VTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup, monGroups=['mbMon:shifter']),
        ChainProp(name='HLT_mb_sp_pix200_L1ZDC_A_C_VTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_nototpix70_excl_0trk2_pt0p2_L1ZDC_A_C_VTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_nototpix100_excl_0trk2_pt0p2_L1ZDC_A_C_VTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream, 'express'], groups=MinBiasGroup+PrimaryLegGroup, monGroups=['mbMon:shifter']),

        ChainProp(name='HLT_mb_sp_pix20_L1ZDC_A_C_VTE10_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp_pix50_L1ZDC_A_C_VTE10_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp_pix100_L1ZDC_A_C_VTE10_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp_pix200_L1ZDC_A_C_VTE10_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),

        ChainProp(name='HLT_mb_sp_pix20_L1ZDC_A_C_VTE10_UNPAIRED_NONISO', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp_pix50_L1ZDC_A_C_VTE10_UNPAIRED_NONISO', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp_pix100_L1ZDC_A_C_VTE10_UNPAIRED_NONISO', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp_pix200_L1ZDC_A_C_VTE10_UNPAIRED_NONISO', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),

        ChainProp(name='HLT_mb_sp_pix20_L1ZDC_A_C_VTE10_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp_pix50_L1ZDC_A_C_VTE10_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp_pix100_L1ZDC_A_C_VTE10_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp_pix200_L1ZDC_A_C_VTE10_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),

        ChainProp(name='HLT_mb_sp_pix20_L1ZDC_XOR_VTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp_pix50_L1ZDC_XOR_VTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp_pix100_L1ZDC_XOR_VTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp_pix200_L1ZDC_XOR_VTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),

        ChainProp(name='HLT_mb_sp_nototpix70_L1ZDC_A_C_VTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp_nototpix100_L1ZDC_A_C_VTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup, monGroups=['mbMon:shifter']),
        ChainProp(name='HLT_mb_sp_nototpix200_L1ZDC_A_C_VTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp_nototpix500_L1ZDC_A_C_VTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),

        ChainProp(name='HLT_mb_sp_nototpix70_L1ZDC_A_C_VTE10_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp_nototpix100_L1ZDC_A_C_VTE10_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp_nototpix200_L1ZDC_A_C_VTE10_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp_nototpix500_L1ZDC_A_C_VTE10_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),

        ChainProp(name='HLT_mb_sp_nototpix70_L1ZDC_A_C_VTE10_UNPAIRED_NONISO', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp_nototpix100_L1ZDC_A_C_VTE10_UNPAIRED_NONISO', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp_nototpix200_L1ZDC_A_C_VTE10_UNPAIRED_NONISO', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp_nototpix500_L1ZDC_A_C_VTE10_UNPAIRED_NONISO', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),

        ChainProp(name='HLT_mb_sp_nototpix70_L1ZDC_A_C_VTE10_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp_nototpix100_L1ZDC_A_C_VTE10_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp_nototpix200_L1ZDC_A_C_VTE10_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp_nototpix500_L1ZDC_A_C_VTE10_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),

        ChainProp(name='HLT_mb_sp_nototpix70_L1ZDC_XOR_VTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp_nototpix100_L1ZDC_XOR_VTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp_nototpix200_L1ZDC_XOR_VTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp_nototpix500_L1ZDC_XOR_VTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_nototpix70_excl_0trk2_pt0p2_L1ZDC_XOR_VTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_nototpix100_excl_0trk2_pt0p2_L1ZDC_XOR_VTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup, monGroups=['mbMon:shifter']),

        #----------- sptrk
        ChainProp(name='HLT_mb_sptrk_L1VTE50',          l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sptrk_L1TRT_VTE20',          l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sptrk_L1MBTS_1_1_VTE50', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_A_C_VTE50',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportLegGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sptrk_pc_L1ZDC_A_C_VTE50',l1SeedThresholds=['FSNOSEED'], stream=[PCStream, 'express'], groups=MinBiasGroup+SupportLegGroup, monGroups=['mbMon:t0']),

        #----------- UPC HMT -legacy
        #supporting
        ChainProp(name='HLT_mb_sptrk_L11ZDC_A_1ZDC_C_VTE200',    l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_1XOR5_VTE200',    l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_XOR_VTE200',    l1SeedThresholds=['FSNOSEED'], stream=[UPCStream, 'express'], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_sp50_trk15_hmt_L1MBTS_1_1ZDC_A_1ZDC_C_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp50_trk15_hmt_L1MBTS_1_ZDC_1XOR5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_sp50_trk15_hmt_L1MBTS_1_ZDC_XOR_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_sp400_trk25_hmt_L11ZDC_A_1ZDC_C_TE3_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_sp400_trk25_hmt_L1ZDC_1XOR5_TE3_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_sp400_trk25_hmt_L1ZDC_XOR_TE3_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_sp700_trk35_hmt_L11ZDC_A_1ZDC_C_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_sp700_trk35_hmt_L1ZDC_1XOR5_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_sp700_trk35_hmt_L1ZDC_XOR_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_OR_VTE200_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_sp50_trk15_hmt_L1MBTS_1_ZDC_OR_VTE200_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),

        #----------- UPC exclusive dileptons - legacy
        #supporting
        ChainProp(name='HLT_mb_excl_1trk5_pt1_L12TAU1_VTE200', l1SeedThresholds=['FSNOSEED'],stream=[UPCStream],groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_L1TAU1_TE4_VTE200', l1SeedThresholds=['FSNOSEED'],stream=[UPCStream],groups=MinBiasGroup+SupportLegGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_L1TAU2_TE4_VTE200', l1SeedThresholds=['FSNOSEED'],stream=[UPCStream],groups=MinBiasGroup+SupportLegGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_L1TAU8_VTE200', l1SeedThresholds=['FSNOSEED'],stream=[UPCStream],groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_L1TAU1_TE4_VTE200_EMPTY', l1SeedThresholds=['FSNOSEED'],stream=[UPCStream],groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_L1TRT_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_L1TRT_VZDC_A_VZDC_C_VTE50', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_L1VZDC_A_VZDC_C_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_L11ZDC_A_1ZDC_C_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_L1ZDC_XOR4_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),

        ChainProp(name='HLT_mb_excl_1trk5_pt1_L1TAU1_TRT_VTE50', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_L1TRT_VTE50', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_L1TRT_VTE20', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_L1TRT_VZDC_A_VZDC_C_VTE20', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),

        #----------- UPC exclusive egamma - mixed
        #supporting
        #w/o ZDC
        ChainProp(name='HLT_mb_sptrk_L12eEM1_VTE200', l1SeedThresholds=['FSNOSEED'],stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_L12eEM1_VTE200', l1SeedThresholds=['FSNOSEED'],stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_L1eEM1_TE4_VTE200', l1SeedThresholds=['FSNOSEED'],stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro'], monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_L1eEM2_TE4_VTE200', l1SeedThresholds=['FSNOSEED'],stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro'], monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_L1eEM1_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_excl_1trk5_pt2_L1eEM1_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),

    ]

    chains['Streaming'] = [
        ChainProp(name='HLT_noalg_L1J12',   l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=['PS:NoBulkMCProd']+SupportLegGroup),
        ChainProp(name='HLT_noalg_L12TAU1_VTE200',    l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TAU1_TE4_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TAU1_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TAU2_TE4_VTE200',     l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TAU8_VTE200',     l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1ZDC_XOR_VTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1VZDC_A_VZDC_C_TE5_VTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1ZDC_XOR_TE5_VTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L11ZDC_NZDC_TE5_VTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),

        ChainProp(name='HLT_noalg_L1J15',   l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1J30',   l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),

        ChainProp(name='HLT_noalg_L1EM10',  l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream, 'express'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1EM12',  l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream, 'express'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup,  monGroups=['egammaMon:online','egammaMon:shifter']),
        ChainProp(name='HLT_noalg_L1EM15',  l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        #----TE MinBias----
        ChainProp(name='HLT_noalg_L1TE5',   l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE20',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE50',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE100', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE200', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        #----TE MinBias using restricted eta threshold----
        ChainProp(name='HLT_noalg_L1TE3p0ETA49',   l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE7p0ETA49',   l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE1500p0ETA49',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE3000p0ETA49',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE3500p0ETA49',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE6500p0ETA49',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE8000p0ETA49',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),

        ChainProp(name='HLT_noalg_L1TE50_VTE600p0ETA49',l1SeedThresholds=['FSNOSEED'], stream=[PCStream, 'express'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE600p0ETA49',l1SeedThresholds=['FSNOSEED'], stream=[CCStream, 'express'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),

        #----TE MinBias OVERLAY----
        ChainProp(name='HLT_noalg_L1MBTS_1_VTE50_OVERLAY', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasOverlayStream], groups=['PS:NoBulkMCProd']+['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE50_OVERLAY',         l1SeedThresholds=['FSNOSEED'], stream=[MinBiasOverlayStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE600p0ETA49_OVERLAY', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasOverlayStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE1500p0ETA49_OVERLAY',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasOverlayStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE3000p0ETA49_OVERLAY',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasOverlayStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE3500p0ETA49_OVERLAY',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasOverlayStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE6500p0ETA49_OVERLAY',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasOverlayStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE8000p0ETA49_OVERLAY',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasOverlayStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),

        ChainProp(name='HLT_noalg_L1MBTS_1_VTE50', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE50_VTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TRT_VTE200',   l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TRT_VTE20',    l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),


    ]


    return chains

def setupMenu():

    chains = physics_menu.getPhysicsHISignatures()

    log.info('[setupMenu] going to add the MC menu chains now')

    for sig,chainsInSig in getMCSignatures().items():
        chains[sig] += chainsInSig

    return chains


