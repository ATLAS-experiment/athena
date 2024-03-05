# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# PhysicsP1_HI_run3_v1.py menu
#------------------------------------------------------------------------#

# This defines the input format of the chain and it's properties with the defaults set
# always required are: name, stream and groups
#['name', 'L1chainParts'=[], 'stream', 'groups', 'merging'=[], 'topoStartFrom'=False],
from ..Config.Utility.ChainDefInMenu import ChainProp
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
        SupportLegGroup,
        SupportPhIGroup,
        PrimaryLegGroup,
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
### following stream tags not used yet, need to be implemented in StreamInfo.py before use
#UCCStream="UCC"
#PCpebStream="PCPEB"
#CCpebStream="CCPEB"

LowMuGroup = ['LowMu']
LowMuGroupPhI = ['LowMuPhaseI']
LowMuGroupLeg = ['LowMuLegacy']

def getPhysicsHISignatures():
    chains = ChainStore()

    chains['Muon'] = [
        #-- 1 mu
        ChainProp(name='HLT_mu4_L1MU3V', stream=[HardProbesStream], groups=SingleMuonGroup),
        ChainProp(name='HLT_mu6_L1MU3V',   stream=[HardProbesStream, 'express'], groups=SingleMuonGroup+PrimaryL1MuGroup, monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu6_L1MU5VF',  stream=[HardProbesStream], groups=SingleMuonGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu8_L1MU5VF',  stream=[HardProbesStream, 'express'], groups=SingleMuonGroup+PrimaryL1MuGroup, monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu10_L1MU8F',  stream=[HardProbesStream], groups=SingleMuonGroup+PrimaryL1MuGroup, monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu10_L1MU5VF', stream=[HardProbesStream], groups=SingleMuonGroup+PrimaryL1MuGroup),

        #-- 2 mu
        ChainProp(name='HLT_2mu4_L12MU3V', stream=[HardProbesStream, 'express'], groups=MultiMuonGroup+PrimaryL1MuGroup, monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu4_mu4noL1_L1MU3V', stream=[HardProbesStream], l1SeedThresholds=['MU3V','FSNOSEED'], groups=MultiMuonGroup+PrimaryL1MuGroup, monGroups=['muonMon:shifter','muonMon:online']),

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

        #-- UPC
        ChainProp(name='HLT_mu3_L1MU3V_VTE50',        stream=[UPCStream], groups=SingleMuonGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu4_L1MU3V_VTE50',        stream=[UPCStream, 'express'], groups=SingleMuonGroup+PrimaryL1MuGroup, monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu4_L1MU3V_VTE200',        stream=[UPCStream], groups=SingleMuonGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu6_L1MU3V_VTE50',        stream=[UPCStream], groups=SingleMuonGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu8_L1MU5VF_VTE50',       stream=[UPCStream], groups=SingleMuonGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_2mu4_L12MU3V_VTE50',      stream=[UPCStream], groups=MultiMuonGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu4_mu4noL1_L1MU3V_VTE50',stream=[UPCStream], l1SeedThresholds=['MU3V','FSNOSEED'], groups=MultiMuonGroup+PrimaryL1MuGroup),

        #-- UPC - phase-1
        ChainProp(name='HLT_mu4_L1MU3V_VjTE50',        stream=[UPCStream], groups=SingleMuonGroup+PrimaryL1MuGroup+PrimaryPhIGroup+['PS:NoHLTRepro'], monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu6_L1MU3V_VjTE50',        stream=[UPCStream], groups=SingleMuonGroup+PrimaryL1MuGroup+PrimaryPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mu8_L1MU5VF_VjTE50',       stream=[UPCStream], groups=SingleMuonGroup+PrimaryL1MuGroup+PrimaryPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_2mu4_L12MU3V_VjTE50',      stream=[UPCStream], groups=MultiMuonGroup+PrimaryL1MuGroup+PrimaryPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mu4_mu4noL1_L1MU3V_VjTE50',stream=[UPCStream], l1SeedThresholds=['MU3V','FSNOSEED'], groups=MultiMuonGroup+PrimaryL1MuGroup+PrimaryPhIGroup+['PS:NoHLTRepro']),
    ]
    chains['Bphysics'] += [

        #BLS triggers
        ChainProp(name='HLT_2mu4_bDimu_L12MU3V', stream=[HardProbesStream, 'express'], groups=PrimaryL1MuGroup+BphysicsGroup, monGroups=['bphysMon:online','bphysMon:shifter']),

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
        ChainProp(name='HLT_g15_loose_ion_L1eEM12',  stream=[HardProbesStream], groups=SinglePhotonGroup+PrimaryPhIGroup, monGroups=['egammaMon:online','egammaMon:shifter','egammaMon:val','caloMon:t0']),
        ChainProp(name='HLT_g15_loose_ion_L1eEM15', stream=[HardProbesStream], groups=SinglePhotonGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_g20_loose_ion_L1eEM15', stream=[HardProbesStream], groups=SinglePhotonGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_g20_loose_ion_L1eEM18', stream=[HardProbesStream], groups=SinglePhotonGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_g30_loose_ion_L1eEM18',  stream=[HardProbesStream], groups=SinglePhotonGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_g50_loose_ion_L1eEM26',  stream=[HardProbesStream, 'express'], groups=SinglePhotonGroup+PrimaryPhIGroup,  monGroups=['egammaMon:online','egammaMon:shifter','egammaMon:val','caloMon:t0']),
        ChainProp(name='HLT_2g15_loose_ion_L12eEM12',stream=[HardProbesStream], groups=SinglePhotonGroup+PrimaryPhIGroup),

        # UPC photon chains
        #phase-1
        ChainProp(name='HLT_g10_loose_L1eEM9_VTE200',  stream=[UPCStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_g10_medium_L1eEM9_VTE200',  stream=[UPCStream], groups=SingleElectronGroup+PrimaryPhIGroup),

    ]

    chains['Jet'] = [
        # HI jets
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

        #--- phase-1 HI jets
        ChainProp(name='HLT_j60_ion_L1jJ40', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j75_ion_L1jJ50', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j75_ion_L1jJ60', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j85_ion_L1jJ40', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j85_ion_L1jJ60', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j100_ion_L1jJ60', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j120_ion_L1jJ60', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j150_ion_L1jJ90', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup),

        ChainProp(name='HLT_j50f_ion_L1jJ40p30ETA49', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream, 'express'], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j60f_ion_L1jJ40p30ETA49', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j70f_ion_L1jJ60p30ETA49', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup),

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

        #----------- mu+j PrimaryLeg
        ChainProp(name='HLT_mu4_j50_ion_dRAB05_L1MU3V',     l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream, 'express'], groups=PrimaryLegGroup+SingleBjetGroup, monGroups=['bJetMon:t0','muonMon:online','bJetMon:online']),
        ChainProp(name='HLT_mu4_j50_ion_dRAB05_L1MU3V_J12', l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=PrimaryLegGroup+SingleBjetGroup, monGroups=['muonMon:online','bJetMon:online']),
        ChainProp(name='HLT_mu4_j60_ion_dRAB05_L1MU3V',     l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=PrimaryLegGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu6_j40_ion_dRAB05_L1MU5VF',     l1SeedThresholds=['MU5VF','FSNOSEED'], stream=[HardProbesStream, 'express'], groups=PrimaryLegGroup+SingleBjetGroup, monGroups=['bJetMon:t0','muonMon:online','bJetMon:online']),
        ChainProp(name='HLT_mu6_j50_ion_dRAB05_L1MU5VF',     l1SeedThresholds=['MU5VF','FSNOSEED'], stream=[HardProbesStream], groups=PrimaryLegGroup+SingleBjetGroup),
        #----------- mu+j SupportLeg
        ChainProp(name='HLT_mu4_j20_L1MU3V_J12',            l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=SupportLegGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu4_j40_ion_dRAB05_L1MU3V',     l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=SupportLegGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu6_j30_ion_dRAB05_L1MU5VF',     l1SeedThresholds=['MU5VF','FSNOSEED'], stream=[HardProbesStream], groups=SupportLegGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu4_j40_ion_L1MU3V',     l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=SupportLegGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu4_j50_ion_L1MU3V',     l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=SupportLegGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu4_j60_ion_L1MU3V',     l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=SupportLegGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu6_j30_ion_L1MU3V',     l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=SupportLegGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu6_j40_ion_L1MU3V',     l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=SupportLegGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu6_j50_ion_L1MU3V',     l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=SupportLegGroup+SingleBjetGroup),

        #----------- mu+j with new calo
        ChainProp(name='HLT_mu4_j60_ion_dRAB05_L1MU3V_jJ40', l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=PrimaryPhIGroup+SingleBjetGroup),

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

        #----------- UPC HMT - phase-1
        #supporting
        ChainProp(name='HLT_mb_sp50_trk15_hmt_hi_FgapA5_L1MBTS_1_VZDC_A_ZDC_C_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp50_trk15_hmt_hi_FgapA5_L1MBTS_1_1ZDC_A_1ZDC_C_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp50_trk15_hmt_hi_FgapA5_L1MBTS_1_ZDC_1XOR5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp50_trk15_hmt_hi_FgapC5_L1MBTS_1_ZDC_A_VZDC_C_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp50_trk15_hmt_hi_FgapC5_L1MBTS_1_1ZDC_A_1ZDC_C_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp50_trk15_hmt_hi_FgapC5_L1MBTS_1_ZDC_1XOR5_VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),

        ChainProp(name='HLT_mb_sp400_trk25_hmt_hi_FgapA5_L1VZDC_A_ZDC_C_jTE5_VjTE200_GAP_A', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp400_trk25_hmt_hi_FgapA5_L11ZDC_A_1ZDC_C_jTE5_VjTE200_GAP_A', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp400_trk25_hmt_hi_FgapA5_L1ZDC_1XOR5_jTE5_VjTE200_GAP_A', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp400_trk25_hmt_hi_FgapC5_L1ZDC_A_VZDC_C_jTE5_VjTE200_GAP_C', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp400_trk25_hmt_hi_FgapC5_L11ZDC_A_1ZDC_C_jTE5_VjTE200_GAP_C', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp400_trk25_hmt_hi_FgapC5_L1ZDC_1XOR5_jTE5_VjTE200_GAP_C', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),

        ChainProp(name='HLT_mb_sp700_trk35_hmt_hi_FgapA5_L1VZDC_A_ZDC_C_jTE5_VjTE200_GAP_A', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp700_trk35_hmt_hi_FgapA5_L11ZDC_A_1ZDC_C_jTE5_VjTE200_GAP_A', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp700_trk35_hmt_hi_FgapA5_L1ZDC_1XOR5_jTE5_VjTE200_GAP_A', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp700_trk35_hmt_hi_FgapC5_L1ZDC_A_VZDC_C_jTE5_VjTE200_GAP_C', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp700_trk35_hmt_hi_FgapC5_L11ZDC_A_1ZDC_C_jTE5_VjTE200_GAP_C', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp700_trk35_hmt_hi_FgapC5_L1ZDC_1XOR5_jTE5_VjTE200_GAP_C', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),

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

        #----------- UPC diphotons/dielectrons - phase-1
        #primary
        ChainProp(name='HLT_mb_sp_vpix15_hi_FgapAC5_L1eEM1_TE4_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp_vpix15_hi_FgapAC5_L1eEM2_TE4_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp_vpix15_hi_FgapAC5_L12eEM1_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp_vpix15_hi_FgapAC5_L12eEM2_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp_vpix15_hi_FgapAC5_L1eEM5_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L1eEM1_TE4_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream, 'express'],groups=MinBiasGroup+PrimaryPhIGroup+['PS:NoHLTRepro'], monGroups=['mbMon:shifter']),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L1eEM2_TE4_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream, 'express'],groups=MinBiasGroup+PrimaryPhIGroup+['PS:NoHLTRepro'], monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L12eEM1_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L12eEM2_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L12eEM1_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream, 'express'],groups=MinBiasGroup+PrimaryPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L12eEM2_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L1eEM5_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L12eEM1_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L12eEM2_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L12eEM1_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L12eEM2_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1eEM1_TE4_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1eEM2_TE4_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1eEM5_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup+['PS:NoHLTRepro']),

        #supporting
        ChainProp(name='HLT_mb_sp_vpix15_hi_FgapAC5_L12eEM1_VjTE200_GAP_AANDC', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L12eEM1_VjTE200_GAP_AANDC', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L12eEM1_VjTE200_GAP_AANDC', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp_vpix15_hi_FgapAC5_L1eEM1_TE4_VTE200_EMPTY', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L1eEM1_TE4_VTE200_EMPTY', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L12eEM1_VjTE200_EMPTY', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L12eEM1_VjTE200_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC5_L12eEM1_VjTE200_UNPAIRED_NONISO', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1eEM1_TE4_VTE200_EMPTY', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L11ZDC_A_1ZDC_C_VjTE200_GAP_AANDC', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1VZDC_A_VZDC_C_VjTE200_GAP_AANDC', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1ZDC_XOR4_VjTE200_GAP_AANDC', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sptrk_hi_FgapAC5_L12eEM1_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_sptrk_hi_FgapAC5_L1eEM5_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),

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

        #----------- UPC ditaus - Phase-1
        #primary
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1eEM1_VZDC_A_VZDC_C_VjTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1eEM1_ZDC_XOR4_VjTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt2_hi_FgapAC5_L1eEM1_VZDC_A_VZDC_C_VjTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt2_hi_FgapAC5_L1eEM1_ZDC_XOR4_VjTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1eEM2_VZDC_A_VZDC_C_VjTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1eEM2_ZDC_XOR4_VjTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt2_hi_FgapAC5_L1eEM2_VZDC_A_VZDC_C_VjTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt2_hi_FgapAC5_L1eEM2_ZDC_XOR4_VjTE100', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryPhIGroup),

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

        #----------- UPC exclusive egamma - phase-1
        #supporting
        #w/o ZDC
        ChainProp(name='HLT_mb_sptrk_L12eEM1_VTE200', l1SeedThresholds=['FSNOSEED'],stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_L12eEM1_VTE200', l1SeedThresholds=['FSNOSEED'],stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_L1eEM1_TE4_VTE200', l1SeedThresholds=['FSNOSEED'],stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro'], monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_L1eEM2_TE4_VTE200', l1SeedThresholds=['FSNOSEED'],stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro'], monGroups=['mbMon:t0']),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_L1eEM1_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),
        ChainProp(name='HLT_mb_excl_1trk5_pt2_L1eEM1_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup+['PS:NoHLTRepro']),

        #OVERLAY
        ChainProp(name='HLT_mb_sptrk_L1ZDC_A_C_VTE50_OVERLAY', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasOverlayStream], groups=MinBiasGroup+SupportLegGroup),
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
        ChainProp(name='HLT_noalg_L1J12',   l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=['PS:NoBulkMCProd']+SupportLegGroup),

        # Streamers for monitoring TRT fast-OR
        ChainProp(name='HLT_noalg_L1TRT_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup),
        ChainProp(name='HLT_noalg_L1TRT_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:NoBulkMCProd']+MinBiasGroup),

        ChainProp(name='HLT_noalg_L12TAU1_VTE200',    l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TAU1_TE4_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TAU1_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TAU2_TE4_VTE200',     l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TAU8_VTE200',     l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1ZDC_XOR_VTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1eEM1_VjTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1eEM2_VjTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1eEM5_VjTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1VZDC_A_VZDC_C_TE5_VTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1ZDC_XOR_TE5_VTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L11ZDC_NZDC_TE5_VTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_noalg_L1J15',   l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1J30',   l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),

        ChainProp(name='HLT_noalg_L1jJ40',   l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jJ50',   l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jJ60',   l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_noalg_L1EM10',  l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream, 'express'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1EM12',  l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream, 'express'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup,  monGroups=['egammaMon:online','egammaMon:shifter']),
        ChainProp(name='HLT_noalg_L1EM15',  l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),

        ChainProp(name='HLT_noalg_L1eEM5',  l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1eEM9',  l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1eEM12',  l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1eEM15',  l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream, 'express'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup,  monGroups=['egammaMon:online','egammaMon:shifter']),
        ChainProp(name='HLT_noalg_L1eEM18',  l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L12MU3V',  l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportGroup),

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

        ChainProp(name='HLT_noalg_L1jTE200',   l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),

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
        ChainProp(name='HLT_noalg_L1gTE200',        l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream]   , groups=['PS:NoBulkMCProd']+SupportPhIGroup+METPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1gXEJWOJ100',    l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=['PS:NoBulkMCProd']+SupportPhIGroup+METPhaseIStreamersGroup, monGroups=['metMon:t0']),
    ]

    #---- heavy ion EB chains
    chains['EnhancedBias'] += [
        ChainProp(name='HLT_noalg_eb_L1MU3V',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1EM12',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1EM15',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L12EM15',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1eEM15',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1eEM18',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1eEM26',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L12eEM18',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1TE50',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1TE8000p0ETA49',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),

        ChainProp(name='HLT_noalg_eb_L1MBTS_1_1',     l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1TE5_VTE200',   l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1TE50_VTE200',  l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1MU3V_VTE50',   l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1RD1_FILLED',   l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1TAU1_TE4_VTE200',  l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1eEM1_TE4_VTE200',  l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1eEM2_TE4_VTE200',  l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L12TAU1_VTE200',  l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L12eEM1_VTE200',  l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L12eEM2_VTE200',  l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1ZDC_A_C_VTE50',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1ZDC_XOR_VTE10',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1ZDC_XOR_TE5_VTE200',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1VZDC_A_VZDC_C_VTE50',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1VZDC_A_VZDC_C_TE5_VTE200',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1ZDC_A_VTE200',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1ZDC_C_VTE200',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1TRT_ZDC_A_VTE50',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1TRT_ZDC_C_VTE50',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1TAU1_TRT_VTE50',  l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1VTE200',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:NoBulkMCProd', "PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),

    ]

    chains['Monitor'] = [
          ChainProp(name='HLT_noalg_CostMonDS_L1All',        l1SeedThresholds=['FSNOSEED'], stream=['CostMonitoring'], groups=['PS:NoBulkMCProd']+['PS:NoBulkMCProd','RATE:Monitoring','BW:Other']),
    ]
    return chains


def setupMenu(menu_name):

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
