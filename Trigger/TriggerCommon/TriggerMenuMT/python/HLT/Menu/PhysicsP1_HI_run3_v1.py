# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# PhysicsP1_HI_run3_v1.py menu
#------------------------------------------------------------------------#

# All chains are represented as ChainProp objects in a ChainStore
from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp
from .SignatureDicts import ChainStore

from .Physics_pp_run3_v1 import (
        SingleMuonGroup,
        MultiMuonGroup,
        SingleElectronGroup,
        MultiElectronGroup,
        SinglePhotonGroup,
        SingleJetGroup,
        SingleBjetGroup,
        #MultiJetGroup,
        MinBiasGroup,
        SupportGroup,
        #Topo3Group,
        TagAndProbeGroup,
        PrimaryL1MuGroup,
        SupportPhIGroup,
        PrimaryPhIGroup,
        ZeroBiasGroup,
        JetPhaseIStreamersGroup,
        METPhaseIStreamersGroup,
        BphysicsGroup
)
from . import P1_run3_v1

PhysicsStream="Main"
HardProbesStream="HardProbes"
MinBiasStream="MinBias"
UPCStream="UPC"
MinBiasOverlayStream="MinBiasOverlay"
PCStream="PC"
CCStream="CC"
UCCStream="UCC"
### following stream tags not used yet, need to be implemented in StreamInfo.py before use
#PCpebStream="PCPEB"
#CCpebStream="CCPEB"

LowMuGroup = ['LowMu']
LowMuGroupPhI = ['LowMuPhaseI']
LowMuGroupLeg = ['LowMuLegacy']

def getPhysicsHISignatures():
    chains = ChainStore()

    chains['Muon'] = [
        #-- 1 mu
        ChainProp(name='HLT_mu4_L1MU3V', stream=[HardProbesStream], groups=SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_mu6_L1MU3V',   stream=[HardProbesStream, 'express'], groups=SingleMuonGroup+SupportGroup, monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu6_L1MU5VF',  stream=[HardProbesStream], groups=SingleMuonGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu8_L1MU5VF',  stream=[HardProbesStream, 'express'], groups=SingleMuonGroup+PrimaryL1MuGroup, monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu10_L1MU8F',  stream=[HardProbesStream], groups=SingleMuonGroup+PrimaryL1MuGroup, monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu10_L1MU5VF', stream=[HardProbesStream], groups=SingleMuonGroup+PrimaryL1MuGroup),

        #-- 2 mu
        ChainProp(name='HLT_2mu4_L12MU3V', stream=[HardProbesStream, 'express'], groups=MultiMuonGroup+PrimaryL1MuGroup, monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu4_mu4noL1_L1MU3V', stream=[HardProbesStream, 'express'], l1SeedThresholds=['MU3V','FSNOSEED'], groups=MultiMuonGroup+PrimaryL1MuGroup, monGroups=['muonMon:shifter','muonMon:online']),

        #-- tag-and-probe
        ChainProp(name='HLT_mu8_mu4_probe_L1MU5VF', l1SeedThresholds=['MU5VF','PROBEMU3V'], stream=[HardProbesStream], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu6_mu4_probe_L1MU3V',  l1SeedThresholds=['MU3V', 'PROBEMU3V'], stream=[HardProbesStream], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu4_mu4_probe_L1MU3V',  l1SeedThresholds=['MU3V', 'PROBEMU3V'], stream=[HardProbesStream], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu8_mu6_probe_L1MU5VF', l1SeedThresholds=['MU5VF','PROBEMU3V'], stream=[HardProbesStream], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu6_mu6_probe_L1MU3V',  l1SeedThresholds=['MU3V', 'PROBEMU3V'], stream=[HardProbesStream], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu4_mu6_probe_L1MU3V',  l1SeedThresholds=['MU3V', 'PROBEMU3V'], stream=[HardProbesStream], groups=SingleMuonGroup+TagAndProbeGroup),

        #-- mu_idperf for ID monitoring
        ChainProp(name='HLT_mu4_idperf_L1MU3V',  stream=[HardProbesStream,'express'], groups=SupportGroup+SingleMuonGroup, monGroups=['idMon:t0']),
        ChainProp(name='HLT_mu6_idperf_L1MU5VF', stream=[HardProbesStream,'express'], groups=SupportGroup+SingleMuonGroup, monGroups=['idMon:t0']),
        ChainProp(name='HLT_mu8_idperf_L1MU5VF', stream=[HardProbesStream,'express'], groups=SupportGroup+SingleMuonGroup, monGroups=['idMon:t0']),
        ChainProp(name='HLT_mu4_mu4_idperf_1invmAB5_L12MU3V',      l1SeedThresholds=['MU3V', 'MU3V'],  stream=[HardProbesStream,'express'], groups=MultiMuonGroup+SupportGroup, monGroups=['idMon:t0']),
        ChainProp(name='HLT_mu10_mu10_idperf_50invmAB130_L12MU5VF',l1SeedThresholds=['MU5VF','MU5VF'], stream=[HardProbesStream,'express'], groups=MultiMuonGroup+SupportGroup, monGroups=['idMon:shifter','idMon:t0']),

        #BLS
        ChainProp(name='HLT_2mu4_l2io_invmDimu_L12MU3V', stream=[HardProbesStream], groups=PrimaryL1MuGroup+MultiMuonGroup, monGroups=['bphysMon:shifter']),
        ChainProp(name='HLT_mu14_mu14_idtp_idZmumu_L12MU8F', stream=[HardProbesStream], groups=PrimaryL1MuGroup+MultiMuonGroup,  monGroups=['idMon:shifter','idMon:t0']),

        #-- UPC - phase-1
        ChainProp(name='HLT_mu3_L1MU3V_VjTE50',        stream=[UPCStream], groups=SingleMuonGroup+PrimaryL1MuGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mu4_L1MU3V_VjTE50',        stream=[UPCStream, 'express'], groups=SingleMuonGroup+PrimaryL1MuGroup+PrimaryPhIGroup, monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu4_L1MU3V_VjTE200',        stream=[UPCStream], groups=SingleMuonGroup+PrimaryL1MuGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mu6_L1MU3V_VjTE50',        stream=[UPCStream], groups=SingleMuonGroup+PrimaryL1MuGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mu8_L1MU5VF_VjTE50',       stream=[UPCStream], groups=SingleMuonGroup+PrimaryL1MuGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_2mu4_L12MU3V_VjTE50',      stream=[UPCStream], groups=MultiMuonGroup+PrimaryL1MuGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mu4_mu4noL1_L1MU3V_VjTE50',stream=[UPCStream], l1SeedThresholds=['MU3V','FSNOSEED'], groups=MultiMuonGroup+PrimaryL1MuGroup+PrimaryPhIGroup),
    ]
    chains['Bphysics'] += [

        #BLS triggers
        ChainProp(name='HLT_2mu4_bDimu_L12MU3V', stream=[HardProbesStream, 'express'], groups=PrimaryL1MuGroup+BphysicsGroup, monGroups=['bphysMon:online','bphysMon:shifter']),

    ]

    chains['Egamma'] = [

        # ElectronChains----------
        #--------- phase-1 supporting electron chains
        # replace L1eEM9 with L1eEM15 and L1eEM15 with eEM18, ATR-26366
        ChainProp(name='HLT_e15_etcut_ion_L1eEM15',    stream=[HardProbesStream, 'express'], groups=SingleElectronGroup+SupportPhIGroup, monGroups=['egammaMon:online','egammaMon:shifter_tag','egammaMon:shifter']),
        ChainProp(name='HLT_e20_etcut_ion_L1eEM18',   stream=[HardProbesStream, 'express'], groups=SingleElectronGroup+SupportPhIGroup, monGroups=['egammaMon:online','egammaMon:shifter_tag','egammaMon:shifter']),
        ChainProp(name='HLT_e30_etcut_ion_L1eEM26', stream=[HardProbesStream, 'express'] ,groups=SingleElectronGroup+SupportPhIGroup, monGroups=['egammaMon:online','egammaMon:shifter_tag','egammaMon:shifter']),
        ChainProp(name='HLT_e50_etcut_ion_L1eEM26', stream=[HardProbesStream] ,groups=SingleElectronGroup+SupportPhIGroup),
        ChainProp(name='HLT_e20_idperf_loose_nogsf_ion_L1eEM18', stream=[HardProbesStream, 'express'], groups=SingleElectronGroup+SupportPhIGroup, monGroups=['idMon:t0']),

        #--------- phase-1 physics electron chains
        ChainProp(name='HLT_e15_lhloose_nogsf_ion_L1eEM15',  stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryPhIGroup, monGroups=['egammaMon:t0_tp', 'egammaMon:shifter', 'caloMon:t0']),
        ChainProp(name='HLT_e15_loose_nogsf_ion_L1eEM15',    stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryPhIGroup, monGroups=['egammaMon:t0_tp', 'egammaMon:shifter']),
        ChainProp(name='HLT_e15_lhmedium_nogsf_ion_L1eEM15', stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryPhIGroup, monGroups=['caloMon:t0']),
        ChainProp(name='HLT_e15_medium_nogsf_ion_L1eEM15',   stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryPhIGroup),

        ChainProp(name='HLT_e20_lhloose_nogsf_ion_L1eEM18', stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e20_lhmedium_nogsf_ion_L1eEM18',stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e20_loose_nogsf_ion_L1eEM18',   stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e20_loose_nogsf_ion_L1eEM18L',  stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e20_medium_nogsf_ion_L1eEM18',  stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_2e20_loose_nogsf_ion_L12eEM18',stream=[HardProbesStream, 'express'], groups=MultiElectronGroup+PrimaryPhIGroup, monGroups=['egammaMon:online','egammaMon:shifter_tag','egammaMon:shifter']),

        # UPC electron chains
        #phase-1
        ChainProp(name='HLT_e10_lhloose_L1eEM9_VjTE200',  stream=[UPCStream], groups=SingleElectronGroup+PrimaryPhIGroup, monGroups=['caloMon:t0']),
        ChainProp(name='HLT_e10_loose_L1eEM9_VjTE200',  stream=[UPCStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e10_lhmedium_L1eEM9_VjTE200',  stream=[UPCStream], groups=SingleElectronGroup+PrimaryPhIGroup, monGroups=['caloMon:t0']),
        ChainProp(name='HLT_e10_medium_L1eEM9_VjTE200',  stream=[UPCStream], groups=SingleElectronGroup+PrimaryPhIGroup),

        # PhotonChains----------
        #----------- phase-1 support photon chains
        ChainProp(name='HLT_g13_etcut_ion_L1eEM12', stream=[HardProbesStream], groups=SinglePhotonGroup+SupportPhIGroup),
        ChainProp(name='HLT_g18_etcut_ion_L1eEM12', stream=[HardProbesStream], groups=SinglePhotonGroup+SupportPhIGroup),
        ChainProp(name='HLT_g28_etcut_ion_L1eEM18', stream=[HardProbesStream], groups=SinglePhotonGroup+SupportPhIGroup),
        ChainProp(name='HLT_g15_etcut_ion_L1eEM15', stream=[HardProbesStream], groups=SinglePhotonGroup+SupportPhIGroup), 
        ChainProp(name='HLT_g18_etcut_ion_L1eEM15', stream=[HardProbesStream], groups=SinglePhotonGroup+SupportPhIGroup),
        ChainProp(name='HLT_g20_etcut_ion_L1eEM15', stream=[HardProbesStream, 'express'], groups=SinglePhotonGroup+SupportPhIGroup,  monGroups=['egammaMon:online','egammaMon:shifter','egammaMon:val']),
        ChainProp(name='HLT_g18_etcut_L1eEM12',     stream=[HardProbesStream], groups=SinglePhotonGroup+SupportPhIGroup),
        ChainProp(name='HLT_g20_loose_L1eEM15',     stream=[HardProbesStream], groups=SinglePhotonGroup+SupportPhIGroup),
        ChainProp(name='HLT_2g15_loose_L12eEM12',   stream=[HardProbesStream], groups=SinglePhotonGroup+SupportPhIGroup),

        #----------- phase-1 primary photon chains
        ChainProp(name='HLT_g15_loose_ion_L1eEM12',  stream=[HardProbesStream], groups=SinglePhotonGroup+SupportPhIGroup, monGroups=['egammaMon:online','egammaMon:shifter','egammaMon:val','caloMon:t0']),
        ChainProp(name='HLT_g15_loose_ion_L1eEM15', stream=[HardProbesStream], groups=SinglePhotonGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_g20_loose_ion_L1eEM15', stream=[HardProbesStream], groups=SinglePhotonGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_g20_loose_ion_L1eEM18', stream=[HardProbesStream], groups=SinglePhotonGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_g30_loose_ion_L1eEM18',  stream=[HardProbesStream], groups=SinglePhotonGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_g50_loose_ion_L1eEM26',  stream=[HardProbesStream, 'express'], groups=SinglePhotonGroup+PrimaryPhIGroup,  monGroups=['egammaMon:online','egammaMon:shifter','egammaMon:val','caloMon:t0']),
        ChainProp(name='HLT_2g15_loose_ion_L12eEM12',stream=[HardProbesStream], groups=SinglePhotonGroup+PrimaryPhIGroup),

        # UPC photon chains
        #phase-1
        ChainProp(name='HLT_g10_loose_L1eEM9_VjTE200',  stream=[UPCStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_g10_medium_L1eEM9_VjTE200',  stream=[UPCStream], groups=SingleElectronGroup+PrimaryPhIGroup),

    ]

    chains['Jet'] = [
        # HI jets
       
        #HI jet chains with L1jTE
        ChainProp(name='HLT_j50_ion_L1jTE50', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j60_ion_L1jTE50', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j30f_ion_L1jTE20', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j50f_ion_L1jTE50', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+SupportPhIGroup),

        #--- phase-1 HI jets
        ChainProp(name='HLT_j60_ion_L1jJ40', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream, 'express'], groups=SingleJetGroup+SupportPhIGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j75_ion_L1jJ50', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j75_ion_L1jJ55', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream, 'express'], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j85_ion_L1jJ50', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j85_ion_L1jJ55', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream, 'express'], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j100_ion_L1jJ55', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j120_ion_L1jJ55', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j150_ion_L1jJ80', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j200_ion_L1jJ80', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream, 'express'], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:t0','jetMon:online']),

        ChainProp(name='HLT_j50f_ion_L1jJ40p30ETA49', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream, 'express'], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j60f_ion_L1jJ40p30ETA49', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j70f_ion_L1jJ60p30ETA49', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j80f_ion_L1jJ60p30ETA49', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j90f_ion_L1jJ90p30ETA49', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup),

        ChainProp(name='HLT_j40_a2_ion_L1jJ40', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j50_a2_ion_L1jJ50', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j60_a2_ion_L1jJ55', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j75_a2_ion_L1jJ55', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:t0','jetMon:online']),

        #UPC jets - primary candidates with jTE5
        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10a_pf_jes_ftf_L1VZDC_A_VZDC_C_jTE5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10a_pf_jes_ftf_L1ZDC_XOR_jTE5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j15a_pf_jes_ftf_L1VZDC_A_VZDC_C_jTE5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j15a_pf_jes_ftf_L1ZDC_XOR_jTE5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j20a_pf_jes_ftf_L1VZDC_A_VZDC_C_jTE5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j20a_pf_jes_ftf_L1ZDC_XOR_jTE5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j30a_pf_jes_ftf_L1VZDC_A_VZDC_C_jTE5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j30a_pf_jes_ftf_L1ZDC_XOR_jTE5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j40a_pf_jes_ftf_L1VZDC_A_VZDC_C_jTE5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j40a_pf_jes_ftf_L1ZDC_XOR_jTE5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        #backup with jTE10
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j15a_pf_jes_ftf_L1VZDC_A_VZDC_C_jTE10_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j15a_pf_jes_ftf_L1ZDC_XOR_jTE10_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j15a_pf_jes_ftf_L11ZDC_NZDC_jTE10_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j20a_pf_jes_ftf_L1VZDC_A_VZDC_C_jTE10_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j20a_pf_jes_ftf_L1ZDC_XOR_jTE10_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j20a_pf_jes_ftf_L11ZDC_NZDC_jTE10_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j30a_pf_jes_ftf_L1VZDC_A_VZDC_C_jTE10_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j30a_pf_jes_ftf_L1ZDC_XOR_jTE10_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j30a_pf_jes_ftf_L11ZDC_NZDC_jTE10_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j40a_pf_jes_ftf_L1VZDC_A_VZDC_C_jTE10_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j40a_pf_jes_ftf_L1ZDC_XOR_jTE10_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j40a_pf_jes_ftf_L11ZDC_NZDC_jTE10_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        #UPC jets supporting without lower TE
        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10a_pf_jes_ftf_L1VZDC_A_VZDC_C_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10a_pf_jes_ftf_L1ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        #UPC jets supporting with jTE5
        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10a_pf_jes_ftf_L15ZDC_A_5ZDC_C_jTE5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j15a_pf_jes_ftf_L15ZDC_A_5ZDC_C_jTE5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j20a_pf_jes_ftf_L15ZDC_A_5ZDC_C_jTE5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j30a_pf_jes_ftf_L15ZDC_A_5ZDC_C_jTE5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j40a_pf_jes_ftf_L15ZDC_A_5ZDC_C_jTE5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10a_pf_jes_ftf_L1VZDC_A_VZDC_C_jTE5_VjTE200_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10a_pf_jes_ftf_L1ZDC_XOR_jTE5_VjTE200_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j15a_pf_jes_ftf_L1VZDC_A_VZDC_C_jTE5_VjTE200_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j15a_pf_jes_ftf_L1ZDC_XOR_jTE5_VjTE200_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        # UPC jets - supporting with jJ5
        # ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10a_pf_jes_ftf_L1VZDC_A_VZDC_C_jJ5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        # ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10a_pf_jes_ftf_L1ZDC_XOR_jJ5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        # ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10a_pf_jes_ftf_L11ZDC_NZDC_jJ5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        # ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j15a_pf_jes_ftf_L1VZDC_A_VZDC_C_jJ5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        # ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j15a_pf_jes_ftf_L1ZDC_XOR_jJ5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        # ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j15a_pf_jes_ftf_L11ZDC_NZDC_jJ5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        # ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j20a_pf_jes_ftf_L1VZDC_A_VZDC_C_jJ5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        # ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j20a_pf_jes_ftf_L1ZDC_XOR_jJ5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        # ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j20a_pf_jes_ftf_L11ZDC_NZDC_jJ5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        # UPC jets - supporting with jJ10
        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10a_pf_jes_ftf_L1VZDC_A_VZDC_C_jJ10_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10a_pf_jes_ftf_L1ZDC_XOR_jJ10_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10a_pf_jes_ftf_L11ZDC_NZDC_jJ10_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j15a_pf_jes_ftf_L1VZDC_A_VZDC_C_jJ10_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j15a_pf_jes_ftf_L1ZDC_XOR_jJ10_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j15a_pf_jes_ftf_L11ZDC_NZDC_jJ10_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j20a_pf_jes_ftf_L1VZDC_A_VZDC_C_jJ10_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j20a_pf_jes_ftf_L1ZDC_XOR_jJ10_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j20a_pf_jes_ftf_L11ZDC_NZDC_jJ10_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        #HF UPC jets, ATR-30208
        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10_pf_jes_ftf_L1TRT_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10_pf_jes_ftf_L1eEM1_TRT_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10_pf_jes_ftf_L1eTAU1_TRT_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10_pf_jes_ftf_L1TRT_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),

        # Pb+Pb 2025:
        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10a_pf_jes_ftf_L1TRT_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j10a_preselVETOMULT11a10_pf_jes_ftf_L1TRT_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),

        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10a_pf_jes_ftf_L1TRT_ZDC_XOR_VjTE200',       l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10a_pf_jes_ftf_L1TRT_ZDC_A_C_VjTE200',       l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j10a_preselVETOMULT11a10_pf_jes_ftf_L1TRT_ZDC_XOR_VjTE200',       l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j10a_preselVETOMULT11a10_pf_jes_ftf_L1TRT_ZDC_A_C_VjTE200',       l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),

        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10a_pf_jes_ftf_L1TRT_VZDC_A_VZDC_C_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream, 'express'], groups=SingleJetGroup+SupportPhIGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10a_pf_jes_ftf_L1TRT_ZDC_OR_VjTE200',        l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream, 'express'], groups=SingleJetGroup+SupportPhIGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j15a_pf_jes_ftf_L1TRT_VZDC_A_VZDC_C_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j15a_pf_jes_ftf_L1TRT_ZDC_OR_VjTE200',        l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j20a_pf_jes_ftf_L1TRT_VZDC_A_VZDC_C_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j20a_pf_jes_ftf_L1TRT_ZDC_OR_VjTE200',        l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j10a_preselVETOMULT11a10_pf_jes_ftf_L1TRT_VZDC_A_VZDC_C_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream, 'express'], groups=SingleJetGroup+SupportPhIGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j10a_preselVETOMULT11a10_pf_jes_ftf_L1TRT_ZDC_OR_VjTE200',        l1SeedThresholds=['FSNOSEED'], stream=[UPCStream, 'express'], groups=SingleJetGroup+SupportPhIGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j15a_preselVETOMULT11a15_pf_jes_ftf_L1TRT_VZDC_A_VZDC_C_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j15a_preselVETOMULT11a15_pf_jes_ftf_L1TRT_ZDC_OR_VjTE200',        l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j20a_preselVETOMULT11a15_pf_jes_ftf_L1TRT_VZDC_A_VZDC_C_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j20a_preselVETOMULT11a15_pf_jes_ftf_L1TRT_ZDC_OR_VjTE200',        l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),

        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10a_pf_jes_ftf_L1TEA_VZDC_A_VZDC_C_jJ5_VjTE200',  l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j15a_pf_jes_ftf_L1TEA_VZDC_A_VZDC_C_jJ5_VjTE200',  l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j20a_pf_jes_ftf_L1TEA_VZDC_A_VZDC_C_jJ5_VjTE200',  l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10a_pf_jes_ftf_L1TEA_ZDC_XOR_jJ5_VjTE200',        l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j15a_pf_jes_ftf_L1TEA_ZDC_XOR_jJ5_VjTE200',        l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j20a_pf_jes_ftf_L1TEA_ZDC_XOR_jJ5_VjTE200',        l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j30a_pf_jes_ftf_L1TEA_VZDC_A_VZDC_C_jJ10_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j30a_pf_jes_ftf_L1TEA_ZDC_XOR_jJ10_VjTE200',       l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j40a_pf_jes_ftf_L1TEA_VZDC_A_VZDC_C_jJ10_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j40a_pf_jes_ftf_L1TEA_ZDC_XOR_jJ10_VjTE200',       l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j10a_preselVETOMULT11a10_pf_jes_ftf_L1TEA_VZDC_A_VZDC_C_jJ5_VjTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j15a_preselVETOMULT11a15_pf_jes_ftf_L1TEA_VZDC_A_VZDC_C_jJ5_VjTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j20a_preselVETOMULT11a15_pf_jes_ftf_L1TEA_VZDC_A_VZDC_C_jJ5_VjTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j10a_preselVETOMULT11a10_pf_jes_ftf_L1TEA_ZDC_XOR_jJ5_VjTE200',        l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j15a_preselVETOMULT11a15_pf_jes_ftf_L1TEA_ZDC_XOR_jJ5_VjTE200',        l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j20a_preselVETOMULT11a15_pf_jes_ftf_L1TEA_ZDC_XOR_jJ5_VjTE200',        l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j30a_preselVETOMULT11a15_pf_jes_ftf_L1TEA_VZDC_A_VZDC_C_jJ10_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j30a_preselVETOMULT11a15_pf_jes_ftf_L1TEA_ZDC_XOR_jJ10_VjTE200',       l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j40a_preselVETOMULT11a15_pf_jes_ftf_L1TEA_VZDC_A_VZDC_C_jJ10_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j40a_preselVETOMULT11a15_pf_jes_ftf_L1TEA_ZDC_XOR_jJ10_VjTE200',       l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),

        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j30a_pf_jes_ftf_L1ESP_VZDC_A_VZDC_C_jJ10_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j30a_pf_jes_ftf_L1ESP_ZDC_XOR_jJ10_VjTE200',       l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j40a_pf_jes_ftf_L1ESP_VZDC_A_VZDC_C_jJ10_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j40a_pf_jes_ftf_L1ESP_ZDC_XOR_jJ10_VjTE200',       l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j30a_preselVETOMULT11a15_pf_jes_ftf_L1ESP_VZDC_A_VZDC_C_jJ10_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j30a_preselVETOMULT11a15_pf_jes_ftf_L1ESP_ZDC_XOR_jJ10_VjTE200',       l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j40a_preselVETOMULT11a15_pf_jes_ftf_L1ESP_VZDC_A_VZDC_C_jJ10_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j40a_preselVETOMULT11a15_pf_jes_ftf_L1ESP_ZDC_XOR_jJ10_VjTE200',       l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),

        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10f_pf_jes_ftf_L1TEA_VZDC_A_VZDC_C_jJ5p30ETA49_VjTE200',  l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10f_pf_jes_ftf_L1TEA_ZDC_XOR_jJ5p30ETA49_VjTE200',        l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j15f_pf_jes_ftf_L1TEA_VZDC_A_VZDC_C_jJ5p30ETA49_VjTE200',  l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j15f_pf_jes_ftf_L1TEA_ZDC_XOR_jJ5p30ETA49_VjTE200',        l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j20f_pf_jes_ftf_L1TEA_VZDC_A_VZDC_C_jJ5p30ETA49_VjTE200',  l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j20f_pf_jes_ftf_L1TEA_ZDC_XOR_jJ5p30ETA49_VjTE200',        l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j30f_pf_jes_ftf_L1TEA_VZDC_A_VZDC_C_jJ10p30ETA49_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j30f_pf_jes_ftf_L1TEA_ZDC_XOR_jJ10p30ETA49_VjTE200',       l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j40f_pf_jes_ftf_L1TEA_VZDC_A_VZDC_C_jJ10p30ETA49_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j40f_pf_jes_ftf_L1TEA_ZDC_XOR_jJ10p30ETA49_VjTE200',       l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j10f_preselVETOMULT11a10_pf_jes_ftf_L1TEA_VZDC_A_VZDC_C_jJ5p30ETA49_VjTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j10f_preselVETOMULT11a10_pf_jes_ftf_L1TEA_ZDC_XOR_jJ5p30ETA49_VjTE200',        l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j15f_preselVETOMULT11a15_pf_jes_ftf_L1TEA_VZDC_A_VZDC_C_jJ5p30ETA49_VjTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j15f_preselVETOMULT11a15_pf_jes_ftf_L1TEA_ZDC_XOR_jJ5p30ETA49_VjTE200',        l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j20f_preselVETOMULT11a15_pf_jes_ftf_L1TEA_VZDC_A_VZDC_C_jJ5p30ETA49_VjTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j20f_preselVETOMULT11a15_pf_jes_ftf_L1TEA_ZDC_XOR_jJ5p30ETA49_VjTE200',        l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j30f_preselVETOMULT11a15_pf_jes_ftf_L1TEA_VZDC_A_VZDC_C_jJ10p30ETA49_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j30f_preselVETOMULT11a15_pf_jes_ftf_L1TEA_ZDC_XOR_jJ10p30ETA49_VjTE200',       l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j40f_preselVETOMULT11a15_pf_jes_ftf_L1TEA_VZDC_A_VZDC_C_jJ10p30ETA49_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j50f_preselVETOMULT11a15_pf_jes_ftf_L1TEA_ZDC_XOR_jJ10p30ETA49_VjTE200',       l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),

        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10f_pf_jes_ftf_L1ESP_VZDC_A_VZDC_C_jJ5p30ETA49_VjTE200',  l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10f_pf_jes_ftf_L1ESP_ZDC_XOR_jJ5p30ETA49_VjTE200',        l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j15f_pf_jes_ftf_L1ESP_VZDC_A_VZDC_C_jJ5p30ETA49_VjTE200',  l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j15f_pf_jes_ftf_L1ESP_ZDC_XOR_jJ5p30ETA49_VjTE200',        l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j20f_pf_jes_ftf_L1ESP_VZDC_A_VZDC_C_jJ5p30ETA49_VjTE200',  l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j20f_pf_jes_ftf_L1ESP_ZDC_XOR_jJ5p30ETA49_VjTE200',        l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j30f_pf_jes_ftf_L1ESP_VZDC_A_VZDC_C_jJ10p30ETA49_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j30f_pf_jes_ftf_L1ESP_ZDC_XOR_jJ10p30ETA49_VjTE200',       l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j40f_pf_jes_ftf_L1ESP_VZDC_A_VZDC_C_jJ10p30ETA49_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j40f_pf_jes_ftf_L1ESP_ZDC_XOR_jJ10p30ETA49_VjTE200',       l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j10f_preselVETOMULT11a10_pf_jes_ftf_L1ESP_VZDC_A_VZDC_C_jJ5p30ETA49_VjTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j10f_preselVETOMULT11a10_pf_jes_ftf_L1ESP_ZDC_XOR_jJ5p30ETA49_VjTE200',        l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j15f_preselVETOMULT11a15_pf_jes_ftf_L1ESP_VZDC_A_VZDC_C_jJ5p30ETA49_VjTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j15f_preselVETOMULT11a15_pf_jes_ftf_L1ESP_ZDC_XOR_jJ5p30ETA49_VjTE200',        l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j20f_preselVETOMULT11a15_pf_jes_ftf_L1ESP_VZDC_A_VZDC_C_jJ5p30ETA49_VjTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j20f_preselVETOMULT11a15_pf_jes_ftf_L1ESP_ZDC_XOR_jJ5p30ETA49_VjTE200',        l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j30f_preselVETOMULT11a15_pf_jes_ftf_L1ESP_VZDC_A_VZDC_C_jJ10p30ETA49_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j30f_preselVETOMULT11a15_pf_jes_ftf_L1ESP_ZDC_XOR_jJ10p30ETA49_VjTE200',       l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j40f_preselVETOMULT11a15_pf_jes_ftf_L1ESP_VZDC_A_VZDC_C_jJ10p30ETA49_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j50f_preselVETOMULT11a15_pf_jes_ftf_L1ESP_ZDC_XOR_jJ10p30ETA49_VjTE200',       l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),

        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10a_pf_jes_ftf_L1TEA_ZDC_5XOR_jJ5_VjTE200',  l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j15a_pf_jes_ftf_L1TEA_ZDC_5XOR_jJ5_VjTE200',  l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j20a_pf_jes_ftf_L1TEA_ZDC_5XOR_jJ5_VjTE200',  l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j30a_pf_jes_ftf_L1TEA_ZDC_5XOR_jJ10_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j40a_pf_jes_ftf_L1TEA_ZDC_5XOR_jJ10_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j10a_preselVETOMULT11a10_pf_jes_ftf_L1TEA_ZDC_5XOR_jJ5_VjTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j15a_preselVETOMULT11a15_pf_jes_ftf_L1TEA_ZDC_5XOR_jJ5_VjTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j20a_preselVETOMULT11a15_pf_jes_ftf_L1TEA_ZDC_5XOR_jJ5_VjTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j30a_preselVETOMULT11a15_pf_jes_ftf_L1TEA_ZDC_5XOR_jJ10_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j40a_preselVETOMULT11a15_pf_jes_ftf_L1TEA_ZDC_5XOR_jJ10_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),

        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j30a_pf_jes_ftf_L1ESP_ZDC_5XOR_jJ10_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j40a_pf_jes_ftf_L1ESP_ZDC_5XOR_jJ10_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j30a_preselVETOMULT11a15_pf_jes_ftf_L1ESP_ZDC_5XOR_jJ10_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j40a_preselVETOMULT11a15_pf_jes_ftf_L1ESP_ZDC_5XOR_jJ10_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),

        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10a_pf_jes_ftf_L1TEA_1ZDC_NZDC_jJ5_VjTE200',      l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j15a_pf_jes_ftf_L1TEA_1ZDC_NZDC_jJ5_VjTE200',      l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j20a_pf_jes_ftf_L1TEA_1ZDC_NZDC_jJ5_VjTE200',      l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j30a_pf_jes_ftf_L1TEA_1ZDC_NZDC_jJ10_VjTE200',     l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j40a_pf_jes_ftf_L1TEA_1ZDC_NZDC_jJ10_VjTE200',     l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10a_pf_jes_ftf_L1TEA_5ZDC_A_5ZDC_C_jJ5_VjTE200',  l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j15a_pf_jes_ftf_L1TEA_5ZDC_A_5ZDC_C_jJ5_VjTE200',  l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j20a_pf_jes_ftf_L1TEA_5ZDC_A_5ZDC_C_jJ5_VjTE200',  l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j30a_pf_jes_ftf_L1TEA_5ZDC_A_5ZDC_C_jJ10_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j40a_pf_jes_ftf_L1TEA_5ZDC_A_5ZDC_C_jJ10_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j10a_preselVETOMULT11a10_pf_jes_ftf_L1TEA_1ZDC_NZDC_jJ5_VjTE200',      l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j15a_preselVETOMULT11a15_pf_jes_ftf_L1TEA_1ZDC_NZDC_jJ5_VjTE200',      l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j20a_preselVETOMULT11a15_pf_jes_ftf_L1TEA_1ZDC_NZDC_jJ5_VjTE200',      l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j30a_preselVETOMULT11a15_pf_jes_ftf_L1TEA_1ZDC_NZDC_jJ10_VjTE200',     l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j40a_preselVETOMULT11a15_pf_jes_ftf_L1TEA_1ZDC_NZDC_jJ10_VjTE200',     l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j10a_preselVETOMULT11a10_pf_jes_ftf_L1TEA_5ZDC_A_5ZDC_C_jJ5_VjTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j15a_preselVETOMULT11a15_pf_jes_ftf_L1TEA_5ZDC_A_5ZDC_C_jJ5_VjTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j20a_preselVETOMULT11a15_pf_jes_ftf_L1TEA_5ZDC_A_5ZDC_C_jJ5_VjTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j30a_preselVETOMULT11a15_pf_jes_ftf_L1TEA_5ZDC_A_5ZDC_C_jJ10_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j40a_preselVETOMULT11a15_pf_jes_ftf_L1TEA_5ZDC_A_5ZDC_C_jJ10_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),

        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j30a_pf_jes_ftf_L1ESP_1ZDC_NZDC_jJ10_VjTE200',     l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j40a_pf_jes_ftf_L1ESP_1ZDC_NZDC_jJ10_VjTE200',     l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j30a_pf_jes_ftf_L1ESP_5ZDC_A_5ZDC_C_jJ10_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j40a_pf_jes_ftf_L1ESP_5ZDC_A_5ZDC_C_jJ10_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j30a_preselVETOMULT11a15_pf_jes_ftf_L1ESP_1ZDC_NZDC_jJ10_VjTE200',     l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j40a_preselVETOMULT11a15_pf_jes_ftf_L1ESP_1ZDC_NZDC_jJ10_VjTE200',     l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j30a_preselVETOMULT11a15_pf_jes_ftf_L1ESP_5ZDC_A_5ZDC_C_jJ10_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j40a_preselVETOMULT11a15_pf_jes_ftf_L1ESP_5ZDC_A_5ZDC_C_jJ10_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),

        # Jets seeded from UPC HMT items
        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10a_pf_jes_ftf_L1TEA_ASYM0_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10a_pf_jes_ftf_L1TEA_ASYM1_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10a_pf_jes_ftf_L1TEA_ASYM2_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10a_pf_jes_ftf_L1TEA_ASYM3_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j10a_preselVETOMULT11a10_pf_jes_ftf_L1TEA_ASYM0_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j10a_preselVETOMULT11a10_pf_jes_ftf_L1TEA_ASYM1_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j10a_preselVETOMULT11a10_pf_jes_ftf_L1TEA_ASYM2_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j10a_preselVETOMULT11a10_pf_jes_ftf_L1TEA_ASYM3_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),

        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10a_pf_jes_ftf_L1ESP_ASYM0_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10a_pf_jes_ftf_L1ESP_ASYM1_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10a_pf_jes_ftf_L1ESP_ASYM2_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX10ptXX0eta490_j10a_pf_jes_ftf_L1ESP_ASYM3_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j10a_preselVETOMULT11a10_pf_jes_ftf_L1ESP_ASYM0_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j10a_preselVETOMULT11a10_pf_jes_ftf_L1ESP_ASYM1_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j10a_preselVETOMULT11a10_pf_jes_ftf_L1ESP_ASYM2_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j10a_preselVETOMULT11a10_pf_jes_ftf_L1ESP_ASYM3_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),

        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j15a_pf_jes_ftf_L1TEA_ASYM0_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j15a_pf_jes_ftf_L1TEA_ASYM1_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j15a_pf_jes_ftf_L1TEA_ASYM2_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j15a_pf_jes_ftf_L1TEA_ASYM3_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j15a_preselVETOMULT11a15_pf_jes_ftf_L1TEA_ASYM0_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j15a_preselVETOMULT11a15_pf_jes_ftf_L1TEA_ASYM1_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j15a_preselVETOMULT11a15_pf_jes_ftf_L1TEA_ASYM2_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j15a_preselVETOMULT11a15_pf_jes_ftf_L1TEA_ASYM3_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),

        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j15a_pf_jes_ftf_L1ESP_ASYM0_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j15a_pf_jes_ftf_L1ESP_ASYM1_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j15a_pf_jes_ftf_L1ESP_ASYM2_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j15a_pf_jes_ftf_L1ESP_ASYM3_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j15a_preselVETOMULT11a15_pf_jes_ftf_L1ESP_ASYM0_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j15a_preselVETOMULT11a15_pf_jes_ftf_L1ESP_ASYM1_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j15a_preselVETOMULT11a15_pf_jes_ftf_L1ESP_ASYM2_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j15a_preselVETOMULT11a15_pf_jes_ftf_L1ESP_ASYM3_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
    ]


    chains['Combined'] = [

        #----------- mu + UPC Fgap
        ChainProp(name='HLT_mu3_hi_FgapAC5_L1MU3V_VjTE50', l1SeedThresholds=['MU3V','FSNOSEED'], stream=[UPCStream], groups=SingleMuonGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mu4_hi_FgapAC5_L1MU3V_VjTE50', l1SeedThresholds=['MU3V','FSNOSEED'], stream=[UPCStream], groups=SingleMuonGroup+PrimaryPhIGroup),

        #----------- mu+j Primary
        ChainProp(name='HLT_mu4_j50_ion_dRAB05_L1MU3V',     l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream, 'express'], groups=SupportGroup+SingleBjetGroup, monGroups=['bJetMon:t0','muonMon:online','bJetMon:online']),
        ChainProp(name='HLT_mu4_j60_ion_dRAB05_L1MU3V',     l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=SupportGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu6_j40_ion_dRAB05_L1MU5VF',     l1SeedThresholds=['MU5VF','FSNOSEED'], stream=[HardProbesStream, 'express'], groups=PrimaryPhIGroup+SingleBjetGroup, monGroups=['bJetMon:t0','muonMon:online','bJetMon:online']),
        ChainProp(name='HLT_mu6_j50_ion_dRAB05_L1MU5VF',     l1SeedThresholds=['MU5VF','FSNOSEED'], stream=[HardProbesStream], groups=PrimaryPhIGroup+SingleBjetGroup),
        #----------- mu+j Support
        ChainProp(name='HLT_mu4_j40_ion_dRAB05_L1MU3V',     l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=SupportGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu6_j30_ion_dRAB05_L1MU5VF',     l1SeedThresholds=['MU5VF','FSNOSEED'], stream=[HardProbesStream], groups=SupportGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu4_j40_ion_L1MU3V',     l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=SupportGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu4_j50_ion_L1MU3V',     l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=SupportGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu4_j60_ion_L1MU3V',     l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=SupportGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu6_j30_ion_L1MU3V',     l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=SupportGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu6_j40_ion_L1MU3V',     l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=SupportGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu6_j50_ion_L1MU3V',     l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=SupportGroup+SingleBjetGroup),

        #----------- mu+j with new calo
        ChainProp(name='HLT_mu4_j60_ion_dRAB05_L1MU3V_jJ40', l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=PrimaryPhIGroup+SingleBjetGroup),

        #----------- UPC HMT - phase-1
        #supporting
        #trk15
        #MBTS_2
        ChainProp(name='HLT_mb_sp50_trk15_hmt_hi_FgapA5_L1MBTS_2_VZDC_A_ZDC_C_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream, 'express'], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp50_trk15_hmt_hi_FgapA5_L1MBTS_2_1ZDC_NZDC_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp50_trk15_hmt_hi_FgapC5_L1MBTS_2_ZDC_A_VZDC_C_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream, 'express'], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp50_trk15_hmt_hi_FgapC5_L1MBTS_2_1ZDC_NZDC_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        #MBTS_2 with GAPs
        ChainProp(name='HLT_mb_sp50_trk15_hmt_hi_FgapA5_L1MBTS_2_VZDC_A_ZDC_C_VjTE200_GAP_A', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream, 'express'], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp50_trk15_hmt_hi_FgapA5_L1MBTS_2_1ZDC_NZDC_VjTE200_GAP_A', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp50_trk15_hmt_hi_FgapC5_L1MBTS_2_ZDC_A_VZDC_C_VjTE200_GAP_C', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream, 'express'], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp50_trk15_hmt_hi_FgapC5_L1MBTS_2_1ZDC_NZDC_VjTE200_GAP_C', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),

        #MBTS_1
        ChainProp(name='HLT_mb_sp50_trk15_hmt_hi_FgapA5_L1MBTS_1_VZDC_A_ZDC_C_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream, 'express'], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp50_trk15_hmt_hi_FgapA5_L1MBTS_1_1ZDC_NZDC_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp50_trk15_hmt_hi_FgapC5_L1MBTS_1_ZDC_A_VZDC_C_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream, 'express'], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp50_trk15_hmt_hi_FgapC5_L1MBTS_1_1ZDC_NZDC_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),

        #trk25
        ChainProp(name='HLT_mb_sp400_trk25_hmt_hi_FgapA5_L1VZDC_A_ZDC_C_jTE3_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp400_trk25_hmt_hi_FgapC5_L1ZDC_A_VZDC_C_jTE3_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),

        #trk25 - MBTS_1_jTE3 wih gaps
        ChainProp(name='HLT_mb_sp400_trk25_hmt_hi_FgapA5_L1MBTS_1_VZDC_A_ZDC_C_jTE3_VjTE200_GAP_A', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp400_trk25_hmt_hi_FgapA5_L1MBTS_1_1ZDC_NZDC_jTE3_VjTE200_GAP_A', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp400_trk25_hmt_hi_FgapC5_L1MBTS_1_ZDC_A_VZDC_C_jTE3_VjTE200_GAP_C', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp400_trk25_hmt_hi_FgapC5_L1MBTS_1_1ZDC_NZDC_jTE3_VjTE200_GAP_C', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),

        #trk35
        ChainProp(name='HLT_mb_sp700_trk35_hmt_hi_FgapA5_L1VZDC_A_ZDC_C_jTE5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp700_trk35_hmt_hi_FgapC5_L1ZDC_A_VZDC_C_jTE5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),

        #----------- UPC diphotons/dielectrons - phase-1
        #primary
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L1eEM1_jTE4_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_L1eEM1_jTE4_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1eEM1_jTE4_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_L1eEM2_jTE4_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L1eTAU1_jTE4_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_L1eTAU1_jTE4_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1eTAU1_jTE4_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1eEM1_TRT_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1eTAU1_TRT_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1eTAU1_TRT_VjTE200_GAP_AANDC', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L1DPHI-2eEM1_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_L1DPHI-2eEM1_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_L1DPHI-2eTAU1_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_2g0_etcut_L1DPHI-2eEM1_VjTE200', l1SeedThresholds=['FSNOSEED']*2+['eEM1'],stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_2g0_etcut_L1DPHI-2eTAU1_VjTE200', l1SeedThresholds=['FSNOSEED']*2+['eTAU1'],stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_2g0_etcut_25dphiCC_L1DPHI-2eEM1_VjTE200', l1SeedThresholds=['FSNOSEED']*2+['eEM1'],stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_2g0_etcut_25dphiCC_L1DPHI-2eTAU1_VjTE200', l1SeedThresholds=['FSNOSEED']*2+['eTAU1'],stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1DPHI-2eEM1_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1DPHI-2eTAU1_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),

        # --- ATR-30728
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L1CALMTEA_23INVM-27DPHI-2eTAU1_VjTE200',                    l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_2g0_etcut_L1CALMTEA_23INVM-27DPHI-2eTAU1_VjTE200',          l1SeedThresholds=['FSNOSEED']*2+['eTAU1'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_2g0_etcut_25dphiCC_L1CALMTEA_23INVM-27DPHI-2eTAU1_VjTE200', l1SeedThresholds=['FSNOSEED']*2+['eTAU1'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_L1CALMTEA_23INVM-27DPHI-2eTAU1_VjTE200',                    l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_2g0_etcut_L1CALMTEA_23INVM-27DPHI-2eTAU1_VjTE200',          l1SeedThresholds=['FSNOSEED']*2+['eTAU1'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_2g0_etcut_25dphiCC_L1CALMTEA_23INVM-27DPHI-2eTAU1_VjTE200', l1SeedThresholds=['FSNOSEED']*2+['eTAU1'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup, monGroups=['mbMon:t0']),

        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L1MATCHA_23INVM-27DPHI-2eTAU1_VjTE200',                    l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_2g0_etcut_L1MATCHA_23INVM-27DPHI-2eTAU1_VjTE200',          l1SeedThresholds=['FSNOSEED']*2+['eTAU1'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_2g0_etcut_25dphiCC_L1MATCHA_23INVM-27DPHI-2eTAU1_VjTE200', l1SeedThresholds=['FSNOSEED']*2+['eTAU1'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_L1MATCHA_23INVM-27DPHI-2eTAU1_VjTE200',                    l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_2g0_etcut_L1MATCHA_23INVM-27DPHI-2eTAU1_VjTE200',          l1SeedThresholds=['FSNOSEED']*2+['eTAU1'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_2g0_etcut_25dphiCC_L1MATCHA_23INVM-27DPHI-2eTAU1_VjTE200', l1SeedThresholds=['FSNOSEED']*2+['eTAU1'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup, monGroups=['mbMon:t0']),

        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L1CALMTEA_DPHI-2eTAU1_VjTE200',                    l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_2g0_etcut_L1CALMTEA_DPHI-2eTAU1_VjTE200',          l1SeedThresholds=['FSNOSEED']*2+['eTAU1'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_2g0_etcut_25dphiCC_L1CALMTEA_DPHI-2eTAU1_VjTE200', l1SeedThresholds=['FSNOSEED']*2+['eTAU1'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_L1CALMTEA_DPHI-2eTAU1_VjTE200',                    l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_2g0_etcut_L1CALMTEA_DPHI-2eTAU1_VjTE200',          l1SeedThresholds=['FSNOSEED']*2+['eTAU1'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_2g0_etcut_25dphiCC_L1CALMTEA_DPHI-2eTAU1_VjTE200', l1SeedThresholds=['FSNOSEED']*2+['eTAU1'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup, monGroups=['mbMon:t0']),

        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L1MATCHA_DPHI-2eTAU1_VjTE200',                    l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_2g0_etcut_L1MATCHA_DPHI-2eTAU1_VjTE200',          l1SeedThresholds=['FSNOSEED']*2+['eTAU1'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_2g0_etcut_25dphiCC_L1MATCHA_DPHI-2eTAU1_VjTE200', l1SeedThresholds=['FSNOSEED']*2+['eTAU1'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_L1MATCHA_DPHI-2eTAU1_VjTE200',                    l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_2g0_etcut_L1MATCHA_DPHI-2eTAU1_VjTE200',          l1SeedThresholds=['FSNOSEED']*2+['eTAU1'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_2g0_etcut_25dphiCC_L1MATCHA_DPHI-2eTAU1_VjTE200', l1SeedThresholds=['FSNOSEED']*2+['eTAU1'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup, monGroups=['mbMon:t0']),

        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L123INVM-25DPHI-2eTAU1_VjTE200',                    l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_2g0_etcut_L123INVM-25DPHI-2eTAU1_VjTE200',          l1SeedThresholds=['FSNOSEED']*2+['eTAU1'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_2g0_etcut_25dphiCC_L123INVM-25DPHI-2eTAU1_VjTE200', l1SeedThresholds=['FSNOSEED']*2+['eTAU1'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_L123INVM-25DPHI-2eTAU1_VjTE200',                    l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_2g0_etcut_L123INVM-25DPHI-2eTAU1_VjTE200',          l1SeedThresholds=['FSNOSEED']*2+['eTAU1'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_2g0_etcut_25dphiCC_L123INVM-25DPHI-2eTAU1_VjTE200', l1SeedThresholds=['FSNOSEED']*2+['eTAU1'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup, monGroups=['mbMon:t0']),

        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L133INVM-25DPHI-2eTAU1_VjTE200',                    l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_2g0_etcut_L133INVM-25DPHI-2eTAU1_VjTE200',          l1SeedThresholds=['FSNOSEED']*2+['eTAU1'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_2g0_etcut_25dphiCC_L133INVM-25DPHI-2eTAU1_VjTE200', l1SeedThresholds=['FSNOSEED']*2+['eTAU1'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_L133INVM-25DPHI-2eTAU1_VjTE200',                    l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_2g0_etcut_L133INVM-25DPHI-2eTAU1_VjTE200',          l1SeedThresholds=['FSNOSEED']*2+['eTAU1'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_2g0_etcut_25dphiCC_L133INVM-25DPHI-2eTAU1_VjTE200', l1SeedThresholds=['FSNOSEED']*2+['eTAU1'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup, monGroups=['mbMon:t0']),

        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L123INVM-27DPHI-2eTAU1_VjTE200',                    l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_2g0_etcut_L123INVM-27DPHI-2eTAU1_VjTE200',          l1SeedThresholds=['FSNOSEED']*2+['eTAU1'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_2g0_etcut_25dphiCC_L123INVM-27DPHI-2eTAU1_VjTE200', l1SeedThresholds=['FSNOSEED']*2+['eTAU1'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_L123INVM-27DPHI-2eTAU1_VjTE200',                    l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_2g0_etcut_L123INVM-27DPHI-2eTAU1_VjTE200',          l1SeedThresholds=['FSNOSEED']*2+['eTAU1'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_2g0_etcut_25dphiCC_L123INVM-27DPHI-2eTAU1_VjTE200', l1SeedThresholds=['FSNOSEED']*2+['eTAU1'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup, monGroups=['mbMon:t0']),

        # Background
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_L1CALMTEA_23INVM-27DPHI-2eTAU1_VjTE200_EMPTY', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_L1MATCHA_23INVM-27DPHI-2eTAU1_VjTE200_EMPTY',  l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_L1CALMTEA_DPHI-2eTAU1_VjTE200_EMPTY',          l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_L1MATCHA_DPHI-2eTAU1_VjTE200_EMPTY',           l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportGroup, monGroups=['mbMon:t0']),

        # Electon
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L123INVM-25DPHI-2eTAU1_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L133INVM-25DPHI-2eTAU1_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L123INVM-27DPHI-2eTAU1_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportGroup),

        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1CALMTEA_23INVM-27DPHI-2eTAU1_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1MATCHA_23INVM-27DPHI-2eTAU1_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportGroup),

        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1CALMTEA_DPHI-2eTAU1_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1MATCHA_DPHI-2eTAU1_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        # ---

        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L12eEM1_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L12eEM1_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_L1eEM5_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_L1MATCHA_eEM5_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_sp_vpix15_hi_FgapAC5_L12eTAU2_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L12eTAU2_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream, 'express'],groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L12eTAU2_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),

        #supporting
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_L1eTAU1_TRT_VjTE200_EMPTY', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_L1eTAU1_TRT_VjTE200_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_L1eTAU1_TRT_VjTE200_UNPAIRED_NONISO', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L11ZDC_A_1ZDC_C_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1VZDC_A_VZDC_C_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_hi_FgapAC5_L12eEM1_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_hi_FgapAC5_L1eTAU1_TRT_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_L1eEM9_VjTE200_EMPTY', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_L1DPHI-2eTAU1_VjTE200_EMPTY', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),

        # 2025
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_L1eTAU2_VjTE200_EMPTY', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_L1eEM2_VjTE200_EMPTY',  l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_L1eEM5_VjTE200_EMPTY',  l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_L1MATCHA_eTAU2_VjTE200_EMPTY', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_L1MATCHA_eEM2_VjTE200_EMPTY',  l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_L1MATCHA_eEM5_VjTE200_EMPTY',  l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),

        #----------- UPC ditaus - Phase-1
        #primary
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1eEM1_TRT_VZDC_A_VZDC_C_VjTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream, 'express'], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1eEM1_TRT_ZDC_XOR4_VjTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1eTAU1_TRT_VZDC_A_VZDC_C_VjTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream, 'express'], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1eTAU1_TRT_ZDC_XOR4_VjTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream, 'express'], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_excl_1trk5_pt2_hi_FgapAC5_L1eEM1_TRT_VZDC_A_VZDC_C_VjTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt2_hi_FgapAC5_L1eEM1_TRT_ZDC_XOR4_VjTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt2_hi_FgapAC5_L1eTAU1_TRT_VZDC_A_VZDC_C_VjTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt2_hi_FgapAC5_L1eTAU1_TRT_ZDC_XOR4_VjTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        #supporting
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1eEM1_VZDC_A_VZDC_C_VjTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1eEM1_ZDC_XOR4_VjTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt2_hi_FgapAC5_L1eEM1_VZDC_A_VZDC_C_VjTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt2_hi_FgapAC5_L1eEM1_ZDC_XOR4_VjTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1eEM2_VZDC_A_VZDC_C_VjTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream, 'express'], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1eEM2_ZDC_XOR4_VjTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt2_hi_FgapAC5_L1eEM2_VZDC_A_VZDC_C_VjTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt2_hi_FgapAC5_L1eEM2_ZDC_XOR4_VjTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),

        #ATR-29025 - J/Psi chains
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1TRT_VjTE20', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream, 'express'],groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_excl_1trk5_pt0p5_hi_FgapAC5_L1TRT_VjTE20', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        
        #--- phase-1 HI 0.2 jets in UCC collisions
        ChainProp(name='HLT_j40_a2_ion_hi_uccTh3_L1jTEFWD6600', l1SeedThresholds=['FSNOSEED']*2, stream=[HardProbesStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j50_a2_ion_hi_uccTh3_L1jTEFWD6600', l1SeedThresholds=['FSNOSEED']*2, stream=[HardProbesStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j60_a2_ion_hi_uccTh3_L1jTEFWD6600', l1SeedThresholds=['FSNOSEED']*2, stream=[HardProbesStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j40_a2_ion_hi_uccTh2_L1jTEFWD6300', l1SeedThresholds=['FSNOSEED']*2, stream=[HardProbesStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j50_a2_ion_hi_uccTh2_L1jTEFWD6300', l1SeedThresholds=['FSNOSEED']*2, stream=[HardProbesStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j60_a2_ion_hi_uccTh2_L1jTEFWD6300', l1SeedThresholds=['FSNOSEED']*2, stream=[HardProbesStream], groups=SingleJetGroup+SupportPhIGroup),

        ChainProp(name='HLT_j40_a2_ion_hi_uccTh3_L1ZDC_PU_jTEFWD6600', l1SeedThresholds=['FSNOSEED']*2, stream=[HardProbesStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j50_a2_ion_hi_uccTh3_L1ZDC_PU_jTEFWD6600', l1SeedThresholds=['FSNOSEED']*2, stream=[HardProbesStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j60_a2_ion_hi_uccTh3_L1ZDC_PU_jTEFWD6600', l1SeedThresholds=['FSNOSEED']*2, stream=[HardProbesStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j40_a2_ion_hi_uccTh2_L1ZDC_PU_jTEFWD6300', l1SeedThresholds=['FSNOSEED']*2, stream=[HardProbesStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j50_a2_ion_hi_uccTh2_L1ZDC_PU_jTEFWD6300', l1SeedThresholds=['FSNOSEED']*2, stream=[HardProbesStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j60_a2_ion_hi_uccTh2_L1ZDC_PU_jTEFWD6300', l1SeedThresholds=['FSNOSEED']*2, stream=[HardProbesStream], groups=SingleJetGroup+SupportPhIGroup),

        ChainProp(name='HLT_j40_a2_ion_L1ZDC_HELT20_jTEFWD2600', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j50_a2_ion_L1ZDC_HELT20_jTEFWD2600', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j60_a2_ion_L1ZDC_HELT20_jTEFWD2600', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+SupportPhIGroup),

        ChainProp(name='HLT_j40_a2_ion_L1ZDC_HELT35_jTEFWD2600', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j50_a2_ion_L1ZDC_HELT35_jTEFWD2600', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j60_a2_ion_L1ZDC_HELT35_jTEFWD2600', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+SupportPhIGroup),

        ChainProp(name='HLT_j40_a2_ion_L1ZDC_HELT50_jTEFWD2600', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j50_a2_ion_L1ZDC_HELT50_jTEFWD2600', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j60_a2_ion_L1ZDC_HELT50_jTEFWD2600', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+SupportPhIGroup),

        #--- muons in UCC collisions
        ChainProp(name='HLT_mu4noL1_hi_uccTh3_L1jTEFWD6600',        l1SeedThresholds=['FSNOSEED']*2, stream=[HardProbesStream], groups=SingleMuonGroup),
        ChainProp(name='HLT_mu4noL1_hi_uccTh2_L1jTEFWD6300',        l1SeedThresholds=['FSNOSEED']*2, stream=[HardProbesStream], groups=SingleMuonGroup),
        ChainProp(name='HLT_mu4noL1_hi_uccTh3_L1ZDC_PU_jTEFWD6600', l1SeedThresholds=['FSNOSEED']*2, stream=[HardProbesStream], groups=SingleMuonGroup),
        ChainProp(name='HLT_mu4noL1_hi_uccTh2_L1ZDC_PU_jTEFWD6300', l1SeedThresholds=['FSNOSEED']*2, stream=[HardProbesStream], groups=SingleMuonGroup),

        ChainProp(name='HLT_mu4noL1_L1ZDC_HELT20_jTEFWD2600',       l1SeedThresholds=['FSNOSEED'],   stream=[HardProbesStream], groups=SingleMuonGroup),
        ChainProp(name='HLT_mu4noL1_L1ZDC_HELT35_jTEFWD2600',       l1SeedThresholds=['FSNOSEED'],   stream=[HardProbesStream], groups=SingleMuonGroup),
        ChainProp(name='HLT_mu4noL1_L1ZDC_HELT50_jTEFWD2600',       l1SeedThresholds=['FSNOSEED'],   stream=[HardProbesStream], groups=SingleMuonGroup),

        # Pb+Pb 2025:
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1TRT_VjTE200',               l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1TRT_ZDC_OR_VjTE200',        l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1TRT_ZDC_XOR_VjTE200',       l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1TRT_ZDC_A_C_VjTE200',       l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_excl_1trk5_pt0p5_hi_FgapAC5_L1TRT_VjTE200',               l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt0p5_hi_FgapAC5_L1TRT_ZDC_OR_VjTE200',        l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt0p5_hi_FgapAC5_L1TRT_ZDC_XOR_VjTE200',       l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt0p5_hi_FgapAC5_L1TRT_ZDC_A_C_VjTE200',       l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_excl_1trk5_pt1_L1TRT_ZDC_OR_VjTE200',   l1SeedThresholds=['FSNOSEED'],stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt0p5_L1TRT_ZDC_OR_VjTE200', l1SeedThresholds=['FSNOSEED'],stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1ZDC_A_C_VjTE10', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),

        # For excl efficiency studies
        ChainProp(name='HLT_mb_sptrk_hi_FgapAC5_L1DPHI-2eTAU1_VjTE200',          l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_hi_FgapAC5_L123INVM-27DPHI-2eTAU1_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),

        # Ditaus
        ChainProp(name='HLT_mb_excl_1trk5_pt0p5_hi_FgapAC5_L1TRT_VZDC_A_VZDC_C_VjTE200',  l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup), 
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1TRT_VZDC_A_VZDC_C_VjTE200',  l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup), 

        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1eEM2_TRT_VZDC_A_VZDC_C_VjTE200',  l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup), 
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1eTAU2_TRT_VZDC_A_VZDC_C_VjTE200',  l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup), 

        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1CALMTEA_eEM2_VjTE200',  l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1CALMTEA_eTAU2_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1MATCHA_eEM2_VjTE200',  l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1MATCHA_eTAU2_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),

        # UPC egamma
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_L1CALMTEA_eEM2_VjTE200',  l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_L1CALMTEA_eTAU2_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_2g0_etcut_L1CALMTEA_eEM2_VjTE200',  l1SeedThresholds=['FSNOSEED']*2+['eEM2'],  stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_2g0_etcut_L1CALMTEA_eTAU2_VjTE200', l1SeedThresholds=['FSNOSEED']*2+['eTAU2'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_2g0_etcut_25dphiCC_L1CALMTEA_eEM2_VjTE200',  l1SeedThresholds=['FSNOSEED']*2+['eEM2'],  stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_2g0_etcut_25dphiCC_L1CALMTEA_eTAU2_VjTE200', l1SeedThresholds=['FSNOSEED']*2+['eTAU2'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_L1MATCHA_eEM2_VjTE200',  l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_L1MATCHA_eTAU2_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_2g0_etcut_L1MATCHA_eEM2_VjTE200',  l1SeedThresholds=['FSNOSEED']*2+['eEM2'],  stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_2g0_etcut_L1MATCHA_eTAU2_VjTE200', l1SeedThresholds=['FSNOSEED']*2+['eTAU2'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_2g0_etcut_25dphiCC_L1MATCHA_eEM2_VjTE200',  l1SeedThresholds=['FSNOSEED']*2+['eEM2'],  stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp_vpix60_hi_FgapAC5_2g0_etcut_25dphiCC_L1MATCHA_eTAU2_VjTE200', l1SeedThresholds=['FSNOSEED']*2+['eTAU2'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
    ]


    chains['MinBias'] = [

        #----------- sptrk
        ChainProp(name='HLT_mb_sptrk_L1MBTS_1_1_VjTE50', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_A_C_VjTE50',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sptrk_pc_L1ZDC_A_C_VjTE50',l1SeedThresholds=['FSNOSEED'], stream=[PCStream, 'express'], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        #MinBiasOverlay
        ChainProp(name='HLT_mb_sptrk_L1ZDC_A_C_VjTE50_OVERLAY', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasOverlayStream], groups=MinBiasGroup+SupportPhIGroup),

        #sptrk & supporting for UPC hmt
        ChainProp(name='HLT_mb_sptrk_L1ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream, 'express'], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sptrk_L11ZDC_A_1ZDC_C_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:shifter']),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_1XOR5_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sptrk_L1TRT_1ZDC_NZDC_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sptrk_pt1_L1TRT_ZDC_XOR_jTE5_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_sp50_trk15_hmt_L1MBTS_1_1ZDC_NZDC_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp50_trk15_hmt_L1MBTS_1_ZDC_A_VZDC_C_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp50_trk15_hmt_L1MBTS_1_VZDC_A_ZDC_C_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_OR_VjTE200_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp50_trk15_hmt_L1MBTS_1_ZDC_OR_VjTE200_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),

        #magnetic monopoles, physics chains from ATR-29741
        ChainProp(name='HLT_mb_sp_nototpix100_L1ZDC_A_C_VjTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp_nototpix100_L1ZDC_XOR_VjTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp_nototpix100_L1ZDC_XOR_VjTE10_UNPAIRED_NONISO', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp_nototpix50_q2_L1ZDC_A_C_VjTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream, 'express'], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp_nototpix50_q2_L1ZDC_XOR_VjTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp_nototpix50_q2_L1ZDC_XOR_VjTE10_UNPAIRED_NONISO', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp_nototpix30_q2_L1TRT_ZDC_A_C_VjTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp_nototpix30_q2_L1TRT_ZDC_XOR_VjTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp_nototpix30_q2_L1TRT_ZDC_XOR_VjTE10_UNPAIRED_NONISO', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_sp_nototpix100_L1ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp_nototpix50_q2_L1ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_pixsptrk_nototpix20_q2_L1TRT_ZDC_A_C_VjTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0','idMon:t0']),
        ChainProp(name='HLT_mb_pixsptrk_nototpix20_q2_L1TRT_ZDC_XOR_VjTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_pixsptrk_nototpix20_q2_L1TRT_ZDC_XOR_VjTE10_UNPAIRED_NONISO', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_pixsptrk_nototpix30_q2_L1TRT_ZDC_A_C_VjTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_pixsptrk_nototpix30_q2_L1TRT_ZDC_XOR_VjTE10', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_pixsptrk_nototpix30_q2_L1TRT_ZDC_XOR_VjTE10_UNPAIRED_NONISO', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),

        #UPC diphotons/dielectrons supporting
        ChainProp(name='HLT_mb_excl_1trk5_pt1_L1eTAU1_jTE4_VjTE200', l1SeedThresholds=['FSNOSEED'],stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_L1eTAU1_TRT_VjTE200', l1SeedThresholds=['FSNOSEED'],stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),

        #UPC ditaus supporting
        ChainProp(name='HLT_mb_excl_1trk5_pt1_L1eEM1_TRT_VZDC_A_VZDC_C_VjTE100', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_L1eTAU1_TRT_VZDC_A_VZDC_C_VjTE100', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),

        #J/Psi chains supporting
        ChainProp(name='HLT_mb_excl_1trk5_pt1_L1TRT_VjTE20', l1SeedThresholds=['FSNOSEED'],stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt0p5_L1TRT_VjTE20', l1SeedThresholds=['FSNOSEED'],stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        #HF UPC jets, ATR-30208
        ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream, 'express'], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sptrk_L1eEM1_TRT_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1eTAU1_TRT_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),

        # Pb+Pb 2025:
        ChainProp(name='HLT_mb_sptrk_L1TRT_VjTE200',               l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1TRT_VZDC_A_VZDC_C_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_OR_VjTE200',        l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup,  monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sptrk_L1TRT_ZDC_A_C_VjTE200',       l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_sptrk_pt1_L1TRT_VjTE200',               l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_pt1_L1TRT_VZDC_A_VZDC_C_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_pt1_L1TRT_ZDC_OR_VjTE200',        l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup,  monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sptrk_pt1_L1TRT_ZDC_XOR_VjTE200',       l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_pt1_L1TRT_ZDC_A_C_VjTE200',       l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_sptrk_pt1_trk2_L1TRT_ZDC_OR_VjTE200',   l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sptrk_pt0p8_L1TRT_ZDC_OR_VjTE200',      l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sptrk_pt0p8_trk2_L1TRT_ZDC_OR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),

        # Ditaus
        ChainProp(name='HLT_mb_excl_1trk5_pt0p5_L1TRT_VZDC_A_VZDC_C_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_L1TRT_VZDC_A_VZDC_C_VjTE200',   l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_sptrk_trk2_pt0p8_L1eEM2_TRT_VZDC_A_VZDC_C_VjTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_sptrk_pt0p8_L1eEM2_TRT_VZDC_A_VZDC_C_VjTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_pt0p8_L1eTAU2_TRT_VZDC_A_VZDC_C_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_sptrk_pt0p8_L1eEM2_TRT_ZDC_OR_VjTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_pt0p8_L1eTAU2_TRT_ZDC_OR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_sptrk_trk2_pt0p8_L1eEM2_TRT_ZDC_OR_VjTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_trk2_pt0p8_L1eTAU2_TRT_ZDC_OR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),

        # UPC HMT with TeAsymmetry for 2025
        ChainProp(name='HLT_mb_sptrk_pt0p8_trk2_L1TEA_ASYM0_TRT_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_pt0p8_trk2_L1TEA_ASYM1_TRT_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_pt0p8_trk2_L1TEA_ASYM2_TRT_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_pt0p8_trk2_L1TEA_ASYM3_TRT_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_sptrk_pt0p8_trk2_L1ESP_ASYM0_TRT_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_pt0p8_trk2_L1ESP_ASYM1_TRT_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_pt0p8_trk2_L1ESP_ASYM2_TRT_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_pt0p8_trk2_L1ESP_ASYM3_TRT_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_sptrk_pt0p8_trk2_L1TEA_ASYM0_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_pt0p8_trk2_L1TEA_ASYM1_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_pt0p8_trk2_L1TEA_ASYM2_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_pt0p8_trk2_L1TEA_ASYM3_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_sptrk_pt0p8_trk2_L1ESP_ASYM0_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_pt0p8_trk2_L1ESP_ASYM1_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_pt0p8_trk2_L1ESP_ASYM2_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_pt0p8_trk2_L1ESP_ASYM3_ZDC_XOR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),

        # For Fgap efficiency studies
        ChainProp(name='HLT_mb_excl_1trk5_pt1_L1DPHI-2eTAU1_VjTE200',          l1SeedThresholds=['FSNOSEED'],stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_L123INVM-27DPHI-2eTAU1_VjTE200', l1SeedThresholds=['FSNOSEED'],stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
    ]

    chains['HeavyIon'] += [
        ChainProp(name='HLT_hi_uccTh1_L1jTEFWD5600', l1SeedThresholds=['FSNOSEED'], stream=[UCCStream], groups=MinBiasGroup+SupportGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_hi_uccTh2_L1jTEFWD6300', l1SeedThresholds=['FSNOSEED'], stream=[UCCStream], groups=MinBiasGroup+SupportGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_hi_uccTh3_L1jTEFWD6600', l1SeedThresholds=['FSNOSEED'], stream=[UCCStream], groups=MinBiasGroup+SupportGroup, monGroups=['mbMon:t0']),

        ChainProp(name='HLT_hi_uccTh1_L1ZDC_PU_jTEFWD5600', l1SeedThresholds=['FSNOSEED'], stream=[UCCStream], groups=MinBiasGroup+SupportGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_hi_uccTh2_L1ZDC_PU_jTEFWD6300', l1SeedThresholds=['FSNOSEED'], stream=[UCCStream], groups=MinBiasGroup+SupportGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_hi_uccTh3_L1ZDC_PU_jTEFWD6600', l1SeedThresholds=['FSNOSEED'], stream=[UCCStream], groups=MinBiasGroup+SupportGroup, monGroups=['mbMon:t0']),
    ]

    chains['Streaming'] = [

        ChainProp(name='HLT_noalg_L1RD0_EMPTY',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportGroup), 

        #Run2-style Heavy Ion ZDC streamers
        ChainProp(name='HLT_noalg_L1ZDC_A', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1ZDC_C', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1ZDC_A_C', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportGroup),


        ChainProp(name='HLT_noalg_mb_L1MBTS_1',   l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup),
        ChainProp(name='HLT_noalg_mb_L1MBTS_1_1', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup),
        ChainProp(name='HLT_noalg_mb_L1MBTS_2',   l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup),
        ChainProp(name='HLT_noalg_L1MBTS_2_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup),
        ChainProp(name='HLT_noalg_mb_L1RD0_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup),

        ChainProp(name='HLT_noalg_L1MU3V',  l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU5VF', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportGroup),

        # Streamers for monitoring TRT fast-OR
        ChainProp(name='HLT_noalg_L1TRT_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_noalg_L1TRT_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup),

        ChainProp(name='HLT_noalg_L1eEM1_VjTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1eEM2_VjTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jTE5_VjTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1TRT_VjTE50',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream, 'express'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),

        ChainProp(name='HLT_noalg_L1eTAU2_VjTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_noalg_L1jJ40',   l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jJ50',   l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jJ55',   l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_noalg_L1eEM5',  l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1eEM9',  l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1eEM12',  l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1eEM15',  l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream, 'express'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup,  monGroups=['egammaMon:online','egammaMon:shifter']),
        ChainProp(name='HLT_noalg_L1eEM18',  l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L12MU3V',  l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportGroup),

        #----jTE MinBias streamers----
        ChainProp(name='HLT_noalg_L1jTE3',   l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jTE4',   l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jTE5',   l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jTE10',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jTE20',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jTE50',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jTE100', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jTE200',   l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jTE1500', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jTE6500', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_noalg_L1jTEFWD5600', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jTEFWD6300', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jTEFWD6600', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_noalg_L1ZDC_PU_jTEFWD5600', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1ZDC_PU_jTEFWD6300', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1ZDC_PU_jTEFWD6600', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        #---- PC & CC stream ----
        ChainProp(name='HLT_noalg_L1jTE50_VjTE600',l1SeedThresholds=['FSNOSEED'], stream=[PCStream, 'express'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_noalg_L1jTE600',l1SeedThresholds=['FSNOSEED'], stream=[CCStream, 'express'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),
        #----ZeroBias
        ChainProp(name='HLT_noalg_zb_L1ZeroBias',    l1SeedThresholds=['FSNOSEED'], stream=['ZeroBias'], groups=['PS:NoBulkMCProd']+ZeroBiasGroup),

        #ZDC bits streamer
        #Commented out for the 2022 Nov Pb+Pb test run as the corresponding L1 ZDC items were commented out in the L1 menu
        #These trigger will be needed for 2023 heavy ion runs
        # ChainProp(name='HLT_noalg_L1ZDC_BIT2',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+['PS:NoHLTRepro']+MinBiasGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_BIT1',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+['PS:NoHLTRepro']+MinBiasGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_BIT0',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+['PS:NoHLTRepro']+MinBiasGroup),

        ChainProp(name='HLT_noalg_L1gJ20p0ETA25',   l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1gJ400p0ETA25',  l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1gLJ80p0ETA25',  l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1gXEJWOJ100',    l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=['PS:NoBulkMCProd']+SupportPhIGroup+METPhaseIStreamersGroup, monGroups=['metMon:t0']),
        #MinBiasOverlay
        ChainProp(name='HLT_noalg_L1jTE50_OVERLAY',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasOverlayStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jTE1500_OVERLAY',   l1SeedThresholds=['FSNOSEED'], stream=[MinBiasOverlayStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jTE4000_OVERLAY',   l1SeedThresholds=['FSNOSEED'], stream=[MinBiasOverlayStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        #new ZDC streamers
        ChainProp(name='HLT_noalg_L1ZDC_HELT20_jTEFWD2600',  l1SeedThresholds=['FSNOSEED'], stream=[UCCStream], groups=['PS:NoBulkMCProd']+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1ZDC_HELT35_jTEFWD2600',  l1SeedThresholds=['FSNOSEED'], stream=[UCCStream], groups=['PS:NoBulkMCProd']+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1ZDC_HELT50_jTEFWD2600',  l1SeedThresholds=['FSNOSEED'], stream=[UCCStream], groups=['PS:NoBulkMCProd']+SupportPhIGroup),
    ]

    #---- heavy ion EB chains
    chains['EnhancedBias'] += [
        ChainProp(name='HLT_noalg_eb_L1MU3V',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),

        ChainProp(name='HLT_noalg_eb_L1eEM15',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1eEM18',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1eEM26',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L12eEM18',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),

        ChainProp(name='HLT_noalg_eb_L1jTE50',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1jTE6500',       l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),

        ChainProp(name='HLT_noalg_eb_L1jTE5_VjTE200',  l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1jTE50_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1MU3V_VjTE50',   l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1VjTE200',       l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),

        ChainProp(name='HLT_noalg_eb_L1eTAU1_jTE4_VjTE200',  l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1eEM1_jTE4_VjTE200',   l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1eEM2_jTE4_VjTE200',   l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L12eTAU1_VjTE200',      l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L12eEM1_VjTE200',       l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1DPHI-2eTAU1_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1eTAU2_VjTE200',       l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1eEM2_VjTE200',        l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L12eTAU2_VjTE200',      l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L12eEM2_VjTE200',       l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),

        ChainProp(name='HLT_noalg_eb_L1eEM1_TRT_VjTE50',  l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1eTAU1_TRT_VjTE50', l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),

        ChainProp(name='HLT_noalg_eb_L1eTAU2_TRT_ZDC_OR_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1eEM2_TRT_ZDC_OR_VjTE200',  l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),

        ChainProp(name='HLT_noalg_eb_L1ZDC_A_C_VjTE50',             l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1ZDC_XOR_VjTE10',             l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1ZDC_XOR_jTE5_VjTE200',       l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1VZDC_A_VZDC_C_VjTE50',       l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1VZDC_A_VZDC_C_jTE5_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1ZDC_A_VjTE200',              l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1ZDC_C_VjTE200',              l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),

        ChainProp(name='HLT_noalg_eb_L1TRT_ZDC_A_VjTE50', l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1TRT_ZDC_C_VjTE50', l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1TRT_VjTE200',      l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),

        # ChainProp(name='HLT_noalg_eb_L1ZDC_XOR_jJ5_VjTE200',  l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1ZDC_XOR_jJ10_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1jJ5p30ETA49_VjTE200',  l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1jJ10p30ETA49_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),

        ChainProp(name='HLT_noalg_eb_L1MBTS_1_1',     l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1RD1_FILLED',   l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),

    ]

    chains['Monitor'] = [
          ChainProp(name='HLT_noalg_CostMonDS_L1All',        l1SeedThresholds=['FSNOSEED'], stream=['CostMonitoring'], groups=['PS:NoBulkMCProd']+['PS:NoBulkMCProd','RATE:Monitoring','BW:Other']),
    ]
    return chains


def setupMenu():

    from AthenaCommon.Logging import logging
    log = logging.getLogger( __name__ )
    log.info('setupMenu ...')

    chains = getPhysicsHISignatures()

    # We could use the menu name here now for other filtering
    P1_run3_v1.addCommonP1Signatures(chains)
    P1_run3_v1.addHeavyIonP1Signatures(chains)

    final_chains = ChainStore()
    for sig, chainsInSig in chains.items():
        for c in chainsInSig:
                if "EM3" in c.name: # EM3 without VTE and AFP is removed from HI L1 menu to avoid L1Calo EM overflow
                    raise RuntimeError(f"EM3 not available in HI L1 menu, requested by chain {c.name}")
                elif "EM7" in c.name: # EM7 without VTE and AFP is removed from HI L1 menu to avoid L1Calo EM overflow 
                    raise RuntimeError(f"EM7 not available in HI L1 menu, requested by chain {c.name}")
                else:
                    final_chains[sig].append(c)
    return final_chains
