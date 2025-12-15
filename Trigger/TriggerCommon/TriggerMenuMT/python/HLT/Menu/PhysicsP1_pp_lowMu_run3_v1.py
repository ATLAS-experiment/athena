# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# PhysicsP1_pp_lowMu_run3_v1.py menu
#------------------------------------------------------------------------#

# All chains are represented as ChainProp objects in a ChainStore
from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp
from .SignatureDicts import ChainStore

from .Physics_pp_run3_v1 import (
    PhysicsStream,
    SingleMuonGroup,
    MultiMuonGroup,
    MultiElectronGroup,
    SingleElectronGroup,
    Topo2Group,
    MinBiasGroup,
    SupportGroup,
    SupportPhIGroup,
    JetStreamersGroup,
    METStreamersGroup,
    TauStreamersGroup,
    EgammaStreamersGroup,
    PrimaryL1MuGroup,
    PrimaryPhIGroup,
    SinglePhotonGroup,
    SingleJetGroup,
    SingleBjetGroup,
    JetPhaseIStreamersGroup,
    TagAndProbeGroup,
    BphysicsGroup
)
from . import P1_run3_v1

LowMuGroup = ['LowMu']
LowMuGroupPhI = ['LowMuPhaseI']


def getLowMuPhysicsSignatures():

    chains = ChainStore()

    chains['Muon'] = [

        #ART-23577
        ChainProp(name='HLT_mu20_L1MU5VF_AFP_A_OR_C',     l1SeedThresholds=['MU5VF'],   groups=SingleMuonGroup+LowMuGroup),
        ChainProp(name='HLT_mu20_L1MU5VF_AFP_A_AND_C',     l1SeedThresholds=['MU5VF'],   groups=SingleMuonGroup+LowMuGroup),
        ChainProp(name='HLT_mu20_L1MU5VF',     l1SeedThresholds=['MU5VF'],   groups=SingleMuonGroup+LowMuGroup),

        #ATR-27744
        #-- 1 mu
        ChainProp(name='HLT_mu4_L1MU3V',   stream=[PhysicsStream, 'express'], groups=SingleMuonGroup+SupportGroup, monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu6_L1MU3V',  stream=[PhysicsStream], groups=SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_mu8_L1MU5VF',  stream=[PhysicsStream, 'express'], groups=SingleMuonGroup+PrimaryL1MuGroup, monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu10_L1MU8F',  stream=[PhysicsStream], groups=SingleMuonGroup+PrimaryL1MuGroup, monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu12_L1MU8F', stream=[PhysicsStream], groups=SingleMuonGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu15_L1MU8F', stream=[PhysicsStream], groups=SingleMuonGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu15_L1MU14FCH', stream=[PhysicsStream], groups=SingleMuonGroup+PrimaryL1MuGroup), #ATR-30147
        ChainProp(name='HLT_mu4noL1_L1MBTS_1', stream=[PhysicsStream], l1SeedThresholds=['FSNOSEED'], groups=SingleMuonGroup+SupportGroup),

        # Bell measurement at low mu (ATR-30256)
        ChainProp(name='HLT_2mu3_L12MU3V', stream=[PhysicsStream], groups=MultiMuonGroup+SupportGroup),

        #-- 2 mu
        ChainProp(name='HLT_2mu4_L12MU3V', stream=[PhysicsStream, 'express'], groups=MultiMuonGroup+PrimaryL1MuGroup, monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu4_mu6_L12MU3V', stream=[PhysicsStream], groups=MultiMuonGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu4_mu4noL1_L1MU3V', stream=[PhysicsStream], l1SeedThresholds=['MU3V','FSNOSEED'], groups=MultiMuonGroup+SupportGroup, monGroups=['muonMon:shifter','muonMon:online']),

        #-- tag-and-probe
        ChainProp(name='HLT_mu8_mu4_probe_L1MU5VF', l1SeedThresholds=['MU5VF','PROBEMU3V'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu6_mu4_probe_L1MU3V',  l1SeedThresholds=['MU3V', 'PROBEMU3V'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu4_mu4_probe_L1MU3V',  l1SeedThresholds=['MU3V', 'PROBEMU3V'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu8_mu6_probe_L1MU5VF', l1SeedThresholds=['MU5VF','PROBEMU3V'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu6_mu6_probe_L1MU3V',  l1SeedThresholds=['MU3V', 'PROBEMU3V'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu4_mu6_probe_L1MU3V',  l1SeedThresholds=['MU3V', 'PROBEMU3V'], groups=SingleMuonGroup+TagAndProbeGroup),

        #BLS
        ChainProp(name='HLT_2mu4_l2io_invmDimu_L12MU3V', stream=[PhysicsStream], groups=PrimaryL1MuGroup+MultiMuonGroup, monGroups=['bphysMon:shifter']),
        ChainProp(name='HLT_mu14_mu14_idtp_idZmumu_L12MU8F', stream=[PhysicsStream], groups=PrimaryL1MuGroup+MultiMuonGroup,  monGroups=['idMon:shifter','idMon:t0']),
        ChainProp(name='HLT_mu4_mu4_idperf_1invmAB5_L12MU3V',      l1SeedThresholds=['MU3V', 'MU3V'],  stream=[PhysicsStream], groups=MultiMuonGroup+SupportGroup, monGroups=['idMon:t0']),
    ]

    chains['Bphysics'] += [

        #BLS triggers
        ChainProp(name='HLT_2mu4_bDimu_L12MU3V', stream=[PhysicsStream], groups=PrimaryL1MuGroup+BphysicsGroup, monGroups=['bphysMon:online','bphysMon:shifter']),



    ]

    chains['Egamma'] = [

        # ElectronChains----------

        #ART-23577
        ChainProp(name='HLT_e20_lhloose_L1eEM9_AFP_A_OR_C', l1SeedThresholds=['eEM9'], groups=SingleElectronGroup+LowMuGroupPhI),
        ChainProp(name='HLT_e20_lhloose_L1eEM9_AFP_A_AND_C', l1SeedThresholds=['eEM9'], groups=SingleElectronGroup+LowMuGroupPhI),
        ChainProp(name='HLT_e20_lhloose_L1eEM9', l1SeedThresholds=['eEM9'], groups=LowMuGroupPhI),

        #--------- phase-1 supporting electron chains
        # replace L1eEM9 with L1eEM15 and L1eEM15 with eEM18, ATR-26366
        ChainProp(name='HLT_e15_etcut_L1eEM15',    stream=[PhysicsStream], groups=SingleElectronGroup+SupportPhIGroup),
        ChainProp(name='HLT_e20_etcut_L1eEM18',   stream=[PhysicsStream], groups=SingleElectronGroup+SupportPhIGroup),
        ChainProp(name='HLT_e30_etcut_L1eEM18', stream=[PhysicsStream] ,groups=SingleElectronGroup+SupportPhIGroup),
        ChainProp(name='HLT_e50_etcut_L1eEM26', stream=[PhysicsStream] ,groups=SingleElectronGroup+SupportPhIGroup),
        ChainProp(name='HLT_e20_idperf_loose_L1eEM18', stream=[PhysicsStream, 'express'], groups=SingleElectronGroup+SupportPhIGroup, monGroups=['idMon:t0']),

        ChainProp(name='HLT_e18_etcut_L1eEM18', stream=[PhysicsStream] ,groups=SingleElectronGroup+SupportPhIGroup),
        ChainProp(name='HLT_e18_lhloose_L1eEM18', stream=[PhysicsStream], groups=SingleElectronGroup+SupportPhIGroup, monGroups=['egammaMon:shifter']),
        ChainProp(name='HLT_e18_loose_L1eEM18', stream=[PhysicsStream], groups=SingleElectronGroup+SupportPhIGroup, monGroups=['egammaMon:shifter']),
        ChainProp(name='HLT_e18_lhmedium_L1eEM18', stream=[PhysicsStream], groups=SingleElectronGroup+SupportPhIGroup),
        ChainProp(name='HLT_e18_medium_L1eEM18', stream=[PhysicsStream], groups=SingleElectronGroup+SupportPhIGroup),

        #--------- phase-1 physics electron chains
        ChainProp(name='HLT_e14_lhvloose_L1eEM12L', stream=[PhysicsStream], groups=SingleElectronGroup+SupportPhIGroup, monGroups=['egammaMon:shifter']), #ATR-30147
        ChainProp(name='HLT_e15_lhloose_L1eEM15',  stream=[PhysicsStream, 'express'], groups=SingleElectronGroup+PrimaryPhIGroup, monGroups=['egammaMon:shifter']),
        ChainProp(name='HLT_e15_loose_L1eEM15',    stream=[PhysicsStream, 'express'], groups=SingleElectronGroup+PrimaryPhIGroup, monGroups=['egammaMon:shifter']),
        ChainProp(name='HLT_e15_lhmedium_L1eEM15', stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e15_medium_L1eEM15',   stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryPhIGroup),

        ChainProp(name='HLT_e20_lhloose_L1eEM18', stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e20_lhmedium_L1eEM18',stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e20_loose_L1eEM18',   stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e20_loose_L1eEM18L',  stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e20_medium_L1eEM18',  stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryPhIGroup),

        ChainProp(name='HLT_e30_lhloose_L1eEM18', stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e30_lhmedium_L1eEM18',stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e30_loose_L1eEM18',   stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e30_medium_L1eEM18',  stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryPhIGroup),

        ChainProp(name='HLT_2e20_lhloose_L12eEM18', stream=[PhysicsStream, 'express'], groups=SingleElectronGroup+PrimaryPhIGroup, monGroups=['egammaMon:shifter']),
        ChainProp(name='HLT_2e20_lhmedium_L12eEM18',stream=[PhysicsStream, 'express'], groups=SingleElectronGroup+PrimaryPhIGroup, monGroups=['egammaMon:shifter']),

        # PhotonChains----------
        #----------- phase-1 support photon chains
        ChainProp(name='HLT_g13_etcut_L1eEM12', stream=[PhysicsStream], groups=SinglePhotonGroup+SupportPhIGroup),
        ChainProp(name='HLT_g18_etcut_L1eEM12', stream=[PhysicsStream], groups=SinglePhotonGroup+SupportPhIGroup),
        ChainProp(name='HLT_g28_etcut_L1eEM18', stream=[PhysicsStream], groups=SinglePhotonGroup+SupportPhIGroup),
        ChainProp(name='HLT_g15_etcut_L1eEM15', stream=[PhysicsStream], groups=SinglePhotonGroup+SupportPhIGroup),
        ChainProp(name='HLT_g20_etcut_L1eEM15', stream=[PhysicsStream], groups=SinglePhotonGroup+SupportPhIGroup),

        #----------- phase-1 primary photon chains
        ChainProp(name='HLT_g15_loose_L1eEM12',  stream=[PhysicsStream, 'express'], groups=SinglePhotonGroup+SupportPhIGroup, monGroups=['egammaMon:online','egammaMon:shifter','egammaMon:val']),
        ChainProp(name='HLT_g15_loose_L1eEM12L',  stream=[PhysicsStream, 'express'], groups=SinglePhotonGroup+SupportPhIGroup, monGroups=['egammaMon:online','egammaMon:shifter','egammaMon:val']),
        ChainProp(name='HLT_g15_loose_L1eEM15', stream=[PhysicsStream], groups=SinglePhotonGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_g20_loose_L1eEM15', stream=[PhysicsStream, 'express'], groups=SinglePhotonGroup+PrimaryPhIGroup, monGroups=['egammaMon:online','egammaMon:shifter','egammaMon:val']),
        ChainProp(name='HLT_g30_loose_L1eEM18',  stream=[PhysicsStream], groups=SinglePhotonGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_g40_loose_L1eEM18',  stream=[PhysicsStream], groups=SinglePhotonGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_g40_loose_L1eEM26',  stream=[PhysicsStream], groups=SinglePhotonGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_g50_loose_L1eEM26',  stream=[PhysicsStream], groups=SinglePhotonGroup+PrimaryPhIGroup),

        #----------- Jpsi ee chains - ATR-30145        
        ChainProp(name='HLT_e9_lhtight_e4_etcut_1invmAB5_L1JPSI-1M5-eEM9', stream=[PhysicsStream,'express'], l1SeedThresholds=['eEM9','eEM5'], groups=SupportPhIGroup+MultiElectronGroup+Topo2Group+['RATE:CPS_JPSI-1M5-eEM9'], monGroups=['egammaMon:shifter_topo']),
        ChainProp(name='HLT_e5_lhtight_e9_etcut_1invmAB5_L1JPSI-1M5-eEM9', stream=[PhysicsStream,'express'], l1SeedThresholds=['eEM5','eEM9'], groups=SupportPhIGroup+MultiElectronGroup+Topo2Group+['RATE:CPS_JPSI-1M5-eEM9'], monGroups=['egammaMon:shifter_topo']),
        ChainProp(name='HLT_e5_lhtight_e4_etcut_1invmAB5_L1JPSI-1M5-eEM9', stream=[PhysicsStream,'express'], l1SeedThresholds=['eEM5','eEM5'], groups=SupportPhIGroup+MultiElectronGroup+Topo2Group+['RATE:CPS_JPSI-1M5-eEM9'], monGroups=['egammaMon:shifter_topo']),
        ChainProp(name='HLT_e5_lhtight_e4_etcut_1invmAB5_L1RD0_FILLED', stream=[PhysicsStream,'express'], l1SeedThresholds=['eEM5','eEM5'], groups=SupportPhIGroup+MultiElectronGroup+Topo2Group, monGroups=['egammaMon:shifter_topo']),
    ]

    chains['Jet'] = [
        ChainProp(name='HLT_j20_L1AFP_A_OR_C', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_j20_L1AFP_A_AND_C', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_j20_L1AFP_A_OR_C_jJ20', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j20_L1AFP_A_AND_C_jJ20', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j20_L1AFP_A_OR_C_jJ30', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j20_L1AFP_A_AND_C_jJ30', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j20_L1jJ20', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j20_L1jJ30', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j20_L1MBTS_2', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_j20_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_j20f_L1AFP_A_OR_C', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_j20f_L1AFP_A_AND_C', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_j10f_L1MBTS_2', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_j10f_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_2j20_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_2j20_20detaAA_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_2j20_20detaAA_L1MBTS_2', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_2j10_20detaAA_L1AFP_A_AND_C', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_2j10_20detaAA_L1AFP_A_OR_C', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroup),


        #-------- phase-1
        ChainProp(name='HLT_j40_L1jJ40', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream, 'express'], groups=SingleJetGroup+LowMuGroupPhI, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j50_L1jJ40', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j60_L1jJ50', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j75_L1jJ50', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j85_L1jJ50', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream, 'express'], groups=SingleJetGroup+LowMuGroupPhI, monGroups=['jetMon:t0','jetMon:online','detMon']),
        ChainProp(name='HLT_j100_L1jJ50', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j100_L1jJ60', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j120_L1jJ60', l1SeedThresholds=['FSNOSEED'],  stream=[PhysicsStream, 'express'], groups=SingleJetGroup+LowMuGroupPhI, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j140_L1jJ90', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j175_L1jJ90', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j260_L1jJ125', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream, 'express'], groups=SingleJetGroup+LowMuGroupPhI, monGroups=['jetMon:t0','jetMon:online']),

        ChainProp(name='HLT_j15f_L1jTE5', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j25f_L1jTE10', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j35f_L1jTE20', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j45f_L1jJ40p30ETA49', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j60f_L1jJ50p30ETA49', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j75f_L1jJ50p30ETA49', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j85f_L1jJ50p30ETA49', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+LowMuGroupPhI),

        ChainProp(name='HLT_j30a_L1jTE20', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j40a_L1jTE20', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+LowMuGroupPhI),

        ChainProp(name='HLT_j110C_a10r_L1jJ60', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j110C_a10_lcw_subjes_L1jJ60', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j130C_a10r_L1jJ60', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j130C_a10_lcw_subjes_L1jJ60', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j175C_a10r_L1jJ90', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j175C_a10_lcw_subjes_L1jJ90', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j260C_a10r_L1jJ125', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j260C_a10_lcw_subjes_L1jJ125', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+LowMuGroupPhI),

        ChainProp(name='HLT_j110C_a10t_lcw_jes_L1gLJ80p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_gLJ80p0ETA25']),
        ChainProp(name='HLT_j110C_a10sd_cssk_pf_jes_ftf_preselj80_L1gLJ80p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_gLJ80p0ETA25']),
        ChainProp(name='HLT_noalg_L1gLJ80p0ETA25',         l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),
    ]

    chains['Bjet'] = [

        #Phase1
        #performace chains
        ChainProp(name="HLT_j30_0eta290_020jvt_boffperf_pf_ftf_L1jTE50", l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+SingleBjetGroup, monGroups=['bJetMon:online', 'bJetMon:t0']),
        ChainProp(name="HLT_j45_0eta290_020jvt_boffperf_pf_ftf_L1jJ40", l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_j60_0eta290_020jvt_boffperf_pf_ftf_L1jJ50", l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream, 'express'], groups=SupportPhIGroup+SingleBjetGroup, monGroups=['bJetMon:t0']),
        ChainProp(name="HLT_j80_0eta290_020jvt_boffperf_pf_ftf_L1jJ60", l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_j100_0eta290_020jvt_boffperf_pf_ftf_L1jJ60", l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+SingleBjetGroup),

        #bjet chains
        ChainProp(name="HLT_j30_0eta290_020jvt_bgn260_pf_ftf_L1jTE50", l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_j45_0eta290_020jvt_bgn260_pf_ftf_L1jJ40", l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_j60_0eta290_020jvt_bgn260_pf_ftf_L1jJ50", l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream, 'express'], groups=PrimaryPhIGroup+SingleBjetGroup, monGroups=['bJetMon:t0']),
        ChainProp(name="HLT_j80_0eta290_020jvt_bgn260_pf_ftf_L1jJ60", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_j100_0eta290_020jvt_bgn260_pf_ftf_L1jJ60", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),

    ]

    chains['Combined'] = [
        # AFP + dijet
        ChainProp(name='HLT_2j120_mb_afprec_afpdijet_L1AFP_A_AND_C_TOF_jJ90', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],groups=MinBiasGroup+LowMuGroupPhI, monGroups=['mbMon:shifter']),
        ChainProp(name='HLT_2j175_mb_afprec_afpdijet_L1AFP_A_AND_C_TOF_jJ125', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],groups=MinBiasGroup+LowMuGroupPhI, monGroups=['mbMon:t0']),

        ChainProp(name='HLT_2j20_mb_afprec_afpdijet_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:t0']),

        # AFP ToF Vertex Delta Z: ATR-15719
        ChainProp(name='HLT_2j20_ftf_mb_afprec_afpdz5_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream], groups=MinBiasGroup),
        ChainProp(name='HLT_2j20_ftf_mb_afprec_afpdz10_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream], groups=MinBiasGroup),

        #ATR-27744
        ChainProp(name='HLT_mu4_j40_L1MU3V', l1SeedThresholds=['MU3V','FSNOSEED'], stream=[PhysicsStream], groups=SingleBjetGroup+LowMuGroup),
        ChainProp(name='HLT_mu4_j50_L1MU3V', l1SeedThresholds=['MU3V','FSNOSEED'], stream=[PhysicsStream], groups=SingleBjetGroup+LowMuGroup),
        ChainProp(name='HLT_mu4_j60_L1MU3V', l1SeedThresholds=['MU3V','FSNOSEED'], stream=[PhysicsStream], groups=SingleBjetGroup+LowMuGroup),
        ChainProp(name='HLT_mu4_j40_dRAB05_L1MU3V', l1SeedThresholds=['MU3V','FSNOSEED'], stream=[PhysicsStream], groups=SingleBjetGroup+LowMuGroup),
        ChainProp(name='HLT_mu4_j50_dRAB05_L1MU3V', l1SeedThresholds=['MU3V','FSNOSEED'], stream=[PhysicsStream], groups=SingleBjetGroup+LowMuGroup),
        ChainProp(name='HLT_mu4_j60_dRAB05_L1MU3V', l1SeedThresholds=['MU3V','FSNOSEED'], stream=[PhysicsStream], groups=SingleBjetGroup+LowMuGroup),
        #ATR-28139
        ChainProp(name='HLT_mu6_j40_dRAB05_L1MU5VF', l1SeedThresholds=['MU5VF','FSNOSEED'], stream=[PhysicsStream], groups=SingleBjetGroup+LowMuGroup),
        ChainProp(name='HLT_mu6_j40_L1MU5VF', l1SeedThresholds=['MU5VF','FSNOSEED'], stream=[PhysicsStream], groups=SingleBjetGroup+LowMuGroup),
        ChainProp(name='HLT_mu4_j60_dRAB05_L1MU3V_jJ60', l1SeedThresholds=['MU3V','FSNOSEED'], stream=[PhysicsStream], groups=SingleBjetGroup+LowMuGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mu4_j60_L1MU3V_jJ60', l1SeedThresholds=['MU3V','FSNOSEED'], stream=[PhysicsStream], groups=SingleBjetGroup+LowMuGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mu4_j60_dRAB05_L1MU3V_jJ50', l1SeedThresholds=['MU3V','FSNOSEED'], stream=[PhysicsStream], groups=SingleBjetGroup+LowMuGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mu4_j60_L1MU3V_jJ50', l1SeedThresholds=['MU3V','FSNOSEED'], stream=[PhysicsStream], groups=SingleBjetGroup+LowMuGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mu4_j50_dRAB05_L1MU3V_jJ50', l1SeedThresholds=['MU3V','FSNOSEED'], stream=[PhysicsStream], groups=SingleBjetGroup+LowMuGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mu4_j50_L1MU3V_jJ50', l1SeedThresholds=['MU3V','FSNOSEED'], stream=[PhysicsStream], groups=SingleBjetGroup+LowMuGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mu4_j40_dRAB05_L1MU3V_jJ50', l1SeedThresholds=['MU3V','FSNOSEED'], stream=[PhysicsStream, 'express'], groups=SingleBjetGroup+LowMuGroup+PrimaryPhIGroup, monGroups=['muonMon:t0','bJetMon:t0']),
        ChainProp(name='HLT_mu4_j40_L1MU3V_jJ50', l1SeedThresholds=['MU3V','FSNOSEED'], stream=[PhysicsStream], groups=SingleBjetGroup+LowMuGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L1eTAU1', l1SeedThresholds=['FSNOSEED']*2,stream=['MinBias'],groups=MinBiasGroup+SupportPhIGroup),
        # ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L1jTAU1', l1SeedThresholds=['FSNOSEED']*2,stream=['MinBias'],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L1eEM1', l1SeedThresholds=['FSNOSEED']*2,stream=['MinBias'],groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),

    ]

    chains['MinBias'] = [
        ChainProp(name='HLT_mb_sptrk_L1RD0_FILLED',    l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=LowMuGroup+MinBiasGroup+['RATE:CPS_RD0_FILLED'], monGroups=['mbMon:online','mbMon:shifter']),
        ChainProp(name='HLT_mb_sptrk_L1MBTS_1', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sptrk_L1MBTS_1_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_L1MBTS_1_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp_L1RD0_FILLED',       l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=LowMuGroup+MinBiasGroup+['RATE:CPS_RD0_FILLED'], monGroups=['mbMon:shifter']),

        ChainProp(name='HLT_mb_excl_2trk6_pt1_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:shifter']),
        ChainProp(name='HLT_mb_excl_1trk5_pt4_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:shifter']),

        ChainProp(name='HLT_mb_excl_1trk2_pt1_vetombts2in_L1TRT_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_excl_1trk2_pt1_vetombts2in_L1TRT_EMPTY',  l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_excl_1trk2_pt1_L1TRT_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_excl_1trk2_pt1_L1TRT_EMPTY',  l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),



        ChainProp(name='HLT_mb_sptrk_pt2_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:shifter']),
        ChainProp(name='HLT_mb_sptrk_pt2_L1MBTS_2', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:shifter']),
        ChainProp(name='HLT_mb_sptrk_pt4_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sptrk_pt4_L1MBTS_2', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_pt6_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sptrk_pt6_L1MBTS_2', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_pt8_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sptrk_pt8_L1MBTS_2', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_pt2_L1AFP_A_OR_C', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_pt4_L1AFP_A_OR_C', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_pt6_L1AFP_A_OR_C', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_pt8_L1AFP_A_OR_C', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_pt2_L1AFP_A_AND_C', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_pt4_L1AFP_A_AND_C', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_pt6_L1AFP_A_AND_C', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_pt8_L1AFP_A_AND_C', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name="HLT_mb_mbts_L1MBTS_1_EMPTY",               l1SeedThresholds=['FSNOSEED'], stream=['MinBias','express'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:t0']),
        ChainProp(name="HLT_mb_mbts_L1MBTS_1_UNPAIRED_ISO",               l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:t0']),
        ChainProp(name="HLT_mb_mbts_L1MBTS_1",                     l1SeedThresholds=['FSNOSEED'], stream=['MinBias','express'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:shifter']),
        ChainProp(name="HLT_mb_mbts_L1MBTS_1_1",                   l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:t0']),
        ChainProp(name="HLT_mb_mbts_L1MBTS_2",                     l1SeedThresholds=['FSNOSEED'], stream=['MinBias','express'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:shifter']),
        ChainProp(name="HLT_mb_mbts_L1RD0_FILLED",                 l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:t0']),
        ChainProp(name="HLT_mb_mbts_L1RD0_EMPTY",                  l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:t0']),
        ChainProp(name="HLT_mb_mbts_L1RD0_UNPAIRED_ISO",                  l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:t0']),


        ChainProp(name='HLT_mb_sptrk_L1MBTS_2', l1SeedThresholds=['FSNOSEED'], stream=['MinBias','express'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sptrk_L1MBTS_2_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_L1MBTS_2_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp900_trk60_hmt_L1MBTS_1_1', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp1400_trk90_hmt_L1jTE5', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup+LowMuGroupPhI),
        ChainProp(name='HLT_mb_sptrk_pt4_L1MBTS_1', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_pt6_L1MBTS_1', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_pt8_L1MBTS_1', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),


        #ATR-27744
        ChainProp(name='HLT_mb_sptrk_L1ZDC_OR', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_A_AND_C', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp100_trk30_hmt_L1ZDC_OR', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp100_trk30_hmt_L1ZDC_A_AND_C', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp100_trk30_hmt_L1jTE3', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroupPhI),

        # Phase I jet inputs ATR-24411
        # AFP
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_jJ50', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupPhI),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_T0T1_jJ50', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupPhI),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_jJ60', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupPhI),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_T0T1_jJ60', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupPhI),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_jJ90', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupPhI),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_T0T1_jJ90', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupPhI),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_jJ125', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupPhI),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_T0T1_jJ125', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupPhI),

        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_jJ20', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupPhI),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_jJ30', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupPhI),

        # HMT
        ChainProp(name='HLT_mb_sp500_trk40_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp500_trk40_hmt_L1MBTS_2',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp500_trk40_hmt_L1MBTS_2_2',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp500_trk40_hmt_L1MBTS_3_3',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp600_trk50_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias','express'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:shifter']),
        ChainProp(name='HLT_mb_sp600_trk50_hmt_L1jTE3',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias','express'], groups=MinBiasGroup+LowMuGroup+LowMuGroupPhI, monGroups=['mbMon:shifter']),
        ChainProp(name='HLT_mb_sp600_trk50_hmt_L1MBTS_2',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:shifter']),
        ChainProp(name='HLT_mb_sp600_trk50_hmt_L1MBTS_2_2',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp600_trk50_hmt_L1MBTS_3_3',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias', 'express'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp600_trk60_hmt_L1MBTS_2',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp600_trk60_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp800_trk60_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp800_trk60_hmt_L1MBTS_2',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp800_trk60_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp800_trk60_hmt_L1jTE3',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup+LowMuGroupPhI),
        ChainProp(name='HLT_mb_sp1000_trk80_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp1000_trk80_hmt_L1MBTS_2',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp1000_trk80_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp1000_trk80_hmt_L1jTE5',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup+LowMuGroupPhI),
        ChainProp(name='HLT_mb_sp1500_trk100_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp1500_trk100_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp1500_trk100_hmt_L1jTE5',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup+LowMuGroupPhI),
        ChainProp(name='HLT_mb_sp2000_trk130_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp2000_trk130_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp4100_trk260_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp4100_trk260_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp5000_trk290_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp5000_trk290_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),

        ChainProp(name='HLT_mb_sp2000_trk130_hmt_L1jTE5',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup+LowMuGroupPhI),
        ChainProp(name='HLT_mb_sp3000_trk200_hmt_L1jTE20',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup+LowMuGroupPhI),
        ChainProp(name='HLT_mb_sp4100_trk260_hmt_L1jTE20',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup+LowMuGroupPhI),
        ChainProp(name='HLT_mb_sp5000_trk290_hmt_L1jTE50',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup+LowMuGroupPhI),
        
        # HMT with pileup suppression
        ChainProp(name='HLT_mb_sp15_pusup0_trk5_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp500_pusup7_trk40_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp500_pusup7_trk40_hmt_L1MBTS_2',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp500_pusup7_trk40_hmt_L1MBTS_2_2',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp500_pusup7_trk40_hmt_L1MBTS_3_3',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp600_pusup10_trk50_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:shifter']),
        ChainProp(name='HLT_mb_sp600_pusup10_trk50_hmt_L1MBTS_2',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:shifter']),
        ChainProp(name='HLT_mb_sp600_pusup10_trk50_hmt_L1MBTS_2_2',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp600_pusup10_trk50_hmt_L1MBTS_3_3',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp800_pusup15_trk60_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp800_pusup15_trk60_hmt_L1MBTS_2_2',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp800_pusup15_trk60_hmt_L1MBTS_3_3',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp800_pusup15_trk60_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp1000_pusup30_trk80_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp1000_pusup30_trk80_hmt_L1MBTS_3_3',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp1000_pusup30_trk80_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp1500_pusup40_trk100_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp1500_pusup40_trk100_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp2000_pusup50_trk130_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp2000_pusup50_trk130_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp2000_pusup60_trk130_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp2000_pusup60_trk130_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp3000_pusup100_trk200_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp3000_pusup100_trk200_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp3000_pusup120_trk200_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp3000_pusup120_trk200_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp4100_pusup150_trk260_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp4100_pusup150_trk260_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp4100_pusup180_trk260_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp4100_pusup180_trk260_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp4100_pusup200_trk260_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp4100_pusup200_trk260_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp5000_pusup180_trk290_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp5000_pusup180_trk290_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp5000_pusup220_trk290_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp5000_pusup220_trk290_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp5000_pusup250_trk290_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp5000_pusup250_trk290_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),

        # afprec chains
        ChainProp(name='HLT_mb_afprec_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=MinBiasGroup+LowMuGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_afprec_afptof_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_OR_C', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroup,monGroups=['mbMon:online','mbMon:shifter']),

        # To follow up with forward physics
        ChainProp(name='HLT_mb_sptrk_vetombts2in_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
    ]

    chains['Monitor'] = [
        ChainProp(name='HLT_noalg_CostMonDS_L1All',        l1SeedThresholds=['FSNOSEED'], stream=['CostMonitoring'], groups=['PS:NoBulkMCProd', 'Primary:CostAndRate', 'RATE:Monitoring', 'BW:Other']), # HLT_costmonitor
    ]

    chains['Streaming'] = [

        ChainProp(name='HLT_noalg_L1TRT_EMPTY',  l1SeedThresholds=['FSNOSEED'], stream=['IDCosmic','express'],groups=['RATE:SeededStreamers','BW:Other'],monGroups=['detMon'] ),
        ChainProp(name='HLT_noalg_L1TRT_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['IDCosmic'],groups=['RATE:SeededStreamers','BW:Other']),

        ChainProp(name='HLT_noalg_L1RD0_EMPTY',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+MinBiasGroup),
        ChainProp(name='HLT_noalg_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+MinBiasGroup),

        ChainProp(name='HLT_noalg_mb_L1MBTS_1', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup), 
        ChainProp(name='HLT_noalg_mb_L1MBTS_1_1', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup), 
        ChainProp(name='HLT_noalg_mb_L1MBTS_2', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup), 
        ChainProp(name='HLT_noalg_L1MBTS_1_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup), 
        ChainProp(name='HLT_noalg_L1MBTS_1_1_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup), 
        ChainProp(name='HLT_noalg_L1MBTS_2_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup), 
        ChainProp(name='HLT_noalg_mb_L1RD0_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+LowMuGroup), 
        #ATR-27744
        ChainProp(name='HLT_noalg_L1jTE3',       l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jTE10',      l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jTE50',      l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jTE100',     l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        # ChainProp(name='HLT_noalg_L1jTAU1',     l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1eTAU1',     l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1eEM1',     l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_noalg_L1MU3V',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SingleMuonGroup),
        ChainProp(name='HLT_noalg_L1MU8VF',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SingleMuonGroup),
        ChainProp(name='HLT_noalg_L1MU8F',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SingleMuonGroup), #ATR-30147 
        ChainProp(name='HLT_noalg_L1MU5VF',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SingleMuonGroup),
        ChainProp(name='HLT_noalg_L1MU14FCH',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SingleMuonGroup), #ATR-30147

        #EM3 is removed from HI L1 menu
        ChainProp(name='HLT_noalg_L1eEM15',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+EgammaStreamersGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1eEM18',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+EgammaStreamersGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1eEM26',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+EgammaStreamersGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1eEM12L',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+EgammaStreamersGroup+SupportPhIGroup),
                
        ChainProp(name='HLT_noalg_L1eEM5',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+EgammaStreamersGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1eEM9',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+EgammaStreamersGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1eEM26M',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+EgammaStreamersGroup+SupportPhIGroup),

        ChainProp(name='HLT_noalg_L1eTAU80',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+TauStreamersGroup+SupportPhIGroup),

        ChainProp(name='HLT_noalg_L1jJ20',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+JetStreamersGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jJ30',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+JetStreamersGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jJ40',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+JetStreamersGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jJ50',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+JetStreamersGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jJ55',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+JetStreamersGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jJ60',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+JetStreamersGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jJ80',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+JetStreamersGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jJ90',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+JetStreamersGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jJ125',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+JetStreamersGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jJ160',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+JetStreamersGroup+SupportPhIGroup),

        

        #ATR-27986
        ChainProp(name='HLT_noalg_L1jXE110',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+METStreamersGroup),
        ChainProp(name='HLT_noalg_L1jXE120',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+METStreamersGroup),
        ChainProp(name='HLT_noalg_L1gXEJWOJ110',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+METStreamersGroup),
        ChainProp(name='HLT_noalg_L1gXEJWOJ120',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+METStreamersGroup),
        

        # Low mu AFP
        ChainProp(name='HLT_noalg_L1MU5VF_AFP_A_OR_C', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU5VF_AFP_A_AND_C', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportGroup),

        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_jJ20', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_OR_C_jJ20', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_jJ30', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_OR_C_jJ30', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_OR_C', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_TOF_jJ20', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_TOF_jJ30', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_OR_C_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_noalg_L1AFP_A_OR_C_UNPAIRED_NONISO', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_noalg_L1AFP_A_OR_C_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportGroup, monGroups=['mbMon:t0']),
        ChainProp(name='HLT_noalg_L1AFP_A_OR_C_FIRSTEMPTY', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportGroup, monGroups=['mbMon:t0']),



        # Calibration AFP
        # low mu
        ChainProp(name='HLT_noalg_L1AFP_NSA_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_NSC_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportGroup),

        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_TOF_T0T1_jJ90', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_TOF_T0T1_jJ125', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_TOF_jJ90', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_TOF_jJ125', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_TOF_jJ50', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_TOF_jJ60', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),

        # Calibration AFP
        # all mu
        ChainProp(name='HLT_noalg_L1AFP_FSA_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSC_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSA_TOF_T0_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSA_TOF_T1_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSC_TOF_T0_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSC_TOF_T1_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportGroup),

        ChainProp(name='HLT_noalg_L1ZDC_A_AND_C', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportGroup),
    ]

    chains['EnhancedBias'] += [
        ChainProp(name='HLT_noalg_eb_L1MU3V',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),

        ChainProp(name='HLT_noalg_eb_L1eEM12',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportPhIGroup ),
        ChainProp(name='HLT_noalg_eb_L1eEM15',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportPhIGroup ),
        ChainProp(name='HLT_noalg_eb_L1eEM18',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportPhIGroup ),
        ChainProp(name='HLT_noalg_eb_L1eEM26',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportPhIGroup ),
        ChainProp(name='HLT_noalg_eb_L12eEM18',        l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportPhIGroup ),

        ChainProp(name='HLT_noalg_eb_L1jTE3',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportPhIGroup ),
        ChainProp(name='HLT_noalg_eb_L1jTE10',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportPhIGroup ),
        ChainProp(name='HLT_noalg_eb_L1jTE50',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportPhIGroup ),
        ChainProp(name='HLT_noalg_eb_L1jTE100',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportPhIGroup ),

        ChainProp(name='HLT_noalg_eb_L1MBTS_1',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),

        ChainProp(name='HLT_noalg_eb_L1jJ40',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportPhIGroup ),
        ChainProp(name='HLT_noalg_eb_L1jJ50',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportPhIGroup ),
        ChainProp(name='HLT_noalg_eb_L1jJ60',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportPhIGroup ),

        ChainProp(name='HLT_noalg_eb_L1RD1_FILLED',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
    ]

    return chains

def setupMenu():
    # Add all standard monitoring chains from addP1Signatures function
    final_chains = ChainStore()

    chains = getLowMuPhysicsSignatures()
    P1_run3_v1.addCommonP1Signatures(chains)
    P1_run3_v1.addLowMuP1Signatures(chains)
    for sig, chainsInSig in chains.items():
        for c in chainsInSig:
                if "EM3" in c.name: # EM3 without VTE or AFP is removed from HI L1 menu to avoid L1Calo EM overflow
                    raise RuntimeError(f"EM3 not available in HI L1 menu, requested by chain {c.name}")
                if "EM7" in c.name: # EM7 without VTE or AFP is removed from HI L1 menu to avoid L1Calo EM overflow
                    raise RuntimeError(f"EM7 not available in HI L1 menu, requested by chain {c.name}")
                else:
                    final_chains[sig].append(c)

    return final_chains
