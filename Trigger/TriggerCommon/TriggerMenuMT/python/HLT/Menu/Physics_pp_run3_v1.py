# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# Physics_pp_run3_v1.py menu -- contains physics chains for MC and data
#------------------------------------------------------------------------#

# All chains are represented as ChainProp objects in a ChainStore
from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp
from .SignatureDicts import ChainStore

PhysicsStream='Main'
SingleMuonGroup = ['RATE:SingleMuon', 'BW:Muon']
MultiMuonGroup = ['RATE:MultiMuon', 'BW:Muon']
SingleElectronGroup = ['RATE:SingleElectron', 'BW:Electron']
MultiElectronGroup = ['RATE:MultiElectron', 'BW:Electron']
SinglePhotonGroup = ['RATE:SinglePhoton', 'BW:Photon']
MultiPhotonGroup = ['RATE:MultiPhoton', 'BW:Photon']
METGroup = ['RATE:MET', 'BW:MET']
SingleJetGroup = ['RATE:SingleJet', 'BW:Jet']
MultiJetGroup = ['RATE:MultiJet', 'BW:Jet']
SingleBjetGroup = ['RATE:SingleBJet', 'BW:BJet']
MultiBjetGroup  = ['RATE:MultiBJet',  'BW:BJet']
EgammaBjetGroup = ['RATE:EgammaBjet', 'BW:BJet']
MuonBjetGroup = ['RATE:MuonBjet', 'BW:BJet']
BjetMETGroup = ['RATE:BjetMET', 'BW:BJet']
SingleTauGroup = ['RATE:SingleTau', 'BW:Tau']
DiTauGroup = ['RATE:Ditau', 'BW:Tau']
MultiTauGroup = ['RATE:MultiTau', 'BW:Tau']
BphysicsGroup = ['RATE:Bphysics', 'BW:Bphysics']
BphysElectronGroup = ['RATE:BphysicsElectron', 'BW:BphysicsElectron']
EgammaMuonGroup = ['RATE:EgammaMuon', 'BW:Egamma', 'BW:Muon']
EgammaMETGroup = ['RATE:EgammaMET', 'BW:Egamma', 'BW:MET']
MuonJetGroup =['RATE:MuonJet','BW:Muon', 'BW:Jet']
TauMETGroup =['RATE:TauMET', 'BW:Tau']
TauJetGroup =['RATE:TauJet', 'BW:Tau']
TauBJetGroup =['RATE:TauBJet', 'BW:Tau']
TauPhotonGroup =['RATE:TauPhoton', 'BW:Tau']
DiTauPhotonGroup =['RATE:DiTauPhoton', 'BW:Tau']
MuonMETGroup =['RATE:MuonMET', 'BW:Muon']
EgammaJetGroup = ['RATE:EgammaJet', 'BW:Egamma']
EgammaTauGroup =['RATE:EgammaTau', 'BW:Egamma', 'BW:Tau']
MuonTauGroup =['RATE:MuonTau', 'BW:Muon', 'BW:Tau']
JetMETGroup = ['RATE:JetMET', 'BW:Jet']
MinBiasGroup = ['RATE:MinBias', 'BW:MinBias']
ZeroBiasGroup = ['RATE:ZeroBias', 'BW:ZeroBias']
MuonXStreamersGroup = ['RATE:SeededStreamers', 'BW:Muon']
MuonXPhaseIStreamersGroup = ['RATE:PhaseISeededStreamers', 'BW:Muon']
EgammaStreamersGroup = ['RATE:SeededStreamers', 'BW:Egamma']
EgammaPhaseIStreamersGroup = ['RATE:PhaseISeededStreamers', 'BW:Egamma']
TauStreamersGroup = ['RATE:SeededStreamers', 'BW:Tau']
TauPhaseIStreamersGroup = ['RATE:PhaseISeededStreamers', 'BW:Tau']
JetStreamersGroup = ['RATE:SeededStreamers', 'BW:Jet']
JetPhaseIStreamersGroup = ['RATE:PhaseISeededStreamers', 'BW:Jet']
METStreamersGroup = ['RATE:SeededStreamers', 'BW:MET']
METPhaseIStreamersGroup = ['RATE:PhaseISeededStreamers', 'BW:MET']
# For chains seeded by L1 muon (no calo items)
PrimaryL1MuGroup = ['Primary:L1Muon']
# For chains containing a legacy L1 calo / topo item
PrimaryLegGroup = ['Primary:Legacy']
# For chains containing a phase 1 calo / topo item
PrimaryPhIGroup = ['Primary:PhaseI']
SupportGroup = ['Support']
SupportLegGroup = ['Support:Legacy']
SupportPhIGroup = ['Support:PhaseI']
# For the chains with the TAgAndProbe labels, we flag the rate group as being that of the tag leg and NOT the full chain selection
TagAndProbeGroup = ['Support:TagAndProbe']
TagAndProbeLegGroup = ['Support:LegacyTagAndProbe']
TagAndProbePhIGroup = ['Support:PhaseITagAndProbe']
EOFL1MuGroup = ['EOF:L1Muon']
EOFEgammaLegGroup = ['EOF:EgammaLegacy']
EOFEgammaPhIGroup = ['EOF:EgammaPhaseI']
EOFBPhysL1MuGroup = ['EOF:BPhysL1Muon']
EOFBeePhIGroup = ['EOF:BeePhaseI']
EOFTLALegGroup = ['EOF:TLALegacy']
EOFTLAPhIGroup = ['EOF:TLAPhaseI']
EOFTauPhIGroup = ['EOF:TauPhaseI']
EOFMultiBjetGroup = ['EOF:MultiBjetPhaseI']
EOFTauBjetGroup = ['EOF:TauBjetPhaseI']

# For unconventional tracking chains (ATR-23797)
UnconvTrkGroup = ['RATE:UnconvTrk', 'BW:UnconvTrk'] 
LowMuGroup = ['LowMu']
# Topo boards
Topo2Group = ['Topo2']
Topo3Group = ['Topo3']
LegacyTopoGroup = ['LegacyTopo']

def setupMenu():

    from AthenaCommon.Logging import logging
    log = logging.getLogger( __name__ )
    log.info('[setupMenu] going to add the Physics menu chains now')

    chains = ChainStore()
    chains['Muon'] += [
        #ATR-19985 and ATR-24367
        ChainProp(name='HLT_mu6_mu6noL1_L1MU5VF', l1SeedThresholds=['MU5VF','FSNOSEED'], groups=SupportGroup+MultiMuonGroup+['RATE:CPS_MU5VF'], stream=[PhysicsStream,'express'], monGroups=['muonMon:shifter']),

        #ATR-31517
        ChainProp(name='HLT_mu24_idReuse_ivarmedium_L1MU14FCH', groups=SupportGroup+SingleMuonGroup, stream=[PhysicsStream], monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu24_idReuse_ivarmedium_L1MU18VFCH', groups=SupportGroup+SingleMuonGroup, stream=[PhysicsStream], monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu50_idReuse_L1MU14FCH', groups=SupportGroup+SingleMuonGroup, monGroups=['muonMon:online','muonMon:shifter']),
        ChainProp(name='HLT_mu50_idReuse_L1MU18VFCH', groups=SupportGroup+SingleMuonGroup, monGroups=['muonMon:online','muonMon:shifter']),
        ChainProp(name='HLT_mu4_l2io_idReuse_L1MU3V', stream=[PhysicsStream], groups=SupportGroup+SingleMuonGroup+['RATE:CPS_MU3V'], monGroups=['muonMon:shifter']),
        ChainProp(name='HLT_mu10_l2mt_idReuse_L1MU10BOM', groups=SupportGroup+SingleMuonGroup),
        ChainProp(name='HLT_2mu14_idReuse_L12MU8F', groups=SupportGroup+MultiMuonGroup, stream=[PhysicsStream], monGroups=['muonMon:online','muonMon:shifter']),
        
        #ATR-20049
        ChainProp(name='HLT_2mu6_L12MU5VF',     l1SeedThresholds=['MU5VF'],   groups=SupportGroup+MultiMuonGroup+['RATE:CPS_2MU5VF']),

        #Planned Primaries
        #-- 1 mu iso
        ChainProp(name='HLT_mu24_ivarmedium_L1MU14FCH', groups=PrimaryL1MuGroup+SingleMuonGroup, stream=[PhysicsStream,'express'], monGroups=['muonMon:shifter','muonMon:online','detMon']),
        ChainProp(name='HLT_mu26_ivarmedium_L1MU14FCH', groups=PrimaryL1MuGroup+SingleMuonGroup, stream=[PhysicsStream,'express'], monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu28_ivarmedium_L1MU14FCH', groups=PrimaryL1MuGroup+SingleMuonGroup),
        # ATR-25512
        ChainProp(name='HLT_mu24_ivarmedium_L1MU18VFCH', groups=PrimaryL1MuGroup+SingleMuonGroup, stream=[PhysicsStream,'express'], monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu26_ivarmedium_L1MU18VFCH', groups=PrimaryL1MuGroup+SingleMuonGroup, stream=[PhysicsStream,'express'], monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu28_ivarmedium_L1MU18VFCH', groups=PrimaryL1MuGroup+SingleMuonGroup),
        
        #-- 1 mu
        ChainProp(name='HLT_mu50_L1MU14FCH', groups=PrimaryL1MuGroup+SingleMuonGroup, monGroups=['muonMon:online','muonMon:shifter']),
        ChainProp(name='HLT_mu60_0eta105_msonly_L1MU14FCH', groups=PrimaryL1MuGroup+SingleMuonGroup, monGroups=['muonMon:shifter']),
        ChainProp(name='HLT_mu60_L1MU14FCH', groups=PrimaryL1MuGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu80_L1MU14FCH', groups=PrimaryL1MuGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu80_msonly_3layersEC_L1MU14FCH', groups=PrimaryL1MuGroup+SingleMuonGroup),
        # ATR-25512
        ChainProp(name='HLT_mu50_L1MU18VFCH', groups=PrimaryL1MuGroup+SingleMuonGroup, monGroups=['muonMon:online','muonMon:shifter']),
        ChainProp(name='HLT_mu60_0eta105_msonly_L1MU18VFCH', groups=PrimaryL1MuGroup+SingleMuonGroup, monGroups=['muonMon:shifter']),
        ChainProp(name='HLT_mu60_L1MU18VFCH', groups=PrimaryL1MuGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu80_L1MU18VFCH', groups=PrimaryL1MuGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu80_msonly_3layersEC_L1MU18VFCH', groups=PrimaryL1MuGroup+SingleMuonGroup),

        #-- 2 mu
        ChainProp(name='HLT_mu22_mu8noL1_L1MU14FCH',  l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryL1MuGroup+MultiMuonGroup, stream=[PhysicsStream,'express'], monGroups=['muonMon:online','muonMon:shifter','detMon']),
        ChainProp(name='HLT_mu22_mu10noL1_L1MU14FCH', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryL1MuGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu24_mu8noL1_L1MU14FCH',  l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryL1MuGroup+MultiMuonGroup),
        # ATR-25512
        ChainProp(name='HLT_mu23_mu8noL1_L1MU18VFCH',  l1SeedThresholds=['MU18VFCH','FSNOSEED'], groups=PrimaryL1MuGroup+MultiMuonGroup, stream=[PhysicsStream,'express'], monGroups=['muonMon:online','muonMon:shifter']),
        ChainProp(name='HLT_mu23_mu10noL1_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','FSNOSEED'], groups=PrimaryL1MuGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu24_mu8noL1_L1MU18VFCH',  l1SeedThresholds=['MU18VFCH','FSNOSEED'], groups=PrimaryL1MuGroup+MultiMuonGroup),

        ChainProp(name='HLT_2mu14_L12MU8F', groups=PrimaryL1MuGroup+MultiMuonGroup, stream=[PhysicsStream,'express'], monGroups=['muonMon:online','muonMon:shifter']),
        ChainProp(name='HLT_2mu15_L12MU8F', groups=PrimaryL1MuGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_ivarmedium_mu8noL1_L1MU14FCH', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryL1MuGroup+MultiMuonGroup,monGroups=['detMon']),
        # ATR-25512
        ChainProp(name='HLT_mu23_ivarmedium_mu8noL1_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','FSNOSEED'], groups=PrimaryL1MuGroup+MultiMuonGroup),
        #ATR-22107
        ChainProp(name='HLT_mu20_ivarmedium_mu4noL1_10invmAB70_L1MU14FCH', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryL1MuGroup+MultiMuonGroup),
        # ATR-25512
        ChainProp(name='HLT_mu23_ivarmedium_mu4noL1_10invmAB70_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','FSNOSEED'], groups=PrimaryL1MuGroup+MultiMuonGroup),

        #-- 2 mu iso invm
        ChainProp(name='HLT_mu10_ivarmedium_mu10_10invmAB70_L12MU8F', groups=PrimaryL1MuGroup+MultiMuonGroup),
        #-- 3 mu
        ChainProp(name='HLT_mu20_2mu4noL1_L1MU14FCH', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryL1MuGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu22_2mu4noL1_L1MU14FCH', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryL1MuGroup+MultiMuonGroup),
        #ATR-25512
        ChainProp(name='HLT_mu23_2mu4noL1_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','FSNOSEED'], groups=PrimaryL1MuGroup+MultiMuonGroup),
        
        ChainProp(name='HLT_3mu6_L13MU5VF', l1SeedThresholds=['MU5VF'],   groups=PrimaryL1MuGroup+MultiMuonGroup, monGroups=['muonMon:online']),
        ChainProp(name='HLT_3mu6_msonly_L13MU5VF', l1SeedThresholds=['MU5VF'],   groups=PrimaryL1MuGroup+MultiMuonGroup, monGroups=['muonMon:online']),
        ChainProp(name='HLT_3mu8_msonly_L13MU5VF', groups=PrimaryL1MuGroup+MultiMuonGroup),
        #-- 4 mu
        ChainProp(name='HLT_4mu4_L14MU3V', l1SeedThresholds=['MU3V'], groups=PrimaryL1MuGroup+MultiMuonGroup, monGroups=['muonMon:online']),

        # -- LRT mu
        ChainProp(name='HLT_mu6_LRT_idperf_L1MU5VF',      groups=SupportGroup+SingleMuonGroup+['RATE:CPS_MU5VF'], monGroups=['idMon:t0']),
        ChainProp(name='HLT_mu20_LRT_d0loose_L1MU14FCH',  groups=PrimaryL1MuGroup+SingleMuonGroup, monGroups=['muonMon:online']),
        ChainProp(name='HLT_mu20_LRT_d0tight_L1MU14FCH',  groups=PrimaryL1MuGroup+SingleMuonGroup), #back-up
        # ATR-25512
        ChainProp(name='HLT_mu23_LRT_d0loose_L1MU18VFCH',  groups=PrimaryL1MuGroup+SingleMuonGroup, monGroups=['muonMon:online']),
        ChainProp(name='HLT_mu23_LRT_d0tight_L1MU18VFCH',  groups=PrimaryL1MuGroup+SingleMuonGroup), #back-up

        # -- LLP mu RoI Cluster Trigger (ATR-22697)
        ChainProp(name='HLT_mu3vtx_L12MU8F', l1SeedThresholds=['MU8F'], groups=PrimaryL1MuGroup+MultiMuonGroup),

        # ATR-20505
        ChainProp(name='HLT_2mu50_msonly_L1MU14FCH', groups=PrimaryL1MuGroup+MultiMuonGroup),
        # ATR-25512
        ChainProp(name='HLT_2mu50_msonly_L1MU18VFCH', groups=PrimaryL1MuGroup+MultiMuonGroup),

        # Close-by muons (ATR-22537, ATR-22243)
        ChainProp(name='HLT_2mu10_l2mt_L1MU10BOM', groups=MultiMuonGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_2mu10_l2mt_L1MU12BOM', groups=MultiMuonGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_2mu10_l2mt_L1MU10BO', groups=MultiMuonGroup+SupportGroup+['RATE:CPS_MU10BO']),
        ChainProp(name='HLT_2mu4_l2mt_L1MU4BOM', groups=MultiMuonGroup+SupportGroup),

        # Late muons (ATR-26284)
        ChainProp(name='HLT_mu10_lateMu_L1LATE-MU8F_jJ90', l1SeedThresholds=['FSNOSEED'], groups=Topo2Group+SingleMuonGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mu10_lateMu_L1LATE-MU8F_jXE70', l1SeedThresholds=['FSNOSEED'], groups=Topo2Group+SingleMuonGroup+PrimaryPhIGroup),

        # Bell measurement at low-mu run (ATR-23494)
        ChainProp(name='HLT_2mu3_L12MU3V',  groups=MultiMuonGroup+SupportGroup+['RATE:CPS_2MU3V']),
        ChainProp(name='HLT_2mu3_L12MU3VF', groups=MultiMuonGroup+SupportGroup+['RATE:CPS_2MU3VF']),

        # Late stream for LLP
        ChainProp(name='HLT_3mu6_msonly_L12MU5VF_EMPTY', l1SeedThresholds=['MU5VF'], stream=['Late'], groups=PrimaryL1MuGroup+MultiMuonGroup),
        ChainProp(name='HLT_3mu6_msonly_L1MU3V_UNPAIRED_ISO', l1SeedThresholds=['MU3V'], stream=['Late'], groups=PrimaryL1MuGroup+MultiMuonGroup),

        # Support chains
        ChainProp(name='HLT_mu6_idperf_L1MU5VF', stream=[PhysicsStream,'express'], groups=SupportGroup+SingleMuonGroup+['RATE:CPS_MU5VF'], monGroups=['idMon:t0']),
        ChainProp(name='HLT_mu6_msonly_L1MU5VF', groups=SupportGroup+SingleMuonGroup+['RATE:CPS_MU5VF'], monGroups=['muonMon:shifter']),
        ChainProp(name='HLT_mu20_msonly_L1MU14FCH', groups=SupportGroup+SingleMuonGroup+['RATE:CPS_MU14FCH']),
        ChainProp(name='HLT_mu26_L1MU14FCH', groups=SupportGroup+SingleMuonGroup+['RATE:CPS_MU14FCH']),
        ChainProp(name='HLT_mu24_idperf_L1MU14FCH', stream=[PhysicsStream,'express'], groups=SupportGroup+SingleMuonGroup+['RATE:CPS_MU14FCH'], monGroups=['idMon:shifter']),
        ChainProp(name='HLT_mu20_LRT_idperf_L1MU14FCH', stream=[PhysicsStream,'express'],   groups=SupportGroup+SingleMuonGroup+['RATE:CPS_MU14FCH'], monGroups=['idMon:shifter']),
        ChainProp(name='HLT_mu26_ivarperf_L1MU14FCH', stream=[PhysicsStream,'express'], groups=SupportGroup+SingleMuonGroup+['RATE:CPS_MU14FCH'], monGroups=['idMon:shifter']), # ATR-21905
        ChainProp(name='HLT_mu40_idperf_L1MU14FCH', stream=[PhysicsStream,'express'], groups=SupportGroup+SingleMuonGroup+['RATE:CPS_MU14FCH'],monGroups=['idMon:t0']),

        # ATR-25512
        ChainProp(name='HLT_mu23_msonly_L1MU18VFCH', groups=SupportGroup+SingleMuonGroup+['RATE:CPS_MU18VFCH']),
        ChainProp(name='HLT_mu26_L1MU18VFCH', groups=SupportGroup+SingleMuonGroup+['RATE:CPS_MU18VFCH']),
        ChainProp(name='HLT_mu24_idperf_L1MU18VFCH', stream=[PhysicsStream,'express'], groups=SupportGroup+SingleMuonGroup+['RATE:CPS_MU18VFCH'], monGroups=['idMon:shifter']),
        ChainProp(name='HLT_mu23_LRT_idperf_L1MU18VFCH', stream=[PhysicsStream,'express'],   groups=SupportGroup+SingleMuonGroup+['RATE:CPS_MU18VFCH'], monGroups=['idMon:shifter']),
        ChainProp(name='HLT_mu26_ivarperf_L1MU18VFCH', stream=[PhysicsStream,'express'], groups=SupportGroup+SingleMuonGroup+['RATE:CPS_MU18VFCH'], monGroups=['idMon:shifter']), # ATR-21905
        ChainProp(name='HLT_mu40_idperf_L1MU18VFCH', stream=[PhysicsStream,'express'], groups=SupportGroup+SingleMuonGroup+['RATE:CPS_MU18VFCH'], monGroups=['muonMon:shifter']),


        # Support and ES, ATR-24367
        ChainProp(name='HLT_mu22_L1MU14FCH', stream=[PhysicsStream,'express'], groups=SupportGroup+SingleMuonGroup+['RATE:CPS_MU14FCH'], monGroups=['muonMon:shifter']),
        ChainProp(name='HLT_mu24_ivarperf_L1MU14FCH', stream=[PhysicsStream,'express'], groups=SupportGroup+SingleMuonGroup+['RATE:CPS_MU14FCH'], monGroups=['muonMon:shifter']),

        # ATR-25512
        ChainProp(name='HLT_mu23_L1MU18VFCH', stream=[PhysicsStream,'express'], groups=SupportGroup+SingleMuonGroup+['RATE:CPS_MU18VFCH'], monGroups=['muonMon:shifter']),
        ChainProp(name='HLT_mu24_ivarperf_L1MU18VFCH', stream=[PhysicsStream,'express'], groups=SupportGroup+SingleMuonGroup+['RATE:CPS_MU18VFCH'], monGroups=['muonMon:shifter']),

        # ATR-27264
        ChainProp(name='HLT_mu24_L1MU14FCH', stream=[PhysicsStream,'express'], groups=SupportGroup+SingleMuonGroup+['RATE:CPS_MU14FCH'], monGroups=['muonMon:shifter']),
        ChainProp(name='HLT_mu24_L1MU18VFCH', stream=[PhysicsStream,'express'], groups=SupportGroup+SingleMuonGroup+['RATE:CPS_MU18VFCH'], monGroups=['muonMon:shifter']),


        # Support for l2io and l2mt, ATR-24844
        ChainProp(name='HLT_mu4_L1MU3V', stream=[PhysicsStream,'express'], groups=SupportGroup+SingleMuonGroup+['RATE:CPS_MU3V'], monGroups=['muonMon:shifter']),
        ChainProp(name='HLT_mu4_l2io_L1MU3V', stream=[PhysicsStream,'express'], groups=SupportGroup+SingleMuonGroup+['RATE:CPS_MU3V'], monGroups=['muonMon:shifter']),
        ChainProp(name='HLT_mu10_l2mt_L1MU10BO', groups=SupportGroup+SingleMuonGroup+['RATE:CPS_MU10BO']),
        ChainProp(name='HLT_mu10_l2mt_L1MU10BOM', groups=SupportGroup+SingleMuonGroup),
        #
        ChainProp(name='HLT_mu26_ivarmedium_mu6_l2io_probe_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEMU5VF'], groups=TagAndProbeGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_mu6_l2mt_probe_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEMU5VF'], groups=TagAndProbeGroup+SingleMuonGroup),
        # ATR 25512
        ChainProp(name='HLT_mu26_ivarmedium_mu6_l2io_probe_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEMU5VF'], groups=TagAndProbeGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_mu6_l2mt_probe_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEMU5VF'], groups=TagAndProbeGroup+SingleMuonGroup),
        # ATR-25512 Duplicate tag and probe mu26_ivarmedium -> mu24_ivarmedium
        ChainProp(name='HLT_mu24_ivarmedium_mu6_l2io_probe_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEMU5VF'], groups=TagAndProbeGroup+SingleMuonGroup, monGroups=['muonMon:shifter']),
        ChainProp(name='HLT_mu24_ivarmedium_mu6_l2mt_probe_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEMU5VF'], groups=TagAndProbeGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_mu6_l2io_probe_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEMU5VF'], groups=TagAndProbeGroup+SingleMuonGroup, monGroups=['muonMon:shifter']),
        ChainProp(name='HLT_mu24_ivarmedium_mu6_l2mt_probe_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEMU5VF'], groups=TagAndProbeGroup+SingleMuonGroup),
        # JPsi tag-and-probe
        # ATR-23614
        ChainProp(name='HLT_mu20_mu2noL1_invmJPsiOS_L1MU14FCH',  l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=SupportGroup+MultiMuonGroup+['RATE:CPS_MU14FCH']),
        ChainProp(name='HLT_mu23_mu2noL1_invmJPsiOS_L1MU18VFCH',  l1SeedThresholds=['MU18VFCH','FSNOSEED'], groups=SupportGroup+MultiMuonGroup+['RATE:CPS_MU18VFCH']),

        ## ATR-24198
        ChainProp(name='HLT_mu26_ivarmedium_mu4_probe_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEMU3V'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu26_ivarmedium_mu6_probe_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEMU5VF'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu26_ivarmedium_mu14_probe_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEMU8F'], groups=SingleMuonGroup+TagAndProbeGroup),        
        ChainProp(name='HLT_mu26_ivarmedium_mu20_probe_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEMU14FCH'], groups=SingleMuonGroup+TagAndProbeGroup),        
        ChainProp(name='HLT_mu26_ivarmedium_mu22_probe_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEMU14FCH'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu26_ivarmedium_mu24_probe_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEMU14FCH'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu26_ivarmedium_mu4_probe_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEMU3V'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu26_ivarmedium_mu6_probe_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEMU5VF'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu26_ivarmedium_mu14_probe_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEMU8F'], groups=SingleMuonGroup+TagAndProbeGroup),        
        ChainProp(name='HLT_mu26_ivarmedium_mu20_probe_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEMU14FCH'], groups=SingleMuonGroup+TagAndProbeGroup),        
        ChainProp(name='HLT_mu26_ivarmedium_mu22_probe_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEMU14FCH'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu26_ivarmedium_mu24_probe_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEMU14FCH'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu26_ivarmedium_mu14_ivarloose_probe_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEMU8F'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu26_ivarmedium_mu10_ivarmedium_probe_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEMU8F'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu26_ivarmedium_mu20_ivarloose_probe_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEMU14FCH'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu26_ivarmedium_mu20_ivarmedium_probe_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEMU14FCH'], groups=SingleMuonGroup+TagAndProbeGroup),
        # ATR-25512
        ChainProp(name='HLT_mu26_ivarmedium_mu14_ivarloose_probe_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEMU8F'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu26_ivarmedium_mu10_ivarmedium_probe_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEMU8F'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu26_ivarmedium_mu20_ivarloose_probe_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEMU14FCH'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu26_ivarmedium_mu20_ivarmedium_probe_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEMU14FCH'], groups=SingleMuonGroup+TagAndProbeGroup),
        # ATR-25512 Duplicate tag and probe chains mu26_ivarmedium -> mu24_ivarmedium
        ChainProp(name='HLT_mu24_ivarmedium_mu4_probe_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEMU3V'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu24_ivarmedium_mu6_probe_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEMU5VF'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu24_ivarmedium_mu14_probe_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEMU8F'], groups=SingleMuonGroup+TagAndProbeGroup),        
        ChainProp(name='HLT_mu24_ivarmedium_mu20_probe_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEMU14FCH'], groups=SingleMuonGroup+TagAndProbeGroup),        
        ChainProp(name='HLT_mu24_ivarmedium_mu22_probe_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEMU14FCH'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu24_ivarmedium_mu24_probe_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEMU14FCH'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu24_ivarmedium_mu4_probe_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEMU3V'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu24_ivarmedium_mu6_probe_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEMU5VF'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu24_ivarmedium_mu14_probe_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEMU8F'], groups=SingleMuonGroup+TagAndProbeGroup),        
        ChainProp(name='HLT_mu24_ivarmedium_mu20_probe_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEMU14FCH'], groups=SingleMuonGroup+TagAndProbeGroup),        
        ChainProp(name='HLT_mu24_ivarmedium_mu22_probe_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEMU14FCH'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu24_ivarmedium_mu24_probe_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEMU14FCH'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu24_ivarmedium_mu14_ivarloose_probe_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEMU8F'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu24_ivarmedium_mu10_ivarmedium_probe_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEMU8F'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu24_ivarmedium_mu20_ivarloose_probe_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEMU14FCH'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu24_ivarmedium_mu20_ivarmedium_probe_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEMU14FCH'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu24_ivarmedium_mu14_ivarloose_probe_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEMU8F'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu24_ivarmedium_mu10_ivarmedium_probe_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEMU8F'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu24_ivarmedium_mu20_ivarloose_probe_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEMU14FCH'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu24_ivarmedium_mu20_ivarmedium_probe_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEMU14FCH'], groups=SingleMuonGroup+TagAndProbeGroup),
        ## msonlyProbe
        ChainProp(name='HLT_mu26_ivarmedium_mu6_msonly_probe_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEMU5VF'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu26_ivarmedium_mu8_msonly_probe_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEMU5VF'], groups=SingleMuonGroup+TagAndProbeGroup),
        # ATR-25512
        ChainProp(name='HLT_mu26_ivarmedium_mu6_msonly_probe_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEMU5VF'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu26_ivarmedium_mu8_msonly_probe_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEMU5VF'], groups=SingleMuonGroup+TagAndProbeGroup),
        # ATR-25512 Duplicate tag and probe chains mu26_ivarmedium -> mu24_ivarmedium
        ChainProp(name='HLT_mu24_ivarmedium_mu6_msonly_probe_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEMU5VF'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu24_ivarmedium_mu8_msonly_probe_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEMU5VF'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu24_ivarmedium_mu6_msonly_probe_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEMU5VF'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu24_ivarmedium_mu8_msonly_probe_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEMU5VF'], groups=SingleMuonGroup+TagAndProbeGroup),

        # ATR-24399, support chains for the measurement dimuon trigger efficiency (replacement for HLT_2mu4_bDimu_novtx_noos_L12MU3V)
        ChainProp(name='HLT_2mu4_l2io_invmDimu_L12MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=MultiMuonGroup+SupportGroup+['RATE:CPS_2MU3V']),
        ChainProp(name='HLT_2mu4_l2io_invmDimu_L12MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=MultiMuonGroup+SupportGroup+['RATE:CPS_2MU3VF']),
        ChainProp(name='HLT_2mu4_l2io_invmDimu_L1BPH-2M9-0DR15-2MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=MultiMuonGroup+SupportGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3V']+Topo2Group),
        ChainProp(name='HLT_2mu4_l2io_invmDimu_L1BPH-2M9-0DR15-2MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=MultiMuonGroup+SupportGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3VF']+Topo3Group),
        ChainProp(name='HLT_mu6_l2io_mu4_l2io_invmDimu_L1BPH-2M9-0DR15-MU5VFMU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=MultiMuonGroup+SupportGroup+['RATE:CPS_BPH-2M9-0DR15-MU5VFMU3V']+Topo2Group),
        ChainProp(name='HLT_mu6_l2io_mu4_l2io_invmDimu_L1BPH-2M9-0DR15-C-MU5VFMU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=MultiMuonGroup+SupportGroup+['RATE:CPS_BPH-2M9-0DR15-C-MU5VFMU3V']+Topo3Group),
        ChainProp(name='HLT_2mu6_l2io_invmDimu_L1BPH-2M9-2DR15-2MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=MultiMuonGroup+SupportGroup+['RATE:CPS_BPH-2M9-2DR15-2MU5VF']+Topo2Group),
        ChainProp(name='HLT_2mu6_l2io_invmDimu_L1LFV-MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=MultiMuonGroup+SupportGroup+['RATE:CPS_LFV-MU5VF']+Topo2Group),
        ChainProp(name='HLT_mu11_l2io_mu6_l2io_invmDimu_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed','express'], groups=MultiMuonGroup+SupportGroup, monGroups=['bphysMon:shifter']),
        ChainProp(name='HLT_mu11_l2io_mu6_l2io_invmDimu_L1LFV-MU8VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=MultiMuonGroup+SupportGroup+Topo2Group),
        # ATR-22782, ATR-28868, 4muon
        ChainProp(name='HLT_mu4_ivarloose_mu4_mu3noL1_L1BPH-7M14-0DR25-MU5VFMU3VF', l1SeedThresholds=['MU3VF', 'MU3VF', 'FSNOSEED'], stream=['BphysDelayed'], groups=MultiMuonGroup+SupportGroup+Topo3Group),
        ChainProp(name='HLT_mu4_ivarloose_2mu3noL1_L1BPH-7M14-0DR25-MU5VFMU3VF', l1SeedThresholds=['MU3VF','FSNOSEED'], stream=['BphysDelayed'], groups=MultiMuonGroup+SupportGroup+Topo3Group),
        ChainProp(name='HLT_2mu4_L1BPH-7M11-25DR99-2MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=MultiMuonGroup+SupportGroup+Topo3Group),
        ChainProp(name='HLT_2mu4_ivarloose_L1BPH-7M14-0DR25-MU5VFMU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=MultiMuonGroup+SupportGroup+Topo3Group),



        # ATR-19354, low mass Drell-Yan triggers
        # L1Topo chains
        ChainProp(name='HLT_mu4_ivarloose_mu4_7invmAB9_L1DY-BOX-2MU3VF',          l1SeedThresholds=['MU3VF','MU3VF'], groups=MultiMuonGroup+EOFL1MuGroup+Topo2Group+['RATE:CPS_DY-BOX-2MU3VF']),
        ChainProp(name='HLT_mu4_ivarloose_mu4_b7invmAB9vtx20_L1DY-BOX-2MU3VF',    l1SeedThresholds=['MU3VF','MU3VF'], groups=MultiMuonGroup+EOFL1MuGroup+Topo2Group+['RATE:CPS_DY-BOX-2MU3VF']),
        ChainProp(name='HLT_mu4_ivarloose_mu4_11invmAB60_L1DY-BOX-2MU3VF',        l1SeedThresholds=['MU3VF','MU3VF'], groups=MultiMuonGroup+EOFL1MuGroup+Topo2Group+['RATE:CPS_DY-BOX-2MU3VF']),
        ChainProp(name='HLT_mu4_ivarloose_mu4_b11invmAB60vtx20_L1DY-BOX-2MU3VF',  l1SeedThresholds=['MU3VF','MU3VF'], groups=MultiMuonGroup+EOFL1MuGroup+Topo2Group+['RATE:CPS_DY-BOX-2MU3VF']),
        ChainProp(name='HLT_mu6_ivarloose_mu6_11invmAB24_L1DY-BOX-2MU5VF',       l1SeedThresholds=['MU5VF','MU5VF'], groups=MultiMuonGroup+EOFL1MuGroup+Topo2Group+['RATE:CPS_DY-BOX-2MU5VF']),
        ChainProp(name='HLT_mu6_ivarloose_mu6_b11invmAB24vtx20_L1DY-BOX-2MU5VF', l1SeedThresholds=['MU5VF','MU5VF'], groups=MultiMuonGroup+EOFL1MuGroup+Topo2Group+['RATE:CPS_DY-BOX-2MU5VF']),
        ChainProp(name='HLT_mu6_ivarloose_mu6_24invmAB60_L1DY-BOX-2MU5VF',       l1SeedThresholds=['MU5VF','MU5VF'], groups=MultiMuonGroup+EOFL1MuGroup+Topo2Group+['RATE:CPS_DY-BOX-2MU5VF']),
        ChainProp(name='HLT_mu6_ivarloose_mu6_b24invmAB60vtx20_L1DY-BOX-2MU5VF', l1SeedThresholds=['MU5VF','MU5VF'], groups=MultiMuonGroup+EOFL1MuGroup+Topo2Group+['RATE:CPS_DY-BOX-2MU5VF']),
        # non-L1Topo chains (backup)
        ChainProp(name='HLT_mu4_ivarloose_mu4_7invmAB9_L12MU3V', l1SeedThresholds=['MU3V','MU3V'], groups=MultiMuonGroup+SupportGroup+['RATE:CPS_2MU3V']),
        ChainProp(name='HLT_mu4_ivarloose_mu4_b7invmAB9vtx20_L12MU3V', l1SeedThresholds=['MU3V','MU3V'], groups=MultiMuonGroup+SupportGroup+['RATE:CPS_2MU3V']),
        ChainProp(name='HLT_mu4_ivarloose_mu4_11invmAB60_L12MU3V', l1SeedThresholds=['MU3V','MU3V'], groups=MultiMuonGroup+SupportGroup+['RATE:CPS_2MU3V']),
        ChainProp(name='HLT_mu4_ivarloose_mu4_b11invmAB60vtx20_L12MU3V', l1SeedThresholds=['MU3V','MU3V'], groups=MultiMuonGroup+SupportGroup+['RATE:CPS_2MU3V']),
        ChainProp(name='HLT_mu6_ivarloose_mu6_11invmAB24_L12MU5VF', l1SeedThresholds=['MU5VF','MU5VF'], groups=MultiMuonGroup+SupportGroup+['RATE:CPS_2MU5VF']),
        ChainProp(name='HLT_mu6_ivarloose_mu6_b11invmAB24vtx20_L12MU5VF', l1SeedThresholds=['MU5VF','MU5VF'], groups=MultiMuonGroup+SupportGroup+['RATE:CPS_2MU5VF']),
        ChainProp(name='HLT_mu6_ivarloose_mu6_24invmAB60_L12MU5VF', l1SeedThresholds=['MU5VF','MU5VF'], groups=MultiMuonGroup+SupportGroup+['RATE:CPS_2MU5VF']),
        ChainProp(name='HLT_mu6_ivarloose_mu6_b24invmAB60vtx20_L12MU5VF', l1SeedThresholds=['MU5VF','MU5VF'], groups=MultiMuonGroup+SupportGroup+['RATE:CPS_2MU5VF']),
        # support chains
        ChainProp(name='HLT_2mu4_7invmAA9_L1DY-BOX-2MU3VF', l1SeedThresholds=['MU3VF'], groups=MultiMuonGroup+SupportGroup+Topo2Group+['RATE:CPS_DY-BOX-2MU3VF']),
        ChainProp(name='HLT_2mu4_11invmAA60_L1DY-BOX-2MU3VF', l1SeedThresholds=['MU3VF'], groups=MultiMuonGroup+SupportGroup+Topo2Group+['RATE:CPS_DY-BOX-2MU3VF']),
        ChainProp(name='HLT_2mu6_11invmAA24_L1DY-BOX-2MU5VF', l1SeedThresholds=['MU5VF'], groups=MultiMuonGroup+SupportGroup+Topo2Group+['RATE:CPS_DY-BOX-2MU5VF']),
        ChainProp(name='HLT_2mu6_24invmAA60_L1DY-BOX-2MU5VF', l1SeedThresholds=['MU5VF'], groups=MultiMuonGroup+SupportGroup+Topo2Group+['RATE:CPS_DY-BOX-2MU5VF']),
        # backup without L1Topo
        ChainProp(name='HLT_2mu4_7invmAA9_L12MU3V', l1SeedThresholds=['MU3V'], groups=MultiMuonGroup+SupportGroup+['RATE:CPS_2MU3V']),
        ChainProp(name='HLT_2mu4_11invmAA60_L12MU3V', l1SeedThresholds=['MU3V'], groups=MultiMuonGroup+SupportGroup+['RATE:CPS_2MU3V']),
        ChainProp(name='HLT_2mu6_11invmAA24_L12MU5VF', l1SeedThresholds=['MU5VF'], groups=MultiMuonGroup+SupportGroup+['RATE:CPS_2MU5VF']),
        ChainProp(name='HLT_2mu6_24invmAA60_L12MU5VF', l1SeedThresholds=['MU5VF'], groups=MultiMuonGroup+SupportGroup+['RATE:CPS_2MU5VF']),

        # ATR-24367 (express stream for ID)
        ChainProp(name='HLT_mu14_mu14_idtp_idZmumu_L12MU8F', l1SeedThresholds=['MU8F','MU8F'], stream=[PhysicsStream,'express'], groups=MultiMuonGroup+SupportGroup, monGroups=['idMon:shifter']),
        ChainProp(name='HLT_mu14_mu14_idperf_50invmAB130_L12MU8F', l1SeedThresholds=['MU8F','MU8F'], stream=[PhysicsStream,'express'], groups=MultiMuonGroup+SupportGroup, monGroups=['idMon:t0']),
        ChainProp(name='HLT_mu4_mu4_idperf_1invmAB5_L12MU3VF', l1SeedThresholds=['MU3VF','MU3VF'], stream=[PhysicsStream,'express'], groups=MultiMuonGroup+SupportGroup+['RATE:CPS_2MU3VF'], monGroups=['idMon:t0']),
        ChainProp(name='HLT_mu4_mu4_idtp_idJpsimumu_L12MU3VF', l1SeedThresholds=['MU3VF','MU3VF'], stream=[PhysicsStream,'express'], groups=MultiMuonGroup+SupportGroup+['RATE:CPS_2MU3VF'], monGroups=['idMon:shifter']),
     
        # ATR-26929 (muon tag&probe for 2023)
        ChainProp(name="HLT_mu24_ivarmedium_mu14_idtp_idZmumu_L1MU14FCH", l1SeedThresholds=['MU14FCH']*2, stream=[PhysicsStream, 'express'], groups=MultiMuonGroup+SupportGroup, monGroups=['idMon:shifter']),
        ChainProp(name="HLT_mu24_ivarmedium_mu14_idtp_idZmumu_L1MU18VFCH", l1SeedThresholds=['MU18VFCH','MU14FCH'], stream=[PhysicsStream, 'express'], groups=MultiMuonGroup+SupportGroup, monGroups=['idMon:shifter']),
        ChainProp(name="HLT_mu26_ivarmedium_mu14_idtp_idZmumu_L1MU14FCH", l1SeedThresholds=['MU14FCH']*2, stream=[PhysicsStream, 'express'], groups=MultiMuonGroup+SupportGroup, monGroups=['idMon:shifter']),
        ChainProp(name="HLT_mu26_ivarmedium_mu14_idtp_idZmumu_L1MU18VFCH", l1SeedThresholds=['MU18VFCH']*2, stream=[PhysicsStream, 'express'], groups=MultiMuonGroup+SupportGroup, monGroups=['idMon:shifter']),

        # ATR-26151
        ChainProp(name='HLT_mu24_ivarmedium_mu14_idperf_probe_50invmAB130_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEMU8F'], stream=[PhysicsStream,'express'], groups=MultiMuonGroup+SupportGroup, monGroups=['idMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_mu14_idperf_probe_50invmAB130_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEMU8F'], stream=[PhysicsStream,'express'], groups=MultiMuonGroup+SupportGroup, monGroups=['idMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_mu14_idperf_probe_50invmAB130_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEMU8F'], stream=[PhysicsStream,'express'], groups=MultiMuonGroup+SupportGroup, monGroups=['idMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_mu14_idperf_probe_50invmAB130_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEMU8F'], stream=[PhysicsStream,'express'], groups=MultiMuonGroup+SupportGroup, monGroups=['idMon:t0']),

        # ATR-19376 new nscan trigger for run 3
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan40_L1MU14FCH_jJ90', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryPhIGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan40_L1MU14FCH_jXE80', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryPhIGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan40_L110DR-MU14FCH-MU5VF', l1SeedThresholds=['MU14FCH','FSNOSEED'],   groups=PrimaryL1MuGroup+MultiMuonGroup+Topo2Group),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan40_L1MU14FCH_UNPAIRED_ISO', l1SeedThresholds=['MU14FCH','FSNOSEED'], stream=['Late'], groups=PrimaryL1MuGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan40_L1MU14FCH_EMPTY', l1SeedThresholds=['MU14FCH','FSNOSEED'], stream=['Late'], groups=PrimaryL1MuGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan40_L110DR-MU14FCH-MU5VF_UNPAIRED_ISO', l1SeedThresholds=['MU14FCH','FSNOSEED'], stream=['Late'],  groups=PrimaryL1MuGroup+MultiMuonGroup+Topo2Group),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan40_L110DR-MU14FCH-MU5VF_EMPTY', l1SeedThresholds=['MU14FCH','FSNOSEED'], stream=['Late'],   groups=PrimaryL1MuGroup+MultiMuonGroup+Topo2Group),
       # ATR-28412 VBF nscan + msonly for run3
        ChainProp(name='HLT_mu10_msonly_L1jMJJ-500-NFF', l1SeedThresholds=['MU5VF'], groups=PrimaryPhIGroup+SingleMuonGroup+Topo3Group),
        ChainProp(name='HLT_mu6_msonly_iloosems_mu6noL1_msonly_nscan40_L1jMJJ-500-NFF', l1SeedThresholds=['MU3V','FSNOSEED'], groups=PrimaryPhIGroup+MultiMuonGroup+Topo3Group),



    ]

    chains['Egamma'] += [
        # Electron Chains----------


        # Phase1 eEM chains 
        ChainProp(name='HLT_e26_lhtight_ivarloose_L1eEM26M', stream=[PhysicsStream,'express'], groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:online','egammaMon:shifter_tp','caloMon:t0']),
        ChainProp(name='HLT_e26_lhtight_ivarloose_L1eEM26T', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:shifter_tp']),
        # ATR-27325 Primary -> Support
        ChainProp(name='HLT_e26_lhtight_ivarloose_L1eEM26L', groups=SupportPhIGroup+SingleElectronGroup, monGroups=['egammaMon:shifter_tp']),
        ChainProp(name='HLT_e26_lhtight_ivarloose_L1eEM26', groups=SupportPhIGroup+SingleElectronGroup, monGroups=['egammaMon:shifter_tp']),  

        # ATR-27373
        ChainProp(name='HLT_e28_lhtight_ivarloose_L1eEM28M', stream=[PhysicsStream,'express'], groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:online','egammaMon:shifter_tp','caloMon:t0']),

        # ATR-27156 Phase-1
        # dnn chains
        # ATR_30971
        ChainProp(name='HLT_e26_dnntight_ivarloose_e17_dnnloose_probe_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeEM18M'],groups=TagAndProbePhIGroup+MultiElectronGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_2e17_dnnloose_L12eEM18M', groups=PrimaryPhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_2e24_dnnloose_L12eEM24L', groups=PrimaryPhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_e5_dnntight_e9_etcut_1invmAB5_L1JPSI-1M5-eEM9', stream=[PhysicsStream,'express'], l1SeedThresholds=['eEM5','eEM9'], groups=SupportPhIGroup+MultiElectronGroup+Topo2Group+['RATE:CPS_JPSI-1M5-eEM9'], monGroups=['egammaMon:shifter_topo']),
        ChainProp(name='HLT_e24_dnnloose_2e12_dnnloose_L1eEM24L_3eEM12L',l1SeedThresholds=['eEM24L','eEM12L'], groups=PrimaryPhIGroup+MultiElectronGroup), 
        ChainProp(name='HLT_e26_dnntight_ivarloose_L1eEM26M', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:online','egammaMon:t0_tp']),
        ChainProp(name='HLT_e60_dnnmedium_L1eEM26M', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),
        ChainProp(name='HLT_e140_dnnloose_L1eEM26M', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),

        #ATR-31145
        ChainProp(name='HLT_e20_lhtight_ivarloose_L1ZAFB-04DPHIM-eEM18M', l1SeedThresholds=['eEM18M'], groups=PrimaryPhIGroup+SingleElectronGroup+Topo3Group),
        ChainProp(name='HLT_e20_lhtight_ivarloose_L1eEM18M_jEM25', l1SeedThresholds=['eEM18M'], groups=PrimaryPhIGroup+SingleElectronGroup),

        # ATR-27373
        ChainProp(name='HLT_e28_dnntight_ivarloose_L1eEM28M', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:online','egammaMon:t0_tp']),
        ChainProp(name='HLT_e60_dnnmedium_L1eEM28M', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),
        ChainProp(name='HLT_e140_dnnloose_L1eEM28M', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),
                   
        # ATR-25512
        #ChainProp(name='HLT_e28_lhtight_ivarloose_L1eEM26T', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),
        
        ChainProp(name='HLT_e60_lhmedium_L1eEM26M', stream=[PhysicsStream,'express'], groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:online','egammaMon:shifter_tp']),
        ChainProp(name='HLT_e140_lhloose_L1eEM26M', stream=[PhysicsStream,'express'], groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:shifter_tp','caloMon:t0']),
        ChainProp(name='HLT_e300_etcut_L1eEM26M', stream=[PhysicsStream,'express'], groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:shifter','caloMon:t0']),

        # ATR-27373
        ChainProp(name='HLT_e60_lhmedium_L1eEM28M', stream=[PhysicsStream,'express'], groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:online','egammaMon:shifter_tp']),
        ChainProp(name='HLT_e140_lhloose_L1eEM28M', stream=[PhysicsStream,'express'], groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:shifter_tp','caloMon:t0']),
        ChainProp(name='HLT_e300_etcut_L1eEM28M', stream=[PhysicsStream,'express'], groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:shifter','caloMon:t0']),

        #--------- primary 1e
        ChainProp(name='HLT_e26_lhtight_ivarmedium_L1eEM26M', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:shifter_tp']), # Phase-1 ATR-27156

        # ATR-27373
        ChainProp(name='HLT_e28_lhtight_ivarmedium_L1eEM28M', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:shifter_tp']),
        ChainProp(name='HLT_e26_lhtight_ivartight_L1eEM26M', groups=PrimaryPhIGroup+SingleElectronGroup), # Phase-1 ATR-27156

        # ATR-27373
        ChainProp(name='HLT_e28_lhtight_ivartight_L1eEM28M', groups=PrimaryPhIGroup+SingleElectronGroup),

        #---------- primary 2e 
        ChainProp(name='HLT_2e17_lhvloose_L12eEM18M', groups=PrimaryPhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_2e24_lhvloose_L12eEM24L', groups=PrimaryPhIGroup+MultiElectronGroup),

        #---------- support 2e + 1g + ZRad triggers 
        #ChainProp(name='HLT_2e17_lhvloose_g20_tight_icaloloose_probe_L12eEM18M', l1SeedThresholds=['eEM18M','PROBEeEM18M'], groups=TagAndProbePhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_2e17_lhvloose_g22_tight_probe_L12eEM18M', l1SeedThresholds=['eEM18M','PROBEeEM18M'], groups=TagAndProbePhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_2e17_lhvloose_g25_medium_probe_L12eEM18M', l1SeedThresholds=['eEM18M','PROBEeEM18M'],groups=TagAndProbePhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_2e17_lhvloose_g50_loose_probe_L12eEM18M', l1SeedThresholds=['eEM18M','PROBEeEM18M'],groups=TagAndProbePhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_2e17_lhvloose_g35_medium_probe_L12eEM18M', l1SeedThresholds=['eEM18M','PROBEeEM18M'],groups=TagAndProbePhIGroup+MultiElectronGroup),
        #ChainProp(name='HLT_2e24_lhvloose_g20_tight_icaloloose_probe_L12eEM24L', l1SeedThresholds=['eEM24L','PROBEeEM24L'],groups=TagAndProbePhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_2e24_lhvloose_g22_tight_probe_L12eEM24L', l1SeedThresholds=['eEM24L','PROBEeEM24L'],groups=TagAndProbePhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_2e24_lhvloose_g25_medium_probe_L12eEM24L', l1SeedThresholds=['eEM24L','PROBEeEM24L'],groups=TagAndProbePhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_2e24_lhvloose_g35_medium_probe_L12eEM24L', l1SeedThresholds=['eEM24L','PROBEeEM24L'],groups=TagAndProbePhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_2e24_lhvloose_g50_loose_probe_L12eEM24L', l1SeedThresholds=['eEM24L','PROBEeEM24L'],groups=TagAndProbePhIGroup+MultiElectronGroup),
        
        #ATR-27251, Phase-I
        ChainProp(name='HLT_2e17_lhvloose_g20_tight_probe_L12eEM18M', l1SeedThresholds=['eEM18M','PROBEeEM18M'], groups=TagAndProbePhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_2e24_lhvloose_g20_tight_probe_L12eEM24L', l1SeedThresholds=['eEM24L','PROBEeEM24L'], groups=TagAndProbePhIGroup+MultiElectronGroup),
        
        #---------- primary 3e
        ChainProp(name='HLT_e24_lhvloose_2e12_lhvloose_L1eEM24L_3eEM12L',l1SeedThresholds=['eEM24L','eEM12L'], groups=PrimaryPhIGroup+MultiElectronGroup),

        # ATR-25512
        ChainProp(name='HLT_e20_lhtight_ivarloose_L12eEM9', l1SeedThresholds=['eEM9'], groups=SupportPhIGroup+SingleElectronGroup), # Phase-1 ATR-27156

        #------------ support alternative lowest unprescaled 1e
        ChainProp(name='HLT_e24_lhtight_ivarloose_L1eEM26M', groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM26M']),

        #------------ support noringer of primary 1e
        ChainProp(name='HLT_e26_lhtight_ivarloose_noringer_L1eEM26M', groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM26M']),   # Phase-1 ATR-27156 

        # ATR-27373
        ChainProp(name='HLT_e28_lhtight_ivarloose_noringer_L1eEM28M', groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM28M']),

        # ATR-25512
        ChainProp(name='HLT_e28_lhtight_ivarloose_noringer_L1eEM26M', groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_e60_lhmedium_noringer_L1eEM26M', groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_e140_lhloose_noringer_L1eEM26M', groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM26M']),

        # ATR-27373
        ChainProp(name='HLT_e30_lhtight_ivarloose_noringer_L1eEM28M', groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_e60_lhmedium_noringer_L1eEM28M', groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_e140_lhloose_noringer_L1eEM28M', groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM28M']),

        #------------ support bootstrap
        ChainProp(name='HLT_e50_etcut_L1eEM26M', groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_e120_etcut_L1eEM26M', groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_e250_etcut_L1eEM26M', groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM26M']),

        # ATR-27373
        ChainProp(name='HLT_e50_etcut_L1eEM28M', groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_e120_etcut_L1eEM28M', groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_e250_etcut_L1eEM28M', groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM28M']),
        
        #ATR-31076
        ChainProp(name='HLT_e10_nopid_L1eEM9', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM9']+['PS:NoBulkMCProd']),
        ChainProp(name='HLT_e15_nopid_L1eEM9', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM9']+['PS:NoBulkMCProd']),
        ChainProp(name='HLT_e20_nopid_L1eEM18M', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM18M']+['PS:NoBulkMCProd']),
        ChainProp(name='HLT_e25_nopid_L1eEM18M', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM18M']+['PS:NoBulkMCProd']),
        ChainProp(name='HLT_e30_nopid_L1eEM18M', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM18M']+['PS:NoBulkMCProd']),
        ChainProp(name='HLT_e40_nopid_L1eEM18M', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM18M']+['PS:NoBulkMCProd']),
        ChainProp(name='HLT_e50_nopid_L1eEM26M', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM26M']+['PS:NoBulkMCProd']), 
        ChainProp(name='HLT_e60_nopid_L1eEM26M', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM26M']+['PS:NoBulkMCProd']),
        ChainProp(name='HLT_e70_nopid_L1eEM26M', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM26M']+['PS:NoBulkMCProd']),
        ChainProp(name='HLT_e80_nopid_L1eEM26M', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM26M']+['PS:NoBulkMCProd']),
        ChainProp(name='HLT_e100_nopid_L1eEM26M', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM26M']+['PS:NoBulkMCProd']),
        ChainProp(name='HLT_e120_nopid_L1eEM26M', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM26M']+['PS:NoBulkMCProd']),
        ChainProp(name='HLT_e300_nopid_L1eEM26M',groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM26M']+['PS:NoBulkMCProd']),  

        # ATR-27373        
        ChainProp(name='HLT_e60_etcut_L1eEM28M', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_e70_etcut_L1eEM28M', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_e80_etcut_L1eEM28M', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_e100_etcut_L1eEM28M', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM28M']),

        #ATR-27251, Phase-I
        ChainProp(name='HLT_e5_etcut_L1eEM5', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM5']+['PS:NoBulkMCProd']),
                #------------ nopid trigger and etcut from ATR-26311
        # ATR-23723
        ChainProp(name='HLT_e5_nopid_L1eEM5', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM5']+['PS:NoBulkMCProd']), #ATR-27264

        #------------ support background studies

        ChainProp(name='HLT_e10_lhvloose_L1eEM9', groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM9']),
        ChainProp(name='HLT_e14_lhvloose_L1eEM12L', groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM12L']),
        ChainProp(name='HLT_e20_lhvloose_L1eEM18L', groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM18L']),
        ChainProp(name='HLT_e26_lhtight_L1eEM26M', groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM26M']), #ATR-27264
        ChainProp(name='HLT_e28_lhtight_L1eEM26M', groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM26M']), #ATR-27264
        ChainProp(name='HLT_e28_lhtight_L1eEM28M', groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM28M']), #ATR-27264
        ChainProp(name='HLT_e30_lhvloose_L1eEM26M', groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_e40_lhvloose_L1eEM26M', groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_e60_lhvloose_L1eEM26M', groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_e80_lhvloose_L1eEM26M', groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_e100_lhvloose_L1eEM26M', groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_e120_lhvloose_L1eEM26M', stream=[PhysicsStream,'express'], monGroups=['egammaMon:shifter_tp'], groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM26M']),

        # ATR-27373
        ChainProp(name='HLT_e30_lhvloose_L1eEM28M', groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_e40_lhvloose_L1eEM28M', groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_e60_lhvloose_L1eEM28M', groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_e80_lhvloose_L1eEM28M', groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_e100_lhvloose_L1eEM28M', groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_e120_lhvloose_L1eEM28M', stream=[PhysicsStream,'express'], monGroups=['egammaMon:shifter_tp'], groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM28M']),

        #--------------- support low lumi 1e removed during ATR-27156

        #ATR-25727
        ChainProp(name='HLT_e26_lhtight_ivarloose_e7_lhmedium_probe_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeEM5'],groups=TagAndProbePhIGroup+MultiElectronGroup+['RATE:CPS_eEM26M']), #  Phase-1 ATR-27156

        # ATR-27373
        ChainProp(name='HLT_e28_lhtight_ivarloose_e7_lhmedium_probe_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeEM5'],groups=TagAndProbePhIGroup+MultiElectronGroup+['RATE:CPS_eEM28M']),

        # ATR-27089 additional support probe triggers for e/mu
        ChainProp(name='HLT_e26_lhtight_ivarloose_e17_lhloose_probe_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeEM18M'],groups=TagAndProbePhIGroup+MultiElectronGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_e26_lhtight_ivarloose_e12_lhloose_probe_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeEM10L'],groups=TagAndProbePhIGroup+MultiElectronGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_e26_lhtight_ivarloose_e26_lhmedium_probe_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeEM26M'],groups=TagAndProbePhIGroup+MultiElectronGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_e26_lhtight_ivarloose_e9_lhvloose_probe_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeEM5'],groups=TagAndProbePhIGroup+MultiElectronGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_e26_lhtight_ivarloose_e17_lhmedium_probe_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeEM18M'],groups=TagAndProbePhIGroup+MultiElectronGroup+['RATE:CPS_eEM26M']),
        # duplicates with eEM28M
        ChainProp(name='HLT_e28_lhtight_ivarloose_e17_lhloose_probe_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeEM18M'],groups=TagAndProbePhIGroup+MultiElectronGroup+['RATE:CPS_eEM28M']), 
        ChainProp(name='HLT_e28_lhtight_ivarloose_e12_lhloose_probe_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeEM10L'],groups=TagAndProbePhIGroup+MultiElectronGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_e28_lhtight_ivarloose_e26_lhmedium_probe_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeEM26M'],groups=TagAndProbePhIGroup+MultiElectronGroup+['RATE:CPS_eEM28M']), 
        ChainProp(name='HLT_e28_lhtight_ivarloose_e9_lhvloose_probe_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeEM5'],groups=TagAndProbePhIGroup+MultiElectronGroup+['RATE:CPS_eEM28M']), 
        ChainProp(name='HLT_e28_lhtight_ivarloose_e17_lhmedium_probe_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeEM18M'],groups=TagAndProbePhIGroup+MultiElectronGroup+['RATE:CPS_eEM28M']),


        # Photon Chains----------
        #----------- primary 1g
        ChainProp(name='HLT_g140_loose_L1eEM26M', groups=PrimaryPhIGroup+SinglePhotonGroup),
        ChainProp(name='HLT_g300_etcut_L1eEM26M', groups=PrimaryPhIGroup+SinglePhotonGroup),

        # ATR-27373
        ChainProp(name='HLT_g140_loose_L1eEM28M', groups=PrimaryPhIGroup+SinglePhotonGroup),
        ChainProp(name='HLT_g300_etcut_L1eEM28M', groups=PrimaryPhIGroup+SinglePhotonGroup),

        #----------- primary 2g
        ChainProp(name='HLT_2g20_tight_icaloloose_L12eEM18M', groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_2g22_tight_L12eEM18M', groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_g35_medium_g25_medium_L12eEM24L', l1SeedThresholds=['eEM24L','eEM24L'], stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_g45_medium_g20_medium_L1eEM40L_2eEM18L', l1SeedThresholds=['eEM40L', 'eEM18L'], stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_2g50_loose_L12eEM24L', groups=PrimaryPhIGroup+MultiPhotonGroup),

        #ATR_30971
        ChainProp(name='HLT_g5_nopid_L1eEM5', groups=SinglePhotonGroup+SupportPhIGroup+['RATE:CPS_eEM5']+['PS:NoBulkMCProd']),
        ChainProp(name='HLT_g25_loose_noringer_L1eEM24L', groups=SupportPhIGroup+SinglePhotonGroup+['RATE:CPS_eEM24L']),
        ChainProp(name='HLT_g35_medium_noringer_L1eEM24L', groups=SupportPhIGroup+SinglePhotonGroup+['RATE:CPS_eEM24L'], monGroups=['egammaMon:shifter']),
        ChainProp(name='HLT_2g22_tight_noringer_L12eEM18M', groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_g140_loose_noringer_L1eEM26M', groups=PrimaryPhIGroup+SinglePhotonGroup, monGroups=['egammaMon:shifter']),

        # ATR-31076
        ChainProp(name='HLT_g10_nopid_L1eEM9', groups=SinglePhotonGroup+SupportPhIGroup+['RATE:CPS_eEM9']+['PS:NoBulkMCProd']),
        ChainProp(name='HLT_g15_nopid_L1eEM9', groups=SinglePhotonGroup+SupportPhIGroup+['RATE:CPS_eEM9']+['PS:NoBulkMCProd']),
        ChainProp(name='HLT_g20_nopid_L1eEM18M', groups=SinglePhotonGroup+SupportPhIGroup+['RATE:CPS_eEM18M']+['PS:NoBulkMCProd']),
        ChainProp(name='HLT_g25_nopid_L1eEM18M', groups=SinglePhotonGroup+SupportPhIGroup+['RATE:CPS_eEM18M']+['PS:NoBulkMCProd']),
        ChainProp(name='HLT_g30_nopid_L1eEM18M', groups=SinglePhotonGroup+SupportPhIGroup+['RATE:CPS_eEM18M']+['PS:NoBulkMCProd']),
        ChainProp(name='HLT_g40_nopid_L1eEM18M', groups=SinglePhotonGroup+SupportPhIGroup+['RATE:CPS_eEM18M']+['PS:NoBulkMCProd']),
        ChainProp(name='HLT_g50_nopid_L1eEM26M', groups=SinglePhotonGroup+SupportPhIGroup+['RATE:CPS_eEM26M']+['PS:NoBulkMCProd']),
        ChainProp(name='HLT_g60_nopid_L1eEM26M', groups=SinglePhotonGroup+SupportPhIGroup+['RATE:CPS_eEM26M']+['PS:NoBulkMCProd']),
        ChainProp(name='HLT_g70_nopid_L1eEM26M', groups=SinglePhotonGroup+SupportPhIGroup+['RATE:CPS_eEM26M']+['PS:NoBulkMCProd']),
        ChainProp(name='HLT_g80_nopid_L1eEM26M', groups=SinglePhotonGroup+SupportPhIGroup+['RATE:CPS_eEM26M']+['PS:NoBulkMCProd']),
        ChainProp(name='HLT_g100_nopid_L1eEM26M', groups=SinglePhotonGroup+SupportPhIGroup+['RATE:CPS_eEM26M']+['PS:NoBulkMCProd']),
        ChainProp(name='HLT_g120_nopid_L1eEM26M', groups=SinglePhotonGroup+SupportPhIGroup+['RATE:CPS_eEM26M']+['PS:NoBulkMCProd']),
        ChainProp(name='HLT_g300_nopid_L1eEM26M', groups=SinglePhotonGroup+SupportPhIGroup+['RATE:CPS_eEM26M']+['PS:NoBulkMCProd']),


        # low-mass diphoton ATR-21637
        ChainProp(name='HLT_2g9_loose_25dphiAA_invmAA80_L1DPHI-M70-2eEM9', l1SeedThresholds=['eEM9'], groups=SupportPhIGroup+MultiPhotonGroup+Topo2Group),
        ChainProp(name='HLT_2g9_loose_25dphiAA_invmAA80_L1DPHI-M70-2eEM9L', l1SeedThresholds=['eEM9'], groups=EOFEgammaPhIGroup+MultiPhotonGroup+Topo2Group),

        # low-mass diphoton ATR-21608
        ChainProp(name='HLT_2g15_tight_25dphiAA_invmAA80_L1DPHI-M70-2eEM15M', l1SeedThresholds=['eEM15'], groups=PrimaryPhIGroup+MultiPhotonGroup+Topo2Group),
        ChainProp(name='HLT_2g15_loose_25dphiAA_invmAA80_L1DPHI-M70-2eEM15M', l1SeedThresholds=['eEM15'], groups=SupportPhIGroup+MultiPhotonGroup+['RATE:CPS_DPHI-M70-2eEM15M']+Topo2Group),
        ChainProp(name='HLT_2g15_tight_25dphiAA_L1DPHI-M70-2eEM15M', l1SeedThresholds=['eEM15'], groups=SupportPhIGroup+MultiPhotonGroup+['RATE:CPS_DPHI-M70-2eEM15M']+Topo2Group),

        # Very LowMass DiPhoton, EGammaPEB
        ChainProp(name='HLT_2g13_loose_EgammaPEBTLA_L12DR15-0M30-2eEM12L',l1SeedThresholds=['eEM12L'],stream=['EgammaPEBTLA'], groups=PrimaryPhIGroup+Topo2Group),
        ChainProp(name='HLT_2g13_loose_EgammaPEBTLA_L113DR25-25M70-2eEM12L',l1SeedThresholds=['eEM12L'],stream=['EgammaPEBTLA'], groups=PrimaryPhIGroup+Topo2Group),
        # FullBuild support for DiPhoton EGammaPEB
        ChainProp(name='HLT_2g13_loose_L12DR15-0M30-2eEM12L',l1SeedThresholds=['eEM12L'], groups=SupportPhIGroup+Topo2Group),
        ChainProp(name='HLT_2g13_loose_L113DR25-25M70-2eEM12L',l1SeedThresholds=['eEM12L'], groups=SupportPhIGroup+Topo2Group),


        # Non-L1Topo backups
        ChainProp(name='HLT_2g9_loose_25dphiAA_invmAA80_L12eEM9', l1SeedThresholds=['eEM9'], groups=EOFEgammaPhIGroup+MultiPhotonGroup+['RATE:CPS_2eEM9']),  # Phase-1 ATR-27156
        #
        ChainProp(name='HLT_2g15_tight_25dphiAA_invmAA80_L12eEM9', l1SeedThresholds=['eEM15'], groups=SupportPhIGroup+MultiPhotonGroup+['RATE:CPS_2eEM9']), # Phase-1 ATR-27156
        ChainProp(name='HLT_2g15_loose_25dphiAA_invmAA80_L12eEM9', l1SeedThresholds=['eEM15'], groups=SupportPhIGroup+MultiPhotonGroup+['RATE:CPS_2eEM9']),  # Phase-1 ATR-27156
        ChainProp(name='HLT_2g15_tight_25dphiAA_L12eEM9', l1SeedThresholds=['eEM15'], groups=SupportPhIGroup+MultiPhotonGroup+['RATE:CPS_2eEM9']),   # Phase-1 ATR-27156

        #--- hipTRT trigger, ATR-27996
        ChainProp(name='HLT_g0_hiptrt_L1eEM26M' , l1SeedThresholds=['eEM26M'], groups=PrimaryPhIGroup+SinglePhotonGroup), 
        ChainProp(name='HLT_g0_hiptrt_L1eEM28M' , l1SeedThresholds=['eEM28M'], groups=PrimaryPhIGroup+SinglePhotonGroup), 
        # support 2g ATR-23425
        ChainProp(name='HLT_2g20_loose_L12eEM18L', groups=SupportPhIGroup+MultiPhotonGroup),

        #------------ primary 3g
        ChainProp(name='HLT_2g25_loose_g15_loose_L12eEM24L', l1SeedThresholds=['eEM24L','eEM12L'], groups=PrimaryPhIGroup+MultiPhotonGroup),

        #------------ support legs of multi-photons

        ChainProp(name='HLT_g25_medium_L1eEM24L', stream=[PhysicsStream,'express'], groups=SupportPhIGroup+SinglePhotonGroup+['RATE:CPS_eEM24L'], monGroups=['egammaMon:online','egammaMon:shifter']),
        ChainProp(name='HLT_g35_medium_L1eEM24L', groups=SupportPhIGroup+SinglePhotonGroup+['RATE:CPS_eEM24L'], monGroups=['egammaMon:shifter']),

        ChainProp(name='HLT_g20_tight_icaloloose_L1eEM18M', stream=[PhysicsStream,'express'], groups=SupportPhIGroup+SinglePhotonGroup+['RATE:CPS_eEM18M'], monGroups=['egammaMon:online']),
        ChainProp(name='HLT_g15_tight_L1eEM12L', groups=SupportPhIGroup+SinglePhotonGroup+['RATE:CPS_eEM12L']),
        ChainProp(name='HLT_g20_tight_L1eEM18M', groups=SupportPhIGroup+SinglePhotonGroup+['RATE:CPS_eEM18M']),
        ChainProp(name='HLT_g22_tight_L1eEM18M', groups=SupportPhIGroup+SinglePhotonGroup+['RATE:CPS_eEM18M']),


        ChainProp(name='HLT_2g15_tight_L1DPHI-M70-2eEM15M', l1SeedThresholds=['eEM12L'], groups=SupportPhIGroup+SinglePhotonGroup+Topo2Group), # TODO: mismatch between L1topo threshold and L1 seed to be fixed 
        
        #------------ support bootstrap and background studies

        ChainProp(name='HLT_g250_etcut_L1eEM26M', groups=SupportPhIGroup+SinglePhotonGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g10_loose_L1eEM9', groups=SupportPhIGroup+SinglePhotonGroup+['RATE:CPS_eEM9']),
        ChainProp(name='HLT_g15_loose_L1eEM12L', stream=[PhysicsStream,'express'], monGroups=['egammaMon:shifter'], groups=SupportPhIGroup+SinglePhotonGroup+['RATE:CPS_eEM12L']),
        ChainProp(name='HLT_g20_loose_L1eEM18L', groups=SupportPhIGroup+SinglePhotonGroup+['RATE:CPS_eEM18L']),
        ChainProp(name='HLT_g25_loose_L1eEM24L', groups=SupportPhIGroup+SinglePhotonGroup+['RATE:CPS_eEM24L']),
        ChainProp(name='HLT_g30_loose_L1eEM24L', groups=SupportPhIGroup+SinglePhotonGroup+['RATE:CPS_eEM24L']),
        ChainProp(name='HLT_g40_loose_L1eEM24L', groups=SupportPhIGroup+SinglePhotonGroup+['RATE:CPS_eEM24L']),
        ChainProp(name='HLT_g50_loose_L1eEM24L', groups=SupportPhIGroup+SinglePhotonGroup+['RATE:CPS_eEM24L'], monGroups=['egammaMon:shifter']),
        ChainProp(name='HLT_g60_loose_L1eEM26M', groups=SupportPhIGroup+SinglePhotonGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g80_loose_L1eEM26M', groups=SupportPhIGroup+SinglePhotonGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g100_loose_L1eEM26M', groups=SupportPhIGroup+SinglePhotonGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g120_loose_L1eEM26M', groups=SupportPhIGroup+SinglePhotonGroup+['RATE:CPS_eEM26M']),

        # ATR-27373
        ChainProp(name='HLT_g250_etcut_L1eEM28M', groups=SupportPhIGroup+SinglePhotonGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_g60_loose_L1eEM28M', groups=SupportPhIGroup+SinglePhotonGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_g80_loose_L1eEM28M', groups=SupportPhIGroup+SinglePhotonGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_g100_loose_L1eEM28M', groups=SupportPhIGroup+SinglePhotonGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_g120_loose_L1eEM28M', groups=SupportPhIGroup+SinglePhotonGroup+['RATE:CPS_eEM28M']),

        #ATR-25764 - adding Photon chains with different isolation WPs
        ChainProp(name='HLT_g25_tight_icaloloose_L1eEM24L', groups=SupportPhIGroup+SinglePhotonGroup+['RATE:CPS_eEM24L']),  # Phase-1 ATR-27156

        #------- Electron+Photon Chains
        # primary e-g chains: electron + photon stay in the same step - these need to be parallel merged!
        ChainProp(name='HLT_e24_lhmedium_g25_medium_02dRAB_L12eEM24L', l1SeedThresholds=['eEM24L','eEM24L'], groups=PrimaryPhIGroup+MultiElectronGroup),
        # The dR ComboHypo will not do the standard photon-photon disambiguation,
        # so need to fall back on dR between all possible pairings
        ChainProp(name='HLT_e24_lhmedium_g12_loose_g12_loose_02dRAB_02dRAC_02dRBC_L1eEM24L_3eEM12L', l1SeedThresholds=['eEM24L','eEM12L','eEM12L'], groups=PrimaryPhIGroup+MultiElectronGroup),

        # Electron LRT chains        
        ChainProp(name='HLT_e30_lhloose_nopix_lrtmedium_L1eEM26M', groups=PrimaryPhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e35_lhloose_nopix_lrtmedium_L1eEM26M', groups=PrimaryPhIGroup+SingleElectronGroup),

        # ATR-27373
        ChainProp(name='HLT_e30_lhloose_nopix_lrtmedium_L1eEM28M', groups=PrimaryPhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e35_lhloose_nopix_lrtmedium_L1eEM28M', groups=PrimaryPhIGroup+SingleElectronGroup),

        # Support
        # T&P chains for displaced electrons
        ChainProp(name='HLT_e26_lhtight_ivarloose_e5_idperf_loose_lrtloose_probe_L1eEM26M',l1SeedThresholds=['eEM26M','PROBEeEM5'],groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_e30_lhloose_nopix_lrtmedium_probe_L1eEM26M',l1SeedThresholds=['eEM26M','PROBEeEM26M'],groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e5_idperf_loose_lrtloose_probe_g25_medium_L1eEM24L',l1SeedThresholds=['PROBEeEM5','eEM24L'],groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM24L']),
        ChainProp(name='HLT_e30_lhloose_nopix_lrtmedium_probe_g25_medium_L1eEM24L',l1SeedThresholds=['PROBEeEM26M','eEM24L'],groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM24L']),

        # ATR-27373
        ChainProp(name='HLT_e28_lhtight_ivarloose_e5_idperf_loose_lrtloose_probe_L1eEM28M',l1SeedThresholds=['eEM28M','PROBEeEM5'],groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_e30_lhloose_nopix_lrtmedium_probe_L1eEM28M',l1SeedThresholds=['eEM28M','PROBEeEM28M'],groups=TagAndProbePhIGroup+SingleElectronGroup),

        ChainProp(name='HLT_e26_lhtight_ivarloose_e30_lhloose_nopix_probe_L1eEM26M',l1SeedThresholds=['eEM26M','PROBEeEM26M'],groups=TagAndProbePhIGroup+SingleElectronGroup),

        # ATR-27373
        ChainProp(name='HLT_e28_lhtight_ivarloose_e30_lhloose_nopix_probe_L1eEM28M',l1SeedThresholds=['eEM28M','PROBEeEM28M'],groups=TagAndProbePhIGroup+SingleElectronGroup),

        #------------ ATR-23609
        ChainProp(name='HLT_e25_mergedtight_g35_medium_90invmAB_02dRAB_L12eEM24L', l1SeedThresholds=['eEM24L','eEM24L'], groups=PrimaryPhIGroup+MultiElectronGroup),

        

        ChainProp(name='HLT_e5_idperf_tight_L1eEM5', stream=[PhysicsStream,'express'], groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM5'], monGroups=['idMon:t0']),
        ChainProp(name='HLT_e5_idperf_tight_nogsf_L1eEM5', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM5']),

        #------------ ATR-25648:Raise threshold on Phase-I seeded LRT electrons
        ChainProp(name='HLT_e20_idperf_loose_lrtloose_L1eEM18L', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM18L'], monGroups=['idMon:t0']),
        ChainProp(name='HLT_e26_idperf_loose_L1eEM26M', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_e26_idperf_tight_nogsf_L1eEM26M', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_e26_idperf_tight_L1eEM26M', stream=[PhysicsStream,'express'], groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM26M'], monGroups=['idMon:shifter']),
        ChainProp(name='HLT_e60_idperf_medium_L1eEM26M', stream=[PhysicsStream,'express'], groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM26M'], monGroups=['idMon:t0']),
        ChainProp(name='HLT_e60_idperf_medium_nogsf_L1eEM26M', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_e14_idperf_loose_L1eEM9', stream=[PhysicsStream], groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM9']),
        ChainProp(name='HLT_e14_idperf_tight_L1eEM9', stream=[PhysicsStream], groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM9'], monGroups=['idMon:t0']),
        ChainProp(name='HLT_e14_idperf_tight_nogsf_L1eEM9', stream=[PhysicsStream], groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM9'], monGroups=['idMon:t0']),
        ChainProp(name='HLT_2e17_idperf_loose_L12eEM18M', groups=MultiElectronGroup+SupportPhIGroup+['RATE:CPS_2eEM18M']),
        ChainProp(name='HLT_2e17_idperf_loose_nogsf_L12eEM18M', groups=MultiElectronGroup+SupportPhIGroup+['RATE:CPS_2eEM18M']),
        ChainProp(name='HLT_e30_idperf_loose_lrtloose_L1eEM26M', stream=[PhysicsStream,'express'], groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM26M'], monGroups=['idMon:shifter']),

        # ATR-27373
        ChainProp(name='HLT_e28_idperf_loose_L1eEM28M', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_e28_idperf_tight_nogsf_L1eEM28M', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_e28_idperf_tight_L1eEM28M', stream=[PhysicsStream,'express'], groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM28M'], monGroups=['idMon:shifter']),
        ChainProp(name='HLT_e60_idperf_medium_L1eEM28M', stream=[PhysicsStream,'express'], groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM28M'], monGroups=['idMon:t0']),
        ChainProp(name='HLT_e60_idperf_medium_nogsf_L1eEM28M', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_e30_idperf_loose_lrtloose_L1eEM28M', stream=[PhysicsStream,'express'], groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM28M'], monGroups=['idMon:shifter']),

        ChainProp(name='HLT_e26_lhtight_e14_idperf_tight_probe_50invmAB130_L1eEM26M', stream=[PhysicsStream, 'express'], l1SeedThresholds=['eEM26M','PROBEeEM9'], groups=PrimaryPhIGroup+MultiElectronGroup, monGroups=['idMon:shifter']),
        ChainProp(name='HLT_e26_lhtight_e14_idperf_tight_nogsf_probe_50invmAB130_L1eEM26M', stream=[PhysicsStream, 'express'], l1SeedThresholds=['eEM26M','PROBEeEM9'], groups=PrimaryPhIGroup+MultiElectronGroup, monGroups=['idMon:t0']),
        ChainProp(name='HLT_e9_lhtight_e4_idperf_tight_probe_1invmAB5_L1JPSI-1M5-eEM9', stream=[PhysicsStream], l1SeedThresholds=['eEM9','PROBEeEM5'], groups=SupportPhIGroup+MultiElectronGroup+Topo2Group+['RATE:CPS_JPSI-1M5-eEM9']),
        ChainProp(name='HLT_e9_lhtight_e4_idperf_tight_nogsf_probe_1invmAB5_L1JPSI-1M5-eEM9', stream=[PhysicsStream], l1SeedThresholds=['eEM9','PROBEeEM5'], groups=SupportPhIGroup+MultiElectronGroup+Topo2Group+['RATE:CPS_JPSI-1M5-eEM9']),
        ChainProp(name='HLT_e14_lhtight_e4_idperf_tight_probe_1invmAB5_L1JPSI-1M5-eEM15', stream=[PhysicsStream,'express'], l1SeedThresholds=['eEM15','PROBEeEM5'], groups=SupportPhIGroup+MultiElectronGroup+Topo2Group+['RATE:CPS_JPSI-1M5-eEM15'], monGroups=['idMon:t0']),
        ChainProp(name='HLT_e14_lhtight_e4_idperf_tight_nogsf_probe_1invmAB5_L1JPSI-1M5-eEM15', l1SeedThresholds=['eEM15','PROBEeEM5'], groups=SupportPhIGroup+MultiElectronGroup+Topo2Group+['RATE:CPS_JPSI-1M5-eEM15']),

        # ATR-27373
        ChainProp(name='HLT_e28_lhtight_e14_idperf_tight_probe_50invmAB130_L1eEM28M', stream=[PhysicsStream, 'express'], l1SeedThresholds=['eEM28M','PROBEeEM9'], groups=PrimaryPhIGroup+MultiElectronGroup, monGroups=['idMon:shifter']),
        ChainProp(name='HLT_e28_lhtight_e14_idperf_tight_nogsf_probe_50invmAB130_L1eEM28M', stream=[PhysicsStream, 'express'], l1SeedThresholds=['eEM28M','PROBEeEM9'], groups=PrimaryPhIGroup+MultiElectronGroup, monGroups=['idMon:t0']),

        #----------- egamma Tag&Probe
        ChainProp(name='HLT_e26_lhtight_ivarloose_e4_etcut_probe_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeEM5'], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_topo']),
        ChainProp(name='HLT_e26_lhtight_ivarloose_e12_lhvloose_probe_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeEM12L'], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_topo']),
        ChainProp(name='HLT_e26_lhtight_ivarloose_e17_lhvloose_probe_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeEM18M'], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['egammaMon:online','egammaMon:shifter_topo']),
        ChainProp(name='HLT_e26_lhtight_ivarloose_e24_lhvloose_probe_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeEM24L'], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_topo']),
        ChainProp(name='HLT_e26_lhtight_ivarloose_e26_lhtight_probe_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeEM26M'], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_topo']),
        ChainProp(name='HLT_e26_lhtight_ivarloose_e14_idperf_tight_probe_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeEM9'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_e20_lhtight_ivarloose_probe_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeEM18M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_e15_idperf_tight_probe_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeEM9'], groups=TagAndProbePhIGroup+SingleElectronGroup),

        ChainProp(name='HLT_e26_lhtight_ivarloose_e14_lhtight_probe_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeEM9'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_e14_etcut_probe_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeEM9'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_e9_lhtight_probe_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeEM5'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_e9_etcut_probe_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeEM5'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_e5_lhtight_probe_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeEM5'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_e5_lhtight_noringer_probe_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeEM5'], groups=TagAndProbePhIGroup+SingleElectronGroup),

        # ATR-27373
        ChainProp(name='HLT_e28_lhtight_ivarloose_e4_etcut_probe_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeEM5'], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_topo']),
        ChainProp(name='HLT_e28_lhtight_ivarloose_e12_lhvloose_probe_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeEM12L'], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_topo']),
        ChainProp(name='HLT_e28_lhtight_ivarloose_e17_lhvloose_probe_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeEM18M'], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['egammaMon:online','egammaMon:shifter_topo']),
        ChainProp(name='HLT_e28_lhtight_ivarloose_e24_lhvloose_probe_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeEM24L'], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_topo']),
        ChainProp(name='HLT_e28_lhtight_ivarloose_e26_lhtight_probe_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeEM26M'], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_topo']),
        ChainProp(name='HLT_e28_lhtight_ivarloose_e28_lhtight_probe_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeEM28M'], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_topo']),
        ChainProp(name='HLT_e28_lhtight_ivarloose_e20_lhtight_ivarloose_probe_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeEM18M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_e15_idperf_tight_probe_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeEM9'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_e14_lhtight_probe_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeEM9'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_e14_etcut_probe_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeEM9'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_e9_lhtight_probe_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeEM5'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_e9_etcut_probe_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeEM5'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_e5_lhtight_probe_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeEM5'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_e5_lhtight_noringer_probe_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeEM5'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        
        #ATR-27251, Phase-I
        ChainProp(name='HLT_e28_lhtight_ivarloose_e14_idperf_tight_probe_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeEM9'], groups=TagAndProbePhIGroup+SingleElectronGroup),

        #------------ support validation of tag-and-probe mass cuts
        # Zee
        ChainProp(name='HLT_e26_lhtight_e14_etcut_probe_50invmAB130_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeEM9'], groups=PrimaryPhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_e26_lhtight_e14_etcut_L1eEM26M', l1SeedThresholds=['eEM26M','eEM9'], groups=SupportPhIGroup+MultiElectronGroup+['RATE:CPS_eEM26M']),

        # ATR-27373
        ChainProp(name='HLT_e28_lhtight_e14_etcut_probe_50invmAB130_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeEM9'], groups=PrimaryPhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_e28_lhtight_e14_etcut_L1eEM28M', l1SeedThresholds=['eEM28M','eEM9'], groups=SupportPhIGroup+MultiElectronGroup+['RATE:CPS_eEM28M']),


        ChainProp(name='HLT_e9_lhtight_e4_etcut_1invmAB5_L1JPSI-1M5-eEM9', stream=[PhysicsStream,'express'], l1SeedThresholds=['eEM9','eEM5'], groups=SupportPhIGroup+MultiElectronGroup+Topo2Group+['RATE:CPS_JPSI-1M5-eEM9'], monGroups=['egammaMon:shifter_topo']),
        ChainProp(name='HLT_e5_lhtight_e9_etcut_1invmAB5_L1JPSI-1M5-eEM9', stream=[PhysicsStream,'express'], l1SeedThresholds=['eEM5','eEM9'], groups=SupportPhIGroup+MultiElectronGroup+Topo2Group+['RATE:CPS_JPSI-1M5-eEM9'], monGroups=['egammaMon:shifter_topo']),
        ChainProp(name='HLT_e5_lhtight_e14_etcut_1invmAB5_L1JPSI-1M5-eEM15', stream=[PhysicsStream,'express'], l1SeedThresholds=['eEM5','eEM15'], groups=SupportPhIGroup+MultiElectronGroup+Topo2Group+['RATE:CPS_JPSI-1M5-eEM15'], monGroups=['egammaMon:shifter_topo']),
        ChainProp(name='HLT_e14_lhtight_e4_etcut_1invmAB5_L1JPSI-1M5-eEM15', stream=[PhysicsStream,'express'], l1SeedThresholds=['eEM15','eEM5'], groups=SupportPhIGroup+MultiElectronGroup+Topo2Group+['RATE:CPS_JPSI-1M5-eEM15'], monGroups=['egammaMon:shifter_topo']),
        # ATR-31076
        ChainProp(name='HLT_e9_lhtight_e4_nopid_1invmAB5_L1JPSI-1M5-eEM9', stream=[PhysicsStream,'express'], l1SeedThresholds=['eEM9','eEM5'], groups=SupportPhIGroup+MultiElectronGroup+Topo2Group+['RATE:CPS_JPSI-1M5-eEM9'], monGroups=['egammaMon:shifter_topo']), 
        ChainProp(name='HLT_e5_lhtight_e9_nopid_1invmAB5_L1JPSI-1M5-eEM9', stream=[PhysicsStream,'express'], l1SeedThresholds=['eEM5','eEM9'], groups=SupportPhIGroup+MultiElectronGroup+Topo2Group+['RATE:CPS_JPSI-1M5-eEM9'], monGroups=['egammaMon:shifter_topo']), 
        ChainProp(name='HLT_e5_lhtight_e14_nopid_1invmAB5_L1JPSI-1M5-eEM15', stream=[PhysicsStream,'express'], l1SeedThresholds=['eEM5','eEM15'], groups=SupportPhIGroup+MultiElectronGroup+Topo2Group+['RATE:CPS_JPSI-1M5-eEM15'], monGroups=['egammaMon:shifter_topo']), 
        ChainProp(name='HLT_e14_lhtight_e4_nopid_1invmAB5_L1JPSI-1M5-eEM15', stream=[PhysicsStream,'express'], l1SeedThresholds=['eEM15','eEM5'], groups=SupportPhIGroup+MultiElectronGroup+Topo2Group+['RATE:CPS_JPSI-1M5-eEM15'], monGroups=['egammaMon:shifter_topo']), 


        # ATR-24268, ATR-19501, ATR-28162
        # B->K*ee chains
        ChainProp(name='HLT_e5_lhvloose_bBeeM6000_L1BKeePrimary', l1SeedThresholds=['eEM5'], stream=['BphysDelayed','express'], groups=SupportPhIGroup+BphysElectronGroup, monGroups=['bphysMon:online']),
        ChainProp(name='HLT_2e5_lhvloose_bBeeM6000_L1BKeePrimary', l1SeedThresholds=['eEM5'], stream=['BphysDelayed','express'], groups=SupportPhIGroup+BphysElectronGroup, monGroups=['bphysMon:online','bphysMon:shifter']),
        ChainProp(name='HLT_e5_lhvloose_e3_lhvloose_bBeeM6000_L1BKeePrimary', l1SeedThresholds=['eEM5','eEM5'], stream=['BphysDelayed'], groups=SupportPhIGroup+BphysElectronGroup, monGroups=['bphysMon:online','bphysMon:shifter']),
        ChainProp(name='HLT_e5_lhvloose_bBeeM6000_L1BKeePrescaled', l1SeedThresholds=['eEM5'], stream=['BphysDelayed'], groups=EOFBeePhIGroup+BphysElectronGroup),
        ChainProp(name='HLT_2e5_lhvloose_bBeeM6000_L1BKeePrescaled', l1SeedThresholds=['eEM5'], stream=['BphysDelayed'], groups=EOFBeePhIGroup+BphysElectronGroup),
        ChainProp(name='HLT_e5_lhvloose_e3_lhvloose_bBeeM6000_L1BKeePrescaled', l1SeedThresholds=['eEM5','eEM5'], stream=['BphysDelayed'], groups=EOFBeePhIGroup+BphysElectronGroup),


        ChainProp(name='HLT_g35_medium_g25_medium_L1eEM15_EMPTY', l1SeedThresholds=['eEM15']*2, stream=['Late'], groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_g35_medium_g25_medium_L1eEM9_UNPAIRED_ISO', l1SeedThresholds=['eEM9']*2, stream=['Late'], groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_2g22_tight_L1eEM15_EMPTY', l1SeedThresholds=['eEM15'], stream=['Late'], groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_2g22_tight_L1eEM9_UNPAIRED_ISO', l1SeedThresholds=['eEM9'], stream=['Late'], groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_2g50_tight_L1eEM15_EMPTY', l1SeedThresholds=['eEM15'], stream=['Late'], groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_2g50_tight_L1eEM9_UNPAIRED_ISO', l1SeedThresholds=['eEM9'], stream=['Late'], groups=PrimaryPhIGroup+MultiPhotonGroup),

    ]

    chains['MET'] += [

        #ATR-28679
        ChainProp(name='HLT_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream,'express'], groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        #
        ChainProp(name='HLT_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream,'express'], groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),

        ChainProp(name='HLT_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),

        #ATR-28679
        ChainProp(name='HLT_xe65_cell_xe90_nn_L1jXE110', l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE110'], monGroups=['metMon:shifter']),
        ChainProp(name='HLT_xe65_cell_xe105_nn_L1jXE110', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream,'express'], groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:shifter']),

        ChainProp(name='HLT_xe75_cell_xe65_tcpufit_xe90_trkmht_L1jXE110', l1SeedThresholds=['FSNOSEED']*3, groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe60_cell_xe95_pfsum_cssk_L1jXE110', l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe55_cell_xe70_tcpufit_xe90_pfsum_vssk_L1jXE110', l1SeedThresholds=['FSNOSEED']*3, groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe105_mhtpufit_em_L1jXE110', l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe100_mhtpufit_pf_L1jXE110', l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe55_cell_xe70_tcpufit_xe95_pfsum_cssk_L1jXE110', l1SeedThresholds=['FSNOSEED']*3, groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe95_pfsum_vssk_L1jXE110', l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE110'], monGroups=['metMon:t0']),

        ChainProp(name='HLT_xe80_cell_xe115_tcpufit_L1jXE110',l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        #
        ChainProp(name='HLT_xe65_cell_xe90_nn_L1gXEJWOJ110', l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ110'], monGroups=['metMon:shifter']),
        ChainProp(name='HLT_xe65_cell_xe105_nn_L1gXEJWOJ110', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream,'express'], groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:shifter']),

        ChainProp(name='HLT_xe75_cell_xe65_tcpufit_xe90_trkmht_L1gXEJWOJ110', l1SeedThresholds=['FSNOSEED']*3, groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe60_cell_xe95_pfsum_cssk_L1gXEJWOJ110', l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe55_cell_xe70_tcpufit_xe90_pfsum_vssk_L1gXEJWOJ110', l1SeedThresholds=['FSNOSEED']*3, groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe105_mhtpufit_em_L1gXEJWOJ110', l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe100_mhtpufit_pf_L1gXEJWOJ110', l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe55_cell_xe70_tcpufit_xe95_pfsum_cssk_L1gXEJWOJ110', l1SeedThresholds=['FSNOSEED']*3, groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe95_pfsum_vssk_L1gXEJWOJ110', l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ110'], monGroups=['metMon:t0']),

        ChainProp(name='HLT_xe80_cell_xe115_tcpufit_L1gXEJWOJ110',l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),

        # ATR-25512
        ChainProp(name='HLT_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        
        ChainProp(name='HLT_xe75_cell_xe65_tcpufit_xe90_trkmht_L1jXE100', l1SeedThresholds=['FSNOSEED']*3, groups=SupportPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe60_cell_xe95_pfsum_cssk_L1jXE100', l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe55_cell_xe70_tcpufit_xe90_pfsum_vssk_L1jXE100', l1SeedThresholds=['FSNOSEED']*3, groups=SupportPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe105_mhtpufit_em_L1jXE100', l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe100_mhtpufit_pf_L1jXE100', l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe55_cell_xe70_tcpufit_xe95_pfsum_cssk_L1jXE100', l1SeedThresholds=['FSNOSEED']*3, groups=SupportPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe95_pfsum_vssk_L1jXE100', l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup, monGroups=['metMon:t0']),

        # ATR-28838
        ChainProp(name='HLT_xe55_cell_xe70_tcpufit_xe90_pfsum_vssk_L1gXEJWOJ100', l1SeedThresholds=['FSNOSEED']*3, groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ100'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe55_cell_xe70_tcpufit_xe95_pfsum_cssk_L1gXEJWOJ100', l1SeedThresholds=['FSNOSEED']*3, groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ100'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe60_cell_xe95_pfsum_cssk_L1gXEJWOJ100', l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ100'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe105_mhtpufit_em_L1gXEJWOJ100', l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ100'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe105_nn_L1gXEJWOJ100', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream,'express'], groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:shifter']),
        ChainProp(name='HLT_xe65_cell_xe90_nn_L1gXEJWOJ100', l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ100'], monGroups=['metMon:shifter']),
        ChainProp(name='HLT_xe65_cell_xe95_pfsum_vssk_L1gXEJWOJ100', l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ100'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe75_cell_xe65_tcpufit_xe90_trkmht_L1gXEJWOJ100', l1SeedThresholds=['FSNOSEED']*3, groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ100'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe55_cell_xe90_nn_L1gXEJWOJ100', l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ100'], monGroups=['metMon:shifter']),
        ChainProp(name='HLT_xe55_cell_xe105_nn_L1gXEJWOJ100', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:shifter']),

        ChainProp(name='HLT_xe65_cell_xe105_nn_L1jXE100', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream,'express'], groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:shifter']),
        ChainProp(name='HLT_xe65_cell_xe90_nn_L1jXE100', l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE100'], monGroups=['metMon:shifter']),
        ChainProp(name='HLT_xe55_cell_xe90_nn_L1jXE100', l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE100'], monGroups=['metMon:shifter']),
        ChainProp(name='HLT_xe55_cell_xe105_nn_L1jXE100', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:shifter']),


        ChainProp(name='HLT_xe30_cell_xe30_tcpufit_L1jXE60', l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup, monGroups=['metMon:t0']), #must be FS seeded
        ChainProp(name='HLT_xe30_cell_xe30_tcpufit_L1gXEJWOJ60', l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup, monGroups=['metMon:t0']), #must be FS seeded
        ChainProp(name='HLT_xe65_cell_xe110_tcpufit_L1jXE100',l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE100'], monGroups=['metMon:t0']), # Intended PS=-1
        ChainProp(name='HLT_xe65_cell_xe110_tcpufit_L1gXEJWOJ100',l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ100'], monGroups=['metMon:t0']), #ATR-28838
        ChainProp(name='HLT_xe80_cell_xe115_tcpufit_L1jXE100',l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),

        ChainProp(name='HLT_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        #ChainProp(name='HLT_xe65_cell_xe90_pfopufit_L1gXERHO100', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe90_pfopufit_L1gXENC100', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),

        ChainProp(name='HLT_xe65_cell_xe100_mhtpufit_pf_L1gXEJWOJ100', l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup, monGroups=['metMon:t0']),
        #ChainProp(name='HLT_xe65_cell_xe100_mhtpufit_pf_L1gXERHO100', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe100_mhtpufit_pf_L1gXENC100', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),

        ChainProp(name='HLT_xe80_cell_xe115_tcpufit_L1gXEJWOJ100',l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        #ChainProp(name='HLT_xe80_cell_xe115_tcpufit_L1gXERHO100',l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe80_cell_xe115_tcpufit_L1gXENC100',l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),

        ChainProp(name='HLT_xe55_cell_xe70_tcpufit_L1jXE100',l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream, 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE100'], monGroups=['metMon:shifter']),
        ChainProp(name='HLT_xe55_cell_xe70_tcpufit_L1jXE110',l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream, 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE110'], monGroups=['metMon:t0']), #ATR-28679
        ChainProp(name='HLT_xe55_cell_xe70_tcpufit_L1gXEJWOJ110',l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream, 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ110'], monGroups=['metMon:t0']), #ATR-28679
        ChainProp(name='HLT_xe55_cell_xe70_tcpufit_L1gXEJWOJ100',l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream, 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ100'], monGroups=['metMon:t0']), #ATR-28838
        
        #ATR-28679
        ChainProp(name='HLT_xe65_cell_xe90_pfopufit_sig30_L1jXE110',l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe110_tcpufit_sig30_L1jXE110',l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe80_cell_xe115_tcpufit_sig30_L1jXE110',l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE110'], monGroups=['metMon:t0']),

        ChainProp(name='HLT_xe65_cell_xe90_pfopufit_sig30_L1gXEJWOJ110',l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe110_tcpufit_sig30_L1gXEJWOJ110',l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe80_cell_xe115_tcpufit_sig30_L1gXEJWOJ110',l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ110'], monGroups=['metMon:t0']),

        ChainProp(name='HLT_xe65_cell_xe90_pfopufit_sig30_L1gXEJWOJ100',l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ100'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe110_tcpufit_sig30_L1gXEJWOJ100',l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ100'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe80_cell_xe115_tcpufit_sig30_L1gXEJWOJ100',l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ100'], monGroups=['metMon:t0']),

        # ATR-28910 L1_ZB to L1_ZeroBias
        ChainProp(name='HLT_xe0_cell_L1ZeroBias',  l1SeedThresholds=['FSNOSEED'], stream=['ZeroBias'], groups=SupportPhIGroup+METGroup+['RATE:ZeroBias','RATE:CPS_ZB']), #ATR-26456
        ChainProp(name='HLT_xe0_tcpufit_L1ZeroBias',  l1SeedThresholds=['FSNOSEED'], stream=['ZeroBias'], groups=SupportPhIGroup+METGroup+['RATE:ZeroBias','RATE:CPS_ZB']),
        ChainProp(name='HLT_xe0_trkmht_L1ZeroBias',  l1SeedThresholds=['FSNOSEED'], stream=['ZeroBias'], groups=SupportPhIGroup+METGroup+['RATE:ZeroBias','RATE:CPS_ZB']),
        ChainProp(name='HLT_xe0_pfopufit_L1ZeroBias',  l1SeedThresholds=['FSNOSEED'], stream=['ZeroBias'], groups=SupportPhIGroup+METGroup+['RATE:ZeroBias','RATE:CPS_ZB']),
        ChainProp(name='HLT_xe0_pfsum_L1ZeroBias',  l1SeedThresholds=['FSNOSEED'], stream=['ZeroBias'], groups=SupportPhIGroup+METGroup+['RATE:ZeroBias','RATE:CPS_ZB']),
        ChainProp(name='HLT_xe0_pfsum_cssk_L1ZeroBias',  l1SeedThresholds=['FSNOSEED'], stream=['ZeroBias'], groups=SupportPhIGroup+METGroup+['RATE:ZeroBias','RATE:CPS_ZB']),
        ChainProp(name='HLT_xe0_pfsum_vssk_L1ZeroBias',  l1SeedThresholds=['FSNOSEED'], stream=['ZeroBias'], groups=SupportPhIGroup+METGroup+['RATE:ZeroBias','RATE:CPS_ZB']),
        ChainProp(name='HLT_xe0_mhtpufit_em_L1ZeroBias',  l1SeedThresholds=['FSNOSEED'], stream=['ZeroBias'], groups=SupportPhIGroup+METGroup+['RATE:ZeroBias','RATE:CPS_ZB']),
        ChainProp(name='HLT_xe0_mhtpufit_pf_L1ZeroBias',  l1SeedThresholds=['FSNOSEED'], stream=['ZeroBias'], groups=SupportPhIGroup+METGroup+['RATE:ZeroBias','RATE:CPS_ZB']),
        ChainProp(name='HLT_xe0_nn_L1ZeroBias',  l1SeedThresholds=['FSNOSEED'], stream=['ZeroBias'], groups=SupportPhIGroup+METGroup+['RATE:ZeroBias','RATE:CPS_ZB']),

        #ATR-28679
        ChainProp(name='HLT_xe0_cell_L1jXE110',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_tcpufit_L1jXE110',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_trkmht_L1jXE110',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_pfopufit_L1jXE110',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_pfsum_L1jXE110',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_pfsum_cssk_L1jXE110',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_pfsum_vssk_L1jXE110',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_mhtpufit_em_L1jXE110',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_mhtpufit_pf_L1jXE110',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_nn_L1jXE110',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE110'], monGroups=['metMon:t0']),

        ChainProp(name='HLT_xe0_cell_L1gXEJWOJ110',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_tcpufit_L1gXEJWOJ110',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_trkmht_L1gXEJWOJ110',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_pfopufit_L1gXEJWOJ110',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_pfsum_L1gXEJWOJ110',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_pfsum_cssk_L1gXEJWOJ110',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_pfsum_vssk_L1gXEJWOJ110',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_mhtpufit_em_L1gXEJWOJ110',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_mhtpufit_pf_L1gXEJWOJ110',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_nn_L1gXEJWOJ110',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ110'], monGroups=['metMon:t0']),
        # ATR-28838
        ChainProp(name='HLT_xe0_cell_L1jXE100',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE100'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_tcpufit_L1jXE100',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE100'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_trkmht_L1jXE100',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE100'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_pfopufit_L1jXE100',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE100'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_pfsum_L1jXE100',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE100'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_pfsum_cssk_L1jXE100',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE100'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_pfsum_vssk_L1jXE100',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE100'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_mhtpufit_em_L1jXE100',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE100'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_mhtpufit_pf_L1jXE100',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE100'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_nn_L1jXE100',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE100'], monGroups=['metMon:t0']),

        ChainProp(name='HLT_xe0_cell_L1gXEJWOJ100',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ100'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_tcpufit_L1gXEJWOJ100',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ100'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_trkmht_L1gXEJWOJ100',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ100'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_pfopufit_L1gXEJWOJ100',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ100'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_pfsum_L1gXEJWOJ100',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ100'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_pfsum_cssk_L1gXEJWOJ100',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ100'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_pfsum_vssk_L1gXEJWOJ100',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ100'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_mhtpufit_em_L1gXEJWOJ100',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ100'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_mhtpufit_pf_L1gXEJWOJ100',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ100'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_nn_L1gXEJWOJ100',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ100'], monGroups=['metMon:t0']),


        
    ]

    chains['Jet'] += [
        # High ET chain
        ChainProp(name='HLT_j30_CLEANllp_momemfrac006_calratio_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j30_CLEANllp_momemfrac006_calratiormbib_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        # Calratio backup for jJ160
        ChainProp(name='HLT_j30_CLEANllp_momemfrac006_calratio_L1eTAU140', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j30_CLEANllp_momemfrac006_calratiormbib_L1eTAU140', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j30_CLEANllp_momemfrac006_calratio_L1eTAU80', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j30_CLEANllp_momemfrac006_calratiormbib_L1eTAU80', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        # Calratio Legacy support chains
        ChainProp(name='HLT_j30_CLEANllp_momemfrac006_calratio_L1eTAU60_EMPTY', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j30_CLEANllp_momemfrac006_calratiormbib_L1eTAU60_EMPTY', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j30_CLEANllp_momemfrac006_calratio_L1eTAU60_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j30_CLEANllp_momemfrac006_calratiormbib_L1eTAU60_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        # new version of triggers with pileup recalculation, should be prescaled 
        ChainProp(name='HLT_j30_CLEANllp_momemfrac012_calratiovar_roiftf_preselj20emf12_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        # Calratio LLP-NoMATCH analogs
        ChainProp(name='HLT_j30_CLEANllp_momemfrac006_calratio_L1eTAU40HT', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j30_CLEANllp_momemfrac006_calratiormbib_L1eTAU40HT', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j30_CLEANllp_momemfrac006_calratio_L1eTAU60HM', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j30_CLEANllp_momemfrac006_calratiormbib_L1eTAU60HM', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),

        # Support performance chains (for emulation+calibration studies) ATR-20624
        ChainProp(name='HLT_j0_perf_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_j0_perf_pf_ftf_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),

        # Central single small-R jet chains
        ## PFlow calibration triggers
        ChainProp(name='HLT_j15_pf_ftf_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_j25_pf_ftf_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_j35_pf_ftf_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_j45_pf_ftf_preselj20_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),
        ## EMTopo calibration/monitoring triggers, same threshold as PF for rate evaluation
        ChainProp(name='HLT_j15_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_j25_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_j35_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_j45_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),
        #ATR-27257, Phase-I
        ChainProp(name='HLT_j60_L1jJ50', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ50']),     
        ChainProp(name='HLT_j85_L1jJ50', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ50'], monGroups=['jetMon:t0','detMon']),

        ## Phase-I EMTopo calibration/monitoring triggers, same threshold as PF for rate evaluation
        ChainProp(name='HLT_j110_L1jJ60', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ60']),
        ChainProp(name='HLT_j175_L1jJ90', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ90']),
        ChainProp(name='HLT_j260_L1jJ125', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ125']),
        ChainProp(name='HLT_j360_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ160']),      
        ## Phase-I Monitoring triggers
        ### Phase-I no presel mon
        ChainProp(name='HLT_j400_pf_ftf_L1jJ160', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ160'], monGroups=['jetMon:t0']), 

        #ATR-27257, Phase-I
        ChainProp(name='HLT_j400_pf_ftf_preselj225_L1jJ180', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup), 

        ## EMTopo back-up primary triggers (ATR-20049, ATR-23152)
        ## Phase-I EMTopo back-up primary triggers (ATR-20049, ATR-23152)
        ChainProp(name='HLT_j420_L1jJ180', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SupportPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j440_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j450_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j460_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j480_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j500_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j520_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ## EMTopo+FTF back-up primary triggers
        ## Phase-I EMTopo+FTF back-up primary triggers
        ChainProp(name='HLT_j420_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ160']),
        ChainProp(name='HLT_j440_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j450_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j460_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j480_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j500_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j520_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),

           
        #Forward small-R EMTopo chains (ATR-20049, ATR-24720)
        ChainProp(name='HLT_j15f_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_j25f_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_j35f_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),
        
        # Small-R multijet chains
        ## PFlow primaries
        ChainProp(name='HLT_5j65c_pf_ftf_presel5c50_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),
        # ATR-25512
        ChainProp(name='HLT_5j65c_pf_ftf_presel5c55_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=MultiJetGroup + PrimaryPhIGroup, monGroups=['jetMon:t0']), 
        
        ## FTF+EMTopo
        ChainProp(name='HLT_4j115_ftf_presel4j85_L13jJ90', l1SeedThresholds=['FSNOSEED'],            groups=MultiJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_5j70c_ftf_presel5j50_L14jJ40', l1SeedThresholds=['FSNOSEED'],     groups=MultiJetGroup+SupportPhIGroup+['RATE:CPS_4jJ40']),
        ChainProp(name='HLT_5j85_ftf_presel5j50_L14jJ40', l1SeedThresholds=['FSNOSEED'],             groups=MultiJetGroup+SupportPhIGroup+['RATE:CPS_4jJ40']),
        ChainProp(name='HLT_6j55c_ftf_presel6j40_L14jJ40', l1SeedThresholds=['FSNOSEED'],     groups=MultiJetGroup+SupportPhIGroup+['RATE:CPS_4jJ40']),
        ChainProp(name='HLT_6j70_ftf_presel6j40_L14jJ40', l1SeedThresholds=['FSNOSEED'],             groups=MultiJetGroup+SupportPhIGroup+['RATE:CPS_4jJ40']),
        ChainProp(name='HLT_7j45_ftf_presel7j30_L14jJ40', l1SeedThresholds=['FSNOSEED'],             groups=MultiJetGroup+SupportPhIGroup+['RATE:CPS_4jJ40']),
        ChainProp(name='HLT_10j40_ftf_presel7j30_L14jJ40', l1SeedThresholds=['FSNOSEED'],            groups=MultiJetGroup+SupportPhIGroup+['RATE:CPS_4jJ40']),
        ## Phase-I FTF+EMTopo
        ChainProp(name='HLT_2j250c_j120c_ftf_presel2j180XXj80_L1jJ160', l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup+SupportPhIGroup+['RATE:CPS_jJ160']),
        ChainProp(name='HLT_3j200_ftf_presel3j150_L1jJ160', l1SeedThresholds=['FSNOSEED'],           groups=MultiJetGroup+SupportPhIGroup+['RATE:CPS_jJ160']),
        ## Monitoring triggers
        ### no presel
        ### no jvt
        ChainProp(name='HLT_6j35c_pf_ftf_presel6c25_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=MultiJetGroup+SupportPhIGroup+['RATE:CPS_4jJ40'], monGroups=['jetMon:t0']),
        ### Phase-I no presel
        ChainProp(name='HLT_3j190_pf_ftf_L1jJ160', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MultiJetGroup+SupportPhIGroup+['RATE:CPS_jJ160'], monGroups=['jetMon:t0']),
        ### Phase-I no mass cut
        ChainProp(name='HLT_2j330_a10t_lcw_jes_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+SupportPhIGroup+['RATE:CPS_jJ160'], monGroups=['jetMon:t0']),
        ChainProp(name='HLT_2j330_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+SupportPhIGroup+Topo3Group+['RATE:CPS_SC175-SCjJ10'], monGroups=['jetMon:t0']),
        # Central |eta| < 2.1 chains (ATR-31092)
        ChainProp(name='HLT_2j330C_a10t_lcw_jes_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+SupportPhIGroup+['RATE:CPS_jJ160'], monGroups=['jetMon:t0']),
        ChainProp(name='HLT_2j330C_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+SupportPhIGroup+Topo3Group+['RATE:CPS_SC175-SCjJ10'], monGroups=['jetMon:t0']),


        ## ATR-25456 - Emerging jet
        # [ATR-26377] Backup emerging jet chain
        ChainProp(name='HLT_j200_0eta180_emergingPTF0p08dR1p2_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25',l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j200_0eta160_emergingPTF0p08dR1p2_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25',l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j200_0eta180_emergingPTF0p08dR1p2_a10sd_cssk_pf_jes_ftf_preselj200_L1SC175-SCjJ10',l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+Topo3Group+['RATE:CPS_SC175-SCjJ10']),
        ChainProp(name='HLT_j200_0eta160_emergingPTF0p08dR1p2_a10sd_cssk_pf_jes_ftf_preselj200_L1SC175-SCjJ10',l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+Topo3Group+['RATE:CPS_SC175-SCjJ10']),
        # Phase-I primary emerging jets single-jet chain
        ChainProp(name='HLT_j175_0eta180_emergingPTF0p08dR1p2_a10sd_cssk_pf_jes_ftf_preselj225_L1jJ160', groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ160'], l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j200_0eta180_emergingPTF0p08dR1p2_a10sd_cssk_pf_jes_ftf_preselj200_L1jJ160', groups=SingleJetGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED']),
        # Phase-I Backup emerging jet chain
        ChainProp(name='HLT_j200_0eta160_emergingPTF0p08dR1p2_a10sd_cssk_pf_jes_ftf_preselj200_L1jJ160', groups=SingleJetGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED']),
        
        ## ATR-29738 - Delayed Jets Triggers
        # Single-Jet
        ChainProp(name='HLT_j200_2timing15_L1jJ160', groups=SingleJetGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream]),
        ChainProp(name='HLT_j200_3timeSig15_L1jJ160', groups=SingleJetGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream]),
        ChainProp(name='HLT_j250_2timing15_L1jJ160', groups=SingleJetGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream]),
        ChainProp(name='HLT_j250_3timeSig15_L1jJ160', groups=SingleJetGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream]),
        # Multi-Jet
        ChainProp(name='HLT_2j45_2j55_3timeSig15_L14jJ40', groups=MultiJetGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream]),
        ChainProp(name='HLT_2j100_2timeSig15_L1jJ90', groups=MultiJetGroup+SupportPhIGroup, l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream]),
        ChainProp(name='HLT_j220_j150_2timing15_L1jJ160', groups=MultiJetGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream]), 
        
        # ATR-21596 HT Delayed for Dark Showers 
        ChainProp(name='HLT_j0_HT650XX0eta240_pf_ftf_preselcHT450_L1HT190-jJ40s5pETA21', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SupportPhIGroup+SingleJetGroup+Topo3Group),
        # ATR-21596 HT chain for DarkPEBTLA
        # ATR-31134 Move this chain to PrimaryPhIGroup
        ChainProp(name='HLT_j0_HT500XX0eta240_pf_ftf_preselcHT450_DarkJetPEBTLA_L1HT190-jJ40s5pETA21', l1SeedThresholds=['FSNOSEED'], stream=['DarkJetPEBTLA'],groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group+['RATE:CPS_HT190-jJ40s5pETA21']),
        # FullBuild support
        ChainProp(name='HLT_j0_HT500XX0eta240_pf_ftf_preselcHT450_L1HT190-jJ40s5pETA21', l1SeedThresholds=['FSNOSEED'],groups=SupportPhIGroup+MultiJetGroup+Topo3Group+['RATE:CPS_HT190-jJ40s5pETA21']),

        # ATR-25512

        # Multijet delayed stream
        ChainProp(name='HLT_6j20c_020jvt_pf_ftf_preselZ142XX5c20_PhysicsTLA_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['tlaMon:shifter']), 
        ChainProp(name='HLT_6j20c_020jvt_pf_ftf_preselZ182XX6c20_PhysicsTLA_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['tlaMon:shifter']),    
        ChainProp(name='HLT_6j20c_020jvt_pf_ftf_preselZ197XX6c20_PhysicsTLA_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['tlaMon:shifter']),        



        ## TLA chains
        # Phase-I TLA chains with PFlow
        ChainProp(name='HLT_j20_pf_ftf_preselj140_PhysicsTLA_L1jJ90',  l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=EOFTLAPhIGroup+SingleJetGroup, monGroups=['tlaMon:shifter']),
        # TLA chains with PFlow, ATR-21594
        ChainProp(name='HLT_j20_pf_ftf_preselcHT450_PhysicsTLA_L1HT190-jJ40s5pETA21', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=MultiJetGroup+PrimaryPhIGroup+Topo3Group, monGroups=['tlaMon:shifter']),
        # MultiJet TLA support for intensity ramp up
        # legacy preselection chain to be dropped at end of 2024
        ChainProp(name='HLT_2j20_2j20_pf_ftf_presel2c20XX2c20bgtwo85_PhysicsTLA_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*2, stream=['TLA'], groups=MultiJetGroup+SupportPhIGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_2j20_2j20_pf_ftf_presel3c20XX1c20bgtwo85_PhysicsTLA_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*2, stream=['TLA'], groups=MultiJetGroup+PrimaryPhIGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_j60_j45_j25_j20_pf_ftf_preselc60XXc45XXc25XXc20_PhysicsTLA_L1jJ85p0ETA21_3jJ40p0ETA25',l1SeedThresholds=['FSNOSEED']*4, stream=['TLA'], groups=MultiJetGroup+SupportPhIGroup, monGroups=['tlaMon:shifter']),
        # FTag PEB
        ChainProp(name='HLT_2j20_2j20_pf_ftf_presel2c20XX2c20bgtwo85_FTagPEBTLA_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*2, stream=['FTagPEBTLA'], groups=MultiJetGroup+SupportPhIGroup, monGroups=['tlaMon:shifter']),
        # Flavour Neutral TLA
        ChainProp(name='HLT_j20_j20_pf_ftf_preselj140XX2j45_PhysicsTLA_L1jJ85p0ETA21_3jJ40p0ETA25'       , l1SeedThresholds=['FSNOSEED', 'FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+MultiJetGroup),
        ChainProp(name='HLT_j20_j20_pf_ftf_preselj100XX2j45_PhysicsTLA_L1jJ85p0ETA21_3jJ40p0ETA25'       , l1SeedThresholds=['FSNOSEED', 'FSNOSEED'], stream=['TLA'], groups=EOFTLAPhIGroup+MultiJetGroup),
        ChainProp(name='HLT_j20_j20_pf_ftf_preselj120XX2j45_PhysicsTLA_L1jJ85p0ETA21_3jJ40p0ETA25'       , l1SeedThresholds=['FSNOSEED', 'FSNOSEED'], stream=['TLA'], groups=EOFTLAPhIGroup+MultiJetGroup),


        # ATR-25512
        # ATR-25512 Duplicating with PhaseI items
        ChainProp(name='HLT_j20_pf_ftf_preselj180_PhysicsTLA_L1jJ160', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+SingleJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_j20_pf_ftf_preselj190_PhysicsTLA_L1jJ160', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+SingleJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_j20_pf_ftf_preselj200_PhysicsTLA_L1jJ160', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+SingleJetGroup, monGroups=['tlaMon:shifter']),

        #ATR-24411 Phase I inputs
        # Single jet support 
        ChainProp(name='HLT_j45_pf_ftf_preselj20_L1jJ40', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ40'], monGroups=['idMon:shifter','jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j60_pf_ftf_preselj50_L1jJ50', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ50'], monGroups=['jetMon:shifter', 'jetMon:online']),
        ChainProp(name='HLT_j85_pf_ftf_preselj50_L1jJ50', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ50'], monGroups=['jetMon:t0']),
        ChainProp(name='HLT_j110_pf_ftf_preselj80_L1jJ60', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ60']),
        ChainProp(name='HLT_j175_pf_ftf_preselj140_L1jJ90', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ90']),
        ChainProp(name='HLT_j260_pf_ftf_preselj200_L1jJ125', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ125']),
        ChainProp(name='HLT_j360_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ160']),
        ChainProp(name='HLT_j400_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:shifter','jetMon:online', 'caloMon:t0']),
        ChainProp(name='HLT_j440_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j450_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j460_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j480_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j500_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j520_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        
        # Central single large-R jets
        ChainProp(name='HLT_j110_a10sd_cssk_pf_jes_ftf_preselj80_L1jJ60', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ60']),
        ChainProp(name='HLT_j110_a10t_lcw_jes_L1jJ60', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ60']),
        ChainProp(name='HLT_j175_a10sd_cssk_pf_jes_ftf_preselj140_L1jJ90', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ90'], monGroups=['bJetMon:shifter', 'jetMon:online']),
        ChainProp(name='HLT_j175_a10t_lcw_jes_L1jJ90', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ90']),
        ChainProp(name='HLT_j260_a10sd_cssk_pf_jes_ftf_preselj200_L1jJ125', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ125']),
        ChainProp(name='HLT_j260_a10t_lcw_jes_L1jJ125', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ125']),
        ChainProp(name='HLT_j360_a10sd_cssk_pf_jes_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ160']),
        ChainProp(name='HLT_j360_a10sd_cssk_pf_jes_ftf_preselj225_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+Topo3Group+['RATE:CPS_SC175-SCjJ10']),
        ChainProp(name='HLT_j360_a10t_lcw_jes_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ160']),
        ChainProp(name='HLT_j360_a10t_lcw_jes_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+Topo3Group+['RATE:CPS_SC175-SCjJ10']),
        ChainProp(name='HLT_j400_a10sd_cssk_pf_jes_ftf_preselj225_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+Topo3Group+['RATE:CPS_SC175-SCjJ10']),
        ChainProp(name='HLT_j400_a10t_lcw_jes_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ160']),
        ChainProp(name='HLT_j400_a10t_lcw_jes_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+Topo3Group+['RATE:CPS_SC175-SCjJ10']),
        ChainProp(name='HLT_j420_a10sd_cssk_pf_jes_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ160']),
        ChainProp(name='HLT_j420_a10sd_cssk_pf_jes_ftf_preselj225_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+Topo3Group+['RATE:CPS_SC175-SCjJ10']),
        ChainProp(name='HLT_j420_a10t_lcw_jes_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ160']),
        ChainProp(name='HLT_j420_a10t_lcw_jes_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+Topo3Group+['RATE:CPS_SC175-SCjJ10']),
        ChainProp(name='HLT_j460_a10sd_cssk_pf_jes_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:shifter','jetMon:online']),
        ChainProp(name='HLT_j460_a10sd_cssk_pf_jes_ftf_preselj225_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup+Topo3Group),
        ChainProp(name='HLT_j460_a10t_lcw_jes_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_j460_a10t_lcw_jes_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup+Topo3Group),
        ChainProp(name='HLT_j480_a10sd_cssk_pf_jes_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j480_a10sd_cssk_pf_jes_ftf_preselj225_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup+Topo3Group),
        ChainProp(name='HLT_j480_a10t_lcw_jes_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j480_a10t_lcw_jes_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup+Topo3Group),
        # Low threshold large-R chains (for calibration purposes)
        ChainProp(name='HLT_j85_a10sd_cssk_pf_nojcalib_ftf_preselj50_L1jJ50', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ50']),
        ChainProp(name='HLT_j85_a10sd_cssk_pf_jes_ftf_preselj50_L1jJ50', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ50']),
        ChainProp(name='HLT_j85_a10t_lcw_nojcalib_L1jJ50', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ50']),
        ChainProp(name='HLT_j85_a10t_lcw_jes_L1jJ50',      l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ50']),

        # Central |eta| < 2.1 single large-R jets (ATR-31092)
        ChainProp(name='HLT_j110C_a10sd_cssk_pf_jes_ftf_preselj80_L1jJ60', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ60']),
        ChainProp(name='HLT_j110C_a10t_lcw_jes_L1jJ60', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ60']),
        ChainProp(name='HLT_j175C_a10sd_cssk_pf_jes_ftf_preselj140_L1jJ90', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ90'], monGroups=['bJetMon:shifter', 'jetMon:online']),
        ChainProp(name='HLT_j175C_a10t_lcw_jes_L1jJ90', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ90']),
        ChainProp(name='HLT_j260C_a10sd_cssk_pf_jes_ftf_preselj200_L1jJ125', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ125']),
        ChainProp(name='HLT_j260C_a10t_lcw_jes_L1jJ125', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ125']),
        ChainProp(name='HLT_j360C_a10sd_cssk_pf_jes_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ160']),
        ChainProp(name='HLT_j360C_a10sd_cssk_pf_jes_ftf_preselj225_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+Topo3Group+['RATE:CPS_SC175-SCjJ10']),
        ChainProp(name='HLT_j360C_a10t_lcw_jes_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ160']),
        ChainProp(name='HLT_j360C_a10t_lcw_jes_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+Topo3Group+['RATE:CPS_SC175-SCjJ10']),
        ChainProp(name='HLT_j400C_a10sd_cssk_pf_jes_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ160']),
        ChainProp(name='HLT_j400C_a10sd_cssk_pf_jes_ftf_preselj225_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+Topo3Group+['RATE:CPS_SC175-SCjJ10']),
        ChainProp(name='HLT_j400C_a10t_lcw_jes_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ160']),
        ChainProp(name='HLT_j400C_a10t_lcw_jes_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+Topo3Group+['RATE:CPS_SC175-SCjJ10']),
        ChainProp(name='HLT_j420C_a10sd_cssk_pf_jes_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ160']),
        ChainProp(name='HLT_j420C_a10sd_cssk_pf_jes_ftf_preselj225_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+Topo3Group+['RATE:CPS_SC175-SCjJ10']),
        ChainProp(name='HLT_j420C_a10t_lcw_jes_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ160']),
        ChainProp(name='HLT_j420C_a10t_lcw_jes_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+Topo3Group+['RATE:CPS_SC175-SCjJ10']),
        ChainProp(name='HLT_j460C_a10sd_cssk_pf_jes_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:shifter','jetMon:online']),
        ChainProp(name='HLT_j460C_a10sd_cssk_pf_jes_ftf_preselj225_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup+Topo3Group),
        ChainProp(name='HLT_j460C_a10t_lcw_jes_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_j460C_a10t_lcw_jes_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup+Topo3Group),
        ChainProp(name='HLT_j480C_a10sd_cssk_pf_jes_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j480C_a10sd_cssk_pf_jes_ftf_preselj225_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup+Topo3Group),
        ChainProp(name='HLT_j480C_a10t_lcw_jes_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j480C_a10t_lcw_jes_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup+Topo3Group),
        # Low threshold |eta| < 2.1 large-R chains (for calibration purposes)
        ChainProp(name='HLT_j85C_a10sd_cssk_pf_nojcalib_ftf_preselj50_L1jJ50', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ50']),
        ChainProp(name='HLT_j85C_a10sd_cssk_pf_jes_ftf_preselj50_L1jJ50', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ50']),
        ChainProp(name='HLT_j85C_a10t_lcw_nojcalib_L1jJ50', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ50']),
        ChainProp(name='HLT_j85C_a10t_lcw_jes_L1jJ50',      l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ50']),

        #Forward small-R EMTopo chains
        ChainProp(name='HLT_j45f_L1jJ40p30ETA49', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup, monGroups=['caloMon:t0']),
        ChainProp(name='HLT_j60f_L1jJ50p30ETA49', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ50p30ETA49']),
        ChainProp(name='HLT_j85f_L1jJ50p30ETA49', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ50p30ETA49']),
        ChainProp(name='HLT_j110f_L1jJ60p30ETA49', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j175f_L1jJ90p30ETA49', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j280f_L1jJ125p30ETA49', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j300f_L1jJ125p30ETA49', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),

        # ATR-20049
        ChainProp(name='HLT_j420_L1jJ160', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SupportPhIGroup+SingleJetGroup, monGroups=['jetMon:t0','caloMon:t0']),
        ChainProp(name='HLT_j220f_L1jJ125p30ETA49', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=PrimaryPhIGroup+SingleJetGroup, monGroups=['jetMon:shifter', 'jetMon:online', 'caloMon:t0']),
        ChainProp(name='HLT_j230f_L1jJ125p30ETA49', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j240f_L1jJ125p30ETA49', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j260f_L1jJ125p30ETA49', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j460_a10_lcw_subjes_L1SC175-SCjJ10',         l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup+Topo3Group),
        ChainProp(name='HLT_j460_a10r_L1jJ160', l1SeedThresholds=['FSNOSEED'],  groups=PrimaryPhIGroup+SingleJetGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_j460_a10r_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'],  groups=PrimaryPhIGroup+SingleJetGroup+Topo3Group),
        ChainProp(name='HLT_j460_a10_lcw_subjes_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_j420_35smcINF_a10t_lcw_jes_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup, monGroups=['jetMon:shifter']),
        ChainProp(name='HLT_2j330_35smcINF_a10t_lcw_jes_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_j420_35smcINF_a10sd_cssk_pf_jes_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_2j330_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+PrimaryPhIGroup, monGroups=['jetMon:t0']),

        ChainProp(name='HLT_j420_35smcINF_a10t_lcw_jes_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup+Topo3Group),
        ChainProp(name='HLT_2j330_35smcINF_a10t_lcw_jes_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),
        ChainProp(name='HLT_j420_35smcINF_a10sd_cssk_pf_jes_ftf_preselj225_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SingleJetGroup+PrimaryPhIGroup+Topo3Group, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_2j330_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+PrimaryPhIGroup+Topo3Group, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_j360_60smcINF_j360_a10t_lcw_jes_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),
        ChainProp(name='HLT_j370_35smcINF_j370_a10t_lcw_jes_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),
        ChainProp(name='HLT_j360_60smcINF_j360_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),
        ChainProp(name='HLT_j370_35smcINF_j370_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),

        # Central |eta| < 2.1 chains (ATR-31092)
        ChainProp(name='HLT_j460C_a10_lcw_subjes_L1SC175-SCjJ10',         l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup+Topo3Group),
        ChainProp(name='HLT_j460C_a10r_L1jJ160', l1SeedThresholds=['FSNOSEED'],  groups=PrimaryPhIGroup+SingleJetGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_j460C_a10r_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'],  groups=PrimaryPhIGroup+SingleJetGroup+Topo3Group),
        ChainProp(name='HLT_j460C_a10_lcw_subjes_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_j420C_35smcINF_a10t_lcw_jes_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup, monGroups=['jetMon:shifter']),
        ChainProp(name='HLT_2j330C_35smcINF_a10t_lcw_jes_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_j420C_35smcINF_a10sd_cssk_pf_jes_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_2j330C_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+PrimaryPhIGroup, monGroups=['jetMon:t0']),

        ChainProp(name='HLT_j420C_35smcINF_a10t_lcw_jes_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup+Topo3Group),
        ChainProp(name='HLT_2j330C_35smcINF_a10t_lcw_jes_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),
        ChainProp(name='HLT_j420C_35smcINF_a10sd_cssk_pf_jes_ftf_preselj225_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SingleJetGroup+PrimaryPhIGroup+Topo3Group, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_2j330C_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+PrimaryPhIGroup+Topo3Group, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_j360C_60smcINF_j360_a10t_lcw_jes_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),
        ChainProp(name='HLT_j370C_35smcINF_j370_a10t_lcw_jes_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),
        ChainProp(name='HLT_j360C_60smcINF_j360_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),
        ChainProp(name='HLT_j370C_35smcINF_j370_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),

        # Small-R multijet chains
        # PFlow primaries
        ChainProp(name='HLT_2j235c_j115c_pf_ftf_presel2j180XXj80_L1jJ160', l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup + PrimaryPhIGroup), 
        ChainProp(name='HLT_3j190_pf_ftf_presel3j150_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup), 
        ChainProp(name='HLT_4j110_pf_ftf_presel4j85_L13jJ90', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=MultiJetGroup + PrimaryPhIGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_5j70c_pf_ftf_presel5j50_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),
        # ATR-25512
        ChainProp(name='HLT_5j70c_pf_ftf_presel5j55_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_5j80_pf_ftf_presel5j50_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=MultiJetGroup + PrimaryPhIGroup, monGroups=['jetMon:t0']),
        # ATR-25512
        ChainProp(name='HLT_5j80_pf_ftf_presel5j55_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup), 
        ChainProp(name='HLT_6j55c_pf_ftf_presel6j40_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),
        # ATR-25512
        ChainProp(name='HLT_6j55c_pf_ftf_presel6c45_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_6j65_pf_ftf_presel6j40_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup), 
        # ATR-25512
        ChainProp(name='HLT_6j65_pf_ftf_presel6j45_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup), 
        ChainProp(name='HLT_7j45_pf_ftf_presel7j30_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_10j35_pf_ftf_presel7j30_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup), 
        
        # EMTopo backups
        ChainProp(name='HLT_3j200_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + SupportPhIGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_4j120_L13jJ90', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=MultiJetGroup + SupportPhIGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_5j70c_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + SupportPhIGroup),
        ChainProp(name='HLT_5j85_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + SupportPhIGroup),
        ChainProp(name='HLT_6j55c_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + SupportPhIGroup),
        ChainProp(name='HLT_6j70_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + SupportPhIGroup),
        ChainProp(name='HLT_7j45_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + SupportPhIGroup),
        ChainProp(name='HLT_10j40_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + SupportPhIGroup),

        #HT chains
        ChainProp(name='HLT_j0_HT1000_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup, monGroups=['jetMon:shifter','jetMon:online']),
        ChainProp(name='HLT_j0_HT1000_L1HT190-jJ40s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+SingleJetGroup+Topo3Group, monGroups=['jetMon:shifter']),
        ChainProp(name='HLT_j0_HT940_pf_ftf_preselj180_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        
        # ATR-25512
        ChainProp(name='HLT_j0_HT940_pf_ftf_preselj190_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j0_HT940_pf_ftf_preselj200_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        
        ChainProp(name='HLT_j0_HT940_pf_ftf_preselj180_L1HT190-jJ40s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup+Topo3Group, monGroups=['jetMon:online', 'jetMon:shifter']),
        ChainProp(name='HLT_j0_HT940_pf_ftf_preselcHT450_L1HT190-jJ40s5pETA21', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=PrimaryPhIGroup+SingleJetGroup+Topo3Group, monGroups=['jetMon:online', 'jetMon:shifter']), 

        # Multijet delayed stream
        ChainProp(name='HLT_6j35c_nnJvtv1_pf_ftf_presel6c25_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream, 'express'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_6j45c_nnJvtv1_pf_ftf_presel6c25_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiJetGroup),

        ChainProp(name='HLT_6j35c_020jvt_pf_ftf_presel6c25_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream, 'express'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_6j45c_020jvt_pf_ftf_presel6c25_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiJetGroup),

        # TLA chains
        ChainProp(name='HLT_j20_PhysicsTLA_L1jJ160', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+SingleJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_j20_PhysicsTLA_L1jJ90_DETA20-jJ90J', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=EOFTLAPhIGroup+SingleJetGroup+Topo3Group, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_j20_PhysicsTLA_L1HT190-jJ40s5pETA21', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+SingleJetGroup+Topo3Group, monGroups=['tlaMon:shifter']),

        ChainProp(name='HLT_j70_j50a_j0_DJMASS1000j50dphi200x400deta_L1jMJJ-500-NFF', l1SeedThresholds=['FSNOSEED']*3,stream=['VBFDelayed'],groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),

        # Low-threshold calibration Large-R jets

        # Large R primary jet chains with gLJ L1 items (L1J100->L1gLJ140p0ETA25)
        ChainProp(name='HLT_j460_a10sd_cssk_pf_jes_ftf_preselj225_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:shifter', 'jetMon:online']),
        ChainProp(name='HLT_j460_a10t_lcw_jes_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j460_a10r_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'],  groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j460_a10_lcw_subjes_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j420_35smcINF_a10t_lcw_jes_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_2j330_35smcINF_a10t_lcw_jes_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j420_35smcINF_a10sd_cssk_pf_jes_ftf_preselj225_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_2j330_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:t0']),

        # Central |eta| < 2.1 chains (ATR-31092)
        ChainProp(name='HLT_j460C_a10sd_cssk_pf_jes_ftf_preselj225_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:shifter', 'jetMon:online']),
        ChainProp(name='HLT_j460C_a10t_lcw_jes_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j460C_a10r_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'],  groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j460C_a10_lcw_subjes_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j420C_35smcINF_a10t_lcw_jes_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_2j330C_35smcINF_a10t_lcw_jes_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j420C_35smcINF_a10sd_cssk_pf_jes_ftf_preselj225_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_2j330C_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:t0']),

        # Small R single-jet primaries with gJ L1 items (L1J100 -> L1gJ400p0ETA25)
        ChainProp(name='HLT_j420_pf_ftf_preselj225_L1gJ400p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_3j200_pf_ftf_presel3j150_L1gJ400p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_3j200_L1gJ400p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_j0_HT940_pf_ftf_preselj180_L1gJ400p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup), 
        ChainProp(name='HLT_j0_HT1000_L1gJ400p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),

        # Triggers for gFEX commissioning
        ChainProp(name='HLT_j60_L1gJ20p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_gJ20p0ETA25']),
        ChainProp(name='HLT_j85_L1gJ20p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_gJ20p0ETA25']),
        ChainProp(name='HLT_j110_L1gJ20p25ETA49', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j175_L1gJ50p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j360_L1gJ100p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),

        ChainProp(name='HLT_j110_a10sd_cssk_pf_jes_ftf_preselj80_L1gLJ80p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_gLJ80p0ETA25']),
        ChainProp(name='HLT_j110_a10t_lcw_jes_L1gLJ80p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_gLJ80p0ETA25']),
        ChainProp(name='HLT_j175_a10sd_cssk_pf_jes_ftf_preselj140_L1gLJ100p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_gLJ100p0ETA25'], monGroups=['bJetMon:shifter', 'jetMon:online']),
        ChainProp(name='HLT_j175_a10t_lcw_jes_L1gLJ100p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_gLJ100p0ETA25']),

        # Central |eta| < 2.1 chains (ATR-31092)
        ChainProp(name='HLT_j110C_a10sd_cssk_pf_jes_ftf_preselj80_L1gLJ80p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_gLJ80p0ETA25']),
        ChainProp(name='HLT_j110C_a10t_lcw_jes_L1gLJ80p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_gLJ80p0ETA25']),
        ChainProp(name='HLT_j175C_a10sd_cssk_pf_jes_ftf_preselj140_L1gLJ100p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_gLJ100p0ETA25'], monGroups=['bJetMon:shifter', 'jetMon:online']),
        ChainProp(name='HLT_j175C_a10t_lcw_jes_L1gLJ100p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_gLJ100p0ETA25']),

         # low threshold single jet support chains with JVT
        ChainProp(name='HLT_j25_nnJvtv1_pf_ftf_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_j35_nnJvtv1_pf_ftf_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_j45_nnJvtv1_pf_ftf_preselj20_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),

        ChainProp(name='HLT_j25_020jvt_pf_ftf_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_j35_020jvt_pf_ftf_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_j45_020jvt_pf_ftf_preselj20_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),


        # ATR-28563
        ChainProp(name='HLT_j20_calratiovar_roiftf_preselj20emf24_L1LLPDPHI-jXE40-jJ40', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup+Topo2Group+['PS:NoHLTRepro']),
        ChainProp(name='HLT_j20_calratiovar_roiftf_preselj20emf24_L1LLPNODPHI-jXE40-jJ40', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+Topo2Group+['PS:NoHLTRepro']),




    ]

    chains['Bjet'] += [
        # Legacy L1Calo primaries with gn1
        #  Single b-jet

        # HH4b primaries
        # Muon+jet legacy seeded, backup for L1Topo muon-in-jet
        # Support chain
        ChainProp(name='HLT_j75c_nnJvtv1_j50c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_boffperf_pf_ftf_presel3c20XX1c20bgtwo85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*4, stream=[PhysicsStream],  monGroups=['idMon:t0'], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_boffperf_pf_ftf_presel3c20XX1c20bgtwo85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*4, stream=[PhysicsStream],  monGroups=['idMon:t0'], groups=SupportPhIGroup+MultiBjetGroup),

        # Delayed multijet+b
        ChainProp(name='HLT_j75c_nnJvtv1_j50c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_boffperf_pf_ftf_presel2c20XX2c20bgtwo85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*4, stream=[PhysicsStream],  monGroups=['idMon:t0'], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_boffperf_pf_ftf_presel2c20XX2c20bgtwo85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*4, stream=[PhysicsStream],  monGroups=['idMon:t0'], groups=SupportPhIGroup+MultiBjetGroup),

        # Support chain
        ChainProp(name='HLT_5j35c_nnJvtv1_j25c_nnJvtv1_SHARED_j25c_nnJvtv1_boffperf_pf_ftf_presel6c25_L14jJ40', l1SeedThresholds=['FSNOSEED']*3, stream=['VBFDelayed'], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_5j35c_020jvt_j25c_020jvt_SHARED_j25c_020jvt_boffperf_pf_ftf_presel6c25_L14jJ40', l1SeedThresholds=['FSNOSEED']*3, stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),

        # Phase I inputs GN1
        #  Single b-jet

        # support chains
        ChainProp(name='HLT_j20_0eta290_nnJvtv1_boffperf_pf_ftf_L1jJ30', l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+SingleBjetGroup),
        ChainProp(name='HLT_j30_0eta290_nnJvtv1_boffperf_pf_ftf_L1jJ50', l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+SingleBjetGroup, monGroups=['bJetMon:t0','idMon:t0']),
        ChainProp(name='HLT_j45_0eta290_nnJvtv1_boffperf_pf_ftf_L1jJ50', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SupportPhIGroup+SingleBjetGroup, monGroups=['bJetMon:shifter','idMon:shifter']),
        ChainProp(name='HLT_j60_0eta290_nnJvtv1_boffperf_pf_ftf_L1jJ90', l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+SingleBjetGroup, monGroups=['bJetMon:t0','idMon:t0']),
        ChainProp(name='HLT_j80_0eta290_nnJvtv1_boffperf_pf_ftf_L1jJ90', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SupportPhIGroup+SingleBjetGroup, monGroups=['bJetMon:t0','idMon:t0']),
        ChainProp(name='HLT_j100_0eta290_nnJvtv1_boffperf_pf_ftf_preselj80_L1jJ90', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SupportPhIGroup+SingleBjetGroup, monGroups=['bJetMon:t0','idMon:t0']),
        ChainProp(name='HLT_j150_0eta290_nnJvtv1_boffperf_pf_ftf_preselj120_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+SingleBjetGroup, monGroups=['bJetMon:t0','idMon:t0']),
        ChainProp(name='HLT_j200_0eta290_nnJvtv1_boffperf_pf_ftf_preselj140_L1jJ160', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SupportPhIGroup+SingleBjetGroup, monGroups=['bJetMon:shifter']),
        ChainProp(name='HLT_j300_0eta290_nnJvtv1_boffperf_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SupportPhIGroup+SingleBjetGroup, monGroups=['bJetMon:t0','idMon:shifter']),        

        ChainProp(name='HLT_j20_0eta290_020jvt_boffperf_pf_ftf_L1jJ30', l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+SingleBjetGroup),
        ChainProp(name='HLT_j30_0eta290_020jvt_boffperf_pf_ftf_L1jJ50', l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+SingleBjetGroup, monGroups=['bJetMon:t0','idMon:t0']),
        ChainProp(name='HLT_j45_0eta290_020jvt_boffperf_pf_ftf_L1jJ50', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SupportPhIGroup+SingleBjetGroup, monGroups=['bJetMon:shifter','idMon:shifter']),
        ChainProp(name='HLT_j60_0eta290_020jvt_boffperf_pf_ftf_L1jJ90', l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+SingleBjetGroup, monGroups=['bJetMon:t0','idMon:t0']),
        ChainProp(name='HLT_j80_0eta290_020jvt_boffperf_pf_ftf_L1jJ90', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SupportPhIGroup+SingleBjetGroup, monGroups=['bJetMon:t0','idMon:t0']),
        ChainProp(name='HLT_j100_0eta290_020jvt_boffperf_pf_ftf_preselj80_L1jJ90', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SupportPhIGroup+SingleBjetGroup, monGroups=['bJetMon:t0','idMon:t0']),
        ChainProp(name='HLT_j150_0eta290_020jvt_boffperf_pf_ftf_preselj120_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+SingleBjetGroup, monGroups=['bJetMon:t0','idMon:t0']),
        ChainProp(name='HLT_j200_0eta290_020jvt_boffperf_pf_ftf_preselj140_L1jJ160', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SupportPhIGroup+SingleBjetGroup, monGroups=['bJetMon:shifter']),
        ChainProp(name='HLT_j300_0eta290_020jvt_boffperf_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SupportPhIGroup+SingleBjetGroup, monGroups=['bJetMon:t0','idMon:shifter']),


        # GN2
        # HH4b primaries
        # Muon+jet legacy seeded, backup for L1Topo muon-in-jet
        ChainProp(name='HLT_j75c_nnJvtv1_j50c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_2j20c_nnJvtv1_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_L1MU8F_2jJ40_jJ50', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_nnJvtv1_j50c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_2j20c_nnJvtv1_bgn280_pf_ftf_presel2c20XX2c20bgtwo85_L1MU8F_2jJ40_jJ50', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_nnJvtv1_j50c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_2j20c_nnJvtv1_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L1MU8F_2jJ40_jJ50', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_nnJvtv1_j50c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_2j20c_nnJvtv1_bgn280_pf_ftf_presel3c20XX1c20bgtwo85_L1MU8F_2jJ40_jJ50', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),

        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_L1MU8F_2jJ40_jJ50', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn280_pf_ftf_presel2c20XX2c20bgtwo85_L1MU8F_2jJ40_jJ50', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L1MU8F_2jJ40_jJ50', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn280_pf_ftf_presel3c20XX1c20bgtwo85_L1MU8F_2jJ40_jJ50', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),

        ChainProp(name='HLT_2j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*4, groups=EOFMultiBjetGroup+MultiBjetGroup),
        ChainProp(name='HLT_2j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*4, groups=EOFMultiBjetGroup+MultiBjetGroup),
        ChainProp(name='HLT_2j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn280_pf_ftf_presel3c20XX1c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*4, groups=EOFMultiBjetGroup+MultiBjetGroup),
        ChainProp(name='HLT_2j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn280_pf_ftf_presel3c20XX1c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*4, groups=EOFMultiBjetGroup+MultiBjetGroup),

        ChainProp(name='HLT_j50c_020jvt_2j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*4, groups=EOFMultiBjetGroup+MultiBjetGroup),
        ChainProp(name='HLT_j40c_020jvt_2j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*4, groups=EOFMultiBjetGroup+MultiBjetGroup),
        ChainProp(name='HLT_j50c_020jvt_2j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn280_pf_ftf_presel3c20XX1c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*4, groups=EOFMultiBjetGroup+MultiBjetGroup),
        ChainProp(name='HLT_j40c_020jvt_2j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn280_pf_ftf_presel3c20XX1c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*4, groups=EOFMultiBjetGroup+MultiBjetGroup),
        
        # Phase I inputs GN2
        #  Single b-jet
        ChainProp(name="HLT_j210_0eta290_nnJvtv1_bgn270_pf_ftf_preselj180_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup, monGroups=['bJetMon:online']),
        ChainProp(name="HLT_j280_0eta290_nnJvtv1_bgn277_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_j340_0eta290_nnJvtv1_bgn285_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),

        ChainProp(name="HLT_j210_0eta290_020jvt_bgn270_pf_ftf_preselj180_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup, monGroups=['bJetMon:online']),
        ChainProp(name="HLT_j280_0eta290_020jvt_bgn277_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_j340_0eta290_020jvt_bgn285_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        # Backup
        ChainProp(name="HLT_j210_0eta290_nnJvtv1_bgn270_pf_ftf_preselj190_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup, monGroups=['bJetMon:online']),
        ChainProp(name="HLT_j210_0eta290_nnJvtv1_bgn270_pf_ftf_preselj200_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup, monGroups=['bJetMon:online']),
        ChainProp(name="HLT_j210_0eta290_nnJvtv1_bgn260_pf_ftf_preselj180_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_j255_0eta290_nnJvtv1_bgn270_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_j280_0eta290_nnJvtv1_bgn270_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_j340_0eta290_nnJvtv1_bgn277_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),

        ChainProp(name="HLT_j210_0eta290_020jvt_bgn270_pf_ftf_preselj190_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup, monGroups=['bJetMon:online']),
        ChainProp(name="HLT_j210_0eta290_020jvt_bgn270_pf_ftf_preselj200_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup, monGroups=['bJetMon:online']),
        ChainProp(name="HLT_j210_0eta290_020jvt_bgn260_pf_ftf_preselj180_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_j255_0eta290_020jvt_bgn270_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_j280_0eta290_020jvt_bgn270_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_j340_0eta290_020jvt_bgn277_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        # Looser b-tagging
        ChainProp(name="HLT_j225_0eta290_nnJvtv1_bgn277_pf_ftf_preselj180_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name='HLT_j275_0eta290_nnJvtv1_bgn285_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name='HLT_j300_0eta290_nnJvtv1_bgn285_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
    
        ChainProp(name="HLT_j225_0eta290_020jvt_bgn277_pf_ftf_preselj180_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name='HLT_j275_0eta290_020jvt_bgn285_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name='HLT_j300_0eta290_020jvt_bgn285_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),

        # Multi-b
        ChainProp(name="HLT_3j60_0eta290_nnJvtv1_bgn277_pf_ftf_presel3j45bgtwo95_L13jJ70p0ETA23", l1SeedThresholds=['FSNOSEED'], groups=MultiBjetGroup + PrimaryPhIGroup),
        ChainProp(name="HLT_4j35_0eta290_nnJvtv1_bgn277_pf_ftf_presel4j25bgtwo95_L14jJ40p0ETA25", l1SeedThresholds=['FSNOSEED'], groups=MultiBjetGroup + PrimaryPhIGroup),
        ChainProp(name="HLT_3j35_0eta290_nnJvtv1_bgn270_j35_pf_ftf_presel2j25XX2j25bgtwo85_L14jJ40p0ETA25",      l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j35_0eta290_nnJvtv1_bgn270_2j35_0eta290_nnJvtv1_bgn285_pf_ftf_presel4j25bgtwo95_L14jJ40p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j50_0eta290_nnJvtv1_bgn260_2j50_pf_ftf_presel2j25XX2j25bgtwo85_L14jJ40p0ETA25",        l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j35_0eta290_nnJvtv1_bgn260_3j35_pf_ftf_presel3j25XX2j25bgtwo85_L15jJ40p0ETA25",  l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j45_0eta290_nnJvtv1_bgn260_3j45_pf_ftf_presel3j25XX2j25bgtwo85_L15jJ40p0ETA25",  l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_j70_0eta290_nnJvtv1_bgn260_3j70_pf_ftf_preselj50bgtwo85XX3j50_L14jJ50",           l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j45_0eta290_nnJvtv1_bgn260_2j45_pf_ftf_presel2j25XX2j25bgtwo85_L14jJ40p0ETA25",  l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),

        ChainProp(name="HLT_3j60_0eta290_020jvt_bgn277_pf_ftf_presel3j45bgtwo95_L13jJ70p0ETA23", l1SeedThresholds=['FSNOSEED'], groups=MultiBjetGroup + PrimaryPhIGroup),
        ChainProp(name="HLT_4j35_0eta290_020jvt_bgn277_pf_ftf_presel4j25bgtwo95_L14jJ40p0ETA25", l1SeedThresholds=['FSNOSEED'], groups=MultiBjetGroup + PrimaryPhIGroup),
        ChainProp(name="HLT_3j35_0eta290_020jvt_bgn270_j35_pf_ftf_presel2j25XX2j25bgtwo85_L14jJ40p0ETA25",      l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j35_0eta290_020jvt_bgn270_2j35_0eta290_020jvt_bgn285_pf_ftf_presel4j25bgtwo95_L14jJ40p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j50_0eta290_020jvt_bgn260_2j50_pf_ftf_presel2j25XX2j25bgtwo85_L14jJ40p0ETA25",        l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j35_0eta290_020jvt_bgn260_3j35_pf_ftf_presel3j25XX2j25bgtwo85_L15jJ40p0ETA25",  l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j45_0eta290_020jvt_bgn260_3j45_pf_ftf_presel3j25XX2j25bgtwo85_L15jJ40p0ETA25",  l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_j70_0eta290_020jvt_bgn260_3j70_pf_ftf_preselj50bgtwo85XX3j50_L14jJ50",           l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j45_0eta290_020jvt_bgn260_2j45_pf_ftf_presel2j25XX2j25bgtwo85_L14jJ40p0ETA25",  l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        # Asymmetric, 1j + 2b
        ChainProp(name="HLT_j140_2j50_0eta290_nnJvtv1_bgn270_pf_ftf_preselj80XX2j45bgtwo90_L1jJ140_3jJ60", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),

        ChainProp(name="HLT_j140_2j50_0eta290_020jvt_bgn270_pf_ftf_preselj80XX2j45bgtwo90_L1jJ140_3jJ60", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        # Asymmetric 2b
        ChainProp(name="HLT_j165_0eta290_nnJvtv1_bgn260_j55_0eta290_nnJvtv1_bgn260_pf_ftf_preselj140bgtwo85XXj45bgtwo85_L1jJ160", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),

        ChainProp(name="HLT_j165_0eta290_020jvt_bgn260_j55_0eta290_020jvt_bgn260_pf_ftf_preselj140bgtwo85XXj45bgtwo85_L1jJ160", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        # Run 2 HH4b low-threshold chain
        ChainProp(name="HLT_2j35c_nnJvtv1_bgn260_2j35c_nnJvtv1_pf_ftf_presel2j25XX2j25bgtwo85_L14jJ40p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),

        ChainProp(name="HLT_2j35c_020jvt_bgn260_2j35c_020jvt_pf_ftf_presel2j25XX2j25bgtwo85_L14jJ40p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),

        # HT-seeded
        ChainProp(name='HLT_2j45_0eta290_nnJvtv1_bgn270_j0_HT290_j0_DJMASS700j35_pf_ftf_L1HT150-jJ50s5pETA32_jMJJ-400-CF', l1SeedThresholds=['FSNOSEED']*3, groups=PrimaryPhIGroup+MultiBjetGroup+Topo3Group),

        ChainProp(name='HLT_2j45_0eta290_020jvt_bgn270_j0_HT290_j0_DJMASS700j35_pf_ftf_L1HT150-jJ50s5pETA32_jMJJ-400-CF', l1SeedThresholds=['FSNOSEED']*3, groups=PrimaryPhIGroup+MultiBjetGroup+Topo3Group),

        # VBF chains
        ChainProp(name='HLT_j75c_j55_j45f_SHARED_2j45_0eta290_nnJvtv1_bgn260_pf_ftf_preselc60XXj45XXf40_L1jJ80p0ETA25_2jJ55_jJ50p30ETA49', l1SeedThresholds=['FSNOSEED']*4, groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_j75_0eta290_nnJvtv1_bgn270_j55_0eta290_nnJvtv1_bgn285_j45f_pf_ftf_preselj60XXj45XXf40_L1jJ80p0ETA25_2jJ55_jJ50p30ETA49", l1SeedThresholds=['FSNOSEED']*3,stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_j50_0eta290_nnJvtv1_bgn270_2j45f_pf_ftf_preselj45XX2f40_L1jJ55p0ETA23_2jJ40p30ETA49",l1SeedThresholds=['FSNOSEED']*2,  stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j65a_j45a_2j35a_SHARED_2j35_0eta290_nnJvtv1_bgn270_j0_DJMASS1000j50_pf_ftf_presela60XXa40XX2a25_L1jMJJ-500-NFF', l1SeedThresholds=['FSNOSEED']*5,stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiBjetGroup+Topo3Group),

        ChainProp(name='HLT_j75c_j55_j45f_SHARED_2j45_0eta290_020jvt_bgn260_pf_ftf_preselc60XXj45XXf40_L1jJ80p0ETA25_2jJ55_jJ50p30ETA49', l1SeedThresholds=['FSNOSEED']*4, groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_j75_0eta290_020jvt_bgn270_j55_0eta290_020jvt_bgn285_j45f_pf_ftf_preselj60XXj45XXf40_L1jJ80p0ETA25_2jJ55_jJ50p30ETA49", l1SeedThresholds=['FSNOSEED']*3,stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_j50_0eta290_020jvt_bgn270_2j45f_pf_ftf_preselj45XX2f40_L1jJ55p0ETA23_2jJ40p30ETA49",l1SeedThresholds=['FSNOSEED']*2,  stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j65a_j45a_2j35a_SHARED_2j35_0eta290_020jvt_bgn270_j0_DJMASS1000j50_pf_ftf_presela60XXa40XX2a25_L1jMJJ-500-NFF', l1SeedThresholds=['FSNOSEED']*5,stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiBjetGroup+Topo3Group),

        # HH4b primary triggers
        # 3b asymmetric b-jet pt for Physics_Main
        ChainProp(name='HLT_j75c_nnJvtv1_j50c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_3j20c_nnJvtv1_bgn282_pf_ftf_presel2c20XX2c20bgtwo85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_nnJvtv1_j50c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_3j20c_nnJvtv1_bgn282_pf_ftf_presel3c20XX1c20bgtwo85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),

        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn282_pf_ftf_presel2c20XX2c20bgtwo85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn282_pf_ftf_presel3c20XX1c20bgtwo85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),
        # 2b asymmetric b-jet pt for VBFDelayed
        ChainProp(name='HLT_j75c_nnJvtv1_j50c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_2j20c_nnJvtv1_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], monGroups=['idMon:t0'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_nnJvtv1_j50c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_2j20c_nnJvtv1_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], monGroups=['idMon:t0'], groups=PrimaryPhIGroup+MultiBjetGroup),

        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], monGroups=['idMon:t0'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], monGroups=['idMon:t0'], groups=PrimaryPhIGroup+MultiBjetGroup),
        # 2b asymmetric b-jet pt for with looser b-tagging (ATR-28870)
        ChainProp(name='HLT_j75c_nnJvtv1_j50c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_2j20c_nnJvtv1_bgn280_pf_ftf_presel2c20XX2c20bgtwo85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], monGroups=['idMon:t0'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_nnJvtv1_j50c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_2j20c_nnJvtv1_bgn280_pf_ftf_presel3c20XX1c20bgtwo85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], monGroups=['idMon:t0'], groups=PrimaryPhIGroup+MultiBjetGroup),

        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn280_pf_ftf_presel2c20XX2c20bgtwo85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], monGroups=['idMon:t0'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn280_pf_ftf_presel3c20XX1c20bgtwo85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], monGroups=['idMon:t0'], groups=PrimaryPhIGroup+MultiBjetGroup),
        # Candidates for allhad ttbar delayed stream
        ChainProp(name='HLT_5j35c_nnJvtv1_j25c_nnJvtv1_SHARED_j25c_nnJvtv1_bgn260_pf_ftf_presel5c25XXc25bgtwo85_L14jJ40', l1SeedThresholds=['FSNOSEED']*3, stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_5j45c_nnJvtv1_j25c_nnJvtv1_SHARED_j25c_nnJvtv1_bgn260_pf_ftf_presel5c25XXc25bgtwo85_L14jJ40', l1SeedThresholds=['FSNOSEED']*3, stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),

        ChainProp(name='HLT_5j35c_020jvt_j25c_020jvt_SHARED_j25c_020jvt_bgn260_pf_ftf_presel5c25XXc25bgtwo85_L14jJ40', l1SeedThresholds=['FSNOSEED']*3, stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_5j45c_020jvt_j25c_020jvt_SHARED_j25c_020jvt_bgn260_pf_ftf_presel5c25XXc25bgtwo85_L14jJ40', l1SeedThresholds=['FSNOSEED']*3, stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),


        #HH4b EoF chains
        #ATR-29523
        ChainProp(name='HLT_2j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*4, stream=[PhysicsStream], groups=EOFMultiBjetGroup+MultiBjetGroup),
        ChainProp(name='HLT_2j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*4, stream=[PhysicsStream], groups=EOFMultiBjetGroup+MultiBjetGroup),
        ChainProp(name='HLT_2j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn280_pf_ftf_presel2c20XX2c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*4, stream=[PhysicsStream], groups=EOFMultiBjetGroup+MultiBjetGroup),
        ChainProp(name='HLT_2j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn280_pf_ftf_presel2c20XX2c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*4, stream=[PhysicsStream], groups=EOFMultiBjetGroup+MultiBjetGroup),
        ChainProp(name='HLT_j50c_020jvt_2j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*4, stream=[PhysicsStream], groups=EOFMultiBjetGroup+MultiBjetGroup),
        ChainProp(name='HLT_j40c_020jvt_2j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*4, stream=[PhysicsStream], groups=EOFMultiBjetGroup+MultiBjetGroup),
        ChainProp(name='HLT_j50c_020jvt_2j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn280_pf_ftf_presel2c20XX2c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*4, stream=[PhysicsStream], groups=EOFMultiBjetGroup+MultiBjetGroup),
        ChainProp(name='HLT_j40c_020jvt_2j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn280_pf_ftf_presel2c20XX2c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*4, stream=[PhysicsStream], groups=EOFMultiBjetGroup+MultiBjetGroup),

        # ATR-30800 X to bb GN2X
        ChainProp(name='HLT_j175_35smcINF_86bgntwox_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=PrimaryPhIGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED'], monGroups=['jetMon:online', 'bJetMon:t0']),
        ChainProp(name='HLT_j260_35smcINF_86bgntwox_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=PrimaryPhIGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED'], monGroups=['jetMon:online', 'bJetMon:t0']),
        ChainProp(name='HLT_j175_35smcINF_86bgntwox_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1jJ160', groups=PrimaryPhIGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED'], monGroups=['jetMon:online', 'bJetMon:t0']),
        ChainProp(name='HLT_j175_35smcINF_86bgntwox_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=PrimaryPhIGroup+MultiBjetGroup+Topo3Group, l1SeedThresholds=2*['FSNOSEED'], monGroups=['jetMon:online', 'bJetMon:t0']),
        ChainProp(name='HLT_j260_35smcINF_86bgntwox_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1jJ160', groups=PrimaryPhIGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED'], monGroups=['jetMon:online', 'bJetMon:t0']),
        ChainProp(name='HLT_j260_35smcINF_86bgntwox_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=PrimaryPhIGroup+MultiBjetGroup+Topo3Group, l1SeedThresholds=2*['FSNOSEED'], monGroups=['jetMon:online', 'bJetMon:t0']),
        ChainProp(name='HLT_j175_35smcINF_90bgntwoxt_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=PrimaryPhIGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_90bgntwoxt_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=PrimaryPhIGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_90bgntwoxt_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=PrimaryPhIGroup+MultiBjetGroup+Topo3Group, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_90bgntwoxt_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=PrimaryPhIGroup+MultiBjetGroup+Topo3Group, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_90bgntwoxt_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1jJ160', groups=PrimaryPhIGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_90bgntwoxt_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1jJ160', groups=PrimaryPhIGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),

        # Central |eta| < 2.1 chains (ATR-31092)
        ChainProp(name='HLT_j175C_35smcINF_86bgntwox_j260C_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=PrimaryPhIGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED'], monGroups=['jetMon:online', 'bJetMon:t0']),
        ChainProp(name='HLT_j260C_35smcINF_86bgntwox_j175C_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=PrimaryPhIGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED'], monGroups=['jetMon:online', 'bJetMon:t0']),
        ChainProp(name='HLT_j175C_35smcINF_86bgntwox_j260C_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1jJ160', groups=PrimaryPhIGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED'], monGroups=['jetMon:online', 'bJetMon:t0']),
        ChainProp(name='HLT_j175C_35smcINF_86bgntwox_j260C_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=PrimaryPhIGroup+MultiBjetGroup+Topo3Group, l1SeedThresholds=2*['FSNOSEED'], monGroups=['jetMon:online', 'bJetMon:t0']),
        ChainProp(name='HLT_j260C_35smcINF_86bgntwox_j175C_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1jJ160', groups=PrimaryPhIGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED'], monGroups=['jetMon:online', 'bJetMon:t0']),
        ChainProp(name='HLT_j260C_35smcINF_86bgntwox_j175C_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=PrimaryPhIGroup+MultiBjetGroup+Topo3Group, l1SeedThresholds=2*['FSNOSEED'], monGroups=['jetMon:online', 'bJetMon:t0']),
        ChainProp(name='HLT_j175C_35smcINF_90bgntwoxt_j260C_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=PrimaryPhIGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260C_35smcINF_90bgntwoxt_j175C_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=PrimaryPhIGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175C_35smcINF_90bgntwoxt_j260C_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=PrimaryPhIGroup+MultiBjetGroup+Topo3Group, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260C_35smcINF_90bgntwoxt_j175C_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=PrimaryPhIGroup+MultiBjetGroup+Topo3Group, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175C_35smcINF_90bgntwoxt_j260C_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1jJ160', groups=PrimaryPhIGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260C_35smcINF_90bgntwoxt_j175C_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1jJ160', groups=PrimaryPhIGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
    ]


    chains['Tau'] += [
        # Single tau primaries
        ChainProp(name='HLT_tau160_mediumGNTau_L1eTAU140', stream=[PhysicsStream, 'express'], groups=PrimaryPhIGroup+SingleTauGroup, monGroups=['tauMon:online', 'tauMon:t0']),
        ChainProp(name='HLT_tau200_mediumGNTau_L1eTAU140', groups=PrimaryPhIGroup+SingleTauGroup),

        # Single tau LLP primaries (ATR-21754)
        ChainProp(name='HLT_tau180_mediumRNN_tracktwoLLP_L1eTAU140', stream=[PhysicsStream, 'express'], groups=PrimaryPhIGroup+SingleTauGroup, monGroups=['tauMon:shifter']),
        ChainProp(name='HLT_tau200_mediumRNN_tracktwoLLP_L1eTAU140', groups=PrimaryPhIGroup+SingleTauGroup),
        ChainProp(name='HLT_tau180_tightRNN_tracktwoLLP_L1eTAU140', groups=PrimaryPhIGroup+SingleTauGroup),
        ChainProp(name='HLT_tau200_tightRNN_tracktwoLLP_L1eTAU140', groups=PrimaryPhIGroup+SingleTauGroup),


        # Di-tau primaries and support chains
        ChainProp(name='HLT_tau35_mediumGNTau_tau25_mediumGNTau_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55', l1SeedThresholds=['cTAU30M', 'cTAU20M'], stream=[PhysicsStream, 'express'], groups=PrimaryPhIGroup+MultiTauGroup+Topo2Group, monGroups=['tauMon:online', 'tauMon:shifter']),
        ChainProp(name='HLT_tau35_mediumGNTau_tau25_mediumGNTau_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30LeTAU20L-jJ55', l1SeedThresholds=['cTAU30M', 'cTAU20M'], groups=PrimaryPhIGroup+MultiTauGroup+Topo2Group, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau35_mediumGNTau_tau25_mediumGNTau_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20', l1SeedThresholds=['cTAU30M', 'cTAU20M'], groups=EOFTauPhIGroup+MultiTauGroup+Topo2Group),
        ChainProp(name='HLT_tau35_mediumGNTau_tau25_mediumGNTau_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30LeTAU20L', l1SeedThresholds=['cTAU30M', 'cTAU20M'], groups=SupportPhIGroup+MultiTauGroup+Topo2Group),
        ChainProp(name='HLT_tau35_mediumGNTau_tau25_mediumGNTau_03dRAB_L1cTAU30M_2cTAU20M_4jJ30p0ETA25', l1SeedThresholds=['cTAU30M', 'cTAU20M'], groups=PrimaryPhIGroup+MultiTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau35_mediumGNTau_tau25_mediumGNTau_03dRAB_L1cTAU30M_2cTAU20M', l1SeedThresholds=['cTAU30M', 'cTAU20M'], groups=SupportPhIGroup+MultiTauGroup),

        ChainProp(name='HLT_tau40_mediumGNTau_tau35_mediumGNTau_03dRAB_L1cTAU35M_2cTAU30M_2jJ55_3jJ50', l1SeedThresholds=['cTAU35M', 'cTAU30M'], groups=PrimaryPhIGroup+MultiTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau40_mediumGNTau_tau35_mediumGNTau_03dRAB_L1cTAU35M_2cTAU30M' , l1SeedThresholds=['cTAU35M', 'cTAU30M'], groups=SupportPhIGroup+MultiTauGroup),

        ChainProp(name='HLT_tau80_mediumGNTau_tau35_mediumGNTau_03dRAB30_L1eTAU80_2cTAU30M_DR-eTAU30eTAU20', stream=[PhysicsStream, 'express'], l1SeedThresholds=['eTAU80', 'cTAU30M'], groups=PrimaryPhIGroup+MultiTauGroup+Topo2Group, monGroups=['tauMon:online', 'tauMon:shifter']),

        ChainProp(name='HLT_tau80_mediumGNTau_tau60_mediumGNTau_03dRAB_L1eTAU80_2eTAU60', l1SeedThresholds=['eTAU80', 'eTAU60'], groups=PrimaryPhIGroup+MultiTauGroup, monGroups=['tauMon:t0']),


        # Delayed stream di-tau primaries and support chains
        ChainProp(name='HLT_tau30_mediumGNTau_tau20_mediumGNTau_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55', l1SeedThresholds=['cTAU30M', 'cTAU20M'], stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiTauGroup+Topo2Group),
        ChainProp(name='HLT_tau30_mediumGNTau_tau20_mediumGNTau_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20', l1SeedThresholds=['cTAU30M', 'cTAU20M'], stream=['VBFDelayed'], groups=EOFTauPhIGroup+MultiTauGroup+Topo2Group),
        ChainProp(name='HLT_tau30_mediumGNTau_tau20_mediumGNTau_03dRAB_L1cTAU30M_2cTAU20M_4jJ30p0ETA25', l1SeedThresholds=['cTAU30M', 'cTAU20M'], stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiTauGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_tau20_mediumGNTau_03dRAB_L1cTAU30M_2cTAU20M', l1SeedThresholds=['cTAU30M', 'cTAU20M'], stream=['VBFDelayed'], groups=SupportPhIGroup+MultiTauGroup),


        # Boosted low-pT di-tau chains
        ChainProp(name='HLT_tau20_mediumGNTau_tau20_mediumGNTau_02dRAB10_L1cTAU20M_DR-eTAU20eTAU12-jJ40', l1SeedThresholds=['cTAU20M',  'eTAU12'], groups=MultiTauGroup+SupportPhIGroup+Topo2Group),
        ChainProp(name='HLT_tau20_mediumGNTau_tau20_mediumGNTau_03dRAB10_L1cTAU20M_DR-eTAU20eTAU12-jJ40', l1SeedThresholds=['cTAU20M',  'eTAU12'], groups=MultiTauGroup+SupportPhIGroup+Topo2Group),

        ChainProp(name='HLT_tau25_mediumGNTau_tau20_mediumGNTau_02dRAB10_L1cTAU20M_DR-eTAU20eTAU12-jJ40', l1SeedThresholds=['cTAU20M',  'eTAU12'], groups=MultiTauGroup+SupportPhIGroup+Topo2Group),

        
        # Boosted high-pT di-tau chains (ATR-30999)
        # Primaries (NTrack <= 5)
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni1Trk5_pf_jes_ftf_preselj200_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+DiTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni1Trk5_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+DiTauGroup, monGroups=['tauMon:t0']),

        # Backup primaries (NTrack <= 3)
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni1Trk3_pf_jes_ftf_preselj200_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+DiTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni1Trk3_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+DiTauGroup, monGroups=['tauMon:t0']),

        # Support chains (NTrack <= 4/5)
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni0Trk4_pf_jes_ftf_preselj200_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+DiTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni0Trk4_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+DiTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni0Trk5_pf_jes_ftf_preselj200_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+DiTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni0Trk5_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+DiTauGroup, monGroups=['tauMon:t0']),


        # HH->2b2tau: di-tau support chains (ATR-28890)
        ChainProp(name='HLT_tau30_mediumGNTau_tau20_mediumGNTau_03dRAB_L14jJ30p0ETA24_0DETA24-eTAU30eTAU12', l1SeedThresholds=['eTAU20', 'eTAU12'], groups=SupportPhIGroup+MultiTauGroup+Topo3Group),
        ChainProp(name='HLT_tau30_mediumGNTau_tau20_mediumGNTau_03dRAB_L1cTAU20M_cTAU12M_4jJ30p0ETA24_0DETA24-eTAU30eTAU12', l1SeedThresholds=['cTAU20M', 'cTAU12M'], groups=SupportPhIGroup+MultiTauGroup+Topo3Group),


        # yy->tautau: g-2 tau measurement chains (ATR-30638)
        ChainProp(name='HLT_tau70_mediumGNTau_tau50_mediumGNTau_29dphiAB_L1eTAU70_2cTAU50M_DPHI-2eTAU50', l1SeedThresholds=['eTAU70', 'cTAU50M'], groups=PrimaryPhIGroup+MultiTauGroup+Topo3Group, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_2tau50_mediumGNTau_29dphiAA_L12cTAU50M_DPHI-2eTAU50', l1SeedThresholds=['cTAU50M'], groups=EOFTauPhIGroup+MultiTauGroup+Topo3Group, monGroups=['tauMon:t0']),


        # Di-tau LLP primaries and support chains (ATR-21754)
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoLLP_tau60_mediumRNN_tracktwoLLP_03dRAB_L1eTAU80_2eTAU60', l1SeedThresholds=['eTAU80', 'eTAU60'], groups=PrimaryPhIGroup+MultiTauGroup, monGroups=['tauMon:shifter']),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoLLP_tau60_tightRNN_tracktwoLLP_03dRAB_L1eTAU80_2eTAU60', l1SeedThresholds=['eTAU80', 'eTAU60'], groups=PrimaryPhIGroup+MultiTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau80_tightRNN_tracktwoLLP_tau60_tightRNN_tracktwoLLP_03dRAB_L1eTAU80_2eTAU60', l1SeedThresholds=['eTAU80', 'eTAU60'], groups=PrimaryPhIGroup+MultiTauGroup,  monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau100_mediumRNN_tracktwoLLP_tau80_mediumRNN_tracktwoLLP_03dRAB_L1eTAU80_2eTAU60', l1SeedThresholds=['eTAU80', 'eTAU60'], groups=PrimaryPhIGroup+MultiTauGroup, monGroups=['tauMon:t0']),


        # Single tau support chains
        ChainProp(name="HLT_tau20_idperf_tracktwoMVA_L1eTAU12", stream=[PhysicsStream, 'express'], groups=SingleTauGroup+SupportPhIGroup+['RATE:CPS_eTAU12'], monGroups=['tauMon:online','tauMon:shifter','idMon:shifter']), #ATR-27013
        ChainProp(name="HLT_tau20_perf_tracktwoMVA_L1eTAU12", stream=[PhysicsStream, 'express'], groups=SingleTauGroup+SupportPhIGroup+['RATE:CPS_eTAU12'], monGroups=['tauMon:online','tauMon:shifter']), #ATR-27013
        ChainProp(name='HLT_tau20_mediumGNTau_L1eTAU12', stream=[PhysicsStream, 'express'], groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_eTAU12'], monGroups=['tauMon:online', 'tauMon:shifter']),

        ChainProp(name="HLT_tau25_idperf_tracktwoMVA_L1eTAU20", groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_eTAU20'], monGroups=['tauMon:online','tauMon:shifter']), #ATR-27013
        ChainProp(name="HLT_tau25_perf_tracktwoMVA_L1eTAU20", groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_eTAU20'], monGroups=['tauMon:online','tauMon:shifter']), #ATR-27013
        ChainProp(name='HLT_tau25_mediumGNTau_L1eTAU20', stream=[PhysicsStream,'express'], groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_eTAU20'], monGroups=['tauMon:online', 'tauMon:shifter']),

        ChainProp(name="HLT_tau25_idperf_tracktwoMVA_L1eTAU20L", groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_eTAU20L'], monGroups=['tauMon:online','tauMon:shifter']), #ATR-27013
        ChainProp(name="HLT_tau25_perf_tracktwoMVA_L1eTAU20L", groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_eTAU20L'], monGroups=['tauMon:online','tauMon:shifter']), #ATR-27013
        ChainProp(name="HLT_tau25_mediumGNTau_L1eTAU20L", stream=[PhysicsStream,'express'], groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_eTAU20L'], monGroups=['tauMon:online','tauMon:shifter']),

        ChainProp(name="HLT_tau25_idperf_tracktwoMVA_L1cTAU20M", groups=SingleTauGroup+SupportPhIGroup+['RATE:CPS_cTAU20M'], monGroups=['tauMon:online','tauMon:shifter','idMon:shifter']), #ATR-27013
        ChainProp(name="HLT_tau25_perf_tracktwoMVA_L1cTAU20M", groups=SingleTauGroup+SupportPhIGroup+['RATE:CPS_cTAU20M'], monGroups=['tauMon:online','tauMon:shifter']), #ATR-27013
        ChainProp(name='HLT_tau25_mediumGNTau_L1cTAU20M', stream=[PhysicsStream, 'express'], groups=SingleTauGroup+SupportPhIGroup+['RATE:CPS_cTAU20M'], monGroups=['tauMon:online', 'tauMon:shifter']),
        ChainProp(name="HLT_tau25_mediumRNN_tracktwoLLP_L1cTAU20M", groups=SingleTauGroup+SupportPhIGroup+['RATE:CPS_cTAU20M'], monGroups=['tauMon:online']),

        ChainProp(name="HLT_tau35_idperf_tracktwoMVA_L1cTAU30M", stream=[PhysicsStream, 'express'], groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_cTAU30M'], monGroups=['tauMon:t0','idMon:shifter']),
        ChainProp(name="HLT_tau35_perf_tracktwoMVA_L1cTAU30M", groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_cTAU30M'], monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau35_mediumGNTau_L1cTAU30M', groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_cTAU30M'], monGroups=['tauMon:t0']),

        ChainProp(name="HLT_tau40_idperf_L1cTAU35M", stream=[PhysicsStream, 'express'], groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_cTAU35M'], monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau40_perf_L1cTAU35M", groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_cTAU35M'], monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau40_mediumGNTau_L1cTAU35M', groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_cTAU35M'], monGroups=['tauMon:t0']),

        ChainProp(name='HLT_tau50_idperf_L1cTAU50M', groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_cTAU50M'], monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau50_perf_L1cTAU50M', groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_cTAU50M'], monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau50_mediumGNTau_L1cTAU50M', groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_cTAU50M'], monGroups=['tauMon:t0']),

        ChainProp(name='HLT_tau60_mediumGNTau_L1eTAU60', groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),

        ChainProp(name='HLT_tau70_idperf_L1eTAU70', groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_eTAU70'], monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau70_perf_L1eTAU70', groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_eTAU70'], monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau70_mediumGNTau_L1eTAU70', groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_eTAU70'], monGroups=['tauMon:t0']),

        ChainProp(name="HLT_tau80_idperf_tracktwoMVA_L1eTAU80", stream=[PhysicsStream,'express'], groups=SingleTauGroup+SupportPhIGroup+['RATE:CPS_eTAU80'], monGroups=['idMon:t0']),
        ChainProp(name='HLT_tau80_mediumGNTau_L1eTAU80', groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_eTAU80'], monGroups=['tauMon:t0']),

        ChainProp(name="HLT_tau160_idperf_tracktwoMVA_L1eTAU140", stream=[PhysicsStream, 'express'], groups=SingleTauGroup+SupportPhIGroup+['RATE:CPS_eTAU140'], monGroups=['tauMon:online','tauMon:t0','idMon:t0']),
        ChainProp(name="HLT_tau160_perf_tracktwoMVA_L1eTAU140", stream=[PhysicsStream, 'express'], groups=SingleTauGroup+SupportPhIGroup+['RATE:CPS_eTAU140'], monGroups=['tauMon:online','tauMon:t0']),
    ]


    chains['Bphysics'] += [
        #-- dimuon primary triggers
        ChainProp(name='HLT_2mu10_bJpsimumu_L12MU8F', stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_2mu10_bUpsimumu_L12MU8F', stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        #-- RCP multiple candidate
        ChainProp(name='HLT_mu10_l2mt_mu4_l2mt_bJpsimumu_L1MU10BOM', l1SeedThresholds=['MU10BOM']*2, stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu10_l2mt_mu4_l2mt_bJpsimumu_L1MU12BOM', l1SeedThresholds=['MU12BOM']*2, stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        #-- mu11_mu6 chains
        ChainProp(name='HLT_mu11_mu6_bJpsimumu_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=[PhysicsStream,'BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu11_mu6_bJpsimumu_Lxy0_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=[PhysicsStream,'BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu11_mu6_bJpsimumu_sigmaLxy3_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=[PhysicsStream,'BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu11_mu6_bUpsimumu_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu11_mu6_bBmumu_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu11_mu6_bDimu_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed','express'], groups=BphysicsGroup+PrimaryL1MuGroup, monGroups=['bphysMon:online','bphysMon:shifter']),
        ChainProp(name='HLT_mu11_mu6_bDimu_Lxy0_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu11_mu6_bDimu2700_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu11_mu6_bDimu2700_Lxy0_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu11_mu6_bDimu2700_sigmaLxy3_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu11_mu6_bPhi_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed','express'], groups=BphysicsGroup+PrimaryL1MuGroup, monGroups=['bphysMon:online','bphysMon:shifter']),
        ChainProp(name='HLT_mu11_mu6_bTau_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed','express'], groups=BphysicsGroup+PrimaryL1MuGroup, monGroups=['bphysMon:online','bphysMon:t0']),
        #-- mu11_mu6 chains with L1Topo
        ChainProp(name='HLT_mu11_mu6_bJpsimumu_L1LFV-MU8VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        ChainProp(name='HLT_mu11_mu6_bJpsimumu_sigmaLxy3_L1LFV-MU8VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        ChainProp(name='HLT_mu11_mu6_bUpsimumu_L1LFV-MU8VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        ChainProp(name='HLT_mu11_mu6_bBmumu_L1LFV-MU8VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        ChainProp(name='HLT_mu11_mu6_bDimu_L1LFV-MU8VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        ChainProp(name='HLT_mu11_mu6_bDimu2700_L1LFV-MU8VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        ChainProp(name='HLT_mu11_mu6_bDimu2700_sigmaLxy3_L1LFV-MU8VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        ChainProp(name='HLT_mu11_mu6_bPhi_L1LFV-MU8VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        ChainProp(name='HLT_mu11_mu6_bTau_L1LFV-MU8VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        #-- 2mu6 chains
        ChainProp(name='HLT_2mu6_bJpsimumu_L12MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+SupportGroup+['RATE:CPS_2MU5VF']),
        ChainProp(name='HLT_2mu6_bJpsimumu_Lxy0_L12MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+SupportGroup+['RATE:CPS_2MU5VF']),
        ChainProp(name='HLT_2mu6_bJpsimumu_sigmaLxy3_L12MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+SupportGroup+['RATE:CPS_2MU5VF']),
        ChainProp(name='HLT_2mu6_bBmumu_Lxy0_L12MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+SupportGroup+['RATE:CPS_2MU5VF']),
        ChainProp(name='HLT_2mu6_bUpsimumu_L12MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+SupportGroup+['RATE:CPS_2MU5VF']),
        ChainProp(name='HLT_2mu6_bDimu2700_sigmaLxy3_L12MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+SupportGroup+['RATE:CPS_2MU5VF']),
        #-- 2mu6 chains with L1Topo
        ChainProp(name='HLT_2mu6_bJpsimumu_L1BPH-2M9-2DR15-2MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed','express'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group, monGroups=['bphysMon:online','bphysMon:shifter']),
        ChainProp(name='HLT_2mu6_bJpsimumu_Lxy0_L1BPH-2M9-2DR15-2MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        ChainProp(name='HLT_2mu6_bBmumu_Lxy0_L1BPH-2M9-2DR15-2MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        ChainProp(name='HLT_2mu6_bUpsimumu_L1BPH-8M15-0DR22-2MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed','express'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group, monGroups=['bphysMon:online','bphysMon:t0']),

        #-- dimuon EOF triggers
        #-- 2mu6 chains
        ChainProp(name='HLT_2mu6_bBmumu_L12MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU5VF']),
        ChainProp(name='HLT_2mu6_bDimu_L12MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU5VF']),
        ChainProp(name='HLT_2mu6_bPhi_L12MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU5VF']),
        #-- 2mu6 chains with L1Topo
        ChainProp(name='HLT_2mu6_bBmumu_L1BPH-2M9-2DR15-2MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-2DR15-2MU5VF']+Topo2Group),
        ChainProp(name='HLT_2mu6_bDimu_L1BPH-2M9-2DR15-2MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-2DR15-2MU5VF']+Topo2Group),
        ChainProp(name='HLT_2mu6_bJpsimumu_sigmaLxy3_L1LFV-MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_LFV-MU5VF']+Topo2Group),
        ChainProp(name='HLT_2mu6_bDimu_L1LFV-MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_LFV-MU5VF']+Topo2Group),
        ChainProp(name='HLT_2mu6_bDimu2700_sigmaLxy3_L1LFV-MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_LFV-MU5VF']+Topo2Group),
        ChainProp(name='HLT_2mu6_bPhi_L1LFV-MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_LFV-MU5VF']+Topo2Group),
        #-- mu6_mu4 chains
        ChainProp(name='HLT_mu6_mu4_bJpsimumu_L1MU5VF_2MU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3V']),
        ChainProp(name='HLT_mu6_mu4_bJpsimumu_Lxy0_L1MU5VF_2MU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3V']),
        ChainProp(name='HLT_mu6_mu4_bJpsimumu_sigmaLxy3_L1MU5VF_2MU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3V']),
        ChainProp(name='HLT_mu6_mu4_bBmumu_L1MU5VF_2MU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3V']),
        ChainProp(name='HLT_mu6_mu4_bBmumu_Lxy0_L1MU5VF_2MU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3V']),
        ChainProp(name='HLT_mu6_mu4_bDimu_L1MU5VF_2MU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3V']),
        ChainProp(name='HLT_mu6_mu4_bDimu2700_sigmaLxy3_L1MU5VF_2MU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3V']),
        ChainProp(name='HLT_mu6_mu4_bUpsimumu_L1MU5VF_2MU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3V']),
        #-- mu6_mu4 chains, backup with MU3VF
        ChainProp(name='HLT_mu6_mu4_bJpsimumu_L1MU5VF_2MU3VF', l1SeedThresholds=['MU5VF','MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3VF']),
        ChainProp(name='HLT_mu6_mu4_bJpsimumu_Lxy0_L1MU5VF_2MU3VF', l1SeedThresholds=['MU5VF','MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3VF']),
        ChainProp(name='HLT_mu6_mu4_bJpsimumu_sigmaLxy3_L1MU5VF_2MU3VF', l1SeedThresholds=['MU5VF','MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3VF']),
        ChainProp(name='HLT_mu6_mu4_bBmumu_L1MU5VF_2MU3VF', l1SeedThresholds=['MU5VF','MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3VF']),
        ChainProp(name='HLT_mu6_mu4_bBmumu_Lxy0_L1MU5VF_2MU3VF', l1SeedThresholds=['MU5VF','MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3VF']),
        ChainProp(name='HLT_mu6_mu4_bDimu_L1MU5VF_2MU3VF', l1SeedThresholds=['MU5VF','MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3VF']),
        ChainProp(name='HLT_mu6_mu4_bDimu2700_sigmaLxy3_L1MU5VF_2MU3VF', l1SeedThresholds=['MU5VF','MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3VF']),
        ChainProp(name='HLT_mu6_mu4_bUpsimumu_L1MU5VF_2MU3VF', l1SeedThresholds=['MU5VF','MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3VF']),
        #-- mu6_mu4 chains with L1Topo
        ChainProp(name='HLT_mu6_mu4_bJpsimumu_L1BPH-2M9-0DR15-MU5VFMU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-MU5VFMU3V']+Topo2Group),
        ChainProp(name='HLT_mu6_mu4_bJpsimumu_Lxy0_L1BPH-2M9-0DR15-MU5VFMU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-MU5VFMU3V']+Topo2Group),
        ChainProp(name='HLT_mu6_mu4_bBmumu_L1BPH-2M9-0DR15-MU5VFMU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-MU5VFMU3V']+Topo2Group),
        ChainProp(name='HLT_mu6_mu4_bBmumu_Lxy0_L1BPH-2M9-0DR15-MU5VFMU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-MU5VFMU3V']+Topo2Group),
        ChainProp(name='HLT_mu6_mu4_bDimu_L1BPH-2M9-0DR15-MU5VFMU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-MU5VFMU3V']+Topo2Group),
        ChainProp(name='HLT_mu6_mu4_bUpsimumu_L1BPH-8M15-0DR22-MU5VFMU3V-BO', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+Topo2Group),
        #-- mu6_mu4 chains with L1 charge cut (ATR-19639)
        ChainProp(name='HLT_mu6_mu4_bJpsimumu_L1BPH-2M9-0DR15-C-MU5VFMU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-C-MU5VFMU3V']+Topo3Group),
        ChainProp(name='HLT_mu6_mu4_bJpsimumu_Lxy0_L1BPH-2M9-0DR15-C-MU5VFMU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-C-MU5VFMU3V']+Topo3Group),
        ChainProp(name='HLT_mu6_mu4_bBmumu_L1BPH-2M9-0DR15-C-MU5VFMU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-C-MU5VFMU3V']+Topo3Group),
        ChainProp(name='HLT_mu6_mu4_bBmumu_Lxy0_L1BPH-2M9-0DR15-C-MU5VFMU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-C-MU5VFMU3V']+Topo3Group),
        ChainProp(name='HLT_mu6_mu4_bDimu_L1BPH-2M9-0DR15-C-MU5VFMU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-C-MU5VFMU3V']+Topo3Group),
        #-- 2mu4 chains
        ChainProp(name='HLT_2mu4_bJpsimumu_L12MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3V'], monGroups=['bphysMon:online','bphysMon:t0']),
        ChainProp(name='HLT_2mu4_bJpsimumu_Lxy0_L12MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3V']),
        ChainProp(name='HLT_2mu4_bJpsimumu_sigmaLxy3_L12MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3V']),
        ChainProp(name='HLT_2mu4_bBmumu_Lxy0_L12MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3V']),
        ChainProp(name='HLT_2mu4_bBmumu_L12MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3V']),
        ChainProp(name='HLT_2mu4_bDimu_L12MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3V'], monGroups=['bphysMon:online','bphysMon:t0']),
        ChainProp(name='HLT_2mu4_bDimu2700_sigmaLxy3_L12MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3V']),
        ChainProp(name='HLT_2mu4_bUpsimumu_L12MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3V']),
        #-- 2mu4 chains, backup with MU3VF
        ChainProp(name='HLT_2mu4_bJpsimumu_L12MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3VF']),
        ChainProp(name='HLT_2mu4_bJpsimumu_Lxy0_L12MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3VF']),
        ChainProp(name='HLT_2mu4_bJpsimumu_sigmaLxy3_L12MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3VF']),
        ChainProp(name='HLT_2mu4_bBmumu_Lxy0_L12MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3VF']),
        ChainProp(name='HLT_2mu4_bBmumu_L12MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3VF']),
        ChainProp(name='HLT_2mu4_bDimu_L12MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3VF']),
        ChainProp(name='HLT_2mu4_bDimu2700_sigmaLxy3_L12MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3VF']),
        ChainProp(name='HLT_2mu4_bUpsimumu_L12MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3VF']),
        #-- 2mu4 chains with L1Topo
        ChainProp(name='HLT_2mu4_bJpsimumu_Lxy0_L1BPH-2M9-0DR15-2MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3V']+Topo2Group),
        ChainProp(name='HLT_2mu4_bBmumu_Lxy0_L1BPH-2M9-0DR15-2MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3V']+Topo2Group),
        ChainProp(name='HLT_2mu4_bBmumu_L1BPH-2M9-0DR15-2MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3V']+Topo2Group),
        # backup with MU3VF
        ChainProp(name='HLT_2mu4_bJpsimumu_Lxy0_L1BPH-2M9-0DR15-2MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3VF']+Topo3Group),
        ChainProp(name='HLT_2mu4_bBmumu_Lxy0_L1BPH-2M9-0DR15-2MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3VF']+Topo3Group),
        ChainProp(name='HLT_2mu4_bBmumu_L1BPH-2M9-0DR15-2MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3VF']+Topo3Group),

        #-- multi muon primary triggers (only two muons are fitted to the common vertex except the case of bTau topology)
        #-- 3mu
        ChainProp(name='HLT_3mu6_bJpsi_L13MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_3mu6_bUpsi_L13MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_3mu6_bDimu_L13MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_3mu6_bTau_L13MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_2mu6_mu4_bTau_L12MU5VF_3MU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_2mu6_mu4_bUpsi_L12MU5VF_3MU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu6_2mu4_bJpsi_L1MU5VF_3MU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu6_2mu4_bUpsi_L1MU5VF_3MU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu6_2mu4_bTau_L1MU5VF_3MU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu6_2mu4_bDimu2700_L1MU5VF_3MU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu6_2mu4_bDimu6000_L1MU5VF_3MU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_3mu4_bJpsi_L13MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_3mu4_bUpsi_L13MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_3mu4_bTau_L13MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_3mu4_bPhi_L13MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        # 3mu4 backup with MU3VF (ATR-24747)
        ChainProp(name='HLT_mu6_2mu4_bJpsi_L1MU5VF_3MU3VF', l1SeedThresholds=['MU5VF','MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu6_2mu4_bUpsi_L1MU5VF_3MU3VF', l1SeedThresholds=['MU5VF','MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu6_2mu4_bTau_L1MU5VF_3MU3VF', l1SeedThresholds=['MU5VF','MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu6_2mu4_bDimu2700_L1MU5VF_3MU3VF', l1SeedThresholds=['MU5VF','MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu6_2mu4_bDimu6000_L1MU5VF_3MU3VF', l1SeedThresholds=['MU5VF','MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_3mu4_bJpsi_L13MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_3mu4_bUpsi_L13MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_3mu4_bTau_L13MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_3mu4_bPhi_L13MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        #-- 4mu
        ChainProp(name='HLT_4mu4_bDimu6000_L14MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        #-- EOF triggers
        ChainProp(name='HLT_3mu4_bDimu2700_L13MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup),
        # 3mu4 backup with MU3VF (ATR-24747)
        ChainProp(name='HLT_3mu4_bDimu2700_L13MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup),

        #-- Bmumux primary triggers
        #-- mu11_mu6 chains
        ChainProp(name='HLT_mu11_mu6_bBmumux_Bidperf_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed','express'], groups=BphysicsGroup+SupportGroup, monGroups=['idMon:shifter']),
        ChainProp(name='HLT_mu11_mu6_bBmumux_BpmumuKp_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed','express'], groups=BphysicsGroup+PrimaryL1MuGroup, monGroups=['bphysMon:online','bphysMon:shifter','idMon:t0']),
        ChainProp(name='HLT_mu11_mu6_bBmumux_BcmumuPi_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu11_mu6_bBmumux_BsmumuPhi_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed','express'], groups=BphysicsGroup+PrimaryL1MuGroup, monGroups=['bphysMon:online','bphysMon:shifter','idMon:t0']),
        ChainProp(name='HLT_mu11_mu6_bBmumux_BdmumuKst_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed','express'], groups=BphysicsGroup+PrimaryL1MuGroup, monGroups=['bphysMon:online','bphysMon:shifter','idMon:t0']),
        ChainProp(name='HLT_mu11_mu6_bBmumux_LbPqKm_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu11_mu6_bBmumux_BcmumuDsloose_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu11_mu6_bBmumux_BcmumuDploose_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu11_mu6_bBmumux_BcmumuD0Xloose_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu11_mu6_bBmumux_BcmumuDstarloose_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu11_mu6_bBmux_BdmuDstarX_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        #-- mu11_mu6 chains with L1Topo
        ChainProp(name='HLT_mu11_mu6_bBmumux_BpmumuKp_L1LFV-MU8VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        ChainProp(name='HLT_mu11_mu6_bBmumux_BcmumuPi_L1LFV-MU8VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        ChainProp(name='HLT_mu11_mu6_bBmumux_BsmumuPhi_L1LFV-MU8VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        ChainProp(name='HLT_mu11_mu6_bBmumux_BdmumuKst_L1LFV-MU8VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        ChainProp(name='HLT_mu11_mu6_bBmumux_LbPqKm_L1LFV-MU8VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        ChainProp(name='HLT_mu11_mu6_bBmumux_BcmumuDsloose_L1LFV-MU8VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        ChainProp(name='HLT_mu11_mu6_bBmumux_BcmumuDploose_L1LFV-MU8VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        ChainProp(name='HLT_mu11_mu6_bBmumux_BcmumuD0Xloose_L1LFV-MU8VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        ChainProp(name='HLT_mu11_mu6_bBmumux_BcmumuDstarloose_L1LFV-MU8VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        #-- 2mu6 chains with L1Topo
        ChainProp(name='HLT_2mu6_bBmumux_BpmumuKp_L1BPH-2M9-2DR15-2MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        ChainProp(name='HLT_2mu6_bBmumux_BcmumuPi_L1BPH-2M9-2DR15-2MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        ChainProp(name='HLT_2mu6_bBmumux_BsmumuPhi_L1BPH-2M9-2DR15-2MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        ChainProp(name='HLT_2mu6_bBmumux_BdmumuKst_L1BPH-2M9-2DR15-2MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        ChainProp(name='HLT_2mu6_bBmumux_LbPqKm_L1BPH-2M9-2DR15-2MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        ChainProp(name='HLT_2mu6_bBmumux_BcmumuDsloose_L1BPH-2M9-2DR15-2MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        ChainProp(name='HLT_2mu6_bBmumux_BcmumuDploose_L1BPH-2M9-2DR15-2MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        ChainProp(name='HLT_2mu6_bBmumux_BcmumuD0Xloose_L1BPH-2M9-2DR15-2MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        ChainProp(name='HLT_2mu6_bBmumux_BcmumuDstarloose_L1BPH-2M9-2DR15-2MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        #-- Bmumux EOF triggers
        #-- 2mu6 chains
        ChainProp(name='HLT_2mu6_bBmumux_BpmumuKp_L12MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU5VF']),
        ChainProp(name='HLT_2mu6_bBmumux_BcmumuPi_L12MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU5VF']),
        ChainProp(name='HLT_2mu6_bBmumux_BsmumuPhi_L12MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU5VF']),
        ChainProp(name='HLT_2mu6_bBmumux_BdmumuKst_L12MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU5VF']),
        ChainProp(name='HLT_2mu6_bBmumux_LbPqKm_L12MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU5VF']),
        ChainProp(name='HLT_2mu6_bBmumux_BcmumuDsloose_L12MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU5VF']),
        ChainProp(name='HLT_2mu6_bBmumux_BcmumuDploose_L12MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU5VF']),
        ChainProp(name='HLT_2mu6_bBmumux_BcmumuD0Xloose_L12MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU5VF']),
        ChainProp(name='HLT_2mu6_bBmumux_BcmumuDstarloose_L12MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU5VF']),
        ChainProp(name='HLT_2mu6_bBmux_BdmuDstarX_L12MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU5VF']),
        #-- 2mu6 chains with L1Topo
        ChainProp(name='HLT_2mu6_bBmumux_BpmumuKp_L1LFV-MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_LFV-MU5VF']+Topo2Group),
        ChainProp(name='HLT_2mu6_bBmumux_BcmumuPi_L1LFV-MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_LFV-MU5VF']+Topo2Group),
        ChainProp(name='HLT_2mu6_bBmumux_BsmumuPhi_L1LFV-MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_LFV-MU5VF']+Topo2Group),
        ChainProp(name='HLT_2mu6_bBmumux_BdmumuKst_L1LFV-MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_LFV-MU5VF']+Topo2Group),
        ChainProp(name='HLT_2mu6_bBmumux_LbPqKm_L1LFV-MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_LFV-MU5VF']+Topo2Group),
        ChainProp(name='HLT_2mu6_bBmumux_BcmumuDsloose_L1LFV-MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_LFV-MU5VF']+Topo2Group),
        ChainProp(name='HLT_2mu6_bBmumux_BcmumuDploose_L1LFV-MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_LFV-MU5VF']+Topo2Group),
        ChainProp(name='HLT_2mu6_bBmumux_BcmumuD0Xloose_L1LFV-MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_LFV-MU5VF']+Topo2Group),
        ChainProp(name='HLT_2mu6_bBmumux_BcmumuDstarloose_L1LFV-MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_LFV-MU5VF']+Topo2Group),
        #-- mu6_mu4 chains
        ChainProp(name='HLT_mu6_mu4_bBmumux_BpmumuKp_L1MU5VF_2MU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3V']),
        ChainProp(name='HLT_mu6_mu4_bBmumux_BcmumuPi_L1MU5VF_2MU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3V']),
        ChainProp(name='HLT_mu6_mu4_bBmumux_BsmumuPhi_L1MU5VF_2MU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3V']),
        ChainProp(name='HLT_mu6_mu4_bBmumux_BdmumuKst_L1MU5VF_2MU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3V']),
        ChainProp(name='HLT_mu6_mu4_bBmumux_LbPqKm_L1MU5VF_2MU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3V']),
        ChainProp(name='HLT_mu6_mu4_bBmumux_BcmumuDsloose_L1MU5VF_2MU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3V']),
        ChainProp(name='HLT_mu6_mu4_bBmumux_BcmumuDploose_L1MU5VF_2MU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3V']),
        ChainProp(name='HLT_mu6_mu4_bBmumux_BcmumuD0Xloose_L1MU5VF_2MU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3V']),
        ChainProp(name='HLT_mu6_mu4_bBmumux_BcmumuDstarloose_L1MU5VF_2MU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3V']),
        ChainProp(name='HLT_mu6_mu4_bBmux_BdmuDstarX_L1MU5VF_2MU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3V']),
        #-- mu6_mu4 chains, backup with MU3VF
        ChainProp(name='HLT_mu6_mu4_bBmumux_BpmumuKp_L1MU5VF_2MU3VF', l1SeedThresholds=['MU5VF','MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3VF']),
        ChainProp(name='HLT_mu6_mu4_bBmumux_BcmumuPi_L1MU5VF_2MU3VF', l1SeedThresholds=['MU5VF','MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3VF']),
        ChainProp(name='HLT_mu6_mu4_bBmumux_BsmumuPhi_L1MU5VF_2MU3VF', l1SeedThresholds=['MU5VF','MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3VF']),
        ChainProp(name='HLT_mu6_mu4_bBmumux_BdmumuKst_L1MU5VF_2MU3VF', l1SeedThresholds=['MU5VF','MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3VF']),
        ChainProp(name='HLT_mu6_mu4_bBmumux_LbPqKm_L1MU5VF_2MU3VF', l1SeedThresholds=['MU5VF','MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3VF']),
        ChainProp(name='HLT_mu6_mu4_bBmumux_BcmumuDsloose_L1MU5VF_2MU3VF', l1SeedThresholds=['MU5VF','MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3VF']),
        ChainProp(name='HLT_mu6_mu4_bBmumux_BcmumuDploose_L1MU5VF_2MU3VF', l1SeedThresholds=['MU5VF','MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3VF']),
        ChainProp(name='HLT_mu6_mu4_bBmumux_BcmumuD0Xloose_L1MU5VF_2MU3VF', l1SeedThresholds=['MU5VF','MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3VF']),
        ChainProp(name='HLT_mu6_mu4_bBmumux_BcmumuDstarloose_L1MU5VF_2MU3VF', l1SeedThresholds=['MU5VF','MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3VF']),
        ChainProp(name='HLT_mu6_mu4_bBmux_BdmuDstarX_L1MU5VF_2MU3VF', l1SeedThresholds=['MU5VF','MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3VF']),
        #-- mu6_mu4 chains with L1Topo
        ChainProp(name='HLT_mu6_mu4_bBmumux_BpmumuKp_L1BPH-2M9-0DR15-MU5VFMU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-MU5VFMU3V']+Topo2Group),
        ChainProp(name='HLT_mu6_mu4_bBmumux_BcmumuPi_L1BPH-2M9-0DR15-MU5VFMU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-MU5VFMU3V']+Topo2Group),
        ChainProp(name='HLT_mu6_mu4_bBmumux_BsmumuPhi_L1BPH-2M9-0DR15-MU5VFMU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-MU5VFMU3V']+Topo2Group),
        ChainProp(name='HLT_mu6_mu4_bBmumux_BdmumuKst_L1BPH-2M9-0DR15-MU5VFMU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-MU5VFMU3V']+Topo2Group),
        ChainProp(name='HLT_mu6_mu4_bBmumux_LbPqKm_L1BPH-2M9-0DR15-MU5VFMU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-MU5VFMU3V']+Topo2Group),
        ChainProp(name='HLT_mu6_mu4_bBmumux_BcmumuDsloose_L1BPH-2M9-0DR15-MU5VFMU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-MU5VFMU3V']+Topo2Group),
        ChainProp(name='HLT_mu6_mu4_bBmumux_BcmumuDploose_L1BPH-2M9-0DR15-MU5VFMU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-MU5VFMU3V']+Topo2Group),
        ChainProp(name='HLT_mu6_mu4_bBmumux_BcmumuD0Xloose_L1BPH-2M9-0DR15-MU5VFMU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-MU5VFMU3V']+Topo2Group),
        ChainProp(name='HLT_mu6_mu4_bBmumux_BcmumuDstarloose_L1BPH-2M9-0DR15-MU5VFMU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-MU5VFMU3V']+Topo2Group),
        #-- mu6_mu4 chains with L1 charge cut (ATR-19639)
        ChainProp(name='HLT_mu6_mu4_bBmumux_BpmumuKp_L1BPH-2M9-0DR15-C-MU5VFMU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-C-MU5VFMU3V']+Topo3Group),
        ChainProp(name='HLT_mu6_mu4_bBmumux_BcmumuPi_L1BPH-2M9-0DR15-C-MU5VFMU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-C-MU5VFMU3V']+Topo3Group),
        ChainProp(name='HLT_mu6_mu4_bBmumux_BsmumuPhi_L1BPH-2M9-0DR15-C-MU5VFMU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-C-MU5VFMU3V']+Topo3Group),
        ChainProp(name='HLT_mu6_mu4_bBmumux_BdmumuKst_L1BPH-2M9-0DR15-C-MU5VFMU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-C-MU5VFMU3V']+Topo3Group),
        ChainProp(name='HLT_mu6_mu4_bBmumux_LbPqKm_L1BPH-2M9-0DR15-C-MU5VFMU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-C-MU5VFMU3V']+Topo3Group),
        ChainProp(name='HLT_mu6_mu4_bBmumux_BcmumuDsloose_L1BPH-2M9-0DR15-C-MU5VFMU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-C-MU5VFMU3V']+Topo3Group),
        ChainProp(name='HLT_mu6_mu4_bBmumux_BcmumuDploose_L1BPH-2M9-0DR15-C-MU5VFMU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-C-MU5VFMU3V']+Topo3Group),
        ChainProp(name='HLT_mu6_mu4_bBmumux_BcmumuD0Xloose_L1BPH-2M9-0DR15-C-MU5VFMU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-C-MU5VFMU3V']+Topo3Group),
        ChainProp(name='HLT_mu6_mu4_bBmumux_BcmumuDstarloose_L1BPH-2M9-0DR15-C-MU5VFMU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-C-MU5VFMU3V']+Topo3Group),
        #-- 2mu4 chains
        ChainProp(name='HLT_2mu4_bBmumux_BpmumuKp_L12MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3V']),
        ChainProp(name='HLT_2mu4_bBmumux_BcmumuPi_L12MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3V']),
        ChainProp(name='HLT_2mu4_bBmumux_BsmumuPhi_L12MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3V']),
        ChainProp(name='HLT_2mu4_bBmumux_BdmumuKst_L12MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3V']),
        ChainProp(name='HLT_2mu4_bBmumux_LbPqKm_L12MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3V']),
        ChainProp(name='HLT_2mu4_bBmumux_BcmumuDsloose_L12MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3V']),
        ChainProp(name='HLT_2mu4_bBmumux_BcmumuDploose_L12MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3V']),
        ChainProp(name='HLT_2mu4_bBmumux_BcmumuD0Xloose_L12MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3V']),
        ChainProp(name='HLT_2mu4_bBmumux_BcmumuDstarloose_L12MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3V']),
        ChainProp(name='HLT_2mu4_bBmux_BdmuDstarX_L12MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3V']),
        #-- 2mu4 chains, backup with MU3VF
        ChainProp(name='HLT_2mu4_bBmumux_BpmumuKp_L12MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3VF']),
        ChainProp(name='HLT_2mu4_bBmumux_BcmumuPi_L12MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3VF']),
        ChainProp(name='HLT_2mu4_bBmumux_BsmumuPhi_L12MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3VF']),
        ChainProp(name='HLT_2mu4_bBmumux_BdmumuKst_L12MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3VF']),
        ChainProp(name='HLT_2mu4_bBmumux_LbPqKm_L12MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3VF']),
        ChainProp(name='HLT_2mu4_bBmumux_BcmumuDsloose_L12MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3VF']),
        ChainProp(name='HLT_2mu4_bBmumux_BcmumuDploose_L12MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3VF']),
        ChainProp(name='HLT_2mu4_bBmumux_BcmumuD0Xloose_L12MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3VF']),
        ChainProp(name='HLT_2mu4_bBmumux_BcmumuDstarloose_L12MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3VF']),
        ChainProp(name='HLT_2mu4_bBmux_BdmuDstarX_L12MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3VF']),
        #-- 2mu4 chains with L1Topo
        ChainProp(name='HLT_2mu4_bBmumux_BpmumuKp_L1BPH-2M9-0DR15-2MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3V']+Topo2Group),
        ChainProp(name='HLT_2mu4_bBmumux_BcmumuPi_L1BPH-2M9-0DR15-2MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3V']+Topo2Group),
        ChainProp(name='HLT_2mu4_bBmumux_BsmumuPhi_L1BPH-2M9-0DR15-2MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3V']+Topo2Group),
        ChainProp(name='HLT_2mu4_bBmumux_BdmumuKst_L1BPH-2M9-0DR15-2MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3V']+Topo2Group),
        ChainProp(name='HLT_2mu4_bBmumux_LbPqKm_L1BPH-2M9-0DR15-2MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3V']+Topo2Group),
        ChainProp(name='HLT_2mu4_bBmumux_BcmumuDsloose_L1BPH-2M9-0DR15-2MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3V']+Topo2Group),
        ChainProp(name='HLT_2mu4_bBmumux_BcmumuDploose_L1BPH-2M9-0DR15-2MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3V']+Topo2Group),
        ChainProp(name='HLT_2mu4_bBmumux_BcmumuD0Xloose_L1BPH-2M9-0DR15-2MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3V']+Topo2Group),
        ChainProp(name='HLT_2mu4_bBmumux_BcmumuDstarloose_L1BPH-2M9-0DR15-2MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3V']+Topo2Group),
        # MU3VF backup chains
        ChainProp(name='HLT_2mu4_bBmumux_BpmumuKp_L1BPH-2M9-0DR15-2MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3VF']+Topo3Group),
        ChainProp(name='HLT_2mu4_bBmumux_BcmumuPi_L1BPH-2M9-0DR15-2MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3VF']+Topo3Group),
        ChainProp(name='HLT_2mu4_bBmumux_BsmumuPhi_L1BPH-2M9-0DR15-2MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3VF']+Topo3Group),
        ChainProp(name='HLT_2mu4_bBmumux_BdmumuKst_L1BPH-2M9-0DR15-2MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3VF']+Topo3Group),
        ChainProp(name='HLT_2mu4_bBmumux_LbPqKm_L1BPH-2M9-0DR15-2MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3VF']+Topo3Group),
        ChainProp(name='HLT_2mu4_bBmumux_BcmumuDsloose_L1BPH-2M9-0DR15-2MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3VF']+Topo3Group),
        ChainProp(name='HLT_2mu4_bBmumux_BcmumuDploose_L1BPH-2M9-0DR15-2MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3VF']+Topo3Group),
        ChainProp(name='HLT_2mu4_bBmumux_BcmumuD0Xloose_L1BPH-2M9-0DR15-2MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3VF']+Topo3Group),
        ChainProp(name='HLT_2mu4_bBmumux_BcmumuDstarloose_L1BPH-2M9-0DR15-2MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3VF']+Topo3Group),

        #-- Bmux EOF triggers
        ChainProp(name='HLT_mu20_bBmux_BpmuD0X_L1MU14FCH', l1SeedThresholds=['MU14FCH'], stream=["BphysDelayed"], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU14FCH']),
        ChainProp(name='HLT_mu20_bBmux_BdmuDpX_L1MU14FCH', l1SeedThresholds=['MU14FCH'], stream=["BphysDelayed"], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU14FCH']),
        ChainProp(name='HLT_mu20_bBmux_BdmuDstarX_L1MU14FCH', l1SeedThresholds=['MU14FCH'], stream=["BphysDelayed"], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU14FCH']),
        ChainProp(name='HLT_mu20_mu4_bBmux_BdmuDstarX_L1MU14FCH', l1SeedThresholds=['MU14FCH','MU3V'], stream=["BphysDelayed"], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU14FCH']),
        ChainProp(name='HLT_mu20_bBmux_BsmuDsX_L1MU14FCH', l1SeedThresholds=['MU14FCH'], stream=["BphysDelayed"], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU14FCH']),
        ChainProp(name='HLT_mu20_bBmux_LbmuLcX_L1MU14FCH', l1SeedThresholds=['MU14FCH'], stream=["BphysDelayed"], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU14FCH']),
        # ATR-25512
        ChainProp(name='HLT_mu23_bBmux_BpmuD0X_L1MU18VFCH', l1SeedThresholds=['MU18VFCH'], stream=["BphysDelayed"], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU18VFCH']),
        ChainProp(name='HLT_mu23_bBmux_BdmuDpX_L1MU18VFCH', l1SeedThresholds=['MU18VFCH'], stream=["BphysDelayed"], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU18VFCH']),
        ChainProp(name='HLT_mu23_bBmux_BdmuDstarX_L1MU18VFCH', l1SeedThresholds=['MU18VFCH'], stream=["BphysDelayed"], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU18VFCH']),
        ChainProp(name='HLT_mu23_mu4_bBmux_BdmuDstarX_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','MU3V'], stream=["BphysDelayed"], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU18VFCH']),
        ChainProp(name='HLT_mu23_bBmux_BsmuDsX_L1MU18VFCH', l1SeedThresholds=['MU18VFCH'], stream=["BphysDelayed"], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU18VFCH']),
        ChainProp(name='HLT_mu23_bBmux_LbmuLcX_L1MU18VFCH', l1SeedThresholds=['MU18VFCH'], stream=["BphysDelayed"], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU18VFCH']),

        #-- non-PEB JPsi
        ChainProp(name='HLT_mu10_bJpsimutrk_L1MU8F', l1SeedThresholds=['MU8F'], stream=[PhysicsStream,"express"], groups=SupportGroup+BphysicsGroup+['RATE:CPS_MU8F'], monGroups=['bphysMon:shifter']),
        ChainProp(name='HLT_mu20_bJpsimutrk_L1MU14FCH', l1SeedThresholds=['MU14FCH'], stream=["BphysDelayed","express"], groups=SupportGroup+BphysicsGroup+['RATE:CPS_MU14FCH'], monGroups=['bphysMon:t0']),
        ChainProp(name='HLT_mu20_bJpsimutrk_L1MU18VFCH', l1SeedThresholds=['MU18VFCH'], stream=["BphysDelayed","express"], groups=SupportGroup+BphysicsGroup+['RATE:CPS_MU18VFCH'], monGroups=['bphysMon:t0']),

        #-- supplementary PEB triggers
        ChainProp(name='HLT_mu4_bJpsimutrk_MuonTrkPEB_L1MU3V', l1SeedThresholds=['MU3V'], stream=['BphysPEB'], groups=SupportGroup+BphysicsGroup+['RATE:CPS_MU3V']),
        ChainProp(name='HLT_mu4_bJpsimutrk_MuonTrkPEB_L1MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysPEB'], groups=SupportGroup+BphysicsGroup),
        ChainProp(name='HLT_mu6_bJpsimutrk_MuonTrkPEB_L1MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysPEB'], groups=SupportGroup+BphysicsGroup+['RATE:CPS_MU5VF']),
        ChainProp(name='HLT_mu6_bJpsimutrk_lowpt_MuonTrkPEB_L1MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysPEB'], groups=SupportGroup+BphysicsGroup+['RATE:CPS_MU5VF']),
        ChainProp(name='HLT_mu10_bJpsimutrk_MuonTrkPEB_L1MU8F', l1SeedThresholds=['MU8F'], stream=['BphysPEB'], groups=SupportGroup+BphysicsGroup+['RATE:CPS_MU8F'], monGroups=['bphysMon:online']),
        ChainProp(name='HLT_mu20_bJpsimutrk_MuonTrkPEB_L1MU14FCH', l1SeedThresholds=['MU14FCH'], stream=['BphysPEB'], groups=SupportGroup+BphysicsGroup+['RATE:CPS_MU14FCH'], monGroups=['bphysMon:online']),
        ChainProp(name='HLT_mu20_bJpsimutrk_MuonTrkPEB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH'], stream=['BphysPEB'], groups=SupportGroup+BphysicsGroup+['RATE:CPS_MU18VFCH'], monGroups=['bphysMon:online']),

        # 3mu inv mass (ATR-19355, ATR-19638)
        ChainProp(name='HLT_3mu4_b3mu_noos_L1BPH-0M10-3MU3V', l1SeedThresholds=['MU3V'], stream=["BphysDelayed"], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        ChainProp(name='HLT_3mu4_b3mu_noos_L1BPH-0M10-3MU3VF', l1SeedThresholds=['MU3VF'], stream=["BphysDelayed"], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        ChainProp(name='HLT_3mu4_b3mu_L1BPH-0M10C-3MU3V', l1SeedThresholds=['MU3V'], stream=["BphysDelayed"], groups=BphysicsGroup+PrimaryL1MuGroup+Topo3Group),
        # ATR-25512
        ChainProp(name='HLT_3mu4_b3mu_noos_L13MU3V', l1SeedThresholds=['MU3V'], stream=["BphysDelayed"], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_3mu4_b3mu_noos_L13MU3VF', l1SeedThresholds=['MU3VF'], stream=["BphysDelayed"], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_3mu4_b3mu_L13MU3V', l1SeedThresholds=['MU3V'], stream=["BphysDelayed"], groups=BphysicsGroup+PrimaryL1MuGroup),

    ]

    chains['Combined'] += [
        # ATR-31537
        ChainProp(name='HLT_j40c_020jvt_j35c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_L1BTAG-MU5VFjJ20_2jJ40p0ETA25_jJ50p0ETA25'            , l1SeedThresholds=['FSNOSEED']*5,  stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup+Topo2Group),
        ChainProp(name='HLT_j40c_020jvt_j35c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L1BTAG-MU5VFjJ20_2jJ40p0ETA25_jJ50p0ETA25'            , l1SeedThresholds=['FSNOSEED']*5,  stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup+Topo2Group),
        ChainProp(name='HLT_j40c_nnJvtv1_j35c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_2j20c_nnJvtv1_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L1BTAG-MU5VFjJ20_2jJ40p0ETA25_jJ50p0ETA25'            , l1SeedThresholds=['FSNOSEED']*5,  stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup+Topo2Group),
        ChainProp(name='HLT_j40c_nnJvtv1_j35c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_2j20c_nnJvtv1_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_L1BTAG-MU5VFjJ20_2jJ40p0ETA25_jJ50p0ETA25'            , l1SeedThresholds=['FSNOSEED']*5,  stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup+Topo2Group),
        ChainProp(name='HLT_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_L1BTAG-MU5VFjJ20_2jJ40p0ETA25_jJ50p0ETA25'            , l1SeedThresholds=['FSNOSEED']*5,  stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup+Topo2Group),
        ChainProp(name='HLT_mu6_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_L1BTAG-MU5VFjJ20_2jJ40p0ETA25_jJ50p0ETA25'                         , l1SeedThresholds=['MU5VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup+Topo2Group),
        ChainProp(name='HLT_mu6_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn277_j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_dRAF04_L1BTAG-MU5VFjJ20_2jJ40p0ETA25_jJ50p0ETA25', l1SeedThresholds=['MU5VF']+['FSNOSEED']*6,  stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup+Topo2Group),

        ChainProp(name='HLT_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L1BTAG-MU5VFjJ20_2jJ40p0ETA25_jJ50p0ETA25'            , l1SeedThresholds=['FSNOSEED']*5,  stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup+Topo2Group),
        ChainProp(name='HLT_mu6_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L1BTAG-MU5VFjJ20_2jJ40p0ETA25_jJ50p0ETA25'                         , l1SeedThresholds=['MU5VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup+Topo2Group),
        ChainProp(name='HLT_mu6_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn277_j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_dRAF04_L1BTAG-MU5VFjJ20_2jJ40p0ETA25_jJ50p0ETA25', l1SeedThresholds=['MU5VF']+['FSNOSEED']*6,  stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup+Topo2Group),

        ChainProp(name='HLT_j55c_nnJvtv1_j40c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_2j20c_nnJvtv1_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L1BTAG-MU5VFjJ20_2jJ40p0ETA25_jJ50p0ETA25'            , l1SeedThresholds=['FSNOSEED']*5,  stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup+Topo2Group),
        ChainProp(name='HLT_mu6_j55c_nnJvtv1_j40c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_2j20c_nnJvtv1_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L1BTAG-MU5VFjJ20_2jJ40p0ETA25_jJ50p0ETA25'                         , l1SeedThresholds=['MU5VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup+Topo2Group),
        ChainProp(name='HLT_mu6_j55c_nnJvtv1_j40c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_j20c_nnJvtv1_bgn277_j20c_nnJvtv1_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_dRAF04_L1BTAG-MU5VFjJ20_2jJ40p0ETA25_jJ50p0ETA25', l1SeedThresholds=['MU5VF']+['FSNOSEED']*6,  stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup+Topo2Group),

        # AFP+dijet backup chains, discussed in ATR-24813
        ChainProp(name='HLT_2j120_mb_afprec_afpdijet_L1AFP_A_AND_C', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream], groups=MinBiasGroup+SupportGroup, monGroups=['mbMon:shifter']),
        ChainProp(name='HLT_2j175_mb_afprec_afpdijet_L1AFP_A_AND_C', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream], groups=MinBiasGroup+SupportGroup, monGroups=['mbMon:t0']),

        #ATR-27257
        ChainProp(name='HLT_2j120_mb_afprec_afpdijet_L1jJ160', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:shifter']),
        ChainProp(name='HLT_2j175_mb_afprec_afpdijet_L1jJ160', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),

        ChainProp(name='HLT_2j120_mb_afprec_afpdijet_L1AFP_A_AND_C_TOF_jJ90', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:shifter']),
        ChainProp(name='HLT_2j175_mb_afprec_afpdijet_L1AFP_A_AND_C_TOF_jJ125', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),

        
        # Primary e-mu chains
        ChainProp(name='HLT_e17_lhloose_mu14_L1eEM18L_MU8F', l1SeedThresholds=['eEM18L','MU8F'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),
        #ChainProp(name='HLT_e12_lhloose_2mu10_L12MU8F', l1SeedThresholds=['EM8VH','MU8F'], stream=[PhysicsStream], groups=PrimaryLegGroup+EgammaMuonGroup), #ATR-27654
        ChainProp(name='HLT_e7_lhmedium_L1eEM5_mu24_L1MU14FCH',l1SeedThresholds=['eEM5','MU14FCH'],  stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),# moved from Dev to Phy
        # ATR-25512
        ChainProp(name='HLT_e7_lhmedium_L1eEM5_mu24_L1MU18VFCH',l1SeedThresholds=['eEM5','MU18VFCH'],  stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),# moved from Dev to Phy
        ChainProp(name='HLT_e12_lhloose_L1eEM10L_2mu10_L12MU8F', l1SeedThresholds=['eEM10L','MU8F'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),# moved from Dev to Phy
        ChainProp(name='HLT_2e12_lhloose_mu10_L12eEM10L_MU8F', l1SeedThresholds=['eEM10L','MU8F'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),

        # Primary g-mu chains
        ChainProp(name='HLT_g25_medium_L1eEM18L_mu24_L1MU14FCH',l1SeedThresholds=['eEM18L','MU14FCH'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup), #ATR-22594 moved from Dev to Phy
        # ATR-25512
        ChainProp(name='HLT_g25_medium_L1eEM18L_mu24_L1MU18VFCH',l1SeedThresholds=['eEM18L','MU18VFCH'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup), #ATR-22594 moved from Dev to Phy
        
        ChainProp(name='HLT_g35_loose_mu18_L1eEM28M', l1SeedThresholds=['eEM28M','MU8F'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_2g10_loose_L1eEM9_mu20_L1MU14FCH', l1SeedThresholds=['eEM9','MU14FCH'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup), # unsure what eEM seed should be
        # ATR-25512
        ChainProp(name='HLT_2g10_loose_L1eEM9_mu23_L1MU18VFCH', l1SeedThresholds=['eEM9','MU18VFCH'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup), # unsure what eEM seed should be
        
        #---------- support 2m + 1g + ZRad triggers
        #ChainProp(name='HLT_2mu14_g20_tight_icaloloose_probe_L12MU8F', l1SeedThresholds=['MU8F','PROBEeEM18M'],groups=TagAndProbePhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_g20_tight_probe_L1eEM18M_2mu14_L12MU8F', l1SeedThresholds=['PROBEeEM18M','MU8F'],groups=TagAndProbePhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_g22_tight_probe_L1eEM18M_2mu14_L12MU8F', l1SeedThresholds=['PROBEeEM18M','MU8F'],groups=TagAndProbePhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_g25_medium_probe_L1eEM24L_2mu14_L12MU8F', l1SeedThresholds=['PROBEeEM24L','MU8F'],groups=TagAndProbePhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_g35_medium_probe_L1eEM24L_2mu14_L12MU8F', l1SeedThresholds=['PROBEeEM24L', 'MU8F'],groups=TagAndProbePhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_g50_loose_probe_L1eEM24L_2mu14_L12MU8F', l1SeedThresholds=['PROBEeEM24L', 'MU8F'],groups=TagAndProbePhIGroup+EgammaMuonGroup),

        #LLP
        ChainProp(name='HLT_g15_loose_L1eEM10L_2mu10_msonly_L12MU8F', l1SeedThresholds=['eEM10L','MU8F'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),

        #ATR-23732 Displaced Jet Trigger
        #ATR-27257, Phase-I
        ChainProp(name='HLT_j180_2dispjet50_3d2p_L1jJ160', groups=SingleJetGroup+UnconvTrkGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED']*2, monGroups=['idMon:online', 'idMon:t0']),
        ChainProp(name='HLT_j180_dispjet50_3d2p_dispjet50_1p_L1jJ160', groups=SingleJetGroup+UnconvTrkGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED']*3),
        ChainProp(name='HLT_j180_dispjet90_x3d1p_L1jJ160', groups=SingleJetGroup+UnconvTrkGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED']*2),
        ChainProp(name='HLT_j180_dispjet100_x3d1p_L1jJ160', groups=SingleJetGroup+UnconvTrkGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED']*2, monGroups=['idMon:online', 'idMon:t0']),

        #ATR-25456
        # Combined slice - Primary
        #ATR-2751, Phase-I
        ChainProp(name='HLT_g60_tight_2j50_0eta200_emergingPTF0p1dR0p4_pf_ftf_L1eEM26M', groups=SupportPhIGroup+EgammaJetGroup+['RATE:CPS_eEM26M'], l1SeedThresholds=['eEM26M','FSNOSEED']),
        ChainProp(name='HLT_g45_tight_2j50_0eta200_emergingPTF0p1dR0p4_pf_ftf_L1eEM26M', groups=SupportPhIGroup+EgammaJetGroup+['RATE:CPS_eEM26M'], l1SeedThresholds=['eEM26M','FSNOSEED']),
        ChainProp(name='HLT_g60_tight_icaloloose_2j50_0eta200_emergingPTF0p1dR0p4_pf_ftf_L1eEM26M', groups=PrimaryPhIGroup+EgammaJetGroup, l1SeedThresholds=['eEM26M','FSNOSEED']),
        ChainProp(name='HLT_g45_tight_icaloloose_2j50_0eta200_emergingPTF0p1dR0p4_pf_ftf_L1eEM26M', groups=PrimaryPhIGroup+EgammaJetGroup, l1SeedThresholds=['eEM26M','FSNOSEED']),

#
        ChainProp(name='HLT_g60_tight_2j50_0eta200_emergingPTF0p1dR0p4_pf_ftf_L1eEM28M', groups=SupportPhIGroup+EgammaJetGroup+['RATE:CPS_eEM28M'], l1SeedThresholds=['eEM28M','FSNOSEED']),
        ChainProp(name='HLT_g45_tight_2j50_0eta200_emergingPTF0p1dR0p4_pf_ftf_L1eEM28M', groups=SupportPhIGroup+EgammaJetGroup+['RATE:CPS_eEM28M'], l1SeedThresholds=['eEM28M','FSNOSEED']),
        ChainProp(name='HLT_g60_tight_icaloloose_2j50_0eta200_emergingPTF0p1dR0p4_pf_ftf_L1eEM28M', groups=PrimaryPhIGroup+EgammaJetGroup, l1SeedThresholds=['eEM28M','FSNOSEED']),
        ChainProp(name='HLT_g45_tight_icaloloose_2j50_0eta200_emergingPTF0p1dR0p4_pf_ftf_L1eEM28M', groups=PrimaryPhIGroup+EgammaJetGroup, l1SeedThresholds=['eEM28M','FSNOSEED']),

        #ATR-22107
        ChainProp(name='HLT_e9_lhvloose_L1eEM5_mu20_mu8noL1_L1MU14FCH', l1SeedThresholds=['eEM5','MU14FCH','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),

        
        #ATR-27251, Phase-I
        ChainProp(name='HLT_e26_lhmedium_mu8noL1_L1eEM26M', l1SeedThresholds=['eEM26M','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_e28_lhmedium_mu8noL1_L1eEM28M', l1SeedThresholds=['eEM28M','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),

        # ATR-25512
        ChainProp(name='HLT_e9_lhvloose_L1eEM5_mu23_mu8noL1_L1MU18VFCH', l1SeedThresholds=['eEM5','MU18VFCH','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),

        #ATR-27251, Phase-I
        ChainProp(name='HLT_g35_loose_mu15_mu2noL1_L1eEM28M', l1SeedThresholds=['eEM28M','MU5VF','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_g35_tight_icalotight_mu15noL1_mu2noL1_L1eEM28M', l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_g35_tight_icalotight_mu18noL1_L1eEM28M', l1SeedThresholds=['eEM28M','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_g35_tight_mu15noL1_mu2noL1_L1eEM28M', l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=SupportPhIGroup+EgammaMuonGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_g35_tight_mu18noL1_L1eEM28M', l1SeedThresholds=['eEM28M','FSNOSEED'], stream=[PhysicsStream], groups=SupportPhIGroup+EgammaMuonGroup+['RATE:CPS_eEM28M']),

        # Late stream for LLP
        ChainProp(name='HLT_g15_loose_L1eEM10L_2mu10_msonly_L1MU3V_EMPTY', l1SeedThresholds=['eEM10L','MU3V'], stream=['Late'], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_g15_loose_L1eEM10L_2mu10_msonly_L12MU5VF_EMPTY', l1SeedThresholds=['eEM10L','MU5VF'], stream=['Late'], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_g15_loose_L1eEM10L_2mu10_msonly_L1MU3V_UNPAIRED_ISO', l1SeedThresholds=['eEM10L','MU3V'], stream=['Late'], groups=PrimaryPhIGroup+EgammaMuonGroup),


        # Tau + Muon primaries
        ChainProp(name='HLT_mu14_ivarloose_tau25_mediumGNTau_03dRAB_L1MU8F_cTAU20M_3jJ30', l1SeedThresholds=['MU8F', 'cTAU20M'], groups=PrimaryPhIGroup+MuonTauGroup),
        ChainProp(name='HLT_mu14_ivarloose_tau35_mediumGNTau_03dRAB_L1cTAU30M_3DR35-MU8F-eTAU30', l1SeedThresholds=['MU8F', 'cTAU30M'], groups=PrimaryPhIGroup+MuonTauGroup+Topo3Group),
        ChainProp(name='HLT_mu20_ivarloose_tau20_mediumGNTau_L1eTAU12_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH', 'eTAU12'], groups=PrimaryPhIGroup+MuonTauGroup),
        ChainProp(name='HLT_mu23_ivarloose_tau20_mediumGNTau_L1eTAU12_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH', 'eTAU12'], groups=PrimaryPhIGroup+MuonTauGroup),


        # Tau + Electron primaries
        ChainProp(name='HLT_e17_lhmedium_ivarloose_tau25_mediumGNTau_03dRAB_L1eEM18M_2cTAU20M_4jJ30', l1SeedThresholds=['eEM18M', 'cTAU20M'], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_e24_lhmedium_ivarloose_tau20_mediumGNTau_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M', 'eTAU12'], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_e24_lhmedium_ivarloose_tau20_mediumGNTau_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M', 'eTAU12'], groups=PrimaryPhIGroup+EgammaTauGroup),


        # Tau + MET primaries
        ChainProp(name='HLT_tau50_mediumGNTau_xe80_tcpufit_xe50_cell_L1jXE100', l1SeedThresholds=['cTAU35M','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_xe80_pfopufit_xe50_cell_L1jXE100', l1SeedThresholds=['cTAU35M','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau50_mediumGNTau_xe80_tcpufit_xe50_cell_L1gXEJWOJ100', l1SeedThresholds=['cTAU35M','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_xe80_pfopufit_xe50_cell_L1gXEJWOJ100', l1SeedThresholds=['cTAU35M','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau50_mediumGNTau_xe80_tcpufit_xe50_cell_L1jXE110', l1SeedThresholds=['cTAU35M','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_xe80_pfopufit_xe50_cell_L1jXE110', l1SeedThresholds=['cTAU35M','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau50_mediumGNTau_xe80_tcpufit_xe50_cell_L1gXEJWOJ110', l1SeedThresholds=['cTAU35M','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_xe80_pfopufit_xe50_cell_L1gXEJWOJ110', l1SeedThresholds=['cTAU35M','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),


        # Di-tau (Tau + Muon/Electron/Tau) + MET primaries
        ChainProp(name='HLT_tau60_mediumGNTau_tau25_mediumGNTau_xe50_cell_03dRAB_L1eTAU60_2cTAU20M_jXE80', l1SeedThresholds=['eTAU60', 'cTAU20M', 'FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),  
        ChainProp(name='HLT_e17_lhmedium_tau25_mediumGNTau_xe50_cell_03dRAB_L1eEM18M_2cTAU20M_jXE70', l1SeedThresholds=['eEM18M', 'cTAU20M', 'FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),
        ChainProp(name='HLT_mu14_tau25_mediumGNTau_xe50_cell_03dRAB_L1MU8F_cTAU20M_jXE70', l1SeedThresholds=['MU8F', 'cTAU20M', 'FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),



        # Mu + Tau T&P chains
        ChainProp(name='HLT_mu26_ivarmedium_tau20_mediumGNTau_probe_L1eTAU12_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU12'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau20_mediumGNTau_probe_L1cTAU12M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU12M'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau20_mediumGNTau_probe_L1cTAU20M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau25_mediumGNTau_probe_L1eTAU12_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU12'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau25_mediumGNTau_probe_L1eTAU20_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau25_idperf_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['idMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau25_perf_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau25_mediumGNTau_probe_L1cTAU20M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau30_mediumGNTau_probe_L1eTAU20_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau30_mediumGNTau_probe_L1cTAU20M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau30_mediumGNTau_probe_L1cTAU30M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau35_mediumGNTau_probe_L1cTAU30M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau40_mediumGNTau_probe_L1cTAU35M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU35M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau50_mediumGNTau_probe_L1cTAU35M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU35M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau50_mediumGNTau_probe_L1cTAU50M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU50M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau60_mediumGNTau_probe_L1eTAU60_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU60'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau70_mediumGNTau_probe_L1eTAU70_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU70'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau80_mediumGNTau_probe_L1eTAU80_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU80'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau160_mediumGNTau_probe_L1eTAU140_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU140'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau60_mediumRNN_tracktwoLLP_probe_L1eTAU60_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU60'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau80_mediumRNN_tracktwoLLP_probe_L1eTAU80_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU80'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau180_mediumRNN_tracktwoLLP_probe_L1eTAU140_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU140'], groups=TagAndProbePhIGroup+SingleMuonGroup),

        ChainProp(name='HLT_mu26_ivarmedium_tau20_mediumGNTau_probe_L1eTAU12_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU12'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau20_mediumGNTau_probe_L1cTAU12M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU12M'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau20_mediumGNTau_probe_L1cTAU20M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau25_mediumGNTau_probe_L1eTAU12_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU12'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau25_mediumGNTau_probe_L1eTAU20_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau25_idperf_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['idMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau25_perf_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau25_mediumGNTau_probe_L1cTAU20M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau30_mediumGNTau_probe_L1eTAU20_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau30_mediumGNTau_probe_L1cTAU20M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau30_mediumGNTau_probe_L1cTAU30M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau35_mediumGNTau_probe_L1cTAU30M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau40_mediumGNTau_probe_L1cTAU35M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU35M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau50_mediumGNTau_probe_L1cTAU35M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU35M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau50_mediumGNTau_probe_L1cTAU50M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU50M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau60_mediumGNTau_probe_L1eTAU60_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU60'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau70_mediumGNTau_probe_L1eTAU70_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU70'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau80_mediumGNTau_probe_L1eTAU80_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU80'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau160_mediumGNTau_probe_L1eTAU140_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU140'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau60_mediumRNN_tracktwoLLP_probe_L1eTAU60_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU60'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau80_mediumRNN_tracktwoLLP_probe_L1eTAU80_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU80'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau180_mediumRNN_tracktwoLLP_probe_L1eTAU140_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU140'], groups=TagAndProbePhIGroup+SingleMuonGroup),

        ChainProp(name='HLT_mu24_ivarmedium_tau20_mediumGNTau_probe_L1eTAU12_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU12'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau20_mediumGNTau_probe_L1cTAU12M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU12M'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau20_mediumGNTau_probe_L1cTAU20M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau25_mediumGNTau_probe_L1eTAU12_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU12'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau25_mediumGNTau_probe_L1eTAU20_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau25_idperf_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['idMon:t0']),  
        ChainProp(name='HLT_mu24_ivarmedium_tau25_perf_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau25_mediumGNTau_probe_L1cTAU20M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau30_mediumGNTau_probe_L1eTAU20_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau30_mediumGNTau_probe_L1cTAU20M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau30_mediumGNTau_probe_L1cTAU30M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau35_mediumGNTau_probe_L1cTAU30M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau40_mediumGNTau_probe_L1cTAU35M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU35M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau50_mediumGNTau_probe_L1cTAU35M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU35M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau50_mediumGNTau_probe_L1cTAU50M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU50M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau60_mediumGNTau_probe_L1eTAU60_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU60'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau70_mediumGNTau_probe_L1eTAU70_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU70'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau80_mediumGNTau_probe_L1eTAU80_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU80'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau160_mediumGNTau_probe_L1eTAU140_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU140'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau60_mediumRNN_tracktwoLLP_probe_L1eTAU60_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU60'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau80_mediumRNN_tracktwoLLP_probe_L1eTAU80_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU80'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau180_mediumRNN_tracktwoLLP_probe_L1eTAU140_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU140'], groups=TagAndProbePhIGroup+SingleMuonGroup),

        ChainProp(name='HLT_mu24_ivarmedium_tau20_mediumGNTau_probe_L1eTAU12_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU12'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau20_mediumGNTau_probe_L1cTAU12M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU12M'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau20_mediumGNTau_probe_L1cTAU20M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau25_mediumGNTau_probe_L1eTAU12_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU12'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau25_mediumGNTau_probe_L1eTAU20_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau25_idperf_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['idMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau25_perf_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau25_mediumGNTau_probe_L1cTAU20M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau30_mediumGNTau_probe_L1eTAU20_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau30_mediumGNTau_probe_L1cTAU20M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau30_mediumGNTau_probe_L1cTAU30M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau35_mediumGNTau_probe_L1cTAU30M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau40_mediumGNTau_probe_L1cTAU35M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU35M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau50_mediumGNTau_probe_L1cTAU35M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU35M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau50_mediumGNTau_probe_L1cTAU50M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU50M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau60_mediumGNTau_probe_L1eTAU60_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU60'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau70_mediumGNTau_probe_L1eTAU70_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU70'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau80_mediumGNTau_probe_L1eTAU80_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU80'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau160_mediumGNTau_probe_L1eTAU140_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU140'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau60_mediumRNN_tracktwoLLP_probe_L1eTAU60_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU60'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau80_mediumRNN_tracktwoLLP_probe_L1eTAU80_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU80'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau180_mediumRNN_tracktwoLLP_probe_L1eTAU140_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU140'], groups=TagAndProbePhIGroup+SingleMuonGroup),


        # Electron + Tau T&P chains
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau20_mediumGNTau_probe_L1eTAU12_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeTAU12'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau20_mediumGNTau_probe_L1cTAU12M_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEcTAU12M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau20_mediumGNTau_probe_L1cTAU20M_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau25_mediumGNTau_probe_L1eTAU12_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeTAU12'], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau25_mediumGNTau_probe_L1eTAU20_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau25_mediumGNTau_probe_L1cTAU20M_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau30_mediumGNTau_probe_L1eTAU20_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau30_mediumGNTau_probe_L1cTAU20M_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau30_mediumGNTau_probe_L1cTAU30M_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau35_mediumGNTau_probe_L1cTAU30M_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau40_mediumGNTau_probe_L1cTAU35M_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEcTAU35M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau50_mediumGNTau_probe_L1cTAU35M_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEcTAU35M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau50_mediumGNTau_probe_L1cTAU50M_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEcTAU50M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau60_mediumGNTau_probe_L1eTAU60_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeTAU60'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau70_mediumGNTau_probe_L1eTAU70_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeTAU70'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau80_mediumGNTau_probe_L1eTAU80_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeTAU80'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau160_mediumGNTau_probe_L1eTAU140_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeTAU140'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau60_mediumRNN_tracktwoLLP_probe_L1eTAU60_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeTAU60'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau80_mediumRNN_tracktwoLLP_probe_L1eTAU80_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeTAU80'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau180_mediumRNN_tracktwoLLP_probe_L1eTAU140_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeTAU140'], groups=TagAndProbePhIGroup+SingleElectronGroup), 

        ChainProp(name='HLT_e28_lhtight_ivarloose_tau20_mediumGNTau_probe_L1eTAU12_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeTAU12'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau20_mediumGNTau_probe_L1cTAU12M_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEcTAU12M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau20_mediumGNTau_probe_L1cTAU20M_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau25_mediumGNTau_probe_L1eTAU12_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeTAU12'], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau25_mediumGNTau_probe_L1eTAU20_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau25_mediumGNTau_probe_L1cTAU20M_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau30_mediumGNTau_probe_L1eTAU20_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau30_mediumGNTau_probe_L1cTAU20M_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau30_mediumGNTau_probe_L1cTAU30M_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau35_mediumGNTau_probe_L1cTAU30M_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau40_mediumGNTau_probe_L1cTAU35M_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEcTAU35M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau50_mediumGNTau_probe_L1cTAU35M_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEcTAU35M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau50_mediumGNTau_probe_L1cTAU50M_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEcTAU50M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau60_mediumGNTau_probe_L1eTAU60_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeTAU60'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau70_mediumGNTau_probe_L1eTAU70_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeTAU70'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau80_mediumGNTau_probe_L1eTAU80_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeTAU80'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau160_mediumGNTau_probe_L1eTAU140_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeTAU140'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau60_mediumRNN_tracktwoLLP_probe_L1eTAU60_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeTAU60'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau80_mediumRNN_tracktwoLLP_probe_L1eTAU80_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeTAU80'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau180_mediumRNN_tracktwoLLP_probe_L1eTAU140_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeTAU140'], groups=TagAndProbePhIGroup+SingleElectronGroup),


        # Photon + Tau T&P chains (ATR-28791)
        ChainProp(name='HLT_g140_loose_tau20_mediumGNTau_probe_L1eTAU12_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M', 'PROBEeTAU12'], groups=TagAndProbePhIGroup+TauPhotonGroup),
        ChainProp(name='HLT_g140_loose_tau20_mediumGNTau_probe_L1eTAU12_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M', 'PROBEeTAU12'], groups=TagAndProbePhIGroup+TauPhotonGroup),

        ChainProp(name='HLT_g140_loose_j250_a10sd_cssk_ditauOmni1Trk5_pf_jes_ftf_preselj200_L1eEM26M', l1SeedThresholds=['eEM26M', 'FSNOSEED'], groups=TagAndProbePhIGroup+DiTauPhotonGroup),
        ChainProp(name='HLT_g140_loose_j250_a10sd_cssk_ditauOmni1Trk5_pf_jes_ftf_preselj200_L1eEM28M', l1SeedThresholds=['eEM28M', 'FSNOSEED'], groups=TagAndProbePhIGroup+DiTauPhotonGroup),
        

        # MET + Tau T&P chains (ATR-23507)     
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1cTAU12M_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU12M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1cTAU20M_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU12_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU20_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1cTAU20M_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1eTAU20_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1cTAU20M_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1cTAU30M_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumGNTau_probe_L1cTAU30M_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumGNTau_probe_L1cTAU35M_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_probe_L1cTAU35M_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_probe_L1cTAU50M_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU50M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumGNTau_probe_L1eTAU60_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU60', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau70_mediumGNTau_probe_L1eTAU70_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU70', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumGNTau_probe_L1eTAU80_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU80', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumGNTau_probe_L1eTAU140_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU140', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoLLP_probe_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoLLP_probe_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau180_mediumRNN_tracktwoLLP_probe_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1cTAU12M_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU12M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1cTAU20M_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU12_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU20_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1cTAU20M_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1eTAU20_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1cTAU20M_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1cTAU30M_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumGNTau_probe_L1cTAU30M_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumGNTau_probe_L1cTAU35M_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_probe_L1cTAU35M_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_probe_L1cTAU50M_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU50M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumGNTau_probe_L1eTAU60_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU60', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau70_mediumGNTau_probe_L1eTAU70_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU70', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumGNTau_probe_L1eTAU80_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU80', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumGNTau_probe_L1eTAU140_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU140', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoLLP_probe_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoLLP_probe_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau180_mediumRNN_tracktwoLLP_probe_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1cTAU12M_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU12M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1cTAU20M_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU12_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU20_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1cTAU20M_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1eTAU20_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1cTAU20M_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1cTAU30M_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumGNTau_probe_L1cTAU30M_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumGNTau_probe_L1cTAU35M_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_probe_L1cTAU35M_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_probe_L1cTAU50M_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU50M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumGNTau_probe_L1eTAU60_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU60', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau70_mediumGNTau_probe_L1eTAU70_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU70', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumGNTau_probe_L1eTAU80_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU80', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumGNTau_probe_L1eTAU140_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU140', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoLLP_probe_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoLLP_probe_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau180_mediumRNN_tracktwoLLP_probe_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1cTAU12M_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU12M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1cTAU20M_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU12_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU20_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1cTAU20M_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1eTAU20_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1cTAU20M_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1cTAU30M_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumGNTau_probe_L1cTAU30M_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumGNTau_probe_L1cTAU35M_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_probe_L1cTAU35M_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_probe_L1cTAU50M_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU50M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumGNTau_probe_L1eTAU60_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU60', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau70_mediumGNTau_probe_L1eTAU70_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU70', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumGNTau_probe_L1eTAU80_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU80', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumGNTau_probe_L1eTAU140_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU140', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoLLP_probe_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoLLP_probe_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau180_mediumRNN_tracktwoLLP_probe_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1cTAU12M_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU12M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1cTAU20M_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU12_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU20_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1cTAU20M_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1eTAU20_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1cTAU20M_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1cTAU30M_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumGNTau_probe_L1cTAU30M_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumGNTau_probe_L1cTAU35M_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_probe_L1cTAU35M_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_probe_L1cTAU50M_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU50M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumGNTau_probe_L1eTAU60_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU60', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau70_mediumGNTau_probe_L1eTAU70_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU70', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumGNTau_probe_L1eTAU80_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU80', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumGNTau_probe_L1eTAU140_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU140', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoLLP_probe_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoLLP_probe_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau180_mediumRNN_tracktwoLLP_probe_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1cTAU12M_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU12M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1cTAU20M_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU12_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU20_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1cTAU20M_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1eTAU20_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1cTAU20M_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1cTAU30M_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumGNTau_probe_L1cTAU30M_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumGNTau_probe_L1cTAU35M_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_probe_L1cTAU35M_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_probe_L1cTAU50M_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU50M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumGNTau_probe_L1eTAU60_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU60', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau70_mediumGNTau_probe_L1eTAU70_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU70', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumGNTau_probe_L1eTAU80_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU80', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumGNTau_probe_L1eTAU140_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU140', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoLLP_probe_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoLLP_probe_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau180_mediumRNN_tracktwoLLP_probe_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1cTAU12M_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU12M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1cTAU20M_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU12_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU20_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1cTAU20M_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1eTAU20_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1cTAU20M_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1cTAU30M_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumGNTau_probe_L1cTAU30M_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumGNTau_probe_L1cTAU35M_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_probe_L1cTAU35M_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_probe_L1cTAU50M_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU50M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumGNTau_probe_L1eTAU60_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU60', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau70_mediumGNTau_probe_L1eTAU70_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU70', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumGNTau_probe_L1eTAU80_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU80', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumGNTau_probe_L1eTAU140_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU140', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoLLP_probe_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoLLP_probe_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau180_mediumRNN_tracktwoLLP_probe_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1cTAU12M_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU12M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1cTAU20M_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU12_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU20_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1cTAU20M_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1eTAU20_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1cTAU20M_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1cTAU30M_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumGNTau_probe_L1cTAU30M_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumGNTau_probe_L1cTAU35M_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_probe_L1cTAU35M_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_probe_L1cTAU50M_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU50M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumGNTau_probe_L1eTAU60_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU60', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau70_mediumGNTau_probe_L1eTAU70_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU70', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumGNTau_probe_L1eTAU80_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU80', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumGNTau_probe_L1eTAU140_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU140', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoLLP_probe_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoLLP_probe_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau180_mediumRNN_tracktwoLLP_probe_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1cTAU12M_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU12M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1cTAU20M_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU12_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU20_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1cTAU20M_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1eTAU20_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1cTAU20M_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1cTAU30M_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumGNTau_probe_L1cTAU30M_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumGNTau_probe_L1cTAU35M_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_probe_L1cTAU35M_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_probe_L1cTAU50M_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU50M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumGNTau_probe_L1eTAU60_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU60', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau70_mediumGNTau_probe_L1eTAU70_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU70', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumGNTau_probe_L1eTAU80_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU80', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumGNTau_probe_L1eTAU140_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU140', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoLLP_probe_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoLLP_probe_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau180_mediumRNN_tracktwoLLP_probe_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1cTAU12M_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU12M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1cTAU20M_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU12_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU20_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1cTAU20M_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1eTAU20_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1cTAU20M_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1cTAU30M_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumGNTau_probe_L1cTAU30M_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumGNTau_probe_L1cTAU35M_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_probe_L1cTAU35M_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_probe_L1cTAU50M_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU50M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumGNTau_probe_L1eTAU60_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU60', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau70_mediumGNTau_probe_L1eTAU70_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU70', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumGNTau_probe_L1eTAU80_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU80', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumGNTau_probe_L1eTAU140_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU140', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoLLP_probe_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoLLP_probe_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau180_mediumRNN_tracktwoLLP_probe_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1cTAU12M_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU12M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1cTAU20M_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU12_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU20_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1cTAU20M_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1eTAU20_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1cTAU20M_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1cTAU30M_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumGNTau_probe_L1cTAU30M_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumGNTau_probe_L1cTAU35M_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_probe_L1cTAU35M_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_probe_L1cTAU50M_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU50M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumGNTau_probe_L1eTAU60_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU60', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau70_mediumGNTau_probe_L1eTAU70_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU70', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumGNTau_probe_L1eTAU80_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU80', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumGNTau_probe_L1eTAU140_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU140', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoLLP_probe_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoLLP_probe_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau180_mediumRNN_tracktwoLLP_probe_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1cTAU12M_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU12M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1cTAU20M_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU12_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU20_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1cTAU20M_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1eTAU20_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1cTAU20M_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_probe_L1cTAU30M_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumGNTau_probe_L1cTAU30M_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumGNTau_probe_L1cTAU35M_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_probe_L1cTAU35M_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_probe_L1cTAU50M_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU50M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumGNTau_probe_L1eTAU60_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU60', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau70_mediumGNTau_probe_L1eTAU70_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU70', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumGNTau_probe_L1eTAU80_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU80', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumGNTau_probe_L1eTAU140_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU140', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoLLP_probe_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoLLP_probe_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau180_mediumRNN_tracktwoLLP_probe_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),


        # Jet + Tau T&P chains (ATR-24031)
        ChainProp(name='HLT_tau20_mediumGNTau_L1RD0_FILLED', l1SeedThresholds=['eTAU12'], groups=SupportPhIGroup+TauJetGroup+['RATE:CPS_RD0_FILLED']),
        # Commented for now because the corresponding jet trigger is out due to fluctuating counts
        #   ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_j15_pf_ftf_03dRAB_L1RD0_FILLED', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauJetGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_j25_pf_ftf_03dRAB_L1RD0_FILLED', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=SupportPhIGroup+TauJetGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_j35_pf_ftf_03dRAB_L1RD0_FILLED', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=SupportPhIGroup+TauJetGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_j45_pf_ftf_preselj20_03dRAB_L1jJ40', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=SupportPhIGroup+TauJetGroup+['RATE:CPS_jJ40']),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_j60_pf_ftf_preselj50_03dRAB_L1jJ50', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=SupportPhIGroup+TauJetGroup+['RATE:CPS_jJ50']),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_j85_pf_ftf_preselj50_03dRAB_L1jJ50', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=SupportPhIGroup+TauJetGroup+['RATE:CPS_jJ50']),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_j110_pf_ftf_preselj80_03dRAB_L1jJ60', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=SupportPhIGroup+TauJetGroup+['RATE:CPS_jJ60']),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_j175_pf_ftf_preselj140_03dRAB_L1jJ90', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=SupportPhIGroup+TauJetGroup+['RATE:CPS_jJ90']),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_j260_pf_ftf_preselj200_03dRAB_L1jJ125', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=SupportPhIGroup+TauJetGroup+['RATE:CPS_jJ125']),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_j360_pf_ftf_preselj225_03dRAB_L1jJ160', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=SupportPhIGroup+TauJetGroup+['RATE:CPS_jJ160']),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_j420_pf_ftf_preselj225_03dRAB_L1jJ160', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauJetGroup),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_j440_pf_ftf_preselj225_03dRAB_L1jJ160', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauJetGroup),


        # HH->2b2tau: Bjet + Tau primaries, including backups for additional CPU savings (ATR-28198)
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU12_j65c_nnJvtv1_j40c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_j20c_nnJvtv1_bgn285_pf_ftf_presel3c20XX1c20bgtwo85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU12_j65c_nnJvtv1_j40c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_j20c_nnJvtv1_bgn285_pf_ftf_presel2c20XX1c20bgtwo85XX1c20gntau90_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU12_j65c_nnJvtv1_j40c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_j20c_nnJvtv1_bgn285_pf_ftf_presel2c20XX1c20bgtwo85XX1c20gntau85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU12_j65c_nnJvtv1_j40c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_j20c_nnJvtv1_bgn285_pf_ftf_presel2c20XX1c20bgtwo82XX1c20gntau85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU12_j65c_nnJvtv1_j40c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_j20c_nnJvtv1_bgn285_pf_ftf_presel2c20XX1c20bgtwo82XX1c20gntau80_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU12_j65c_nnJvtv1_j40c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_j20c_nnJvtv1_bgn285_pf_ftf_presel2c20XX1c20bgtwo80XX1c20gntau80_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+TauBJetGroup),

        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU12_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel3c20XX1c20bgtwo85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU12_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo85XX1c20gntau90_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU12_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo85XX1c20gntau85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU12_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo82XX1c20gntau85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU12_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo82XX1c20gntau80_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU12_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo80XX1c20gntau80_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+TauBJetGroup),

        # EoF chains (ATR-29523)
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU12_j50c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel3c20XX1c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], stream=[PhysicsStream], groups=EOFTauBjetGroup+TauBJetGroup),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_j50c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel3c20XX1c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], stream=[PhysicsStream], groups=EOFTauBjetGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU12_2j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel3c20XX1c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+4*['FSNOSEED'], stream=[PhysicsStream], groups=EOFTauBjetGroup+TauBJetGroup),
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_2j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel3c20XX1c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+4*['FSNOSEED'], stream=[PhysicsStream], groups=EOFTauBjetGroup+TauBJetGroup),
 
        
        # b-jet trigger calibration chains
        ChainProp(name='HLT_mu26_ivarmedium_2j20_0eta290_nnJvtv1_boffperf_pf_ftf_L1MU14FCH', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=TagAndProbeGroup+SingleMuonGroup, monGroups=['muonMon:online','bJetMon:online']),
        ChainProp(name='HLT_mu24_ivarmedium_2j20_0eta290_nnJvtv1_boffperf_pf_ftf_L1MU14FCH', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=TagAndProbeGroup+SingleMuonGroup, monGroups=['muonMon:online','bJetMon:online']),

        ChainProp(name='HLT_mu26_ivarmedium_2j20_0eta290_020jvt_boffperf_pf_ftf_L1MU14FCH', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=TagAndProbeGroup+SingleMuonGroup, monGroups=['muonMon:online','bJetMon:online']),
        ChainProp(name='HLT_mu24_ivarmedium_2j20_0eta290_020jvt_boffperf_pf_ftf_L1MU14FCH', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=TagAndProbeGroup+SingleMuonGroup, monGroups=['muonMon:online','bJetMon:online']),
        # ATR-25932
        ChainProp(name='HLT_mu26_ivarmedium_2j20_roiftf_preselj20_L1MU14FCH', l1SeedThresholds=['MU14FCH','FSNOSEED'], stream=[PhysicsStream], groups=TagAndProbeGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_2j20_roiftf_preselj20_L1MU14FCH', l1SeedThresholds=['MU14FCH','FSNOSEED'], stream=[PhysicsStream], groups=TagAndProbeGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_2j20_roiftf_preselj20_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','FSNOSEED'], stream=[PhysicsStream], groups=TagAndProbeGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_2j20_roiftf_preselj20_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','FSNOSEED'], stream=[PhysicsStream], groups=TagAndProbeGroup+SingleMuonGroup),
     
        

        # ATR 25512
        ChainProp(name='HLT_mu24_ivarmedium_2j20_0eta290_nnJvtv1_boffperf_pf_ftf_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','FSNOSEED'], groups=TagAndProbeGroup+SingleMuonGroup, monGroups=['muonMon:online','bJetMon:online']),
        ChainProp(name='HLT_mu26_ivarmedium_2j20_0eta290_nnJvtv1_boffperf_pf_ftf_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','FSNOSEED'], groups=TagAndProbeGroup+SingleMuonGroup, monGroups=['muonMon:online','bJetMon:online']),

        ChainProp(name='HLT_mu24_ivarmedium_2j20_0eta290_020jvt_boffperf_pf_ftf_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','FSNOSEED'], groups=TagAndProbeGroup+SingleMuonGroup, monGroups=['muonMon:online','bJetMon:online']),
        ChainProp(name='HLT_mu26_ivarmedium_2j20_0eta290_020jvt_boffperf_pf_ftf_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','FSNOSEED'], groups=TagAndProbeGroup+SingleMuonGroup, monGroups=['muonMon:online','bJetMon:online']),

        #ATR-27251, Phase-I
        ChainProp(name='HLT_e26_lhtight_ivarloose_mu22noL1_2j20_0eta290_nnJvtv1_bgn285_pf_ftf_L1eEM26M', l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'], stream=[PhysicsStream,'express'], groups=TagAndProbePhIGroup+EgammaBjetGroup, monGroups=['bJetMon:shifter','bJetMon:online']),
        ChainProp(name='HLT_e28_lhtight_ivarloose_mu22noL1_2j20_0eta290_nnJvtv1_bgn285_pf_ftf_L1eEM28M', l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'], stream=[PhysicsStream,'express'], groups=TagAndProbePhIGroup+EgammaBjetGroup, monGroups=['bJetMon:shifter','bJetMon:online']),

        ChainProp(name='HLT_e26_lhtight_ivarloose_mu22noL1_2j20_0eta290_020jvt_bgn285_pf_ftf_L1eEM26M', l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'], stream=[PhysicsStream,'express'], groups=TagAndProbePhIGroup+EgammaBjetGroup, monGroups=['bJetMon:shifter','bJetMon:online']),
        ChainProp(name='HLT_e28_lhtight_ivarloose_mu22noL1_2j20_0eta290_020jvt_bgn285_pf_ftf_L1eEM28M', l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'], stream=[PhysicsStream,'express'], groups=TagAndProbePhIGroup+EgammaBjetGroup, monGroups=['bJetMon:shifter','bJetMon:online']),


        # ATR-24698: muon + bjet chains for calibrations
        ChainProp(name='HLT_mu4_j20_0eta290_nnJvtv1_boffperf_pf_ftf_dRAB04_L1MU3V'    ,   l1SeedThresholds=['MU3V' ,'FSNOSEED'], groups=SupportGroup   +MuonBjetGroup, monGroups=['bJetMon:t0','muonMon:online','bJetMon:online'], stream=[PhysicsStream,'express']),
        ChainProp(name='HLT_mu6_j100_0eta290_nnJvtv1_boffperf_pf_ftf_dRAB04_L1MU5VF_jJ80', l1SeedThresholds=['MU5VF','FSNOSEED'], groups=SupportPhIGroup+MuonBjetGroup, monGroups=['bJetMon:t0','muonMon:online','bJetMon:online'], stream=[PhysicsStream,'express']),

        ChainProp(name='HLT_mu4_j20_0eta290_020jvt_boffperf_pf_ftf_dRAB04_L1MU3V'    ,   l1SeedThresholds=['MU3V' ,'FSNOSEED'], groups=SupportGroup   +MuonBjetGroup, monGroups=['bJetMon:t0','muonMon:online','bJetMon:online'], stream=[PhysicsStream,'express']),
        ChainProp(name='HLT_mu6_j100_0eta290_020jvt_boffperf_pf_ftf_dRAB04_L1MU5VF_jJ80', l1SeedThresholds=['MU5VF','FSNOSEED'], groups=SupportPhIGroup+MuonBjetGroup, monGroups=['bJetMon:t0','muonMon:online','bJetMon:online'], stream=[PhysicsStream,'express']),
        
        
        # ATR-28761 added some  muon + bjet Phase-1 chains
        ChainProp(name='HLT_mu4_j35_0eta290_nnJvtv1_boffperf_pf_ftf_dRAB04_L1MU3V_jJ40',   l1SeedThresholds=['MU3V' ,'FSNOSEED'], groups=SupportPhIGroup+MuonBjetGroup, monGroups=['muonMon:online','bJetMon:online'], stream=[PhysicsStream,         ]),
        ChainProp(name='HLT_mu4_j45_0eta290_nnJvtv1_boffperf_pf_ftf_dRAB04_L1MU3V_jJ40',   l1SeedThresholds=['MU3V' ,'FSNOSEED'], groups=SupportPhIGroup+MuonBjetGroup, monGroups=['bJetMon:shifter','muonMon:online','bJetMon:online'], stream=[PhysicsStream,'express']),
        ChainProp(name='HLT_mu6_j60_0eta290_nnJvtv1_boffperf_pf_ftf_dRAB04_L1MU3V_jJ40',   l1SeedThresholds=['MU3V' ,'FSNOSEED'], groups=SupportPhIGroup+MuonBjetGroup, monGroups=['muonMon:online','bJetMon:online'], stream=[PhysicsStream,         ]),

        ChainProp(name='HLT_mu4_j35_0eta290_020jvt_boffperf_pf_ftf_dRAB04_L1MU3V_jJ40',   l1SeedThresholds=['MU3V' ,'FSNOSEED'], groups=SupportPhIGroup+MuonBjetGroup, monGroups=['muonMon:online','bJetMon:online'], stream=[PhysicsStream,         ]),
        ChainProp(name='HLT_mu4_j45_0eta290_020jvt_boffperf_pf_ftf_dRAB04_L1MU3V_jJ40',   l1SeedThresholds=['MU3V' ,'FSNOSEED'], groups=SupportPhIGroup+MuonBjetGroup, monGroups=['bJetMon:shifter','muonMon:online','bJetMon:online'], stream=[PhysicsStream,'express']),
        ChainProp(name='HLT_mu6_j60_0eta290_020jvt_boffperf_pf_ftf_dRAB04_L1MU3V_jJ40',   l1SeedThresholds=['MU3V' ,'FSNOSEED'], groups=SupportPhIGroup+MuonBjetGroup, monGroups=['muonMon:online','bJetMon:online'], stream=[PhysicsStream,         ]),

        # ATR-24698: add muon + bjet L1Topo chains for calibration
        ChainProp(name='HLT_mu4_j35_0eta290_nnJvtv1_boffperf_pf_ftf_dRAB04_L1BTAG-MU3VjJ40',   l1SeedThresholds=['MU3V' ,'FSNOSEED'], groups=SupportPhIGroup+MuonBjetGroup+Topo2Group, monGroups=['muonMon:online','bJetMon:online'], stream=[PhysicsStream,         ]),
        ChainProp(name='HLT_mu4_j45_0eta290_nnJvtv1_boffperf_pf_ftf_dRAB04_L1BTAG-MU3VjJ40',   l1SeedThresholds=['MU3V' ,'FSNOSEED'], groups=SupportPhIGroup+MuonBjetGroup+Topo2Group, monGroups=['bJetMon:shifter','muonMon:online','bJetMon:online'], stream=[PhysicsStream,'express']),
        ChainProp(name='HLT_mu6_j60_0eta290_nnJvtv1_boffperf_pf_ftf_dRAB04_L1BTAG-MU3VjJ40',   l1SeedThresholds=['MU3V' ,'FSNOSEED'], groups=SupportPhIGroup+MuonBjetGroup+Topo2Group, monGroups=['muonMon:online','bJetMon:online'], stream=[PhysicsStream,         ]),
        ChainProp(name='HLT_mu6_j100_0eta290_nnJvtv1_boffperf_pf_ftf_dRAB04_L1BTAG-MU5VFjJ80', l1SeedThresholds=['MU5VF','FSNOSEED'], groups=SupportPhIGroup+MuonBjetGroup+Topo2Group, monGroups=['bJetMon:t0','muonMon:online','bJetMon:online'], stream=[PhysicsStream,'express']),

        ChainProp(name='HLT_mu4_j35_0eta290_020jvt_boffperf_pf_ftf_dRAB04_L1BTAG-MU3VjJ40',   l1SeedThresholds=['MU3V' ,'FSNOSEED'], groups=SupportPhIGroup+MuonBjetGroup+Topo2Group, monGroups=['muonMon:online','bJetMon:online'], stream=[PhysicsStream,         ]),
        ChainProp(name='HLT_mu4_j45_0eta290_020jvt_boffperf_pf_ftf_dRAB04_L1BTAG-MU3VjJ40',   l1SeedThresholds=['MU3V' ,'FSNOSEED'], groups=SupportPhIGroup+MuonBjetGroup+Topo2Group, monGroups=['bJetMon:shifter','muonMon:online','bJetMon:online'], stream=[PhysicsStream,'express']),
        ChainProp(name='HLT_mu6_j60_0eta290_020jvt_boffperf_pf_ftf_dRAB04_L1BTAG-MU3VjJ40',   l1SeedThresholds=['MU3V' ,'FSNOSEED'], groups=SupportPhIGroup+MuonBjetGroup+Topo2Group, monGroups=['muonMon:online','bJetMon:online'], stream=[PhysicsStream,         ]),
        ChainProp(name='HLT_mu6_j100_0eta290_020jvt_boffperf_pf_ftf_dRAB04_L1BTAG-MU5VFjJ80', l1SeedThresholds=['MU5VF','FSNOSEED'], groups=SupportPhIGroup+MuonBjetGroup+Topo2Group, monGroups=['bJetMon:t0','muonMon:online','bJetMon:online'], stream=[PhysicsStream,'express']),



        # jet JVT calibration triggers 
        ChainProp(name='HLT_mu26_ivarmedium_j20_pf_ftf_L1MU14FCH', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=TagAndProbeGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_j20_pf_ftf_L1MU14FCH', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=TagAndProbeGroup+SingleMuonGroup),
        #ATR-27251, Phase-I
        ChainProp(name='HLT_e26_lhtight_ivarloose_j20_pf_ftf_L1eEM26M', l1SeedThresholds=['eEM26M','FSNOSEED'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_j20_pf_ftf_L1eEM28M', l1SeedThresholds=['eEM28M','FSNOSEED'], groups=TagAndProbePhIGroup+SingleElectronGroup),



        # ATR-25932

        ChainProp(name='HLT_e26_lhtight_ivarloose_2j20_roiftf_preselj20_L1eEM26M', l1SeedThresholds=['eEM26M','FSNOSEED'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),

        ChainProp(name='HLT_e26_lhtight_ivarloose_mu22noL1_j20_roiftf_preselj20_L1eEM26M', l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+EgammaBjetGroup),

        # ATR-27373
        ChainProp(name='HLT_e28_lhtight_ivarloose_2j20_roiftf_preselj20_L1eEM28M', l1SeedThresholds=['eEM28M','FSNOSEED'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_mu22noL1_j20_roiftf_preselj20_L1eEM28M', l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+EgammaBjetGroup),

        # ATR-25512
        ChainProp(name='HLT_mu24_ivarmedium_j20_pf_ftf_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','FSNOSEED'], groups=TagAndProbeGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_j20_pf_ftf_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','FSNOSEED'], groups=TagAndProbeGroup+SingleMuonGroup),

        ChainProp(name='HLT_e26_lhtight_ivarloose_2j20_0eta290_nnJvtv1_boffperf_pf_ftf_L1eEM26M', l1SeedThresholds=['eEM26M','FSNOSEED'], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['bJetMon:online']),
        ChainProp(name='HLT_e26_lhtight_ivarloose_mu22noL1_j20_0eta290_nnJvtv1_boffperf_pf_ftf_L1eEM26M', l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'], stream=[PhysicsStream,'express'], groups=TagAndProbePhIGroup+EgammaBjetGroup, monGroups=['bJetMon:shifter','bJetMon:online']),

        ChainProp(name='HLT_e26_lhtight_ivarloose_2j20_0eta290_020jvt_boffperf_pf_ftf_L1eEM26M', l1SeedThresholds=['eEM26M','FSNOSEED'], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['bJetMon:online']),
        ChainProp(name='HLT_e26_lhtight_ivarloose_mu22noL1_j20_0eta290_020jvt_boffperf_pf_ftf_L1eEM26M', l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'], stream=[PhysicsStream,'express'], groups=TagAndProbePhIGroup+EgammaBjetGroup, monGroups=['bJetMon:shifter','bJetMon:online']),

        # ATR-27373
        ChainProp(name='HLT_e28_lhtight_ivarloose_2j20_0eta290_nnJvtv1_boffperf_pf_ftf_L1eEM28M', l1SeedThresholds=['eEM28M','FSNOSEED'], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['bJetMon:online']),
        ChainProp(name='HLT_e28_lhtight_ivarloose_mu22noL1_j20_0eta290_nnJvtv1_boffperf_pf_ftf_L1eEM28M', l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'], stream=[PhysicsStream,'express'], groups=TagAndProbePhIGroup+EgammaBjetGroup, monGroups=['bJetMon:shifter','bJetMon:online']),
    
        ChainProp(name='HLT_e28_lhtight_ivarloose_2j20_0eta290_020jvt_boffperf_pf_ftf_L1eEM28M', l1SeedThresholds=['eEM28M','FSNOSEED'], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['bJetMon:online']),
        ChainProp(name='HLT_e28_lhtight_ivarloose_mu22noL1_j20_0eta290_020jvt_boffperf_pf_ftf_L1eEM28M', l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'], stream=[PhysicsStream,'express'], groups=TagAndProbePhIGroup+EgammaBjetGroup, monGroups=['bJetMon:shifter','bJetMon:online']),

        # Phase-I L1Calo
        ChainProp(name='HLT_xe80_tcpufit_isotrk120_medium_iaggrmedium_L1jXE100', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream], groups=UnconvTrkGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_xe80_tcpufit_isotrk140_medium_iaggrmedium_L1jXE100', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream], groups=UnconvTrkGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_xe80_tcpufit_isotrk100_medium_iaggrmedium_L1jXE100', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream], groups=UnconvTrkGroup+SupportPhIGroup+['RATE:CPS_jXE100']),
        ChainProp(name='HLT_xe80_tcpufit_isotrk120_medium_iaggrloose_L1jXE100', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],  groups=UnconvTrkGroup+SupportPhIGroup+['RATE:CPS_jXE100']),
        #ATR-28679
        ChainProp(name='HLT_xe80_tcpufit_isotrk120_medium_iaggrmedium_L1jXE110', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream], groups=UnconvTrkGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_xe80_tcpufit_isotrk140_medium_iaggrmedium_L1jXE110', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream], groups=UnconvTrkGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_xe80_tcpufit_isotrk100_medium_iaggrmedium_L1jXE110', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream], groups=UnconvTrkGroup+SupportPhIGroup+['RATE:CPS_jXE110']),
        ChainProp(name='HLT_xe80_tcpufit_isotrk120_medium_iaggrloose_L1jXE110', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],  groups=UnconvTrkGroup+SupportPhIGroup+['RATE:CPS_jXE110']),

        ChainProp(name='HLT_xe80_tcpufit_isotrk120_medium_iaggrmedium_L1gXEJWOJ110', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream], groups=UnconvTrkGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_xe80_tcpufit_isotrk140_medium_iaggrmedium_L1gXEJWOJ110', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream], groups=UnconvTrkGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_xe80_tcpufit_isotrk100_medium_iaggrmedium_L1gXEJWOJ110', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream], groups=UnconvTrkGroup+SupportPhIGroup+['RATE:CPS_gXEJWOJ110']),
        ChainProp(name='HLT_xe80_tcpufit_isotrk120_medium_iaggrloose_L1gXEJWOJ110', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],  groups=UnconvTrkGroup+SupportPhIGroup+['RATE:CPS_gXEJWOJ110']),
        #ATR-28838
        ChainProp(name='HLT_xe80_tcpufit_isotrk120_medium_iaggrmedium_L1gXEJWOJ100', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream], groups=UnconvTrkGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_xe80_tcpufit_isotrk140_medium_iaggrmedium_L1gXEJWOJ100', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream], groups=UnconvTrkGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_xe80_tcpufit_isotrk100_medium_iaggrmedium_L1gXEJWOJ100', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream], groups=UnconvTrkGroup+SupportPhIGroup+['RATE:CPS_gXEJWOJ100']),
        ChainProp(name='HLT_xe80_tcpufit_isotrk120_medium_iaggrloose_L1gXEJWOJ100', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],  groups=UnconvTrkGroup+SupportPhIGroup+['RATE:CPS_gXEJWOJ100']),

        # electron + MET (ATR-22594)
        ChainProp(name='HLT_e70_lhloose_xe70_cell_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMETGroup),

        # ATR-27373
        ChainProp(name='HLT_e70_lhloose_xe70_cell_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMETGroup),


        ChainProp(name='HLT_j75_0eta290_nnJvtv1_bgn260_pf_ftf_xe60_cell_L12jJ90_jXE80', l1SeedThresholds=['FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup),

        ChainProp(name='HLT_j75_0eta290_020jvt_bgn260_pf_ftf_xe60_cell_L12jJ90_jXE80', l1SeedThresholds=['FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup),
        
        ChainProp(name='HLT_g25_medium_L1eEM26M_mu24_ivarmedium_L1MU14FCH', l1SeedThresholds=['eEM26M','MU14FCH'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),



        # ATR-25512
        ChainProp(name='HLT_g25_medium_L1eEM26M_mu24_ivarmedium_L1MU18VFCH', l1SeedThresholds=['eEM26M','MU18VFCH'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),


        # VBF triggers (ATR-22594)
        # Main stream VBF
        ChainProp(name='HLT_g25_medium_4j35a_j0_DJMASS1000j35_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaJetGroup),
        # ATR-27373
        ChainProp(name='HLT_g25_medium_4j35a_j0_DJMASS1000j35_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaJetGroup),

        #ATR-27251, Phase-I
        ChainProp(name='HLT_g35_medium_4j35a_j0_DJMASS1000j35_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaJetGroup),
        ChainProp(name='HLT_g35_tight_4j35a_j0_DJMASS1000j35_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaJetGroup),
        ChainProp(name='HLT_g35_medium_4j35a_j0_DJMASS1000j35_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaJetGroup),
        ChainProp(name='HLT_g35_tight_4j35a_j0_DJMASS1000j35_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaJetGroup),



        # Photon+VBF
        ChainProp(name='HLT_g20_tight_j35_0eta290_nnJvtv1_bgn277_3j35a_j0_DJMASS500j35_pf_ftf_L1eEM22M_jMJJ-300',l1SeedThresholds=['eEM22M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaBjetGroup+Topo2Group),

        ChainProp(name='HLT_g20_tight_j35_0eta290_020jvt_bgn277_3j35a_j0_DJMASS500j35_pf_ftf_L1eEM22M_jMJJ-300',l1SeedThresholds=['eEM22M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaBjetGroup+Topo2Group),

        # Muon-in-jet
        # ATR-26032
        ChainProp(name='HLT_mu10_j225_0eta290_nnJvtv1_boffperf_pf_ftf_preselj180_dRAB04_L1jJ160', l1SeedThresholds=['MU8F','FSNOSEED'], stream=[PhysicsStream], groups=SupportPhIGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu10_j300_0eta290_nnJvtv1_boffperf_pf_ftf_preselj225_dRAB04_L1jJ160', l1SeedThresholds=['MU8F','FSNOSEED'], stream=[PhysicsStream], groups=SupportPhIGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu10_j360_0eta290_nnJvtv1_boffperf_pf_ftf_preselj225_dRAB04_L1jJ160', l1SeedThresholds=['MU8F','FSNOSEED'], stream=[PhysicsStream], groups=SupportPhIGroup+SingleBjetGroup),
    
        ChainProp(name='HLT_mu10_j225_0eta290_020jvt_boffperf_pf_ftf_preselj180_dRAB04_L1jJ160', l1SeedThresholds=['MU8F','FSNOSEED'], stream=[PhysicsStream], groups=SupportPhIGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu10_j300_0eta290_020jvt_boffperf_pf_ftf_preselj225_dRAB04_L1jJ160', l1SeedThresholds=['MU8F','FSNOSEED'], stream=[PhysicsStream], groups=SupportPhIGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu10_j360_0eta290_020jvt_boffperf_pf_ftf_preselj225_dRAB04_L1jJ160', l1SeedThresholds=['MU8F','FSNOSEED'], stream=[PhysicsStream], groups=SupportPhIGroup+SingleBjetGroup),

        # Phase I inputs ATR-24411
        # SUSY
        ChainProp(name='HLT_g45_loose_6j45c_L14jJ40p0ETA25',l1SeedThresholds=['eEM18','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaJetGroup),
        
        # VBF triggers (ATR-22594)
        # Main stream
        ChainProp(name='HLT_e10_lhmedium_ivarloose_j70_j50a_j0_DJMASS900j50_L1jMJJ-500-NFF',l1SeedThresholds=['eEM10L','FSNOSEED','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+EgammaJetGroup+Topo3Group),
        ChainProp(name='HLT_mu10_ivarmedium_j70_j50a_j0_DJMASS900j50_L1jMJJ-500-NFF',l1SeedThresholds=['MU8F','FSNOSEED','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MuonJetGroup+Topo3Group),
        ChainProp(name='HLT_tau25_mediumGNTau_tau20_mediumGNTau_03dRAB_j70_j50a_j0_DJMASS900j50_L1jMJJ-500-NFF', l1SeedThresholds=['eTAU12', 'eTAU12', 'FSNOSEED', 'FSNOSEED', 'FSNOSEED'], groups=PrimaryPhIGroup+TauJetGroup+Topo3Group),
        # Delayed stream
        ChainProp(name='HLT_2mu6_2j50a_j0_DJMASS900j50_L1jMJJ-500-NFF',l1SeedThresholds=['MU5VF','FSNOSEED','FSNOSEED'],stream=['VBFDelayed'], groups=PrimaryPhIGroup+MuonJetGroup+Topo3Group), # Formerly HLT_2mu6_2j50_0eta490_invm900j50                       
        ChainProp(name='HLT_e5_lhvloose_j70_j50a_j0_DJMASS1000j50_xe50_tcpufit_L1jMJJ-500-NFF',l1SeedThresholds=['eEM5','FSNOSEED','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['VBFDelayed'], groups=PrimaryPhIGroup+EgammaJetGroup+Topo3Group),
        ChainProp(name='HLT_2e5_lhmedium_j70_j50a_j0_DJMASS900j50_L1jMJJ-500-NFF',l1SeedThresholds=['eEM5','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['VBFDelayed'], groups=PrimaryPhIGroup+EgammaJetGroup+Topo3Group),
        ChainProp(name='HLT_mu4_j70_j50a_j0_DJMASS1000j50_xe50_tcpufit_L1jMJJ-500-NFF',l1SeedThresholds=['MU3V','FSNOSEED','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['VBFDelayed'], groups=PrimaryPhIGroup+MuonJetGroup+Topo3Group),
        ChainProp(name='HLT_j70_j50a_j0_DJMASS1000j50dphi240_xe90_tcpufit_xe50_cell_L1jMJJ-500-NFF',l1SeedThresholds=['FSNOSEED']*5,stream=['VBFDelayed'], groups=PrimaryPhIGroup+JetMETGroup+Topo3Group),

        # Photon+VBF

        #H->Z+gamma with Z->qq/vv ATR-28443
        ChainProp(name='HLT_g24_tight_icaloloose_j50c_j30c_j24c_03dRAB30_03dRAC30_15dRBC45_50invmBC130_pf_ftf_L1eEM26M', groups=PrimaryPhIGroup+EgammaJetGroup, l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream]),
        ChainProp(name='HLT_g20_tight_icaloloose_j40_j35a_j20c_j0_DJMASS300j35_pf_ftf_L1eEM22M_jMJJ-300', groups=PrimaryPhIGroup+EgammaJetGroup+Topo2Group, l1SeedThresholds=['eEM22M','FSNOSEED','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream]),
        ChainProp(name='HLT_g20_tight_icaloloose_j40_j35a_j25a_j25_j20c_j0_DJMASS300j35_pf_ftf_L1eEM22M_jMJJ-300', groups=PrimaryPhIGroup+EgammaJetGroup+Topo2Group, l1SeedThresholds=['eEM22M','FSNOSEED','FSNOSEED','FSNOSEED','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream]),

        # fastGN2 + GN2
        # Photon+VBF
        ChainProp(name='HLT_g20_tight_icaloloose_j35_0eta290_nnJvtv1_bgn277_3j35a_j0_DJMASS500j35_pf_ftf_L1eEM22M_jMJJ-300',l1SeedThresholds=['eEM22M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaBjetGroup+Topo2Group),

        ChainProp(name='HLT_g20_tight_icaloloose_j35_0eta290_020jvt_bgn277_3j35a_j0_DJMASS500j35_pf_ftf_L1eEM22M_jMJJ-300',l1SeedThresholds=['eEM22M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaBjetGroup+Topo2Group),

        # B-jet preselection
        #ATR-27251, Phase-I
        ChainProp(name='HLT_g25_tight_2j35_0eta290_nnJvtv1_bgn277_2j35a_pf_ftf_presel2a20bgtwo90XX2a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g25_tight_j35_0eta290_nnJvtv1_bgn277_3j35a_j0_DJMASS700j35_pf_ftf_presela20bgtwo85XX3a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g25_tight_2j35_0eta290_nnJvtv1_bgn277_2j35a_pf_ftf_presel2a20bgtwo90XX2a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_g25_tight_j35_0eta290_nnJvtv1_bgn277_3j35a_j0_DJMASS700j35_pf_ftf_presela20bgtwo85XX3a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),
    
        ChainProp(name='HLT_g25_tight_2j35_0eta290_020jvt_bgn277_2j35a_pf_ftf_presel2a20bgtwo90XX2a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g25_tight_j35_0eta290_020jvt_bgn277_3j35a_j0_DJMASS700j35_pf_ftf_presela20bgtwo85XX3a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g25_tight_2j35_0eta290_020jvt_bgn277_2j35a_pf_ftf_presel2a20bgtwo90XX2a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_g25_tight_j35_0eta290_020jvt_bgn277_3j35a_j0_DJMASS700j35_pf_ftf_presela20bgtwo85XX3a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),

        #ATR-27373
        ChainProp(name='HLT_g35_medium_2j35_0eta290_nnJvtv1_bgn277_2j35a_pf_ftf_presel2a20bgtwo90XX2a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g35_medium_2j35_0eta290_nnJvtv1_bgn277_2j35a_pf_ftf_presel2a20bgtwo90XX2a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_g35_medium_j35_0eta290_nnJvtv1_bgn277_3j35a_j0_DJMASS700j35_pf_ftf_presela20bgtwo85XX3a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g35_medium_j35_0eta290_nnJvtv1_bgn277_3j35a_j0_DJMASS700j35_pf_ftf_presela20bgtwo85XX3a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_g35_tight_icaloloose_2j35_0eta290_nnJvtv1_bgn277_2j35a_pf_ftf_presel2a20bgtwo90XX2a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaBjetGroup),
        ChainProp(name='HLT_g35_tight_icaloloose_2j35_0eta290_nnJvtv1_bgn277_2j35a_pf_ftf_presel2a20bgtwo90XX2a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaBjetGroup),
        ChainProp(name='HLT_g35_tight_icaloloose_j35_0eta290_nnJvtv1_bgn277_3j35a_j0_DJMASS700j35_pf_ftf_presela20bgtwo85XX3a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaBjetGroup),
        ChainProp(name='HLT_g35_tight_icaloloose_j35_0eta290_nnJvtv1_bgn277_3j35a_j0_DJMASS700j35_pf_ftf_presela20bgtwo85XX3a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaBjetGroup),

        ChainProp(name='HLT_g35_medium_2j35_0eta290_020jvt_bgn277_2j35a_pf_ftf_presel2a20bgtwo90XX2a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g35_medium_2j35_0eta290_020jvt_bgn277_2j35a_pf_ftf_presel2a20bgtwo90XX2a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_g35_medium_j35_0eta290_020jvt_bgn277_3j35a_j0_DJMASS700j35_pf_ftf_presela20bgtwo85XX3a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g35_medium_j35_0eta290_020jvt_bgn277_3j35a_j0_DJMASS700j35_pf_ftf_presela20bgtwo85XX3a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_g35_tight_icaloloose_2j35_0eta290_020jvt_bgn277_2j35a_pf_ftf_presel2a20bgtwo90XX2a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaBjetGroup),
        ChainProp(name='HLT_g35_tight_icaloloose_2j35_0eta290_020jvt_bgn277_2j35a_pf_ftf_presel2a20bgtwo90XX2a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaBjetGroup),
        ChainProp(name='HLT_g35_tight_icaloloose_j35_0eta290_020jvt_bgn277_3j35a_j0_DJMASS700j35_pf_ftf_presela20bgtwo85XX3a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaBjetGroup),
        ChainProp(name='HLT_g35_tight_icaloloose_j35_0eta290_020jvt_bgn277_3j35a_j0_DJMASS700j35_pf_ftf_presela20bgtwo85XX3a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaBjetGroup),

        # photon + VBF Hbb (ATR-23293) GN1
        # B-jet preselection
        ChainProp(name='HLT_g25_tight_icaloloose_2j35_0eta290_nnJvtv1_bgn277_2j35a_pf_ftf_presel2a20bgtwo90XX2a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaBjetGroup),
        ChainProp(name='HLT_g25_tight_icaloloose_j35_0eta290_nnJvtv1_bgn277_3j35a_j0_DJMASS700j35_pf_ftf_presela20bgtwo85XX3a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaBjetGroup),
        ChainProp(name='HLT_g25_medium_2j35_0eta290_nnJvtv1_bgn277_2j35a_pf_ftf_presel2a20bgtwo90XX2a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g25_medium_j35_0eta290_nnJvtv1_bgn277_3j35a_j0_DJMASS700j35_pf_ftf_presela20bgtwo85XX3a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g25_tight_icaloloose_2j35_0eta290_nnJvtv1_bgn277_2j35a_pf_ftf_presel2a20bgtwo90XX2a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaBjetGroup),
        ChainProp(name='HLT_g25_tight_icaloloose_j35_0eta290_nnJvtv1_bgn277_3j35a_j0_DJMASS700j35_pf_ftf_presela20bgtwo85XX3a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaBjetGroup),
        ChainProp(name='HLT_g25_medium_2j35_0eta290_nnJvtv1_bgn277_2j35a_pf_ftf_presel2a20bgtwo90XX2a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_g25_medium_j35_0eta290_nnJvtv1_bgn277_3j35a_j0_DJMASS700j35_pf_ftf_presela20bgtwo85XX3a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_g35_tight_2j35_0eta290_nnJvtv1_bgn277_2j35a_pf_ftf_presel2a20bgtwo90XX2a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g35_tight_j35_0eta290_nnJvtv1_bgn277_3j35a_j0_DJMASS700j35_pf_ftf_presela20bgtwo85XX3a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g35_tight_2j35_0eta290_nnJvtv1_bgn277_2j35a_pf_ftf_presel2a20bgtwo90XX2a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_g35_tight_j35_0eta290_nnJvtv1_bgn277_3j35a_j0_DJMASS700j35_pf_ftf_presela20bgtwo85XX3a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),

        ChainProp(name='HLT_g25_tight_icaloloose_2j35_0eta290_020jvt_bgn277_2j35a_pf_ftf_presel2a20bgtwo90XX2a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaBjetGroup),
        ChainProp(name='HLT_g25_tight_icaloloose_j35_0eta290_020jvt_bgn277_3j35a_j0_DJMASS700j35_pf_ftf_presela20bgtwo85XX3a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaBjetGroup),
        ChainProp(name='HLT_g25_medium_2j35_0eta290_020jvt_bgn277_2j35a_pf_ftf_presel2a20bgtwo90XX2a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g25_medium_j35_0eta290_020jvt_bgn277_3j35a_j0_DJMASS700j35_pf_ftf_presela20bgtwo85XX3a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g25_tight_icaloloose_2j35_0eta290_020jvt_bgn277_2j35a_pf_ftf_presel2a20bgtwo90XX2a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaBjetGroup),
        ChainProp(name='HLT_g25_tight_icaloloose_j35_0eta290_020jvt_bgn277_3j35a_j0_DJMASS700j35_pf_ftf_presela20bgtwo85XX3a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaBjetGroup),
        ChainProp(name='HLT_g25_medium_2j35_0eta290_020jvt_bgn277_2j35a_pf_ftf_presel2a20bgtwo90XX2a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_g25_medium_j35_0eta290_020jvt_bgn277_3j35a_j0_DJMASS700j35_pf_ftf_presela20bgtwo85XX3a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_g35_tight_2j35_0eta290_020jvt_bgn277_2j35a_pf_ftf_presel2a20bgtwo90XX2a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g35_tight_j35_0eta290_020jvt_bgn277_3j35a_j0_DJMASS700j35_pf_ftf_presela20bgtwo85XX3a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g35_tight_2j35_0eta290_020jvt_bgn277_2j35a_pf_ftf_presel2a20bgtwo90XX2a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_g35_tight_j35_0eta290_020jvt_bgn277_3j35a_j0_DJMASS700j35_pf_ftf_presela20bgtwo85XX3a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),



        # LLP late stream
        ChainProp(name='HLT_j55c_xe50_cell_L1jJ60_EMPTY', l1SeedThresholds=['FSNOSEED']*2, stream=['Late'], groups=PrimaryPhIGroup+JetMETGroup),
        ChainProp(name='HLT_j55c_xe50_cell_L1jJ60_FIRSTEMPTY', l1SeedThresholds=['FSNOSEED']*2, stream=['Late'], groups=PrimaryPhIGroup+JetMETGroup),

        # Muon-in-jet
        ChainProp(name='HLT_mu4_j20_0eta290_nnJvtv1_boffperf_pf_ftf_dRAB03_L1MU3V_jJ40', l1SeedThresholds=['MU3V','FSNOSEED'], groups=SingleBjetGroup+SupportPhIGroup),

        ChainProp(name='HLT_mu4_j20_0eta290_020jvt_boffperf_pf_ftf_dRAB03_L1MU3V_jJ40', l1SeedThresholds=['MU3V','FSNOSEED'], groups=SingleBjetGroup+SupportPhIGroup),

        # ATR-20505
        ChainProp(name='HLT_g40_loose_L1eEM24L_mu40_msonly_L1MU14FCH', l1SeedThresholds=['eEM24L','MU14FCH'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),  
        # ATR-25512
        ChainProp(name='HLT_g40_loose_L1eEM24L_mu40_msonly_L1MU18VFCH', l1SeedThresholds=['eEM24L','MU18VFCH'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),  

        ## Unconventional tracking ATR-23797
        # hit-based DV
        ChainProp(name='HLT_xe80_tcpufit_hitdvjet200_tight_L1jXE110', groups=PrimaryPhIGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']*2), #ATR-28679
        ChainProp(name='HLT_xe80_tcpufit_hitdvjet200_medium_L1jXE110', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_jXE110'], l1SeedThresholds=['FSNOSEED']*2), #ATR-28679
        ChainProp(name='HLT_xe80_tcpufit_hitdvjet200_tight_L1gXEJWOJ110', groups=PrimaryPhIGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']*2), #ATR-28679
        ChainProp(name='HLT_xe80_tcpufit_hitdvjet200_medium_L1gXEJWOJ110', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_gXEJWOJ110'], l1SeedThresholds=['FSNOSEED']*2), #ATR-28679
        # Phase-I L1Calo
        ChainProp(name='HLT_xe80_tcpufit_hitdvjet200_tight_L1jXE100', groups=PrimaryPhIGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']*2),
        ChainProp(name='HLT_xe80_tcpufit_hitdvjet200_medium_L1jXE100', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_jXE100'], l1SeedThresholds=['FSNOSEED']*2),
        ChainProp(name='HLT_xe80_tcpufit_hitdvjet200_tight_L1gXEJWOJ100', groups=PrimaryPhIGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']*2), #ATR-28838
        ChainProp(name='HLT_xe80_tcpufit_hitdvjet200_medium_L1gXEJWOJ100', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_gXEJWOJ100'], l1SeedThresholds=['FSNOSEED']*2), #ATR-28838

        # jet preselection for hit DV
        ChainProp(name='HLT_j180_hitdvjet200_medium_L1jXE110', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_jXE110'], l1SeedThresholds=2*['FSNOSEED']), #ATR-28679
        ChainProp(name='HLT_j180_hitdvjet200_medium_L1gXEJWOJ110', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_gXEJWOJ110'], l1SeedThresholds=2*['FSNOSEED']), #ATR-28679
        ChainProp(name='HLT_j180_hitdvjet200_medium_L1jXE100', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_jXE100'], l1SeedThresholds=2*['FSNOSEED']), #ATR-28838
        ChainProp(name='HLT_j180_hitdvjet200_medium_L1gXEJWOJ100', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_gXEJWOJ100'], l1SeedThresholds=2*['FSNOSEED']), #ATR-28838

        #ATR-27257, Phase-I
        ChainProp(name='HLT_j180_hitdvjet260_tight_L1jJ160', groups=PrimaryPhIGroup+UnconvTrkGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j180_hitdvjet260_medium_L1jJ160', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_jJ160'], l1SeedThresholds=2*['FSNOSEED']),


        #
        ChainProp(name='HLT_xe80_tcpufit_distrk20_tight_L1jXE100',  groups=PrimaryPhIGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']*2),
        ChainProp(name='HLT_xe80_tcpufit_distrk20_medium_L1jXE100', groups=PrimaryPhIGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']*2),
        #
        ChainProp(name='HLT_xe80_tcpufit_dedxtrk25_medium_L1jXE100', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_jXE100'], l1SeedThresholds=['FSNOSEED']*2),
        ChainProp(name='HLT_xe80_tcpufit_dedxtrk50_medium_L1jXE100', groups=PrimaryPhIGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']*2),
        #ATR-28679
        ChainProp(name='HLT_xe80_tcpufit_distrk20_tight_L1jXE110',  groups=PrimaryPhIGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']*2),
        ChainProp(name='HLT_xe80_tcpufit_distrk20_medium_L1jXE110', groups=PrimaryPhIGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']*2),

        ChainProp(name='HLT_xe80_tcpufit_dedxtrk25_medium_L1jXE110', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_jXE110'], l1SeedThresholds=['FSNOSEED']*2),
        ChainProp(name='HLT_xe80_tcpufit_dedxtrk50_medium_L1jXE110', groups=PrimaryPhIGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']*2),
        #
        ChainProp(name='HLT_xe80_tcpufit_distrk20_tight_L1gXEJWOJ110',  groups=PrimaryPhIGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']*2),
        ChainProp(name='HLT_xe80_tcpufit_distrk20_medium_L1gXEJWOJ110', groups=PrimaryPhIGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']*2),

        ChainProp(name='HLT_xe80_tcpufit_dedxtrk25_medium_L1gXEJWOJ110', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_gXEJWOJ110'], l1SeedThresholds=['FSNOSEED']*2),
        ChainProp(name='HLT_xe80_tcpufit_dedxtrk50_medium_L1gXEJWOJ110', groups=PrimaryPhIGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']*2),
        # ATR-28838
        ChainProp(name='HLT_xe80_tcpufit_distrk20_tight_L1gXEJWOJ100',  groups=PrimaryPhIGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']*2),
        ChainProp(name='HLT_xe80_tcpufit_distrk20_medium_L1gXEJWOJ100', groups=PrimaryPhIGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']*2),
        ChainProp(name='HLT_xe80_tcpufit_dedxtrk25_medium_L1gXEJWOJ100', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_gXEJWOJ100'], l1SeedThresholds=['FSNOSEED']*2),
        ChainProp(name='HLT_xe80_tcpufit_dedxtrk50_medium_L1gXEJWOJ100', groups=PrimaryPhIGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']*2),


        # [ATR-25500] Switch to dl1d for bjet+met+met triggers, and now to GN2
        ChainProp(name="HLT_2j45_0eta290_nnJvtv1_bgn260_pf_ftf_xe50_cell_xe85_tcpufit_L12jJ40_jXE110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup),
        ChainProp(name="HLT_3j35_0eta290_nnJvtv1_bgn260_pf_ftf_xe50_cell_xe70_tcpufit_L13jJ40p0ETA25_jXE80", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup),
        ChainProp(name="HLT_2j45_0eta290_nnJvtv1_bgn260_pf_ftf_xe50_cell_xe85_pfopufit_L12jJ40_jXE110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup),
        ChainProp(name="HLT_3j35_0eta290_nnJvtv1_bgn260_pf_ftf_xe50_cell_xe70_pfopufit_L13jJ40p0ETA25_jXE80", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup),

        ChainProp(name="HLT_2j45_0eta290_020jvt_bgn260_pf_ftf_xe50_cell_xe85_tcpufit_L12jJ40_jXE110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup),
        ChainProp(name="HLT_3j35_0eta290_020jvt_bgn260_pf_ftf_xe50_cell_xe70_tcpufit_L13jJ40p0ETA25_jXE80", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup),
        ChainProp(name="HLT_2j45_0eta290_020jvt_bgn260_pf_ftf_xe50_cell_xe85_pfopufit_L12jJ40_jXE110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup),
        ChainProp(name="HLT_3j35_0eta290_020jvt_bgn260_pf_ftf_xe50_cell_xe70_pfopufit_L13jJ40p0ETA25_jXE80", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup),

        #ATR-28679
        ChainProp(name="HLT_j95_0eta290_nnJvtv1_bgn260_pf_ftf_xe50_cell_xe85_tcpufit_L1jXE110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup), 
        ChainProp(name="HLT_j100_0eta290_nnJvtv1_bgn260_pf_ftf_xe50_cell_xe85_tcpufit_L1jXE110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup),
        ChainProp(name="HLT_j95_0eta290_nnJvtv1_bgn260_pf_ftf_xe50_cell_xe85_pfopufit_L1jXE110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup), 
        ChainProp(name="HLT_j100_0eta290_nnJvtv1_bgn260_pf_ftf_xe50_cell_xe85_pfopufit_L1jXE110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup),

        ChainProp(name="HLT_j95_0eta290_020jvt_bgn260_pf_ftf_xe50_cell_xe85_tcpufit_L1jXE110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup), 
        ChainProp(name="HLT_j100_0eta290_020jvt_bgn260_pf_ftf_xe50_cell_xe85_tcpufit_L1jXE110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup),
        ChainProp(name="HLT_j95_0eta290_020jvt_bgn260_pf_ftf_xe50_cell_xe85_pfopufit_L1jXE110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup), 
        ChainProp(name="HLT_j100_0eta290_020jvt_bgn260_pf_ftf_xe50_cell_xe85_pfopufit_L1jXE110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup),

        ChainProp(name="HLT_j95_0eta290_nnJvtv1_bgn260_pf_ftf_xe50_cell_xe85_tcpufit_L1gXEJWOJ110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup), 
        ChainProp(name="HLT_j100_0eta290_nnJvtv1_bgn260_pf_ftf_xe50_cell_xe85_tcpufit_L1gXEJWOJ110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup),
        ChainProp(name="HLT_j95_0eta290_nnJvtv1_bgn260_pf_ftf_xe50_cell_xe85_pfopufit_L1gXEJWOJ110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup), 
        ChainProp(name="HLT_j100_0eta290_nnJvtv1_bgn260_pf_ftf_xe50_cell_xe85_pfopufit_L1gXEJWOJ110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup),

        ChainProp(name="HLT_j95_0eta290_020jvt_bgn260_pf_ftf_xe50_cell_xe85_tcpufit_L1gXEJWOJ110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup), 
        ChainProp(name="HLT_j100_0eta290_020jvt_bgn260_pf_ftf_xe50_cell_xe85_tcpufit_L1gXEJWOJ110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup),
        ChainProp(name="HLT_j95_0eta290_020jvt_bgn260_pf_ftf_xe50_cell_xe85_pfopufit_L1gXEJWOJ110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup), 
        ChainProp(name="HLT_j100_0eta290_020jvt_bgn260_pf_ftf_xe50_cell_xe85_pfopufit_L1gXEJWOJ110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup),
                
        # photon + multijets (ATR-22594)
        ChainProp(name='HLT_g85_tight_3j50_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaJetGroup),
        ChainProp(name='HLT_g85_tight_3j50_pf_ftf_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaJetGroup),

        #ATR-27373
        ChainProp(name='HLT_g85_tight_3j50_pf_ftf_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaJetGroup),
        ChainProp(name='HLT_g85_tight_3j50_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaJetGroup),

        # photon + MET (ATR-22594, ATR-21565)
        ChainProp(name='HLT_g90_loose_xe90_cell_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMETGroup),

        #Photon+MET ATR-21565,ATR-26410
        ChainProp(name='HLT_g50_tight_xe40_cell_xe50_pfopufit_80mTAC_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMETGroup),
        ChainProp(name='HLT_g50_tight_xe40_cell_xe60_pfopufit_80mTAC_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMETGroup),
        ChainProp(name='HLT_g50_tight_xe40_cell_xe70_pfopufit_80mTAC_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMETGroup),
        ChainProp(name='HLT_g50_loose_xe40_cell_xe70_pfopufit_80mTAC_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaMETGroup),
        ChainProp(name='HLT_g50_loose_xe40_cell_xe70_pfopufit_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaMETGroup),
        #
        ChainProp(name='HLT_g50_tight_xe40_cell_xe50_pfopufit_80mTAC_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMETGroup),
        ChainProp(name='HLT_g50_tight_xe40_cell_xe60_pfopufit_80mTAC_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMETGroup),
        ChainProp(name='HLT_g50_tight_xe40_cell_xe70_pfopufit_80mTAC_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMETGroup),
        ChainProp(name='HLT_g50_loose_xe40_cell_xe70_pfopufit_80mTAC_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaMETGroup),
        ChainProp(name='HLT_g50_loose_xe40_cell_xe70_pfopufit_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaMETGroup),


        #ATR-27373
        ChainProp(name='HLT_g90_loose_xe90_cell_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMETGroup),

        # photon + dijets TLA (ATR-19317)
        #ATR-27251, Phase-I
        ChainProp(name='HLT_g35_loose_3j25_pf_ftf_PhysicsTLA_L1eEM26M', l1SeedThresholds=['eEM26M','FSNOSEED'], stream=['TLA'], groups=SupportPhIGroup+EgammaJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name="HLT_g35_tight_3j25_pf_ftf_PhysicsTLA_L1eEM26M", l1SeedThresholds=['eEM26M','FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+EgammaJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_g35_loose_3j25_pf_ftf_PhysicsTLA_L1eEM28M', l1SeedThresholds=['eEM28M','FSNOSEED'], stream=['TLA'], groups=SupportPhIGroup+EgammaJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name="HLT_g35_tight_3j25_pf_ftf_PhysicsTLA_L1eEM28M", l1SeedThresholds=['eEM28M','FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+EgammaJetGroup, monGroups=['tlaMon:shifter']),

 

        # photon + dijets TLA with explicit Photon isolation calculation
        #ATR-27251, Phase-I
        ChainProp(name="HLT_g35_tight_noiso_3j25_pf_ftf_PhysicsTLA_L1eEM26M", l1SeedThresholds=['eEM26M','FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+EgammaJetGroup, monGroups=['tlaMon:shifter']), 
        ChainProp(name="HLT_g40_tight_noiso_3j25_pf_ftf_PhysicsTLA_L1eEM26M", l1SeedThresholds=['eEM26M','FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+EgammaJetGroup, monGroups=['tlaMon:shifter']), 
        ChainProp(name="HLT_g45_tight_noiso_3j25_pf_ftf_PhysicsTLA_L1eEM26M", l1SeedThresholds=['eEM26M','FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+EgammaJetGroup, monGroups=['tlaMon:shifter']), 
        ChainProp(name="HLT_g60_tight_noiso_3j25_pf_ftf_PhysicsTLA_L1eEM26M", l1SeedThresholds=['eEM26M','FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+EgammaJetGroup, monGroups=['tlaMon:shifter']), 
        ChainProp(name='HLT_g50_tight_noiso_3j25_pf_ftf_PhysicsTLA_L1eEM26M', l1SeedThresholds=['eEM26M','FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+EgammaJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name="HLT_g35_tight_noiso_3j25_pf_ftf_PhysicsTLA_L1eEM28M", l1SeedThresholds=['eEM28M','FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+EgammaJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name="HLT_g40_tight_noiso_3j25_pf_ftf_PhysicsTLA_L1eEM28M", l1SeedThresholds=['eEM28M','FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+EgammaJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name="HLT_g45_tight_noiso_3j25_pf_ftf_PhysicsTLA_L1eEM28M", l1SeedThresholds=['eEM28M','FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+EgammaJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name="HLT_g60_tight_noiso_3j25_pf_ftf_PhysicsTLA_L1eEM28M", l1SeedThresholds=['eEM28M','FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+EgammaJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_g50_tight_noiso_3j25_pf_ftf_PhysicsTLA_L1eEM28M', l1SeedThresholds=['eEM28M','FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+EgammaJetGroup, monGroups=['tlaMon:shifter']),


        # backup for LAr ROS access rate mitigation
        #ATR-27251, Phase-I
        ChainProp(name="HLT_g40_tight_3j25_pf_ftf_PhysicsTLA_L1eEM26M", l1SeedThresholds=['eEM26M','FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+EgammaJetGroup, monGroups=['tlaMon:shifter']), 
        ChainProp(name="HLT_g45_tight_3j25_pf_ftf_PhysicsTLA_L1eEM26M", l1SeedThresholds=['eEM26M','FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+EgammaJetGroup, monGroups=['tlaMon:shifter']), 
        ChainProp(name="HLT_g50_tight_3j25_pf_ftf_PhysicsTLA_L1eEM26M", l1SeedThresholds=['eEM26M','FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+EgammaJetGroup, monGroups=['tlaMon:shifter']), 
        ChainProp(name="HLT_g60_tight_3j25_pf_ftf_PhysicsTLA_L1eEM26M", l1SeedThresholds=['eEM26M','FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+EgammaJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name="HLT_g40_tight_3j25_pf_ftf_PhysicsTLA_L1eEM28M", l1SeedThresholds=['eEM28M','FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+EgammaJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name="HLT_g45_tight_3j25_pf_ftf_PhysicsTLA_L1eEM28M", l1SeedThresholds=['eEM28M','FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+EgammaJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name="HLT_g50_tight_3j25_pf_ftf_PhysicsTLA_L1eEM28M", l1SeedThresholds=['eEM28M','FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+EgammaJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name="HLT_g60_tight_3j25_pf_ftf_PhysicsTLA_L1eEM28M", l1SeedThresholds=['eEM28M','FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+EgammaJetGroup, monGroups=['tlaMon:shifter']),
 
        
        # photon + dijet TLA EMTopo backup
        # Backup Support Full Build EMTopo photon + dijet TLA
        #ATR-27251, Phase-I
        ChainProp(name="HLT_g35_tight_3j25_L1eEM26M", l1SeedThresholds=['eEM26M','FSNOSEED'], stream=[PhysicsStream], groups=SupportPhIGroup+EgammaJetGroup),
        ChainProp(name="HLT_g35_tight_3j25_PhysicsTLA_L1eEM26M", l1SeedThresholds=['eEM26M','FSNOSEED'], stream=['TLA'], groups=SupportPhIGroup+EgammaJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name="HLT_g35_tight_3j25_pf_ftf_L1eEM26M", l1SeedThresholds=['eEM26M','FSNOSEED'], stream=[PhysicsStream], groups=SupportPhIGroup+EgammaJetGroup),
        ChainProp(name="HLT_g35_tight_3j25_L1eEM28M", l1SeedThresholds=['eEM28M','FSNOSEED'], stream=[PhysicsStream], groups=SupportPhIGroup+EgammaJetGroup),
        ChainProp(name="HLT_g35_tight_3j25_PhysicsTLA_L1eEM28M", l1SeedThresholds=['eEM28M','FSNOSEED'], stream=['TLA'], groups=SupportPhIGroup+EgammaJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name="HLT_g35_tight_3j25_pf_ftf_L1eEM28M", l1SeedThresholds=['eEM28M','FSNOSEED'], stream=[PhysicsStream], groups=SupportPhIGroup+EgammaJetGroup),


        # meson + photon (ATR-23239)
        # phase-I, primary 
        ChainProp(name='HLT_g25_medium_tau25_dikaonmass_tracktwoMVA_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','eTAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g25_medium_tau25_kaonpi1_tracktwoMVA_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','eTAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g25_medium_tau25_kaonpi2_tracktwoMVA_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','eTAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g25_medium_tau25_singlepion_tracktwoMVA_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','eTAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g25_medium_tau25_dipion1_tracktwoMVA_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','eTAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g25_medium_tau25_dipion2_tracktwoMVA_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','eTAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_medium_tau25_dipion3_tracktwoMVA_L1eTAU12_60invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','eTAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),

        #ATR-27373
        ChainProp(name='HLT_g25_medium_tau25_dikaonmass_tracktwoMVA_50invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','eTAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g25_medium_tau25_kaonpi1_tracktwoMVA_50invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','eTAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g25_medium_tau25_kaonpi2_tracktwoMVA_50invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','eTAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g25_medium_tau25_singlepion_tracktwoMVA_50invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','eTAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g25_medium_tau25_dipion1_tracktwoMVA_50invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','eTAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g25_medium_tau25_dipion2_tracktwoMVA_50invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','eTAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_medium_tau25_dipion3_tracktwoMVA_L1eTAU12_60invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','eTAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ## Higher threshold to mitigate high ROS access 
        ChainProp(name='HLT_g35_medium_tau25_dikaonmass_tracktwoMVA_L1eTAU20_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','eTAU20'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_medium_tau25_kaonpi1_tracktwoMVA_L1eTAU20_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','eTAU20'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_medium_tau25_kaonpi2_tracktwoMVA_L1eTAU20_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','eTAU20'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_medium_tau25_singlepion_tracktwoMVA_L1eTAU20_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','eTAU20'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_medium_tau25_dipion1_tracktwoMVA_L1eTAU20_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','eTAU20'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_medium_tau25_dipion2_tracktwoMVA_L1eTAU20_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','eTAU20'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_medium_tau25_dipion3_tracktwoMVA_L1eTAU20_60invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','eTAU20'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_medium_tau25_dikaonmass_tracktwoMVA_L1eTAU20_50invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','eTAU20'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_medium_tau25_kaonpi1_tracktwoMVA_L1eTAU20_50invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','eTAU20'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_medium_tau25_kaonpi2_tracktwoMVA_L1eTAU20_50invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','eTAU20'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_medium_tau25_singlepion_tracktwoMVA_L1eTAU20_50invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','eTAU20'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_medium_tau25_dipion1_tracktwoMVA_L1eTAU20_50invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','eTAU20'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_medium_tau25_dipion2_tracktwoMVA_L1eTAU20_50invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','eTAU20'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_medium_tau25_dipion3_tracktwoMVA_L1eTAU20_60invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','eTAU20'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),


        ## 
        ChainProp(name='HLT_g35_tight_tau25_dikaonmass_tracktwoMVA_L1eTAU20_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','eTAU20'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_tight_tau25_kaonpi1_tracktwoMVA_L1eTAU20_50invmAB_L1eEM26M'   , l1SeedThresholds=['eEM26M','eTAU20'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_tight_tau25_kaonpi2_tracktwoMVA_L1eTAU20_50invmAB_L1eEM26M'   , l1SeedThresholds=['eEM26M','eTAU20'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_tight_tau25_singlepion_tracktwoMVA_L1eTAU20_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','eTAU20'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_tight_tau25_dipion1_tracktwoMVA_L1eTAU20_50invmAB_L1eEM26M'   , l1SeedThresholds=['eEM26M','eTAU20'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_tight_tau25_dipion2_tracktwoMVA_L1eTAU20_50invmAB_L1eEM26M'   , l1SeedThresholds=['eEM26M','eTAU20'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_tight_tau25_dipion3_tracktwoMVA_L1eTAU20_60invmAB_L1eEM26M'   , l1SeedThresholds=['eEM26M','eTAU20'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_tight_tau25_dikaonmass_tracktwoMVA_L1eTAU20_50invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','eTAU20'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_tight_tau25_kaonpi1_tracktwoMVA_L1eTAU20_50invmAB_L1eEM28M'   , l1SeedThresholds=['eEM28M','eTAU20'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_tight_tau25_kaonpi2_tracktwoMVA_L1eTAU20_50invmAB_L1eEM28M'   , l1SeedThresholds=['eEM28M','eTAU20'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_tight_tau25_singlepion_tracktwoMVA_L1eTAU20_50invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','eTAU20'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_tight_tau25_dipion1_tracktwoMVA_L1eTAU20_50invmAB_L1eEM28M'   , l1SeedThresholds=['eEM28M','eTAU20'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_tight_tau25_dipion2_tracktwoMVA_L1eTAU20_50invmAB_L1eEM28M'   , l1SeedThresholds=['eEM28M','eTAU20'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_tight_tau25_dipion3_tracktwoMVA_L1eTAU20_60invmAB_L1eEM28M'   , l1SeedThresholds=['eEM28M','eTAU20'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),


    
        #ATR-27257
        ChainProp(name='HLT_j180_2dispjet50_2p_L1jJ160', groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ160'], l1SeedThresholds=['FSNOSEED']*2, monGroups=['idMon:t0']),

        # [ATR-26377] To understand the muvtx rates as compared to Run 2
        ChainProp(name='HLT_mu3vtx_j30_L12MU8F', l1SeedThresholds=['MU8F', 'FSNOSEED'], groups=PrimaryL1MuGroup+MultiMuonGroup),

        # muon+MET re-run chains: ATR-27220 / ATR-26456
        ChainProp(name='HLT_mu24_ivarmedium_xe0_cell_L1MU14FCH',  l1SeedThresholds=['MU14FCH', 'FSNOSEED'], stream=['Main'], groups=TagAndProbeGroup+METGroup+['RATE:CPS_MU14FCH']),
        ChainProp(name='HLT_mu24_ivarmedium_xe0_tcpufit_L1MU14FCH',  l1SeedThresholds=['MU14FCH', 'FSNOSEED'], stream=['Main'], groups=TagAndProbeGroup+METGroup+['RATE:CPS_MU14FCH']),
        ChainProp(name='HLT_mu24_ivarmedium_xe0_trkmht_L1MU14FCH',  l1SeedThresholds=['MU14FCH', 'FSNOSEED'], stream=['Main'], groups=TagAndProbeGroup+METGroup+['RATE:CPS_MU14FCH']),
        ChainProp(name='HLT_mu24_ivarmedium_xe0_pfopufit_L1MU14FCH',  l1SeedThresholds=['MU14FCH', 'FSNOSEED'], stream=['Main'], groups=TagAndProbeGroup+METGroup+['RATE:CPS_MU14FCH']),
        ChainProp(name='HLT_mu24_ivarmedium_xe0_pfsum_L1MU14FCH',  l1SeedThresholds=['MU14FCH', 'FSNOSEED'], stream=['Main'], groups=TagAndProbeGroup+METGroup+['RATE:CPS_MU14FCH']),
        ChainProp(name='HLT_mu24_ivarmedium_xe0_pfsum_cssk_L1MU14FCH',  l1SeedThresholds=['MU14FCH', 'FSNOSEED'], stream=['Main'], groups=TagAndProbeGroup+METGroup+['RATE:CPS_MU14FCH']),
        ChainProp(name='HLT_mu24_ivarmedium_xe0_pfsum_vssk_L1MU14FCH',  l1SeedThresholds=['MU14FCH', 'FSNOSEED'], stream=['Main'], groups=TagAndProbeGroup+METGroup+['RATE:CPS_MU14FCH']),
        ChainProp(name='HLT_mu24_ivarmedium_xe0_mhtpufit_em_L1MU14FCH',  l1SeedThresholds=['MU14FCH', 'FSNOSEED'], stream=['Main'], groups=TagAndProbeGroup+METGroup+['RATE:CPS_MU14FCH']),
        ChainProp(name='HLT_mu24_ivarmedium_xe0_mhtpufit_pf_L1MU14FCH',  l1SeedThresholds=['MU14FCH', 'FSNOSEED'], stream=['Main'], groups=TagAndProbeGroup+METGroup+['RATE:CPS_MU14FCH']),
        ChainProp(name='HLT_mu24_ivarmedium_xe0_nn_L1MU14FCH',  l1SeedThresholds=['MU14FCH', 'FSNOSEED'], stream=['Main'], groups=TagAndProbeGroup+METGroup+['RATE:CPS_MU14FCH']),

        ChainProp(name='HLT_mu24_ivarmedium_xe0_cell_L1MU18VFCH',  l1SeedThresholds=['MU18VFCH', 'FSNOSEED'], stream=['Main'], groups=TagAndProbeGroup+METGroup+['RATE:CPS_MU18VFCH']),
        ChainProp(name='HLT_mu24_ivarmedium_xe0_tcpufit_L1MU18VFCH',  l1SeedThresholds=['MU18VFCH', 'FSNOSEED'], stream=['Main'], groups=TagAndProbeGroup+METGroup+['RATE:CPS_MU18VFCH']),
        ChainProp(name='HLT_mu24_ivarmedium_xe0_trkmht_L1MU18VFCH',  l1SeedThresholds=['MU18VFCH', 'FSNOSEED'], stream=['Main'], groups=TagAndProbeGroup+METGroup+['RATE:CPS_MU18VFCH']),
        ChainProp(name='HLT_mu24_ivarmedium_xe0_pfopufit_L1MU18VFCH',  l1SeedThresholds=['MU18VFCH', 'FSNOSEED'], stream=['Main'], groups=TagAndProbeGroup+METGroup+['RATE:CPS_MU18VFCH']),
        ChainProp(name='HLT_mu24_ivarmedium_xe0_pfsum_L1MU18VFCH',  l1SeedThresholds=['MU18VFCH', 'FSNOSEED'], stream=['Main'], groups=TagAndProbeGroup+METGroup+['RATE:CPS_MU18VFCH']),
        ChainProp(name='HLT_mu24_ivarmedium_xe0_pfsum_cssk_L1MU18VFCH',  l1SeedThresholds=['MU18VFCH', 'FSNOSEED'], stream=['Main'], groups=TagAndProbeGroup+METGroup+['RATE:CPS_MU18VFCH']),
        ChainProp(name='HLT_mu24_ivarmedium_xe0_pfsum_vssk_L1MU18VFCH',  l1SeedThresholds=['MU18VFCH', 'FSNOSEED'], stream=['Main'], groups=TagAndProbeGroup+METGroup+['RATE:CPS_MU18VFCH']),
        ChainProp(name='HLT_mu24_ivarmedium_xe0_mhtpufit_em_L1MU18VFCH',  l1SeedThresholds=['MU18VFCH', 'FSNOSEED'], stream=['Main'], groups=TagAndProbeGroup+METGroup+['RATE:CPS_MU18VFCH']),
        ChainProp(name='HLT_mu24_ivarmedium_xe0_mhtpufit_pf_L1MU18VFCH',  l1SeedThresholds=['MU18VFCH', 'FSNOSEED'], stream=['Main'], groups=TagAndProbeGroup+METGroup+['RATE:CPS_MU18VFCH']),
        ChainProp(name='HLT_mu24_ivarmedium_xe0_nn_L1MU18VFCH',  l1SeedThresholds=['MU18VFCH', 'FSNOSEED'], stream=['Main'], groups=TagAndProbeGroup+METGroup+['RATE:CPS_MU18VFCH']),


        # ATR-28712 Lowering MET preselection in the isolated high-pT trigger chains 
        ChainProp(name="HLT_xe50_tcpufit_isotrk120_medium_iaggrmedium_L1jXE100", l1SeedThresholds=["FSNOSEED"]*2, stream=[PhysicsStream], groups=UnconvTrkGroup+PrimaryPhIGroup),
        ChainProp(name="HLT_xe50_tcpufit_isotrk120_medium_iaggrmedium_L1gXEJWOJ100", l1SeedThresholds=["FSNOSEED"]*2, stream=[PhysicsStream], groups=UnconvTrkGroup+PrimaryPhIGroup),
        ChainProp(name="HLT_xe60_tcpufit_isotrk120_medium_iaggrmedium_L1jXE100", l1SeedThresholds=["FSNOSEED"]*2, stream=[PhysicsStream], groups=UnconvTrkGroup+PrimaryPhIGroup),
        ChainProp(name="HLT_xe60_tcpufit_isotrk120_medium_iaggrmedium_L1gXEJWOJ100", l1SeedThresholds=["FSNOSEED"]*2, stream=[PhysicsStream], groups=UnconvTrkGroup+PrimaryPhIGroup),
        ChainProp(name="HLT_xe50_nn_isotrk120_medium_iaggrmedium_L1jXE100", l1SeedThresholds=["FSNOSEED"]*2, stream=[PhysicsStream], groups=UnconvTrkGroup+PrimaryPhIGroup),
        ChainProp(name="HLT_xe50_nn_isotrk120_medium_iaggrmedium_L1gXEJWOJ100", l1SeedThresholds=["FSNOSEED"]*2, stream=[PhysicsStream], groups=UnconvTrkGroup+PrimaryPhIGroup),
        ChainProp(name="HLT_xe60_nn_isotrk120_medium_iaggrmedium_L1jXE100", l1SeedThresholds=["FSNOSEED"]*2, stream=[PhysicsStream], groups=UnconvTrkGroup+PrimaryPhIGroup),
        ChainProp(name="HLT_xe60_nn_isotrk120_medium_iaggrmedium_L1gXEJWOJ100", l1SeedThresholds=["FSNOSEED"]*2, stream=[PhysicsStream], groups=UnconvTrkGroup+PrimaryPhIGroup),

        # Anomaly detection (ATR-30826)
        ChainProp(name='HLT_0e25_nopid_0g25_loose_0mu24_j20_xe0_tcpufit_anomdetM_L1ADVAET', l1SeedThresholds=['eEM18L','eEM18L','MU3V','FSNOSEED','FSNOSEED'], groups=SupportPhIGroup+Topo2Group+["adWrite"]+["adMon:online"]),
        # Anomaly detection support (ATR-31137)
        ChainProp(name='HLT_0e25_nopid_0g25_loose_0mu24_j20_xe0_tcpufit_anomdetL_L1ADVAEL', l1SeedThresholds=['eEM18L','eEM18L','MU3V','FSNOSEED','FSNOSEED'], groups=SupportPhIGroup+Topo2Group+["adMon:online"]),
        ChainProp(name='HLT_0e25_nopid_0g25_loose_0mu24_j20_xe0_tcpufit_anomdetL_L1All', l1SeedThresholds=['eEM18L','eEM18L','MU3V','FSNOSEED','FSNOSEED'], groups=SupportPhIGroup+Topo2Group),

    ]



    chains['Monitor'] = [
        ChainProp(name='HLT_noalg_CostMonDS_L1All',        l1SeedThresholds=['FSNOSEED'], stream=['CostMonitoring'], groups=['PS:NoBulkMCProd', 'Primary:CostAndRate', 'RATE:Monitoring', 'BW:Other']), # HLT_costmonitor
    ]


    chains['UnconventionalTracking'] = [

        # hit-based DV                                 
        ChainProp(name='HLT_hitdvjet200_medium_L1gXEJWOJ100', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_gXEJWOJ100'], l1SeedThresholds=['FSNOSEED']), #ATR-28838

        # Phase I L1Calo inputs
        # hit-based DV                                        
        ChainProp(name='HLT_hitdvjet200_medium_L1jXE100', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_jXE100'], l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_hitdvjet200_medium_L1jXE110', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_jXE110'], l1SeedThresholds=['FSNOSEED']), #ATR-28679
        ChainProp(name='HLT_hitdvjet200_medium_L1gXEJWOJ110', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_gXEJWOJ110'], l1SeedThresholds=['FSNOSEED']), #ATR-28679

        # ATR-28838 MET duplication
        ChainProp(name='HLT_distrk20_tight_L1gXEJWOJ100', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_gXEJWOJ100'], l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_distrk20_medium_L1gXEJWOJ100', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_gXEJWOJ100'], l1SeedThresholds=['FSNOSEED']),

        # Phase-I L1Calo
        ChainProp(name='HLT_distrk20_tight_L1jXE100', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_jXE100'], l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_distrk20_medium_L1jXE100', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_jXE100'], l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_distrk20_tight_L1jXE110', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_jXE110'], l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_distrk20_medium_L1jXE110', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_jXE110'], l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_distrk20_tight_L1gXEJWOJ110', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_gXEJWOJ110'], l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_distrk20_medium_L1gXEJWOJ110', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_gXEJWOJ110'], l1SeedThresholds=['FSNOSEED']),

    ]

    chains['Streaming'] = [
        #AD testing trigger
        ChainProp(name='HLT_noalg_L1ADVAEL', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+Topo2Group+SupportGroup), 
        # Streamers already active in MC for jet/MET monitoring


        # Phase I jet inputs ATR-24411
        ChainProp(name='HLT_noalg_L1jJ500', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+PrimaryPhIGroup+JetStreamersGroup+['BW:Other'], monGroups=['jetMon:t0']), # catch all high-Et

        # Phase I primary candidates
        ChainProp(name='HLT_noalg_L1jJ160',         l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_noalg_L1gJ400p0ETA25',         l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_noalg_L1gLJ160p0ETA25',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_noalg_L1jXE100',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+METPhaseIStreamersGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_noalg_L1gXEJWOJ100',    l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+METPhaseIStreamersGroup, monGroups=['metMon:t0']),
        #ChainProp(name='HLT_noalg_L1gXERHO100',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+METPhaseIStreamersGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_noalg_L1gXENC100',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+METPhaseIStreamersGroup, monGroups=['metMon:t0']),

        ChainProp(name='HLT_noalg_L1RD0_EMPTY',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportGroup),

        #zero bias
        ChainProp(name='HLT_noalg_L1RD1_FILLED',        l1SeedThresholds=['FSNOSEED'], stream=['ZeroBias'],groups=['PS:NoBulkMCProd']+ZeroBiasGroup+SupportGroup),# ATR-25032

        # muon streamers
        ChainProp(name='HLT_noalg_L1MU3V',      l1SeedThresholds=['FSNOSEED'], stream=['Main','express'], groups=['PS:NoBulkMCProd']+SingleMuonGroup+SupportGroup,monGroups=['detMon']),
        ChainProp(name='HLT_noalg_L1MU3VF',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU5VF',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU8F',      l1SeedThresholds=['FSNOSEED'], stream=['Main','express'], groups=['PS:NoBulkMCProd']+SingleMuonGroup+SupportGroup,monGroups=['detMon']),
        ChainProp(name='HLT_noalg_L1MU8VF',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU8VFC',    l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU4BOM',    l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU10BO',    l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU10BOM',   l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU12BOM',   l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU14FCH',   l1SeedThresholds=['FSNOSEED'], stream=['Main','express'], groups=['PS:NoBulkMCProd']+SingleMuonGroup+SupportGroup,monGroups=['detMon']),
        ChainProp(name='HLT_noalg_L1MU18VFCH',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SingleMuonGroup+SupportGroup),


        #ChainProp(name='HLT_noalg_L1XE60',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+METStreamersGroup+SupportLegGroup), #ATR-28679

        #Phase-I
        ChainProp(name='HLT_noalg_L1eTAU12',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+TauPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eTAU20',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+TauPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jTAU20',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+TauPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1cTAU20M',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+TauPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eTAU20L',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+TauPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eTAU30',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+TauPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1cTAU30M',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+TauPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1cTAU35M',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+TauPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eTAU60',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+TauPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eTAU80',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+TauPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eTAU140',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+TauPhaseIStreamersGroup),

        ChainProp(name='HLT_noalg_L1eEM5',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+EgammaPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eEM9',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+EgammaPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eEM12L',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+EgammaPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eEM18L',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+EgammaPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eEM18M',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+EgammaPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eEM24L',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+EgammaPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eEM26',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+EgammaPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eEM26L',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+EgammaPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eEM26M',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+EgammaPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eEM26T',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+EgammaPhaseIStreamersGroup),

        ChainProp(name='HLT_noalg_L1jJ30',          l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ40',          l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup+['RATE:CPS_jJ40']),
        ChainProp(name='HLT_noalg_L1jJ40p30ETA49',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ50',          l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ50p30ETA49',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ60',          l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ60p30ETA49',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ90',          l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ90p30ETA49',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ125',         l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ125p30ETA49', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),

        ChainProp(name='HLT_noalg_L1gJ20p0ETA25',          l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1gJ20p25ETA49',          l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1gJ20p0ETA25_EMPTY',          l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1gJ50p0ETA25',          l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1gJ100p0ETA25',         l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup, monGroups=['jetMon:t0', 'jetMon:online']),

        ChainProp(name='HLT_noalg_L1gLJ80p0ETA25',         l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1gLJ100p0ETA25',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1gLJ140p0ETA25',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),

        ChainProp(name='HLT_noalg_L1jXE60',         l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+METPhaseIStreamersGroup), #ATR-28679
        ChainProp(name='HLT_noalg_L1jXE70',         l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+METPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jXE80',         l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+METPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jXE90',         l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+METPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jXE110',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+METPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jXE120',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+METPhaseIStreamersGroup), #ATR-28679
        ChainProp(name='HLT_noalg_L1jXE500',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+METPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1gXEJWOJ60',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+METPhaseIStreamersGroup), #ATR-28679
        ChainProp(name='HLT_noalg_L1gXEJWOJ70',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+METPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1gXEJWOJ80',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+METPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1gXEJWOJ110',    l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+METPhaseIStreamersGroup), #ATR-28679
        ChainProp(name='HLT_noalg_L1gXEJWOJ120',    l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+METPhaseIStreamersGroup), #ATR-28679
        ChainProp(name='HLT_noalg_L1gXEJWOJ500',    l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+METPhaseIStreamersGroup), #ATR-28679
        #ChainProp(name='HLT_noalg_L1gXERHO70',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+METPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1gXENC70',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+METPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1gMHT500',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+METPhaseIStreamersGroup),

        ChainProp(name='HLT_noalg_L1jXEC100',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+METPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1gTE200',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+METPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jTE200',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+METPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jTEC200',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+METPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jTEFWD100',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+METPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jTEFWDA100',    l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+METPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jTEFWDC100',    l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+METPhaseIStreamersGroup),


        # Exotics support streamers
        ChainProp(name='HLT_noalg_L110DR-MU14FCH-MU5VF_EMPTY',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportGroup+MuonXStreamersGroup+Topo2Group),
        ChainProp(name='HLT_noalg_L110DR-MU14FCH-MU5VF_UNPAIRED_ISO',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportGroup+MuonXStreamersGroup+Topo2Group),

        # TODO add once L1 items/thresholds are in place
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_TOF_T0T1_jJ90', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_TOF_T0T1_jJ125', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_TOF_jJ90', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_TOF_jJ125', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),

        #Muon streamers for L1Topo validation
        ChainProp(name='HLT_noalg_L1BPH-2M9-0DR15-2MU3VF',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportGroup+MuonXStreamersGroup+Topo3Group),
        ChainProp(name='HLT_noalg_L1BPH-8M15-0DR22-2MU5VF',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportGroup+MuonXStreamersGroup+Topo2Group),
        ChainProp(name='HLT_noalg_L12MU5VF',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportGroup+MuonXStreamersGroup),
        
        # ATR-28563 
        ChainProp(name='HLT_noalg_L1LLPDPHI-jXE40-jJ40', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=JetPhaseIStreamersGroup+SupportPhIGroup+Topo2Group),
        ChainProp(name='HLT_noalg_L1LLPNODPHI-jXE40-jJ40', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=JetPhaseIStreamersGroup+SupportPhIGroup+Topo2Group),

    ]

    # check all chains are classified as either primary, EOF (end-of-fill), support or T&P chains
    for sig, chainsInSig in chains.items():
        for chain in chainsInSig:
            groupFound = False
            for group in chain.groups:
                if 'Primary' in group or 'Support' in group or 'EOF' in group:
                   groupFound = True
            if not groupFound:
                log.error("chain %s in Physics menu [%s] needs to be classified as either primary or support chain", chain.name, sig)
                raise RuntimeError("Add either the primary or support group to the chain %s",chain.name)

    return chains
