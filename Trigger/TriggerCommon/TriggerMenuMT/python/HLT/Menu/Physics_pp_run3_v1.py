# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# Physics_pp_run3_v1.py menu -- contains physics chains for MC and data
#------------------------------------------------------------------------#

# This defines the input format of the chain and it's properties with the defaults set
# always required are: name, stream and groups
#['name', 'chainParts'=[], 'stream', 'groups', 'merging'=[], 'topoStartFrom'=False],
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
# For unconventional tracking chains (ATR-23797)
UnconvTrkGroup = ['RATE:UnconvTrk', 'BW:UnconvTrk'] 
LowMuGroup = ['LowMu']
# Topo boards
Topo2Group = ['Topo2']
Topo3Group = ['Topo3']
LegacyTopoGroup = ['LegacyTopo']

def setupMenu(menu_name):

    from AthenaCommon.Logging import logging
    log = logging.getLogger( __name__ )
    log.info('[setupMenu] going to add the Physics menu chains now')

    chains = ChainStore()
    chains['Muon'] += [
        #ATR-19985 and ATR-24367
        ChainProp(name='HLT_mu6_mu6noL1_L1MU5VF', l1SeedThresholds=['MU5VF','FSNOSEED'], groups=SupportGroup+MultiMuonGroup+['RATE:CPS_MU5VF'], stream=[PhysicsStream,'express'], monGroups=['muonMon:shifter']),

        #ATR-20049
        ChainProp(name='HLT_2mu6_L12MU5VF',     l1SeedThresholds=['MU5VF'],   groups=SupportGroup+MultiMuonGroup+['RATE:CPS_2MU5VF']),

        #Planned Primaries
        #-- 1 mu iso
        ChainProp(name='HLT_mu24_ivarmedium_L1MU14FCH', groups=PrimaryL1MuGroup+SingleMuonGroup, stream=[PhysicsStream,'express'], monGroups=['muonMon:shifter','muonMon:online']),
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
        ChainProp(name='HLT_mu22_mu8noL1_L1MU14FCH',  l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryL1MuGroup+MultiMuonGroup, stream=[PhysicsStream,'express'], monGroups=['muonMon:online','muonMon:shifter']),
        ChainProp(name='HLT_mu22_mu10noL1_L1MU14FCH', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryL1MuGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu24_mu8noL1_L1MU14FCH',  l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryL1MuGroup+MultiMuonGroup),
        # ATR-25512
        ChainProp(name='HLT_mu23_mu8noL1_L1MU18VFCH',  l1SeedThresholds=['MU18VFCH','FSNOSEED'], groups=PrimaryL1MuGroup+MultiMuonGroup, stream=[PhysicsStream,'express'], monGroups=['muonMon:online','muonMon:shifter']),
        ChainProp(name='HLT_mu23_mu10noL1_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','FSNOSEED'], groups=PrimaryL1MuGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu24_mu8noL1_L1MU18VFCH',  l1SeedThresholds=['MU18VFCH','FSNOSEED'], groups=PrimaryL1MuGroup+MultiMuonGroup),

        ChainProp(name='HLT_2mu14_L12MU8F', groups=PrimaryL1MuGroup+MultiMuonGroup, stream=[PhysicsStream,'express'], monGroups=['muonMon:online','muonMon:shifter']),
        ChainProp(name='HLT_2mu15_L12MU8F', groups=PrimaryL1MuGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_ivarmedium_mu8noL1_L1MU14FCH', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryL1MuGroup+MultiMuonGroup),
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

        # Late muons - disabled until ATR-25031 is fixed
        # ChainProp(name='HLT_mu10_lateMu_L1LATE-MU8F_jJ90', l1SeedThresholds=['FSNOSEED'], groups=SingleMuonGroup+PrimaryPhIGroup),
        # ChainProp(name='HLT_mu10_lateMu_L1LATE-MU8F_jXE70', l1SeedThresholds=['FSNOSEED'], groups=SingleMuonGroup+PrimaryPhIGroup),

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
        ChainProp(name='HLT_2mu4_l2io_invmDimu_L1BPH-2M9-0DR15-2MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=MultiMuonGroup+SupportGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3VF']+Topo2Group),
        ChainProp(name='HLT_mu6_l2io_mu4_l2io_invmDimu_L1BPH-2M9-0DR15-MU5VFMU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=MultiMuonGroup+SupportGroup+['RATE:CPS_BPH-2M9-0DR15-MU5VFMU3V']+Topo2Group),
        ChainProp(name='HLT_mu6_l2io_mu4_l2io_invmDimu_L1BPH-2M9-0DR15-C-MU5VFMU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=MultiMuonGroup+SupportGroup+['RATE:CPS_BPH-2M9-0DR15-C-MU5VFMU3V']+Topo3Group),
        ChainProp(name='HLT_2mu6_l2io_invmDimu_L1BPH-2M9-2DR15-2MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=MultiMuonGroup+SupportGroup+['RATE:CPS_BPH-2M9-2DR15-2MU5VF']+Topo2Group),
        ChainProp(name='HLT_2mu6_l2io_invmDimu_L1LFV-MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=MultiMuonGroup+SupportGroup+['RATE:CPS_LFV-MU5VF']+Topo2Group),
        ChainProp(name='HLT_mu11_l2io_mu6_l2io_invmDimu_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed','express'], groups=MultiMuonGroup+SupportGroup, monGroups=['bphysMon:shifter']),
        ChainProp(name='HLT_mu11_l2io_mu6_l2io_invmDimu_L1LFV-MU8VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=MultiMuonGroup+SupportGroup+Topo2Group),
        # ATR-22782 4muon
        ChainProp(name='HLT_mu4_ivarloose_mu4_mu3noL1_L1BPH-7M14-0DR25-MU5VFMU3VF', l1SeedThresholds=['MU3VF', 'MU3VF', 'FSNOSEED'], stream=['BphysDelayed'], groups=MultiMuonGroup+SupportGroup+Topo3Group),
        ChainProp(name='HLT_2mu4_L1BPH-7M11-25DR99-2MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=MultiMuonGroup+SupportGroup+Topo2Group),
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
        # ATR-27325 Primary -> Support
        ChainProp(name='HLT_e26_dnntight_ivarloose_L1eEM26M', groups=SupportPhIGroup+SingleElectronGroup, monGroups=['egammaMon:online','egammaMon:t0_tp']),
        ChainProp(name='HLT_e60_dnnmedium_L1eEM26M', groups=SupportPhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),
        ChainProp(name='HLT_e140_dnnloose_L1eEM26M', groups=SupportPhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),

        # ATR-27373
        ChainProp(name='HLT_e28_dnntight_ivarloose_L1eEM28M', groups=SupportPhIGroup+SingleElectronGroup, monGroups=['egammaMon:online','egammaMon:t0_tp']),
        ChainProp(name='HLT_e60_dnnmedium_L1eEM28M', groups=SupportPhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),
        ChainProp(name='HLT_e140_dnnloose_L1eEM28M', groups=SupportPhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),
                   
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
        ChainProp(name='HLT_2e17_lhvloose_g22_tight_probe_L12eEM18M', l1SeedThresholds=['eEM18M','PROBEeEM18M'], groups=TagAndProbePhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_2e17_lhvloose_g25_medium_probe_L12eEM18M', l1SeedThresholds=['eEM18M','PROBEeEM18M'],groups=TagAndProbePhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_2e17_lhvloose_g50_loose_probe_L12eEM18M', l1SeedThresholds=['eEM18M','PROBEeEM18M'],groups=TagAndProbePhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_2e17_lhvloose_g35_medium_probe_L12eEM18M', l1SeedThresholds=['eEM18M','PROBEeEM18M'],groups=TagAndProbePhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_2e24_lhvloose_g22_tight_probe_L12eEM24L', l1SeedThresholds=['eEM24L','PROBEeEM24L'],groups=TagAndProbePhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_2e24_lhvloose_g25_medium_probe_L12eEM24L', l1SeedThresholds=['eEM24L','PROBEeEM24L'],groups=TagAndProbePhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_2e24_lhvloose_g35_medium_probe_L12eEM24L', l1SeedThresholds=['eEM24L','PROBEeEM24L'],groups=TagAndProbePhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_2e24_lhvloose_g50_loose_probe_L12eEM24L', l1SeedThresholds=['eEM24L','PROBEeEM24L'],groups=TagAndProbePhIGroup+MultiElectronGroup),
        
        #ATR-27251, Phase-I
        ChainProp(name='HLT_2e17_lhvloose_g20_tight_probe_L12eEM18M', l1SeedThresholds=['eEM18M','PROBEeEM18M'], groups=TagAndProbePhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_2e24_lhvloose_g20_tight_probe_L12eEM24L', l1SeedThresholds=['eEM24L','PROBEeEM24L'], groups=TagAndProbePhIGroup+MultiElectronGroup),
        
        #---------- primary 3e
        ChainProp(name='HLT_e24_lhvloose_2e12_lhvloose_L1eEM24L_3eEM12L',l1SeedThresholds=['eEM24L','eEM12L'], groups=PrimaryPhIGroup+MultiElectronGroup),
        #--------- primary special
        ChainProp(name='HLT_e20_lhtight_ivarloose_L1ZAFB-25DPHI-eEM18M', l1SeedThresholds=['eEM18M'], groups=PrimaryPhIGroup+SingleElectronGroup+Topo3Group),
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
        
        #ATR-26738
        ChainProp(name='HLT_e10_etcut_L1eEM9', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM9']),   # Phase-1 ATR-27156
        ChainProp(name='HLT_e15_etcut_L1eEM9', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM9']),   # Phase-1 ATR-27156
        ChainProp(name='HLT_e20_etcut_L1eEM18M', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM18M']),   # Phase-1 ATR-27156
        ChainProp(name='HLT_e25_etcut_L1eEM18M', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM18M']),   # Phase-1 ATR-27156
        ChainProp(name='HLT_e30_etcut_L1eEM18M', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM18M']),   # Phase-1 ATR-27156
        ChainProp(name='HLT_e40_etcut_L1eEM18M', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM18M']),   # Phase-1 ATR-27156
        ChainProp(name='HLT_e60_etcut_L1eEM26M', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM26M']),   # Phase-1 ATR-27156
        ChainProp(name='HLT_e70_etcut_L1eEM26M', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM26M']),   # Phase-1 ATR-27156
        ChainProp(name='HLT_e80_etcut_L1eEM26M', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM26M']),   # Phase-1 ATR-27156
        ChainProp(name='HLT_e100_etcut_L1eEM26M', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM26M']),  # Phase-1 ATR-27156

        # ATR-27373        
        ChainProp(name='HLT_e60_etcut_L1eEM28M', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_e70_etcut_L1eEM28M', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_e80_etcut_L1eEM28M', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_e100_etcut_L1eEM28M', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM28M']),

        #ATR-27251, Phase-I
        #ChainProp(name='HLT_e5_etcut_L1eEM5', groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM5']+['PS:NoBulkMCProd']),
        
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
        ChainProp(name='HLT_2g50_loose_L12eEM24L', groups=PrimaryPhIGroup+MultiPhotonGroup),

        # low-mass diphoton ATR-21637
        ChainProp(name='HLT_2g9_loose_25dphiAA_invmAA80_L1DPHI-M70-2eEM9', l1SeedThresholds=['eEM9'], groups=SupportPhIGroup+MultiPhotonGroup+Topo2Group),
        ChainProp(name='HLT_2g9_loose_25dphiAA_invmAA80_L1DPHI-M70-2eEM9L', l1SeedThresholds=['eEM9'], groups=EOFEgammaPhIGroup+MultiPhotonGroup+Topo2Group),

        # low-mass diphoton ATR-21608
        ChainProp(name='HLT_2g15_tight_25dphiAA_invmAA80_L1DPHI-M70-2eEM15M', l1SeedThresholds=['eEM15'], groups=PrimaryPhIGroup+MultiPhotonGroup+Topo2Group),
        ChainProp(name='HLT_2g15_loose_25dphiAA_invmAA80_L1DPHI-M70-2eEM15M', l1SeedThresholds=['eEM15'], groups=SupportPhIGroup+MultiPhotonGroup+['RATE:CPS_DPHI-M70-2eEM15M']+Topo2Group),
        ChainProp(name='HLT_2g15_tight_25dphiAA_L1DPHI-M70-2eEM15M', l1SeedThresholds=['eEM15'], groups=SupportPhIGroup+MultiPhotonGroup+['RATE:CPS_DPHI-M70-2eEM15M']+Topo2Group),

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

        
        # LRT idperf
        ChainProp(name='HLT_e9_lhtight_e4_idperf_tight_probe_1invmAB5_L1JPSI-1M5-EM7', stream=[PhysicsStream], l1SeedThresholds=['EM7','PROBEEM3'], groups=SupportLegGroup+MultiElectronGroup+['RATE:CPS_JPSI-1M5-EM7']+LegacyTopoGroup),
        ChainProp(name='HLT_e9_lhtight_e4_idperf_tight_nogsf_probe_1invmAB5_L1JPSI-1M5-EM7', stream=[PhysicsStream], l1SeedThresholds=['EM7','PROBEEM3'], groups=SupportLegGroup+MultiElectronGroup+['RATE:CPS_JPSI-1M5-EM7']+LegacyTopoGroup),
        ChainProp(name='HLT_e14_lhtight_e4_idperf_tight_probe_1invmAB5_L1JPSI-1M5-EM12', stream=[PhysicsStream,'express'], l1SeedThresholds=['EM12','PROBEEM3'], groups=SupportLegGroup+MultiElectronGroup+['RATE:CPS_JPSI-1M5-EM12']+LegacyTopoGroup, monGroups=['idMon:t0']),
        ChainProp(name='HLT_e14_lhtight_e4_idperf_tight_nogsf_probe_1invmAB5_L1JPSI-1M5-EM12', l1SeedThresholds=['EM12','PROBEEM3'], groups=SupportLegGroup+MultiElectronGroup+['RATE:CPS_JPSI-1M5-EM12']+LegacyTopoGroup),

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

        #Jpsiee        
        ChainProp(name='HLT_e9_lhtight_e4_etcut_1invmAB5_L1JPSI-1M5-EM7',  stream=[PhysicsStream], l1SeedThresholds=['EM7','EM3'], groups=SupportLegGroup+MultiElectronGroup+['RATE:CPS_JPSI-1M5-EM7']+LegacyTopoGroup),
        ChainProp(name='HLT_e5_lhtight_e9_etcut_1invmAB5_L1JPSI-1M5-EM7',  stream=[PhysicsStream], l1SeedThresholds=['EM3','EM7'], groups=SupportLegGroup+MultiElectronGroup+['RATE:CPS_JPSI-1M5-EM7']+LegacyTopoGroup),
        ChainProp(name='HLT_e5_lhtight_e14_etcut_1invmAB5_L1JPSI-1M5-EM12', stream=[PhysicsStream], l1SeedThresholds=['EM3','EM12'], groups=SupportLegGroup+MultiElectronGroup+['RATE:CPS_JPSI-1M5-EM12']+LegacyTopoGroup),
        ChainProp(name='HLT_e14_lhtight_e4_etcut_1invmAB5_L1JPSI-1M5-EM12', stream=[PhysicsStream], l1SeedThresholds=['EM12','EM3'], groups=SupportLegGroup+MultiElectronGroup+['RATE:CPS_JPSI-1M5-EM12']+LegacyTopoGroup),

        ChainProp(name='HLT_e9_lhtight_e4_etcut_1invmAB5_L1JPSI-1M5-eEM9', stream=[PhysicsStream,'express'], l1SeedThresholds=['eEM9','eEM5'], groups=SupportPhIGroup+MultiElectronGroup+Topo2Group+['RATE:CPS_JPSI-1M5-eEM9'], monGroups=['egammaMon:shifter_topo']),
        ChainProp(name='HLT_e5_lhtight_e9_etcut_1invmAB5_L1JPSI-1M5-eEM9', stream=[PhysicsStream,'express'], l1SeedThresholds=['eEM5','eEM9'], groups=SupportPhIGroup+MultiElectronGroup+Topo2Group+['RATE:CPS_JPSI-1M5-eEM9'], monGroups=['egammaMon:shifter_topo']),
        ChainProp(name='HLT_e5_lhtight_e14_etcut_1invmAB5_L1JPSI-1M5-eEM15', stream=[PhysicsStream,'express'], l1SeedThresholds=['eEM5','eEM15'], groups=SupportPhIGroup+MultiElectronGroup+Topo2Group+['RATE:CPS_JPSI-1M5-eEM15'], monGroups=['egammaMon:shifter_topo']),
        ChainProp(name='HLT_e14_lhtight_e4_etcut_1invmAB5_L1JPSI-1M5-eEM15', stream=[PhysicsStream,'express'], l1SeedThresholds=['eEM15','eEM5'], groups=SupportPhIGroup+MultiElectronGroup+Topo2Group+['RATE:CPS_JPSI-1M5-eEM15'], monGroups=['egammaMon:shifter_topo']),

        # ATR-24268
        # B->K*ee chains
        ChainProp(name='HLT_e5_lhvloose_bBeeM6000_L1BKeePrimary', l1SeedThresholds=['eEM5'], stream=['BphysDelayed','express'], groups=SupportPhIGroup+BphysElectronGroup+['RATE:CPS_BKeePrimary'], monGroups=['bphysMon:online','bphysMon:shifter']),
        ChainProp(name='HLT_2e5_lhvloose_bBeeM6000_L1BKeePrimary', l1SeedThresholds=['eEM5'], stream=['BphysDelayed','express'], groups=SupportPhIGroup+BphysElectronGroup+['RATE:CPS_BKeePrimary'], monGroups=['bphysMon:online','bphysMon:shifter']),
        ChainProp(name='HLT_e5_lhvloose_bBeeM6000_L1BKeePrescaled', l1SeedThresholds=['eEM5'], stream=['BphysDelayed'], groups=EOFBeePhIGroup+BphysElectronGroup),
        ChainProp(name='HLT_2e5_lhvloose_bBeeM6000_L1BKeePrescaled', l1SeedThresholds=['eEM5'], stream=['BphysDelayed'], groups=EOFBeePhIGroup+BphysElectronGroup),


        ChainProp(name='HLT_g35_medium_g25_medium_L1eEM9_EMPTY', l1SeedThresholds=['eEM9']*2, stream=['Late'], groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_g35_medium_g25_medium_L1eEM15_EMPTY', l1SeedThresholds=['eEM15']*2, stream=['Late'], groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_g35_medium_g25_medium_L1eEM9_UNPAIRED_ISO', l1SeedThresholds=['eEM9']*2, stream=['Late'], groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_2g22_tight_L1eEM9_EMPTY', l1SeedThresholds=['eEM9'], stream=['Late'], groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_2g22_tight_L1eEM15_EMPTY', l1SeedThresholds=['eEM15'], stream=['Late'], groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_2g22_tight_L1eEM9_UNPAIRED_ISO', l1SeedThresholds=['eEM9'], stream=['Late'], groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_2g50_tight_L1eEM9_EMPTY', l1SeedThresholds=['eEM9'], stream=['Late'], groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_2g50_tight_L1eEM15_EMPTY', l1SeedThresholds=['eEM15'], stream=['Late'], groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_2g50_tight_L1eEM9_UNPAIRED_ISO', l1SeedThresholds=['eEM9'], stream=['Late'], groups=PrimaryPhIGroup+MultiPhotonGroup),


    ]

    chains['MET'] += [
        ChainProp(name='HLT_xe65_cell_xe90_pfopufit_L1XE50', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream,'express'], groups=PrimaryLegGroup+METGroup, monGroups=['metMon:shifter','caloMon:t0']),
        # ATR-25512
        ChainProp(name='HLT_xe65_cell_xe100_pfopufit_L1XE50', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryLegGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe75_cell_xe100_pfopufit_L1XE50', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryLegGroup+METGroup, monGroups=['metMon:t0']),
        
        # ATR-27220 / ATR-26920
        ChainProp(name='HLT_xe65_cell_xe90_nn_L1XE50', l1SeedThresholds=['FSNOSEED']*2, groups=SupportLegGroup+METGroup+['RATE:CPS_XE50'], monGroups=['metMon:shifter']),
        ChainProp(name='HLT_xe65_cell_xe105_nn_L1XE50', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream,'express'], groups=SupportLegGroup+METGroup+['RATE:CPS_XE50'], monGroups=['metMon:shifter']),

        ChainProp(name='HLT_xe75_cell_xe65_tcpufit_xe90_trkmht_L1XE50', l1SeedThresholds=['FSNOSEED']*3, groups=SupportLegGroup+METGroup+['RATE:CPS_XE50'], monGroups=['metMon:shifter']),
        ChainProp(name='HLT_xe60_cell_xe95_pfsum_cssk_L1XE50', l1SeedThresholds=['FSNOSEED']*2, groups=SupportLegGroup+METGroup+['RATE:CPS_XE50'], monGroups=['metMon:shifter']),
        ChainProp(name='HLT_xe55_cell_xe70_tcpufit_xe90_pfsum_vssk_L1XE50', l1SeedThresholds=['FSNOSEED']*3, groups=SupportLegGroup+METGroup+['RATE:CPS_XE50'], monGroups=['metMon:shifter']),
        ChainProp(name='HLT_xe65_cell_xe105_mhtpufit_em_L1XE50', l1SeedThresholds=['FSNOSEED']*2, groups=SupportLegGroup+METGroup+['RATE:CPS_XE50'], monGroups=['metMon:shifter']),
        ChainProp(name='HLT_xe65_cell_xe100_mhtpufit_pf_L1XE50', l1SeedThresholds=['FSNOSEED']*2, groups=SupportLegGroup+METGroup+['RATE:CPS_XE50'], monGroups=['metMon:shifter']),
        ChainProp(name='HLT_xe55_cell_xe70_tcpufit_xe95_pfsum_cssk_L1XE50', l1SeedThresholds=['FSNOSEED']*3, groups=SupportLegGroup+METGroup+['RATE:CPS_XE50'], monGroups=['metMon:shifter']),
        ChainProp(name='HLT_xe65_cell_xe95_pfsum_vssk_L1XE50', l1SeedThresholds=['FSNOSEED']*2, groups=SupportLegGroup+METGroup+['RATE:CPS_XE50'], monGroups=['metMon:shifter']),

        ChainProp(name='HLT_xe30_cell_xe30_tcpufit_L1XE30', l1SeedThresholds=['FSNOSEED']*2, groups=SupportLegGroup+METGroup, monGroups=['metMon:t0']), #must be FS seeded
        ChainProp(name='HLT_xe65_cell_xe110_tcpufit_L1XE50',l1SeedThresholds=['FSNOSEED']*2, groups=SupportLegGroup+METGroup+['RATE:CPS_XE50'], monGroups=['metMon:t0']), # Intended PS=-1
        ChainProp(name='HLT_xe80_cell_xe115_tcpufit_L1XE50',l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryLegGroup+METGroup, monGroups=['metMon:shifter', 'caloMon:t0']),

        # Higher threshold due to L1 rate & ROS impact
        ChainProp(name='HLT_xe65_cell_xe90_pfopufit_L1XE55', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream,'express'], groups=PrimaryLegGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe100_pfopufit_L1XE55', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryLegGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe75_cell_xe100_pfopufit_L1XE55', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryLegGroup+METGroup, monGroups=['metMon:t0']),
        #ATR-28679
        ChainProp(name='HLT_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream,'express'], groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        #
        ChainProp(name='HLT_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream,'express'], groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        
        # ATR-27220 / ATR-26920
        ChainProp(name='HLT_xe65_cell_xe90_nn_L1XE55', l1SeedThresholds=['FSNOSEED']*2, groups=SupportLegGroup+METGroup+['RATE:CPS_XE55'], monGroups=['metMon:shifter']),
        ChainProp(name='HLT_xe65_cell_xe105_nn_L1XE55', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream,'express'], groups=SupportLegGroup+METGroup+['RATE:CPS_XE55'], monGroups=['metMon:shifter']),

        ChainProp(name='HLT_xe75_cell_xe65_tcpufit_xe90_trkmht_L1XE55', l1SeedThresholds=['FSNOSEED']*3, groups=SupportLegGroup+METGroup+['RATE:CPS_XE55'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe60_cell_xe95_pfsum_cssk_L1XE55', l1SeedThresholds=['FSNOSEED']*2, groups=SupportLegGroup+METGroup+['RATE:CPS_XE55'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe55_cell_xe70_tcpufit_xe90_pfsum_vssk_L1XE55', l1SeedThresholds=['FSNOSEED']*3, groups=SupportLegGroup+METGroup+['RATE:CPS_XE55'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe105_mhtpufit_em_L1XE55', l1SeedThresholds=['FSNOSEED']*2, groups=SupportLegGroup+METGroup+['RATE:CPS_XE55'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe100_mhtpufit_pf_L1XE55', l1SeedThresholds=['FSNOSEED']*2, groups=SupportLegGroup+METGroup+['RATE:CPS_XE55'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe55_cell_xe70_tcpufit_xe95_pfsum_cssk_L1XE55', l1SeedThresholds=['FSNOSEED']*3, groups=SupportLegGroup+METGroup+['RATE:CPS_XE55'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe95_pfsum_vssk_L1XE55', l1SeedThresholds=['FSNOSEED']*2, groups=SupportLegGroup+METGroup+['RATE:CPS_XE55'], monGroups=['metMon:t0']),

        ChainProp(name='HLT_xe80_cell_xe115_tcpufit_L1XE55',l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryLegGroup+METGroup, monGroups=['metMon:t0']),

        ChainProp(name='HLT_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),

        #ATR-28679
        ChainProp(name='HLT_xe65_cell_xe90_nn_L1jXE110', l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE110'], monGroups=['metMon:shifter']),
        ChainProp(name='HLT_xe65_cell_xe105_nn_L1jXE110', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream,'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE110'], monGroups=['metMon:shifter']),

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
        ChainProp(name='HLT_xe65_cell_xe105_nn_L1gXEJWOJ110', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream,'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ110'], monGroups=['metMon:shifter']),

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

        ChainProp(name='HLT_xe30_cell_xe30_tcpufit_L1jXE70', l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup, monGroups=['metMon:t0']), #must be FS seeded
        ChainProp(name='HLT_xe65_cell_xe110_tcpufit_L1jXE100',l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE100'], monGroups=['metMon:t0']), # Intended PS=-1
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

        ChainProp(name='HLT_xe55_cell_xe70_tcpufit_L1XE50',l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream, 'express'], groups=SupportLegGroup+METGroup+['RATE:CPS_XE50'], monGroups=['metMon:shifter']),
        ChainProp(name='HLT_xe55_cell_xe70_tcpufit_L1XE55',l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream, 'express'], groups=SupportLegGroup+METGroup+['RATE:CPS_XE55'], monGroups=['metMon:t0']),

        ChainProp(name='HLT_xe55_cell_xe70_tcpufit_L1jXE100',l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream, 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE100'], monGroups=['metMon:shifter']),
        ChainProp(name='HLT_xe55_cell_xe70_tcpufit_L1jXE110',l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream, 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE110'], monGroups=['metMon:t0']), #ATR-28679
        ChainProp(name='HLT_xe55_cell_xe70_tcpufit_L1gXEJWOJ110',l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream, 'express'], groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ110'], monGroups=['metMon:t0']), #ATR-28679

        # Chains added to test nSigma = 3.0
        ChainProp(name='HLT_xe65_cell_xe90_pfopufit_sig30_L1XE50',l1SeedThresholds=['FSNOSEED']*2, groups=SupportLegGroup+METGroup+['RATE:CPS_XE50'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe110_tcpufit_sig30_L1XE50',l1SeedThresholds=['FSNOSEED']*2, groups=SupportLegGroup+METGroup+['RATE:CPS_XE50'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe80_cell_xe115_tcpufit_sig30_L1XE50',l1SeedThresholds=['FSNOSEED']*2, groups=SupportLegGroup+METGroup+['RATE:CPS_XE50'], monGroups=['metMon:t0']),
        #
        ChainProp(name='HLT_xe65_cell_xe90_pfopufit_sig30_L1XE55',l1SeedThresholds=['FSNOSEED']*2, groups=SupportLegGroup+METGroup+['RATE:CPS_XE55'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe110_tcpufit_sig30_L1XE55',l1SeedThresholds=['FSNOSEED']*2, groups=SupportLegGroup+METGroup+['RATE:CPS_XE55'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe80_cell_xe115_tcpufit_sig30_L1XE55',l1SeedThresholds=['FSNOSEED']*2, groups=SupportLegGroup+METGroup+['RATE:CPS_XE55'], monGroups=['metMon:t0']),
        #ATR-28679
        ChainProp(name='HLT_xe65_cell_xe90_pfopufit_sig30_L1jXE110',l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe110_tcpufit_sig30_L1jXE110',l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe80_cell_xe115_tcpufit_sig30_L1jXE110',l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_jXE110'], monGroups=['metMon:t0']),

        ChainProp(name='HLT_xe65_cell_xe90_pfopufit_sig30_L1gXEJWOJ110',l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe110_tcpufit_sig30_L1gXEJWOJ110',l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ110'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe80_cell_xe115_tcpufit_sig30_L1gXEJWOJ110',l1SeedThresholds=['FSNOSEED']*2, groups=SupportPhIGroup+METGroup+['RATE:CPS_gXEJWOJ110'], monGroups=['metMon:t0']),

        # MET re-run chains: ATR-27220 / ATR-26456
        ChainProp(name='HLT_xe0_cell_L1XE50',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportLegGroup+METGroup+['RATE:CPS_XE50'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_tcpufit_L1XE50',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportLegGroup+METGroup+['RATE:CPS_XE50'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_trkmht_L1XE50',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportLegGroup+METGroup+['RATE:CPS_XE50'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_pfopufit_L1XE50',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportLegGroup+METGroup+['RATE:CPS_XE50'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_pfsum_L1XE50',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportLegGroup+METGroup+['RATE:CPS_XE50'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_pfsum_cssk_L1XE50',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportLegGroup+METGroup+['RATE:CPS_XE50'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_pfsum_vssk_L1XE50',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportLegGroup+METGroup+['RATE:CPS_XE50'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_mhtpufit_em_L1XE50',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportLegGroup+METGroup+['RATE:CPS_XE50'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_mhtpufit_pf_L1XE50',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportLegGroup+METGroup+['RATE:CPS_XE50'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_nn_L1XE50',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportLegGroup+METGroup+['RATE:CPS_XE50'], monGroups=['metMon:t0']),

        ChainProp(name='HLT_xe0_cell_L1XE55',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportLegGroup+METGroup+['RATE:CPS_XE55'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_tcpufit_L1XE55',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportLegGroup+METGroup+['RATE:CPS_XE55'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_trkmht_L1XE55',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportLegGroup+METGroup+['RATE:CPS_XE55'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_pfopufit_L1XE55',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportLegGroup+METGroup+['RATE:CPS_XE55'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_pfsum_L1XE55',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportLegGroup+METGroup+['RATE:CPS_XE55'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_pfsum_cssk_L1XE55',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportLegGroup+METGroup+['RATE:CPS_XE55'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_pfsum_vssk_L1XE55',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportLegGroup+METGroup+['RATE:CPS_XE55'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_mhtpufit_em_L1XE55',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportLegGroup+METGroup+['RATE:CPS_XE55'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_mhtpufit_pf_L1XE55',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportLegGroup+METGroup+['RATE:CPS_XE55'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe0_nn_L1XE55',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=SupportLegGroup+METGroup+['RATE:CPS_XE55'], monGroups=['metMon:t0']),

        ChainProp(name='HLT_xe0_cell_L1ZB',  l1SeedThresholds=['FSNOSEED'], stream=['ZeroBias'], groups=SupportLegGroup+METGroup+['RATE:ZeroBias','RATE:CPS_ZB']), #ATR-26456
        ChainProp(name='HLT_xe0_tcpufit_L1ZB',  l1SeedThresholds=['FSNOSEED'], stream=['ZeroBias'], groups=SupportLegGroup+METGroup+['RATE:ZeroBias','RATE:CPS_ZB']),
        ChainProp(name='HLT_xe0_trkmht_L1ZB',  l1SeedThresholds=['FSNOSEED'], stream=['ZeroBias'], groups=SupportLegGroup+METGroup+['RATE:ZeroBias','RATE:CPS_ZB']),
        ChainProp(name='HLT_xe0_pfopufit_L1ZB',  l1SeedThresholds=['FSNOSEED'], stream=['ZeroBias'], groups=SupportLegGroup+METGroup+['RATE:ZeroBias','RATE:CPS_ZB']),
        ChainProp(name='HLT_xe0_pfsum_L1ZB',  l1SeedThresholds=['FSNOSEED'], stream=['ZeroBias'], groups=SupportLegGroup+METGroup+['RATE:ZeroBias','RATE:CPS_ZB']),
        ChainProp(name='HLT_xe0_pfsum_cssk_L1ZB',  l1SeedThresholds=['FSNOSEED'], stream=['ZeroBias'], groups=SupportLegGroup+METGroup+['RATE:ZeroBias','RATE:CPS_ZB']),
        ChainProp(name='HLT_xe0_pfsum_vssk_L1ZB',  l1SeedThresholds=['FSNOSEED'], stream=['ZeroBias'], groups=SupportLegGroup+METGroup+['RATE:ZeroBias','RATE:CPS_ZB']),
        ChainProp(name='HLT_xe0_mhtpufit_em_L1ZB',  l1SeedThresholds=['FSNOSEED'], stream=['ZeroBias'], groups=SupportLegGroup+METGroup+['RATE:ZeroBias','RATE:CPS_ZB']),
        ChainProp(name='HLT_xe0_mhtpufit_pf_L1ZB',  l1SeedThresholds=['FSNOSEED'], stream=['ZeroBias'], groups=SupportLegGroup+METGroup+['RATE:ZeroBias','RATE:CPS_ZB']),
        ChainProp(name='HLT_xe0_nn_L1ZB',  l1SeedThresholds=['FSNOSEED'], stream=['ZeroBias'], groups=SupportLegGroup+METGroup+['RATE:ZeroBias','RATE:CPS_ZB']),

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
    ]

    chains['Jet'] += [
        # Support performance chains (for emulation+calibration studies) ATR-20624
        ChainProp(name='HLT_j0_perf_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_j0_perf_pf_ftf_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),

        # Central single small-R jet chains
        ## PFlow calibration triggers
        ChainProp(name='HLT_j15_pf_ftf_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_j25_pf_ftf_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_j35_pf_ftf_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_j45_pf_ftf_preselj20_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_j45_pf_ftf_preselj20_L1J15', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J15'], monGroups=['jetMon:t0','jetMon:online','idMon:shifter','caloMon:t0']),
        ChainProp(name='HLT_j60_pf_ftf_preselj50_L1J20', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J20'], monGroups=['jetMon:shifter', 'jetMon:online']),
        ChainProp(name='HLT_j85_pf_ftf_preselj50_L1J20', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J20']),
        ChainProp(name='HLT_j175_pf_ftf_preselj140_L1J50', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J50']),
        ChainProp(name='HLT_j360_pf_ftf_preselj225_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J100']),
        ## EMTopo calibration/monitoring triggers, same threshold as PF for rate evaluation
        ChainProp(name='HLT_j15_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_j25_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_j35_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_j45_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_j45_L1J15', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J15'], monGroups=['jetMon:t0','jetMon:online','caloMon:t0']),
        ChainProp(name='HLT_j60_L1J20', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J20']),
        ChainProp(name='HLT_j85_L1J20', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J20']),
        ChainProp(name='HLT_j175_L1J50', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J50']),
        ChainProp(name='HLT_j360_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J100']),
        #ATR-27257, Phase-I
        ChainProp(name='HLT_j60_L1jJ50', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ50']),     
        ChainProp(name='HLT_j85_L1jJ50', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ50']),

        ## Phase-I EMTopo calibration/monitoring triggers, same threshold as PF for rate evaluation
        ChainProp(name='HLT_j45_L1jJ40', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ40'], monGroups=['jetMon:t0','jetMon:online','caloMon:t0']),
        ChainProp(name='HLT_j110_L1jJ60', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ60']),
        ChainProp(name='HLT_j175_L1jJ90', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ90']),
        ChainProp(name='HLT_j260_L1jJ125', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ125']),
        ChainProp(name='HLT_j360_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ160']),      
        ## Monitoring triggers
        ChainProp(name='HLT_j45_ftf_preselj20_L1J15', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J15'], monGroups=['jetMon:t0','jetMon:online']),
        ## Phase-I Monitoring triggers
        ChainProp(name='HLT_j45_ftf_preselj20_L1jJ40', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ40'], monGroups=['jetMon:t0','jetMon:online']),
        ### no presel mon
        ChainProp(name='HLT_j400_pf_ftf_L1J100', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J100'], monGroups=['jetMon:t0']),
        ### Phase-I no presel mon
        ChainProp(name='HLT_j400_pf_ftf_L1jJ160', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ160'], monGroups=['jetMon:t0']), 
        ## PFlow primary triggers
        ChainProp(name='HLT_j400_pf_ftf_preselj225_L1J100', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SingleJetGroup+PrimaryLegGroup, monGroups=['jetMon:shifter','jetMon:online', 'caloMon:t0']),
        ChainProp(name='HLT_j440_pf_ftf_preselj225_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j450_pf_ftf_preselj225_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j460_pf_ftf_preselj225_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j480_pf_ftf_preselj225_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j500_pf_ftf_preselj225_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j520_pf_ftf_preselj225_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryLegGroup),
        #ATR-27257, Phase-I
        ChainProp(name='HLT_j400_pf_ftf_preselj225_L1jJ180', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup), 

        ## EMTopo back-up primary triggers (ATR-20049, ATR-23152)
        ChainProp(name='HLT_j420_L1J100', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SupportLegGroup+SingleJetGroup, monGroups=['jetMon:t0','caloMon:t0']),
        ChainProp(name='HLT_j440_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j450_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j460_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j480_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j500_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j520_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryLegGroup),
        ## Phase-I EMTopo back-up primary triggers (ATR-20049, ATR-23152)
        ChainProp(name='HLT_j420_L1jJ180', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SupportPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j440_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j450_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j460_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j480_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j500_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j520_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ## EMTopo+FTF back-up primary triggers
        ChainProp(name='HLT_j420_ftf_preselj225_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J100']),
        ChainProp(name='HLT_j440_ftf_preselj225_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j450_ftf_preselj225_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j460_ftf_preselj225_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j480_ftf_preselj225_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j500_ftf_preselj225_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j520_ftf_preselj225_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryLegGroup),
        ## Phase-I EMTopo+FTF back-up primary triggers
        ChainProp(name='HLT_j420_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ160']),
        ChainProp(name='HLT_j420_ftf_preselj225_L1jJ180', l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j440_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j450_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j460_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j480_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j500_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j520_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),

        # Central single large-R jet chains (ATR-20049, ATR-23152)
        ChainProp(name='HLT_j175_a10sd_cssk_pf_jes_ftf_preselj140_L1J50', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J50']),
        ChainProp(name='HLT_j175_a10t_lcw_jes_L1J50', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J50']),
        ChainProp(name='HLT_j360_a10sd_cssk_pf_jes_ftf_preselj225_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J100']),
        ChainProp(name='HLT_j360_a10sd_cssk_pf_jes_ftf_preselj225_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup+LegacyTopoGroup+['RATE:CPS_SC111-CJ15']),
        ChainProp(name='HLT_j360_a10t_lcw_jes_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J100']),
        ChainProp(name='HLT_j360_a10t_lcw_jes_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup+LegacyTopoGroup+['RATE:CPS_SC111-CJ15']),
        ChainProp(name='HLT_j420_35smcINF_a10t_lcw_jes_L1J100', l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleJetGroup, monGroups=['jetMon:shifter']),
        ChainProp(name='HLT_j420_35smcINF_a10t_lcw_jes_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j420_35smcINF_a10sd_cssk_pf_jes_ftf_preselj225_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryLegGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_j420_35smcINF_a10sd_cssk_pf_jes_ftf_preselj225_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SingleJetGroup+PrimaryLegGroup+LegacyTopoGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_j460_a10sd_cssk_pf_jes_ftf_preselj225_L1J100', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SingleJetGroup+PrimaryLegGroup, monGroups=['jetMon:shifter', 'jetMon:online']),
        ChainProp(name='HLT_j460_a10sd_cssk_pf_jes_ftf_preselj225_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryLegGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j460_a10t_lcw_jes_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryLegGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_j460_a10t_lcw_jes_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryLegGroup+LegacyTopoGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_j460_a10r_L1J100', l1SeedThresholds=['FSNOSEED'],  groups=PrimaryLegGroup+SingleJetGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_j460_a10r_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED'],  groups=PrimaryLegGroup+SingleJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j460_a10_lcw_subjes_L1J100', l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleJetGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_j460_a10_lcw_subjes_L1SC111-CJ15',         l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleJetGroup+LegacyTopoGroup),
        #ATR-27257, Phase-I
        ChainProp(name='HLT_j360_a10sd_cssk_pf_jes_ftf_preselj225_L1jLJ140', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),

        
        ## back-up
        ChainProp(name='HLT_j480_a10sd_cssk_pf_jes_ftf_preselj225_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j480_a10sd_cssk_pf_jes_ftf_preselj225_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryLegGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j480_a10t_lcw_jes_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j480_a10t_lcw_jes_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryLegGroup+LegacyTopoGroup),
        ##Low-threshold large-R chains (for calibration purposes)
        ChainProp(name='HLT_j85_a10sd_cssk_pf_nojcalib_ftf_preselj50_L1J20', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J20']),
        ChainProp(name='HLT_j85_a10sd_cssk_pf_jes_ftf_preselj50_L1J20', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J20']),
        ChainProp(name='HLT_j85_a10t_lcw_nojcalib_L1J20', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J20']),
        ChainProp(name='HLT_j85_a10t_lcw_jes_L1J20',      l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J20']),

        ## Monitoring triggers
        ### no mass cut
        ChainProp(name='HLT_j420_a10t_lcw_jes_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J100']),
        ChainProp(name='HLT_j420_a10sd_cssk_pf_jes_ftf_preselj225_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J100']),
           
        #Forward small-R EMTopo chains (ATR-20049, ATR-24720)
        ChainProp(name='HLT_j15f_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_j25f_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_j35f_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_j280f_L1J75p31ETA49', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j300f_L1J75p31ETA49', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j220f_L1J75p31ETA49', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=PrimaryLegGroup+SingleJetGroup, monGroups=['jetMon:shifter', 'jetMon:online', 'caloMon:t0']),
        ChainProp(name='HLT_j230f_L1J75p31ETA49', l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleJetGroup),
        ChainProp(name='HLT_j240f_L1J75p31ETA49', l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleJetGroup),
        ChainProp(name='HLT_j260f_L1J75p31ETA49', l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleJetGroup),
      
        # Small-R multijet chains
        ## PFlow primaries
        ChainProp(name='HLT_2j235c_j115c_pf_ftf_presel2j180XXj80_L1J100', l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup + PrimaryLegGroup),
        ChainProp(name='HLT_3j190_pf_ftf_presel3j150_L1J100', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryLegGroup), 
        ChainProp(name='HLT_4j110_pf_ftf_presel4j85_L13J50', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=MultiJetGroup + PrimaryLegGroup, monGroups=['jetMon:t0']), 
        ChainProp(name='HLT_5j65c_pf_ftf_presel5c50_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryLegGroup), 
        ChainProp(name='HLT_5j65c_pf_ftf_presel5c50_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_5j80_pf_ftf_presel5j50_L14J15', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=MultiJetGroup + PrimaryLegGroup, monGroups=['jetMon:t0']),
        # ATR-25512
        ChainProp(name='HLT_5j80_pf_ftf_presel5j55_L14J15', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=MultiJetGroup + PrimaryLegGroup),
        ChainProp(name='HLT_5j65c_pf_ftf_presel5c55_L14J15', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=MultiJetGroup + PrimaryLegGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_5j65c_pf_ftf_presel5c55_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=MultiJetGroup + PrimaryPhIGroup, monGroups=['jetMon:t0']), 

        ChainProp(name='HLT_6j55c_pf_ftf_presel6j40_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryLegGroup),
        ChainProp(name='HLT_6j65_pf_ftf_presel6j40_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryLegGroup), 
        # ATR-25512
        ChainProp(name='HLT_6j65_pf_ftf_presel6j45_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryLegGroup), 
        ChainProp(name='HLT_6j55c_pf_ftf_presel6c45_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryLegGroup),

        ChainProp(name='HLT_7j45_pf_ftf_presel7j30_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryLegGroup),
        ChainProp(name='HLT_10j35_pf_ftf_presel7j30_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryLegGroup),
        
        ## EMTopo backups, should increase thresholds for these and following
        ChainProp(name='HLT_3j200_L1J100', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + SupportLegGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_4j120_L13J50', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=MultiJetGroup + SupportLegGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_5j70c_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + SupportLegGroup),
        ChainProp(name='HLT_5j85_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + SupportLegGroup),
        ChainProp(name='HLT_6j55c_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + SupportLegGroup),
        ChainProp(name='HLT_6j70_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + SupportLegGroup),
        ChainProp(name='HLT_7j45_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + SupportLegGroup),
        ChainProp(name='HLT_10j40_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + SupportLegGroup),
        ## FTF+EMTopo
        ChainProp(name='HLT_2j250c_j120c_ftf_presel2j180XXj80_L1J100', l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup+SupportLegGroup+['RATE:CPS_J100']),
        ChainProp(name='HLT_3j200_ftf_presel3j150_L1J100', l1SeedThresholds=['FSNOSEED'],           groups=MultiJetGroup+SupportLegGroup+['RATE:CPS_J100']),
        ChainProp(name='HLT_4j115_ftf_presel4j85_L13J50', l1SeedThresholds=['FSNOSEED'],            groups=MultiJetGroup+SupportLegGroup),
        ChainProp(name='HLT_4j115_ftf_presel4j85_L13jJ90', l1SeedThresholds=['FSNOSEED'],            groups=MultiJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_5j70c_ftf_presel5j50_L14J15', l1SeedThresholds=['FSNOSEED'],     groups=MultiJetGroup+SupportLegGroup+['RATE:CPS_4J15']),
        ChainProp(name='HLT_5j70c_ftf_presel5j50_L14jJ40', l1SeedThresholds=['FSNOSEED'],     groups=MultiJetGroup+SupportPhIGroup+['RATE:CPS_4jJ40']),
        ChainProp(name='HLT_5j85_ftf_presel5j50_L14J15', l1SeedThresholds=['FSNOSEED'],             groups=MultiJetGroup+SupportLegGroup+['RATE:CPS_4J15']),
        ChainProp(name='HLT_5j85_ftf_presel5j50_L14jJ40', l1SeedThresholds=['FSNOSEED'],             groups=MultiJetGroup+SupportPhIGroup+['RATE:CPS_4jJ40']),
        ChainProp(name='HLT_6j55c_ftf_presel6j40_L14J15', l1SeedThresholds=['FSNOSEED'],     groups=MultiJetGroup+SupportLegGroup+['RATE:CPS_4J15']),
        ChainProp(name='HLT_6j55c_ftf_presel6j40_L14jJ40', l1SeedThresholds=['FSNOSEED'],     groups=MultiJetGroup+SupportPhIGroup+['RATE:CPS_4jJ40']),
        ChainProp(name='HLT_6j70_ftf_presel6j40_L14J15', l1SeedThresholds=['FSNOSEED'],             groups=MultiJetGroup+SupportLegGroup+['RATE:CPS_4J15']),
        ChainProp(name='HLT_6j70_ftf_presel6j40_L14jJ40', l1SeedThresholds=['FSNOSEED'],             groups=MultiJetGroup+SupportPhIGroup+['RATE:CPS_4jJ40']),
        ChainProp(name='HLT_7j45_ftf_presel7j30_L14J15', l1SeedThresholds=['FSNOSEED'],             groups=MultiJetGroup+SupportLegGroup+['RATE:CPS_4J15']),
        ChainProp(name='HLT_7j45_ftf_presel7j30_L14jJ40', l1SeedThresholds=['FSNOSEED'],             groups=MultiJetGroup+SupportPhIGroup+['RATE:CPS_4jJ40']),
        ChainProp(name='HLT_10j40_ftf_presel7j30_L14J15', l1SeedThresholds=['FSNOSEED'],            groups=MultiJetGroup+SupportLegGroup+['RATE:CPS_4J15']),
        ChainProp(name='HLT_10j40_ftf_presel7j30_L14jJ40', l1SeedThresholds=['FSNOSEED'],            groups=MultiJetGroup+SupportPhIGroup+['RATE:CPS_4jJ40']),
        ## Phase-I FTF+EMTopo
        ChainProp(name='HLT_2j250c_j120c_ftf_presel2j180XXj80_L1jJ160', l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup+SupportPhIGroup+['RATE:CPS_jJ160']),
        ChainProp(name='HLT_3j200_ftf_presel3j150_L1jJ160', l1SeedThresholds=['FSNOSEED'],           groups=MultiJetGroup+SupportPhIGroup+['RATE:CPS_jJ160']),
        ## Monitoring triggers
        ### no presel
        ChainProp(name='HLT_3j190_pf_ftf_L1J100', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MultiJetGroup+SupportLegGroup+['RATE:CPS_J100'], monGroups=['jetMon:t0']), 
        ### no jvt
        ChainProp(name='HLT_6j35c_pf_ftf_presel6c25_L14J15', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=MultiJetGroup+SupportLegGroup+['RATE:CPS_4J15'], monGroups=['jetMon:t0']),
        ChainProp(name='HLT_6j35c_pf_ftf_presel6c25_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=MultiJetGroup+SupportPhIGroup+['RATE:CPS_4jJ40'], monGroups=['jetMon:t0']),
        ### Phase-I no presel
        ChainProp(name='HLT_3j190_pf_ftf_L1jJ160', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MultiJetGroup+SupportPhIGroup+['RATE:CPS_jJ160'], monGroups=['jetMon:t0']),
        # Large-R multijet chains (ATR-20049, ATR-23152)
        ChainProp(name='HLT_2j330_35smcINF_a10t_lcw_jes_L1J100', l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+MultiJetGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_2j330_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1J100', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+PrimaryLegGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_2j330_35smcINF_a10t_lcw_jes_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_2j330_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MultiJetGroup+PrimaryLegGroup+LegacyTopoGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_j360_60smcINF_j360_a10t_lcw_jes_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryLegGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j370_35smcINF_j370_a10t_lcw_jes_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryLegGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j360_60smcINF_j360_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryLegGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j370_35smcINF_j370_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryLegGroup+MultiJetGroup+LegacyTopoGroup),
        ## Monitoring triggers
        ### no mass cut
        ChainProp(name='HLT_2j330_a10t_lcw_jes_L1J100', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+SupportLegGroup+['RATE:CPS_J100'], monGroups=['jetMon:t0']),
        ChainProp(name='HLT_2j330_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+SupportLegGroup+LegacyTopoGroup+['RATE:CPS_SC111-CJ15'], monGroups=['jetMon:t0']),
        ### Phase-I no mass cut
        ChainProp(name='HLT_2j330_a10t_lcw_jes_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+SupportPhIGroup+['RATE:CPS_jJ160'], monGroups=['jetMon:t0']),
        ChainProp(name='HLT_2j330_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC111-CjJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+SupportPhIGroup+Topo3Group+['RATE:CPS_SC111-CjJ40'], monGroups=['jetMon:t0']),


        ## ATR-25456 - Emerging jet
        # primary emerging jets single-jet chain
        ChainProp(name='HLT_j175_0eta180_emergingPTF0p08dR1p2_a10sd_cssk_pf_jes_ftf_preselj225_L1J100', groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J100'], l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j200_0eta180_emergingPTF0p08dR1p2_a10sd_cssk_pf_jes_ftf_preselj200_L1J100', groups=SingleJetGroup+PrimaryLegGroup, l1SeedThresholds=['FSNOSEED']),
        # [ATR-26377] Backup emerging jet chain
        ChainProp(name='HLT_j200_0eta160_emergingPTF0p08dR1p2_a10sd_cssk_pf_jes_ftf_preselj200_L1J100', groups=SingleJetGroup+PrimaryLegGroup, l1SeedThresholds=['FSNOSEED']),
        # Phase-I primary emerging jets single-jet chain
        ChainProp(name='HLT_j175_0eta180_emergingPTF0p08dR1p2_a10sd_cssk_pf_jes_ftf_preselj225_L1jJ160', groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ160'], l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j200_0eta180_emergingPTF0p08dR1p2_a10sd_cssk_pf_jes_ftf_preselj200_L1jJ160', groups=SingleJetGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED']),
        # Phase-I Backup emerging jet chain
        ChainProp(name='HLT_j200_0eta160_emergingPTF0p08dR1p2_a10sd_cssk_pf_jes_ftf_preselj200_L1jJ160', groups=SingleJetGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED']),

        
        ##HT chains
        ChainProp(name='HLT_j0_HT940_pf_ftf_preselj180_L1J100', l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleJetGroup, monGroups=['jetMon:shifter','jetMon:online']),
        ChainProp(name='HLT_j0_HT940_pf_ftf_preselcHT450_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=PrimaryLegGroup+SingleJetGroup+LegacyTopoGroup, monGroups=['jetMon:online','jetMon:shifter','caloMon:t0']),

        ChainProp(name='HLT_j0_HT1000_L1J100', l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleJetGroup, monGroups=['jetMon:shifter','jetMon:online']),
        ChainProp(name='HLT_j0_HT1000_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SupportLegGroup+SingleJetGroup+LegacyTopoGroup, monGroups=['jetMon:shifter', 'jetMon:online']),
        
        
        # ATR-21596 HT Delayed for Dark Showers 
        ChainProp(name='HLT_j0_HT650XX0eta240_pf_ftf_preselcHT450_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=PrimaryLegGroup+SingleJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT650XX0eta240_pf_ftf_preselcHT450_L1HT190-jJ40s5pETA21', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+SingleJetGroup+Topo3Group),

        # ATR-25512
        ChainProp(name='HLT_j0_HT940_pf_ftf_preselj190_L1J100', l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleJetGroup),
        ChainProp(name='HLT_j0_HT940_pf_ftf_preselj200_L1J100', l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleJetGroup),
        ChainProp(name='HLT_j0_HT940_pf_ftf_preselj180_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=PrimaryLegGroup+SingleJetGroup+LegacyTopoGroup), 

        # Multijet delayed stream
        ChainProp(name='HLT_6j35c_020jvt_pf_ftf_presel6c25_L14J15', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=PrimaryLegGroup+MultiJetGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_6j45c_020jvt_pf_ftf_presel6c25_L14J15', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=PrimaryLegGroup+MultiJetGroup),
        ChainProp(name='HLT_j70_j50a_j0_DJMASS1000j50dphi200x400deta_L1MJJ-500-NFF', l1SeedThresholds=['FSNOSEED']*3,stream=['VBFDelayed'],groups=PrimaryLegGroup+MultiJetGroup+LegacyTopoGroup), # previously HLT_j70_j50_0eta490_invm1000j70_dphi20_deta40_L1MJJ-500-NFF


        ## TLA chains
        ChainProp(name='HLT_j20_PhysicsTLA_L1J100', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryLegGroup+SingleJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_j20_PhysicsTLA_L1J50_DETA20-J50J', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=EOFTLALegGroup+SingleJetGroup+LegacyTopoGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_j20_PhysicsTLA_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryLegGroup+SingleJetGroup+LegacyTopoGroup, monGroups=['tlaMon:shifter']),
        # TLA chains with PFlow, ATR-20395
        ChainProp(name='HLT_j20_pf_ftf_preselj140_PhysicsTLA_L1J50',  l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=EOFTLALegGroup+SingleJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_j20_pf_ftf_preselj180_PhysicsTLA_L1J100', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryLegGroup+SingleJetGroup, monGroups=['tlaMon:shifter']),
        # Phase-I TLA chains with PFlow
        ChainProp(name='HLT_j20_pf_ftf_preselj140_PhysicsTLA_L1jJ90',  l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=EOFTLAPhIGroup+SingleJetGroup, monGroups=['tlaMon:shifter']),
        # TLA chains with PFlow, ATR-21594
        ChainProp(name='HLT_j20_pf_ftf_preselcHT450_PhysicsTLA_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=MultiJetGroup+PrimaryLegGroup+LegacyTopoGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_j20_pf_ftf_preselcHT450_PhysicsTLA_L1HT190-jJ40s5pETA21', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=MultiJetGroup+PrimaryPhIGroup+Topo3Group, monGroups=['tlaMon:shifter']),
        # MultiJet TLA support for intensity ramp up
        ChainProp(name='HLT_2j20_2j20_pf_ftf_presel2c20XX2c20b85_PhysicsTLA_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*2, stream=['TLA'], groups=MultiJetGroup+SupportPhIGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_j60_j45_j25_j20_pf_ftf_preselc60XXc45XXc25XXc20_PhysicsTLA_L1jJ85p0ETA21_3jJ40p0ETA25',l1SeedThresholds=['FSNOSEED']*4, stream=['TLA'], groups=MultiJetGroup+SupportPhIGroup, monGroups=['tlaMon:shifter']),

        # ATR-25512
        ChainProp(name='HLT_j20_pf_ftf_preselj190_PhysicsTLA_L1J100', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryLegGroup+SingleJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_j20_pf_ftf_preselj200_PhysicsTLA_L1J100', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryLegGroup+SingleJetGroup, monGroups=['tlaMon:shifter']),
        # ATR-25512 Duplicating with PhaseI items
        ChainProp(name='HLT_j20_pf_ftf_preselj180_PhysicsTLA_L1jJ160', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+SingleJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_j20_pf_ftf_preselj190_PhysicsTLA_L1jJ160', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+SingleJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_j20_pf_ftf_preselj200_PhysicsTLA_L1jJ160', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+SingleJetGroup, monGroups=['tlaMon:shifter']),

        #ATR-24411 Phase I inputs
        # Single jet support 
        ChainProp(name='HLT_j45_pf_ftf_preselj20_L1jJ40', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SingleJetGroup+SupportPhIGroup, monGroups=['idMon:shifter','jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j60_pf_ftf_preselj50_L1jJ50', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ50'], monGroups=['jetMon:shifter', 'jetMon:online']),
        ChainProp(name='HLT_j85_pf_ftf_preselj50_L1jJ50', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ50']),
        ChainProp(name='HLT_j110_pf_ftf_preselj80_L1jJ60', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ60']),
        ChainProp(name='HLT_j175_pf_ftf_preselj140_L1jJ90', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ90']),
        ChainProp(name='HLT_j260_pf_ftf_preselj200_L1jJ125', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ125']),
        ChainProp(name='HLT_j360_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ160']),
        ChainProp(name='HLT_j380_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ160']),
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
        ChainProp(name='HLT_j175_a10sd_cssk_pf_jes_ftf_preselj140_L1jJ90', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ90']),
        ChainProp(name='HLT_j175_a10t_lcw_jes_L1jJ90', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ90']),
        ChainProp(name='HLT_j260_a10sd_cssk_pf_jes_ftf_preselj200_L1jJ125', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ125']),
        ChainProp(name='HLT_j260_a10t_lcw_jes_L1jJ125', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ125']),
        ChainProp(name='HLT_j360_a10sd_cssk_pf_jes_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ160']),
        ChainProp(name='HLT_j360_a10sd_cssk_pf_jes_ftf_preselj225_L1SC111-CjJ40', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+Topo3Group+['RATE:CPS_SC111-CjJ40']),
        ChainProp(name='HLT_j360_a10t_lcw_jes_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ160']),
        ChainProp(name='HLT_j360_a10t_lcw_jes_L1SC111-CjJ40', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+Topo3Group+['RATE:CPS_SC111-CjJ40']),
        ChainProp(name='HLT_j400_a10sd_cssk_pf_jes_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ160']),
        ChainProp(name='HLT_j400_a10sd_cssk_pf_jes_ftf_preselj225_L1SC111-CjJ40', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+Topo3Group+['RATE:CPS_SC111-CjJ40']),
        ChainProp(name='HLT_j400_a10t_lcw_jes_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ160']),
        ChainProp(name='HLT_j400_a10t_lcw_jes_L1SC111-CjJ40', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+Topo3Group+['RATE:CPS_SC111-CjJ40']),
        ChainProp(name='HLT_j420_a10sd_cssk_pf_jes_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ160']),
        ChainProp(name='HLT_j420_a10sd_cssk_pf_jes_ftf_preselj225_L1SC111-CjJ40', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+Topo3Group+['RATE:CPS_SC111-CjJ40']),
        ChainProp(name='HLT_j420_a10t_lcw_jes_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ160']),
        ChainProp(name='HLT_j420_a10t_lcw_jes_L1SC111-CjJ40', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+Topo3Group+['RATE:CPS_SC111-CjJ40']),
        ChainProp(name='HLT_j460_a10sd_cssk_pf_jes_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:shifter','jetMon:online']),
        ChainProp(name='HLT_j460_a10sd_cssk_pf_jes_ftf_preselj225_L1SC111-CjJ40', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup+Topo3Group),
        ChainProp(name='HLT_j460_a10t_lcw_jes_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_j460_a10t_lcw_jes_L1SC111-CjJ40', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup+Topo3Group),
        ChainProp(name='HLT_j480_a10sd_cssk_pf_jes_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j480_a10sd_cssk_pf_jes_ftf_preselj225_L1SC111-CjJ40', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup+Topo3Group),
        ChainProp(name='HLT_j480_a10t_lcw_jes_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j480_a10t_lcw_jes_L1SC111-CjJ40', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup+Topo3Group),
        # Low threshold large-R chains (for calibration purposes)
        ChainProp(name='HLT_j85_a10sd_cssk_pf_nojcalib_ftf_preselj50_L1jJ50', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ50']),
        ChainProp(name='HLT_j85_a10sd_cssk_pf_jes_ftf_preselj50_L1jJ50', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ50']),
        ChainProp(name='HLT_j85_a10t_lcw_nojcalib_L1jJ50', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ50']),
        ChainProp(name='HLT_j85_a10t_lcw_jes_L1jJ50',      l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ50']),

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
        ChainProp(name='HLT_j460_a10_lcw_subjes_L1SC111-CjJ40',         l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup+Topo3Group),
        ChainProp(name='HLT_j460_a10r_L1jJ160', l1SeedThresholds=['FSNOSEED'],  groups=PrimaryPhIGroup+SingleJetGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_j460_a10r_L1SC111-CjJ40', l1SeedThresholds=['FSNOSEED'],  groups=PrimaryPhIGroup+SingleJetGroup+Topo3Group),
        ChainProp(name='HLT_j460_a10_lcw_subjes_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_j420_35smcINF_a10t_lcw_jes_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup, monGroups=['jetMon:shifter']),
        ChainProp(name='HLT_2j330_35smcINF_a10t_lcw_jes_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_j420_35smcINF_a10sd_cssk_pf_jes_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_2j330_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+PrimaryPhIGroup, monGroups=['jetMon:t0']),

        ChainProp(name='HLT_j420_35smcINF_a10t_lcw_jes_L1SC111-CjJ40', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup+Topo3Group),
        ChainProp(name='HLT_2j330_35smcINF_a10t_lcw_jes_L1SC111-CjJ40', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),
        ChainProp(name='HLT_j420_35smcINF_a10sd_cssk_pf_jes_ftf_preselj225_L1SC111-CjJ40', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SingleJetGroup+PrimaryPhIGroup+Topo3Group, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_2j330_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC111-CjJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+PrimaryPhIGroup+Topo3Group, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_j360_60smcINF_j360_a10t_lcw_jes_L1SC111-CjJ40', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),
        ChainProp(name='HLT_j370_35smcINF_j370_a10t_lcw_jes_L1SC111-CjJ40', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),
        ChainProp(name='HLT_j360_60smcINF_j360_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC111-CjJ40', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),
        ChainProp(name='HLT_j370_35smcINF_j370_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC111-CjJ40', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),

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
        ChainProp(name='HLT_5j80_pf_ftf_presel5j55_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup, monGroups=['jetMon:t0']), 
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
        
        ChainProp(name='HLT_j0_HT940_pf_ftf_preselj180_L1HT190-jJ40s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup+Topo3Group),
        ChainProp(name='HLT_j0_HT940_pf_ftf_preselcHT450_L1HT190-jJ40s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup+Topo3Group, monGroups=['jetMon:online', 'jetMon:shifter']), 

        # Multijet delayed stream
        ChainProp(name='HLT_6j35c_020jvt_pf_ftf_presel6c25_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=['VBFDelayed', 'express'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_6j45c_020jvt_pf_ftf_presel6c25_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiJetGroup),

        # TLA chains
        ChainProp(name='HLT_j20_PhysicsTLA_L1jJ160', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+SingleJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_j20_PhysicsTLA_L1jJ90_DETA20-jJ90J', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=EOFTLAPhIGroup+SingleJetGroup+Topo3Group, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_j20_PhysicsTLA_L1HT190-jJ40s5pETA21', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+SingleJetGroup+Topo3Group, monGroups=['tlaMon:shifter']),

        ChainProp(name='HLT_j70_j50a_j0_DJMASS1000j50dphi200x400deta_L1jMJJ-500-NFF', l1SeedThresholds=['FSNOSEED']*3,stream=['VBFDelayed'],groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),

        # Low-threshold calibration Large-R jets
        ChainProp(name='HLT_j110_a10sd_cssk_pf_jes_ftf_preselj80_L1jLJ80', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jLJ80']),
        ChainProp(name='HLT_j110_a10t_lcw_jes_L1jLJ80', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jLJ80']),
        ChainProp(name='HLT_j260_a10sd_cssk_pf_jes_ftf_preselj200_L1jLJ120', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jLJ120']),
        ChainProp(name='HLT_j260_a10t_lcw_jes_L1jLJ120', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jLJ120']),

        # ATR-24838 Large R L1J100 jet chains with jLJ L1 items (L1J100->L1jLJ140)
        ChainProp(name='HLT_j460_a10sd_cssk_pf_jes_ftf_preselj225_L1jLJ140', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:shifter', 'jetMon:online']),
        ChainProp(name='HLT_j460_a10t_lcw_jes_L1jLJ140', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j480_a10sd_cssk_pf_jes_ftf_preselj225_L1jLJ140', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j480_a10t_lcw_jes_L1jLJ140', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j460_a10r_L1jLJ140', l1SeedThresholds=['FSNOSEED'],  groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j460_a10_lcw_subjes_L1jLJ140', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j420_35smcINF_a10t_lcw_jes_L1jLJ140', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_2j330_35smcINF_a10t_lcw_jes_L1jLJ140', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j420_35smcINF_a10sd_cssk_pf_jes_ftf_preselj225_L1jLJ140', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_2j330_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1jLJ140', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:t0']),

        # Large R primary jet chains with gLJ L1 items (L1J100->L1gLJ140p0ETA25)
        ChainProp(name='HLT_j460_a10sd_cssk_pf_jes_ftf_preselj225_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:shifter', 'jetMon:online']),
        ChainProp(name='HLT_j460_a10t_lcw_jes_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j460_a10r_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'],  groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j460_a10_lcw_subjes_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j420_35smcINF_a10t_lcw_jes_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_2j330_35smcINF_a10t_lcw_jes_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j420_35smcINF_a10sd_cssk_pf_jes_ftf_preselj225_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_2j330_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:t0']),

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
        ChainProp(name='HLT_j175_a10sd_cssk_pf_jes_ftf_preselj140_L1gLJ100p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_gLJ100p0ETA25']),
        ChainProp(name='HLT_j175_a10t_lcw_jes_L1gLJ100p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_gLJ100p0ETA25']),

         # low threshold single jet support chains with JVT
        ChainProp(name='HLT_j25_020jvt_pf_ftf_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_j35_020jvt_pf_ftf_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_j45_020jvt_pf_ftf_preselj20_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),

        # ATR-22696 calratio primaries
        #ChainProp(name='HLT_j30_CLEANllp_calratio_L1LLP-NOMATCH', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryLegGroup+LegacyTopoGroup),
        #ChainProp(name='HLT_j30_CLEANllp_calratiormbib_L1LLP-NOMATCH', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryLegGroup+LegacyTopoGroup),
        # Phase I duplicates for primary calratio TAU
        #ChainProp(name='HLT_j30_CLEANllp_calratio_L1eTAU140', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        #ChainProp(name='HLT_j30_CLEANllp_calratiormbib_L1eTAU140', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        # calratio support  
        #ChainProp(name='HLT_j30_CLEANllp_calratio_L1eTAU60_EMPTY', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        #ChainProp(name='HLT_j30_CLEANllp_calratiormbib_L1eTAU60_EMPTY', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        #ChainProp(name='HLT_j30_CLEANllp_calratio_L1eTAU60_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        #ChainProp(name='HLT_j30_CLEANllp_calratiormbib_L1eTAU60_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),

    ]

    chains['Bjet'] += [
        # Legacy L1Calo primaries with gn1
        #  Single b-jet
        ChainProp(name="HLT_j210_0eta290_020jvt_bgn170_pf_ftf_preselj180_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup, monGroups=['bJetMon:online']),
        ChainProp(name="HLT_j280_0eta290_020jvt_bgn177_pf_ftf_preselj225_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup),
        ChainProp(name="HLT_j340_0eta290_020jvt_bgn185_pf_ftf_preselj225_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup),
        # Backup
        ChainProp(name="HLT_j210_0eta290_020jvt_bgn170_pf_ftf_preselj190_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup, monGroups=['bJetMon:online']),
        ChainProp(name="HLT_j210_0eta290_020jvt_bgn170_pf_ftf_preselj200_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup, monGroups=['bJetMon:online']),
        ChainProp(name="HLT_j210_0eta290_020jvt_bgn160_pf_ftf_preselj180_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup),
        ChainProp(name="HLT_j255_0eta290_020jvt_bgn170_pf_ftf_preselj225_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup),
        ChainProp(name="HLT_j280_0eta290_020jvt_bgn170_pf_ftf_preselj225_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup),
        ChainProp(name="HLT_j340_0eta290_020jvt_bgn177_pf_ftf_preselj225_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup),
        # Looser b-tagging
        ChainProp(name="HLT_j225_0eta290_020jvt_bgn177_pf_ftf_preselj180_L1J100", l1SeedThresholds=['FSNOSEED'], groups=SupportLegGroup+SingleBjetGroup),
        ChainProp(name='HLT_j275_0eta290_020jvt_bgn185_pf_ftf_preselj225_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SupportLegGroup+SingleBjetGroup),
        ChainProp(name='HLT_j300_0eta290_020jvt_bgn185_pf_ftf_preselj225_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SupportLegGroup+SingleBjetGroup),

        # Asymmetric 2b
        ChainProp(name="HLT_j165_0eta290_020jvt_bgn160_j55_0eta290_020jvt_bgn160_pf_ftf_preselj140b85XXj45b85_L1J100", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup),
        # Run 2 HH4b low-threshold chain

        # HT-seeded
        ChainProp(name='HLT_2j45_0eta290_020jvt_bgn170_j0_HT290_j0_DJMASS700j35_pf_ftf_L1HT150-J20s5pETA31_MJJ-400-CF', l1SeedThresholds=['FSNOSEED']*3, groups=PrimaryLegGroup+MultiBjetGroup+LegacyTopoGroup),

        # VBF chains
        ChainProp(name='HLT_j65a_j45a_2j35a_SHARED_2j35_0eta290_020jvt_bgn170_j0_DJMASS1000j50_pf_ftf_presela60XXa40XX2a25_L1MJJ-500-NFF', l1SeedThresholds=['FSNOSEED']*5,stream=['VBFDelayed'], groups=PrimaryLegGroup+MultiBjetGroup+LegacyTopoGroup),

        # HH4b primaries
        # Muon+jet legacy seeded, backup for L1Topo muon-in-jet
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b85_L1MU8F_2jJ40_jJ50', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiBjetGroup),
        # Support chain
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_boffperf_pf_ftf_presel2c20XX2c20b85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*4, stream=[PhysicsStream],  monGroups=['idMon:t0'], groups=SupportPhIGroup+MultiBjetGroup),

        # Delayed multijet+b
        ChainProp(name='HLT_5j35c_020jvt_j25c_020jvt_SHARED_j25c_020jvt_bgn160_pf_ftf_presel5c25XXc25b85_L14J15', l1SeedThresholds=['FSNOSEED']*3, stream=['VBFDelayed'], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name='HLT_5j45c_020jvt_j25c_020jvt_SHARED_j25c_020jvt_bgn160_pf_ftf_presel5c25XXc25b85_L14J15', l1SeedThresholds=['FSNOSEED']*3, stream=['VBFDelayed'], groups=PrimaryLegGroup+MultiBjetGroup),
        # Support chain
        ChainProp(name='HLT_5j35c_020jvt_j25c_020jvt_SHARED_j25c_020jvt_boffperf_pf_ftf_presel6c25_L14J15', l1SeedThresholds=['FSNOSEED']*3, stream=['VBFDelayed'], groups=SupportLegGroup+MultiBjetGroup+['RATE:CPS_4J15']),
        ChainProp(name='HLT_5j35c_020jvt_j25c_020jvt_SHARED_j25c_020jvt_boffperf_pf_ftf_presel6c25_L14jJ40', l1SeedThresholds=['FSNOSEED']*3, stream=['VBFDelayed'], groups=SupportPhIGroup+MultiBjetGroup),

        # Low-threshold support
        ChainProp(name='HLT_j30_0eta290_020jvt_boffperf_pf_ftf_L1J20', l1SeedThresholds=['FSNOSEED'], groups=SupportLegGroup+SingleBjetGroup+['RATE:CPS_J20'], monGroups=['bJetMon:t0','idMon:t0']),
        ChainProp(name='HLT_j45_0eta290_020jvt_boffperf_pf_ftf_L1J20', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SupportLegGroup+SingleBjetGroup+['RATE:CPS_J20'], monGroups=['bJetMon:shifter','idMon:shifter']),
        ChainProp(name='HLT_j60_0eta290_020jvt_boffperf_pf_ftf_L1J50', l1SeedThresholds=['FSNOSEED'], groups=SupportLegGroup+SingleBjetGroup+['RATE:CPS_J50'], monGroups=['bJetMon:t0','idMon:t0']),
        ChainProp(name='HLT_j80_0eta290_020jvt_boffperf_pf_ftf_L1J50', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SupportLegGroup+SingleBjetGroup+['RATE:CPS_J50'], monGroups=['bJetMon:t0','idMon:t0']),
        ChainProp(name='HLT_j100_0eta290_020jvt_boffperf_pf_ftf_preselj80_L1J50', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SupportLegGroup+SingleBjetGroup+['RATE:CPS_J50'], monGroups=['bJetMon:t0','idMon:t0']),
        ChainProp(name='HLT_j150_0eta290_020jvt_boffperf_pf_ftf_preselj120_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SupportLegGroup+SingleBjetGroup+['RATE:CPS_J100'], monGroups=['bJetMon:t0','idMon:t0']),
        ChainProp(name='HLT_j200_0eta290_020jvt_boffperf_pf_ftf_preselj140_L1J100', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SupportLegGroup+SingleBjetGroup+['RATE:CPS_J100'], monGroups=['bJetMon:shifter']),
        ChainProp(name='HLT_j300_0eta290_020jvt_boffperf_pf_ftf_preselj225_L1J100', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SupportLegGroup+SingleBjetGroup+['RATE:CPS_J100'], monGroups=['bJetMon:t0','idMon:shifter']),
        
        # Phase I inputs GN1
        #  Single b-jet
        ChainProp(name="HLT_j210_0eta290_020jvt_bgn170_pf_ftf_preselj180_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup, monGroups=['bJetMon:online']),
        ChainProp(name="HLT_j280_0eta290_020jvt_bgn177_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_j340_0eta290_020jvt_bgn185_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        # Backup
        ChainProp(name="HLT_j210_0eta290_020jvt_bgn170_pf_ftf_preselj190_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup, monGroups=['bJetMon:online']),
        ChainProp(name="HLT_j210_0eta290_020jvt_bgn170_pf_ftf_preselj200_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup, monGroups=['bJetMon:online']),
        ChainProp(name="HLT_j210_0eta290_020jvt_bgn160_pf_ftf_preselj180_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_j255_0eta290_020jvt_bgn170_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_j280_0eta290_020jvt_bgn170_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_j340_0eta290_020jvt_bgn177_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        # Looser b-tagging
        ChainProp(name="HLT_j225_0eta290_020jvt_bgn177_pf_ftf_preselj180_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+SingleBjetGroup),
        ChainProp(name='HLT_j275_0eta290_020jvt_bgn185_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+SingleBjetGroup),
        ChainProp(name='HLT_j300_0eta290_020jvt_bgn185_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+SingleBjetGroup),

        # Multi-b
        ChainProp(name="HLT_3j60_0eta290_020jvt_bgn177_pf_ftf_presel3j45b95_L13jJ70p0ETA23", l1SeedThresholds=['FSNOSEED'], groups=MultiBjetGroup + PrimaryPhIGroup),
        ChainProp(name="HLT_4j35_0eta290_020jvt_bgn177_pf_ftf_presel4j25b95_L14jJ40p0ETA25", l1SeedThresholds=['FSNOSEED'], groups=MultiBjetGroup + PrimaryPhIGroup),
        ChainProp(name="HLT_3j35_0eta290_020jvt_bgn170_j35_pf_ftf_presel2j25XX2j25b85_L14jJ40p0ETA25",      l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j35_0eta290_020jvt_bgn170_2j35_0eta290_020jvt_bgn185_pf_ftf_presel4j25b95_L14jJ40p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j50_0eta290_020jvt_bgn160_2j50_pf_ftf_presel2j25XX2j25b85_L14jJ40p0ETA25",        l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j35_0eta290_020jvt_bgn160_3j35_pf_ftf_presel3j25XX2j25b85_L15jJ40p0ETA25",  l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j45_0eta290_020jvt_bgn160_3j45_pf_ftf_presel3j25XX2j25b85_L15jJ40p0ETA25",  l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_j70_0eta290_020jvt_bgn160_3j70_pf_ftf_preselj50b85XX3j50_L14jJ50",           l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j45_0eta290_020jvt_bgn160_2j45_pf_ftf_presel2j25XX2j25b85_L14jJ40p0ETA25",  l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        # Asymmetric, 1j + 2b
        ChainProp(name="HLT_j140_2j50_0eta290_020jvt_bgn170_pf_ftf_preselj80XX2j45b90_L1jJ140_3jJ60", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        # Asymmetric 2b
        ChainProp(name="HLT_j165_0eta290_020jvt_bgn160_j55_0eta290_020jvt_bgn160_pf_ftf_preselj140b85XXj45b85_L1jJ160", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        # Run 2 HH4b low-threshold chain
        ChainProp(name="HLT_2j35c_020jvt_bgn160_2j35c_020jvt_pf_ftf_presel2j25XX2j25b85_L14jJ40p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),

        # HT-seeded
        ChainProp(name='HLT_2j45_0eta290_020jvt_bgn170_j0_HT290_j0_DJMASS700j35_pf_ftf_L1HT150-jJ50s5pETA32_jMJJ-400-CF', l1SeedThresholds=['FSNOSEED']*3, groups=PrimaryPhIGroup+MultiBjetGroup+Topo3Group),

        # VBF chains
        ChainProp(name='HLT_j75c_j55_j45f_SHARED_2j45_0eta290_020jvt_bgn160_pf_ftf_preselc60XXj45XXf40_L1jJ80p0ETA25_2jJ55_jJ50p30ETA49', l1SeedThresholds=['FSNOSEED']*4, groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_j75_0eta290_020jvt_bgn170_j55_0eta290_020jvt_bgn185_j45f_pf_ftf_preselj60XXj45XXf40_L1jJ80p0ETA25_2jJ55_jJ50p30ETA49", l1SeedThresholds=['FSNOSEED']*3,stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_j50_0eta290_020jvt_bgn170_2j45f_pf_ftf_preselj45XX2f40_L1jJ55p0ETA23_2jJ40p30ETA49",l1SeedThresholds=['FSNOSEED']*2,  stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j65a_j45a_2j35a_SHARED_2j35_0eta290_020jvt_bgn170_j0_DJMASS1000j50_pf_ftf_presela60XXa40XX2a25_L1jMJJ-500-NFF', l1SeedThresholds=['FSNOSEED']*5,stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiBjetGroup+Topo3Group),

        # HH4b primary triggers
        # 3b symmetric b-jet pt for Physics_Main
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn182_pf_ftf_presel2c20XX2c20b85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),
        # 2b symmetric b-jet pt for VBFDelayed
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], monGroups=['idMon:t0'], groups=PrimaryPhIGroup+MultiBjetGroup),
        # Candidates for allhad ttbar delayed stream
        ChainProp(name='HLT_5j35c_020jvt_j25c_020jvt_SHARED_j25c_020jvt_bgn160_pf_ftf_presel5c25XXc25b85_L14jJ40', l1SeedThresholds=['FSNOSEED']*3, stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_5j45c_020jvt_j25c_020jvt_SHARED_j25c_020jvt_bgn160_pf_ftf_presel5c25XXc25b85_L14jJ40', l1SeedThresholds=['FSNOSEED']*3, stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiBjetGroup),

        # support chains
        ChainProp(name='HLT_j20_0eta290_020jvt_boffperf_pf_ftf_L1jJ30', l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+SingleBjetGroup),
        ChainProp(name='HLT_j30_0eta290_020jvt_boffperf_pf_ftf_L1jJ50', l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+SingleBjetGroup, monGroups=['bJetMon:t0','idMon:t0']),
        ChainProp(name='HLT_j45_0eta290_020jvt_boffperf_pf_ftf_L1jJ50', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SupportPhIGroup+SingleBjetGroup, monGroups=['bJetMon:shifter','idMon:shifter']),
        ChainProp(name='HLT_j60_0eta290_020jvt_boffperf_pf_ftf_L1jJ90', l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+SingleBjetGroup, monGroups=['bJetMon:t0','idMon:t0']),
        ChainProp(name='HLT_j80_0eta290_020jvt_boffperf_pf_ftf_L1jJ90', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SupportPhIGroup+SingleBjetGroup, monGroups=['bJetMon:t0','idMon:t0']),
        ChainProp(name='HLT_j100_0eta290_020jvt_boffperf_pf_ftf_preselj80_L1jJ90', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SupportPhIGroup+SingleBjetGroup, monGroups=['bJetMon:t0','idMon:t0']),
        ChainProp(name='HLT_j150_0eta290_020jvt_boffperf_pf_ftf_preselj120_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+SingleBjetGroup, monGroups=['bJetMon:t0','idMon:t0']),
        ChainProp(name='HLT_j200_0eta290_020jvt_boffperf_pf_ftf_preselj140_L1jJ160', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SupportPhIGroup+SingleBjetGroup, monGroups=['bJetMon:shifter']),
        ChainProp(name='HLT_j300_0eta290_020jvt_boffperf_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SupportPhIGroup+SingleBjetGroup, monGroups=['bJetMon:t0','idMon:shifter']),

        ]

    chains['Tau'] += [
        #Primaries
        ChainProp(name="HLT_tau160_mediumRNN_tracktwoMVA_L1eTAU140", stream=[PhysicsStream,'express'], groups=PrimaryPhIGroup+SingleTauGroup, monGroups=['tauMon:online','tauMon:t0']),
        ChainProp(name='HLT_tau200_mediumRNN_tracktwoMVA_L1eTAU140', groups=PrimaryPhIGroup+SingleTauGroup),

        # displaced tau (ATR-21754)
        ChainProp(name='HLT_tau180_mediumRNN_tracktwoLLP_L1eTAU140', stream=[PhysicsStream,'express'], groups=PrimaryPhIGroup+SingleTauGroup, monGroups=['tauMon:shifter']),
        ChainProp(name="HLT_tau200_mediumRNN_tracktwoLLP_L1eTAU140", groups=PrimaryPhIGroup+SingleTauGroup, monGroups=['tauMon:online','tauMon:t0']),
        ChainProp(name="HLT_tau180_tightRNN_tracktwoLLP_L1eTAU140", groups=PrimaryPhIGroup+SingleTauGroup),
        ChainProp(name="HLT_tau200_tightRNN_tracktwoLLP_L1eTAU140", groups=PrimaryPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),

        # ATR-21797
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_tau60_mediumRNN_tracktwoMVA_03dRAB_L1eTAU80_2eTAU60', l1SeedThresholds=['eTAU80', 'eTAU60'], groups=PrimaryPhIGroup+MultiTauGroup, monGroups=['tauMon:t0']),

        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_tau35_mediumRNN_tracktwoMVA_03dRAB30_L1eTAU80_2cTAU30M_DR-eTAU30eTAU20', stream=[PhysicsStream,'express'], l1SeedThresholds=['eTAU80','cTAU30M'], groups=PrimaryPhIGroup+MultiTauGroup+Topo2Group, monGroups=['tauMon:online','tauMon:shifter']), 

        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55', l1SeedThresholds=['cTAU30M','cTAU20M'], stream=[PhysicsStream,'express'], groups=PrimaryPhIGroup+MultiTauGroup+Topo2Group, monGroups=['tauMon:online','tauMon:shifter']), 
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30MeTAU20M-jJ55', l1SeedThresholds=['cTAU30M','cTAU20M'], groups=SupportPhIGroup+MultiTauGroup+Topo2Group, monGroups=['tauMon:t0']), # Backup item with dR between isolated eTAU

        ### New Boosted Di Tau chain
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB10_L1cTAU20M_DR-eTAU20eTAU12-jJ40', l1SeedThresholds=['cTAU20M','eTAU12'], groups=MultiTauGroup+SupportPhIGroup+Topo2Group),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_02dRAB10_L1cTAU20M_DR-eTAU20eTAU12-jJ40', l1SeedThresholds=['cTAU20M','eTAU12'], groups=MultiTauGroup+SupportPhIGroup+Topo2Group),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_02dRAB10_L1cTAU20M_DR-eTAU20eTAU12-jJ40', l1SeedThresholds=['cTAU20M','eTAU12'], groups=MultiTauGroup+SupportPhIGroup+Topo2Group),

        # 2tau+j support
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB_L1TAU20IM_2TAU12IM', l1SeedThresholds=['TAU20IM','TAU12IM'], groups=SupportLegGroup+MultiTauGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20', l1SeedThresholds=['cTAU30M','cTAU20M'], groups=SupportPhIGroup+MultiTauGroup+Topo2Group),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30MeTAU20M', l1SeedThresholds=['cTAU30M','cTAU20M'], groups=SupportPhIGroup+MultiTauGroup+Topo2Group),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB_L1cTAU30M_2cTAU20M', l1SeedThresholds=['cTAU30M','cTAU20M'], groups=SupportPhIGroup+MultiTauGroup),

        # ATR-22230
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L1TAU20IM_2TAU12IM_4J12p0ETA25', l1SeedThresholds=['TAU20IM','TAU12IM'], stream=['VBFDelayed'], groups=PrimaryLegGroup+MultiTauGroup),
        #ATR-27546
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L1TAU20IM_2TAU12IM', l1SeedThresholds=['TAU20IM','TAU12IM'], stream=['VBFDelayed'], groups=SupportLegGroup+MultiTauGroup),


        # ATR-27252
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L1cTAU30M_2cTAU20M_4jJ30p0ETA25', l1SeedThresholds=['cTAU30M','cTAU20M'], stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiTauGroup),
        # ATR-27415
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55', l1SeedThresholds=['cTAU30M','cTAU20M'], stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiTauGroup+Topo2Group),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20', l1SeedThresholds=['cTAU30M','cTAU20M'], stream=['VBFDelayed'], groups=SupportPhIGroup+MultiTauGroup+Topo2Group),

        
        # ATR-20450 
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB_L1TAU20IM_2TAU12IM_4J12p0ETA25', l1SeedThresholds=['TAU20IM','TAU12IM'], groups=PrimaryLegGroup+MultiTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB_L1cTAU30M_2cTAU20M_4jJ30p0ETA25', l1SeedThresholds=['cTAU30M','cTAU20M'], groups=PrimaryPhIGroup+MultiTauGroup, monGroups=['tauMon:t0']), 

        # ATR-28692
        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_tau35_mediumRNN_tracktwoMVA_03dRAB_L1cTAU35M_2cTAU30M_2jJ55_3jJ50', l1SeedThresholds=['cTAU35M','cTAU30M'], groups=PrimaryPhIGroup+MultiTauGroup, monGroups=['tauMon:t0']),
        
        
        # 2tau+j support
        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_tau35_mediumRNN_tracktwoMVA_03dRAB_L1eTAU35M_2eTAU30M', l1SeedThresholds=['eTAU35M','eTAU30M'], groups=SupportPhIGroup+MultiTauGroup),
        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_tau35_mediumRNN_tracktwoMVA_03dRAB_L1cTAU35M_2cTAU30M' , l1SeedThresholds=['cTAU35M','cTAU30M'], groups=SupportPhIGroup+MultiTauGroup),

        # displaced tau+X (ATR-21754)
        ChainProp(name="HLT_tau80_mediumRNN_tracktwoLLP_tau60_mediumRNN_tracktwoLLP_03dRAB_L1eTAU80_2eTAU60", l1SeedThresholds=['eTAU80','eTAU60'], groups=PrimaryPhIGroup+MultiTauGroup, monGroups=['tauMon:shifter'] ),
        ChainProp(name="HLT_tau80_mediumRNN_tracktwoLLP_tau60_tightRNN_tracktwoLLP_03dRAB_L1eTAU80_2eTAU60", l1SeedThresholds=['eTAU80','eTAU60'], groups=PrimaryPhIGroup+MultiTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau80_tightRNN_tracktwoLLP_tau60_tightRNN_tracktwoLLP_03dRAB_L1eTAU80_2eTAU60", l1SeedThresholds=['eTAU80','eTAU60'], groups=PrimaryPhIGroup+MultiTauGroup,  monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau100_mediumRNN_tracktwoLLP_tau80_mediumRNN_tracktwoLLP_03dRAB_L1eTAU80_2eTAU60", l1SeedThresholds=['eTAU80','eTAU60'], groups=PrimaryPhIGroup+MultiTauGroup, monGroups=['tauMon:t0']),

        #------- 
        # Single tau support chains
        ChainProp(name="HLT_tau0_ptonly_L1TAU8", stream=[PhysicsStream,'express'], groups=SingleTauGroup+SupportLegGroup+['RATE:CPS_TAU8'], monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_L1TAU8', groups=SupportLegGroup+SingleTauGroup+['RATE:CPS_TAU8'], monGroups=['tauMon:online','tauMon:shifter']),
        ChainProp(name="HLT_tau20_idperf_tracktwoMVA_L1TAU8", stream=[PhysicsStream,'express'], groups=SingleTauGroup+SupportLegGroup+['RATE:CPS_TAU8'], monGroups=['tauMon:online','tauMon:shifter','idMon:shifter']), #ATR-27013
        ChainProp(name="HLT_tau20_perf_tracktwoMVA_L1TAU8", stream=[PhysicsStream,'express'], groups=SingleTauGroup+SupportLegGroup+['RATE:CPS_TAU8'], monGroups=['tauMon:online','tauMon:shifter']), #ATR-27013
        ChainProp(name="HLT_tau25_idperf_tracktwoMVA_L1TAU12IM", stream=[PhysicsStream], groups=SingleTauGroup+SupportLegGroup+['RATE:CPS_TAU12IM'], monGroups=['tauMon:online','tauMon:shifter','idMon:shifter']), #ATR-27013
        ChainProp(name="HLT_tau25_perf_tracktwoMVA_L1TAU12IM", stream=[PhysicsStream], groups=SingleTauGroup+SupportLegGroup+['RATE:CPS_TAU12IM'], monGroups=['tauMon:online','tauMon:shifter']), #ATR-27013
        ChainProp(name="HLT_tau25_mediumRNN_tracktwoMVA_L1TAU12IM", stream=[PhysicsStream,'express'], groups=SingleTauGroup+SupportLegGroup+['RATE:CPS_TAU12IM'], monGroups=['tauMon:online','tauMon:shifter', 'caloMon:t0']),
        ChainProp(name="HLT_tau25_mediumRNN_tracktwoLLP_L1TAU12IM", groups=SingleTauGroup+SupportLegGroup+['RATE:CPS_TAU12IM'], monGroups=['tauMon:online']),
        ChainProp(name="HLT_tau35_idperf_tracktwoMVA_L1TAU20IM", stream=[PhysicsStream,'express'], groups=SupportLegGroup+SingleTauGroup+['RATE:CPS_TAU20IM'], monGroups=['tauMon:t0','idMon:shifter']),
        ChainProp(name="HLT_tau35_perf_tracktwoMVA_L1TAU20IM", groups=SupportLegGroup+SingleTauGroup+['RATE:CPS_TAU20IM'], monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau35_mediumRNN_tracktwoMVA_L1TAU20IM", groups=SupportLegGroup+SingleTauGroup+['RATE:CPS_TAU20IM'], monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_L1TAU40', groups=SupportLegGroup+SingleTauGroup, monGroups=['tauMon:t0']),

        #------ Phase-I
        ChainProp(name="HLT_tau0_ptonly_L1eTAU12", stream=[PhysicsStream,'express'], groups=SingleTauGroup+SupportPhIGroup+['RATE:CPS_eTAU12'], monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau0_ptonly_L1eTAU80", stream=[PhysicsStream,'express'], groups=SingleTauGroup+SupportPhIGroup+['RATE:CPS_eTAU80'], monGroups=['tauMon:online','tauMon:shifter']),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_L1eTAU12', stream=[PhysicsStream, 'express'], groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_eTAU12'], monGroups=['tauMon:online','tauMon:shifter']),
        ChainProp(name="HLT_tau20_idperf_tracktwoMVA_L1eTAU12", stream=[PhysicsStream, 'express'], groups=SingleTauGroup+SupportPhIGroup+['RATE:CPS_eTAU12'], monGroups=['tauMon:online','tauMon:shifter','idMon:shifter']), #ATR-27013
        ChainProp(name="HLT_tau20_perf_tracktwoMVA_L1eTAU12", stream=[PhysicsStream, 'express'], groups=SingleTauGroup+SupportPhIGroup+['RATE:CPS_eTAU12'], monGroups=['tauMon:online','tauMon:shifter']), #ATR-27013
        ChainProp(name="HLT_tau25_idperf_tracktwoMVA_L1cTAU20M", stream=[PhysicsStream], groups=SingleTauGroup+SupportPhIGroup+['RATE:CPS_cTAU20M'], monGroups=['tauMon:online','tauMon:shifter','idMon:shifter']), #ATR-27013
        ChainProp(name="HLT_tau25_perf_tracktwoMVA_L1cTAU20M", stream=[PhysicsStream], groups=SingleTauGroup+SupportPhIGroup+['RATE:CPS_cTAU20M'], monGroups=['tauMon:online','tauMon:shifter']), #ATR-27013
        ChainProp(name="HLT_tau25_mediumRNN_tracktwoMVA_L1cTAU20M", stream=[PhysicsStream, 'express'], groups=SingleTauGroup+SupportPhIGroup+['RATE:CPS_cTAU20M'], monGroups=['tauMon:online','tauMon:shifter']),
        ChainProp(name="HLT_tau25_mediumRNN_tracktwoLLP_L1cTAU20M", groups=SingleTauGroup+SupportPhIGroup+['RATE:CPS_cTAU20M'], monGroups=['tauMon:online']),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_L1eTAU60', groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_L1eTAU80', groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_eTAU80'], monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau160_idperf_tracktwoMVA_L1eTAU140", stream=[PhysicsStream, 'express'], groups=SingleTauGroup+SupportPhIGroup, monGroups=['tauMon:online','tauMon:t0','idMon:t0']),
        ChainProp(name="HLT_tau160_perf_tracktwoMVA_L1eTAU140", stream=[PhysicsStream, 'express'], groups=SingleTauGroup+SupportPhIGroup, monGroups=['tauMon:online','tauMon:t0']),

        ChainProp(name="HLT_tau25_idperf_tracktwoMVA_L1eTAU20",   stream=[PhysicsStream], groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_eTAU20'], monGroups=['tauMon:online','tauMon:shifter']), #ATR-27013
        ChainProp(name="HLT_tau25_idperf_tracktwoMVA_L1eTAU20M",  stream=[PhysicsStream], groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_eTAU20M'], monGroups=['tauMon:online','tauMon:shifter']), #ATR-27013
        ChainProp(name="HLT_tau25_perf_tracktwoMVA_L1eTAU20",   stream=[PhysicsStream], groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_eTAU20'], monGroups=['tauMon:online','tauMon:shifter']), #ATR-27013
        ChainProp(name="HLT_tau25_perf_tracktwoMVA_L1eTAU20M",  stream=[PhysicsStream], groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_eTAU20M'], monGroups=['tauMon:online','tauMon:shifter']), #ATR-27013
        ChainProp(name="HLT_tau25_mediumRNN_tracktwoMVA_L1eTAU20",   stream=[PhysicsStream,'express'], groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_eTAU20'], monGroups=['tauMon:online','tauMon:shifter']),
        ChainProp(name="HLT_tau25_mediumRNN_tracktwoMVA_L1eTAU20M",  stream=[PhysicsStream,'express'], groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_eTAU20M'], monGroups=['tauMon:online','tauMon:shifter']),

        ChainProp(name="HLT_tau35_idperf_tracktwoMVA_L1cTAU30M", stream=[PhysicsStream, 'express'], groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_cTAU30M'], monGroups=['tauMon:t0','idMon:shifter']),
        ChainProp(name="HLT_tau35_perf_tracktwoMVA_L1cTAU30M", groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_cTAU30M'], monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau35_mediumRNN_tracktwoMVA_L1cTAU30M", groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_cTAU30M'], monGroups=['tauMon:t0']),

        # ATR-24367 (express stream for ID)
        ChainProp(name="HLT_tau80_idperf_tracktwoMVA_L1eTAU80", stream=[PhysicsStream,'express'], groups=SingleTauGroup+SupportPhIGroup+['RATE:CPS_eTAU80'], monGroups=['idMon:t0']),

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
        ChainProp(name='HLT_mu11_mu6_bUpsimumu_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu11_mu6_bBmumu_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu11_mu6_bDimu_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed','express'], groups=BphysicsGroup+PrimaryL1MuGroup, monGroups=['bphysMon:online','bphysMon:shifter']),
        ChainProp(name='HLT_mu11_mu6_bDimu_Lxy0_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu11_mu6_bDimu2700_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu11_mu6_bDimu2700_Lxy0_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu11_mu6_bPhi_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed','express'], groups=BphysicsGroup+PrimaryL1MuGroup, monGroups=['bphysMon:online','bphysMon:shifter']),
        ChainProp(name='HLT_mu11_mu6_bTau_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed','express'], groups=BphysicsGroup+PrimaryL1MuGroup, monGroups=['bphysMon:online','bphysMon:t0']),
        #-- mu11_mu6 chains with L1Topo
        ChainProp(name='HLT_mu11_mu6_bJpsimumu_L1LFV-MU8VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        ChainProp(name='HLT_mu11_mu6_bUpsimumu_L1LFV-MU8VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        ChainProp(name='HLT_mu11_mu6_bBmumu_L1LFV-MU8VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        ChainProp(name='HLT_mu11_mu6_bDimu_L1LFV-MU8VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        ChainProp(name='HLT_mu11_mu6_bDimu2700_L1LFV-MU8VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        ChainProp(name='HLT_mu11_mu6_bPhi_L1LFV-MU8VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        ChainProp(name='HLT_mu11_mu6_bTau_L1LFV-MU8VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup+Topo2Group),
        #-- 2mu6 chains
        ChainProp(name='HLT_2mu6_bJpsimumu_L12MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+SupportGroup+['RATE:CPS_2MU5VF']),
        ChainProp(name='HLT_2mu6_bJpsimumu_Lxy0_L12MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+SupportGroup+['RATE:CPS_2MU5VF']),
        ChainProp(name='HLT_2mu6_bBmumu_Lxy0_L12MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+SupportGroup+['RATE:CPS_2MU5VF']),
        ChainProp(name='HLT_2mu6_bUpsimumu_L12MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+SupportGroup+['RATE:CPS_2MU5VF']),
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
        ChainProp(name='HLT_2mu6_bDimu_L1LFV-MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_LFV-MU5VF']+Topo2Group),
        ChainProp(name='HLT_2mu6_bPhi_L1LFV-MU5VF', l1SeedThresholds=['MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_LFV-MU5VF']+Topo2Group),
        #-- mu6_mu4 chains
        ChainProp(name='HLT_mu6_mu4_bJpsimumu_L1MU5VF_2MU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3V']),
        ChainProp(name='HLT_mu6_mu4_bJpsimumu_Lxy0_L1MU5VF_2MU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3V']),
        ChainProp(name='HLT_mu6_mu4_bBmumu_L1MU5VF_2MU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3V']),
        ChainProp(name='HLT_mu6_mu4_bBmumu_Lxy0_L1MU5VF_2MU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3V']),
        ChainProp(name='HLT_mu6_mu4_bDimu_L1MU5VF_2MU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3V']),
        ChainProp(name='HLT_mu6_mu4_bUpsimumu_L1MU5VF_2MU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3V']),
        #-- mu6_mu4 chains, backup with MU3VF
        ChainProp(name='HLT_mu6_mu4_bJpsimumu_L1MU5VF_2MU3VF', l1SeedThresholds=['MU5VF','MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3VF']),
        ChainProp(name='HLT_mu6_mu4_bJpsimumu_Lxy0_L1MU5VF_2MU3VF', l1SeedThresholds=['MU5VF','MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3VF']),
        ChainProp(name='HLT_mu6_mu4_bBmumu_L1MU5VF_2MU3VF', l1SeedThresholds=['MU5VF','MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3VF']),
        ChainProp(name='HLT_mu6_mu4_bBmumu_Lxy0_L1MU5VF_2MU3VF', l1SeedThresholds=['MU5VF','MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3VF']),
        ChainProp(name='HLT_mu6_mu4_bDimu_L1MU5VF_2MU3VF', l1SeedThresholds=['MU5VF','MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU5VF_2MU3VF']),
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
        ChainProp(name='HLT_2mu4_bJpsimumu_L12MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3V'], monGroups=['bphysMon:online','bphysMon:val']),
        ChainProp(name='HLT_2mu4_bJpsimumu_Lxy0_L12MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3V']),
        ChainProp(name='HLT_2mu4_bBmumu_Lxy0_L12MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3V']),
        ChainProp(name='HLT_2mu4_bBmumu_L12MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3V']),
        ChainProp(name='HLT_2mu4_bDimu_L12MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3V'], monGroups=['bphysMon:online','bphysMon:val']),
        ChainProp(name='HLT_2mu4_bUpsimumu_L12MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3V']),
        #-- 2mu4 chains, backup with MU3VF
        ChainProp(name='HLT_2mu4_bJpsimumu_L12MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3VF']),
        ChainProp(name='HLT_2mu4_bJpsimumu_Lxy0_L12MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3VF']),
        ChainProp(name='HLT_2mu4_bBmumu_Lxy0_L12MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3VF']),
        ChainProp(name='HLT_2mu4_bBmumu_L12MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3VF']),
        ChainProp(name='HLT_2mu4_bDimu_L12MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3VF']),
        ChainProp(name='HLT_2mu4_bUpsimumu_L12MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_2MU3VF']),
        #-- 2mu4 chains with L1Topo
        ChainProp(name='HLT_2mu4_bJpsimumu_Lxy0_L1BPH-2M9-0DR15-2MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3V']+Topo2Group),
        ChainProp(name='HLT_2mu4_bBmumu_Lxy0_L1BPH-2M9-0DR15-2MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3V']+Topo2Group),
        ChainProp(name='HLT_2mu4_bBmumu_L1BPH-2M9-0DR15-2MU3V', l1SeedThresholds=['MU3V'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3V']+Topo2Group),
        # backup with MU3VF
        ChainProp(name='HLT_2mu4_bJpsimumu_Lxy0_L1BPH-2M9-0DR15-2MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3VF']+Topo2Group),
        ChainProp(name='HLT_2mu4_bBmumu_Lxy0_L1BPH-2M9-0DR15-2MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3VF']+Topo2Group),
        ChainProp(name='HLT_2mu4_bBmumu_L1BPH-2M9-0DR15-2MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3VF']+Topo2Group),

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
        ChainProp(name='HLT_2mu4_bBmumux_BpmumuKp_L1BPH-2M9-0DR15-2MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3VF']+Topo2Group),
        ChainProp(name='HLT_2mu4_bBmumux_BcmumuPi_L1BPH-2M9-0DR15-2MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3VF']+Topo2Group),
        ChainProp(name='HLT_2mu4_bBmumux_BsmumuPhi_L1BPH-2M9-0DR15-2MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3VF']+Topo2Group),
        ChainProp(name='HLT_2mu4_bBmumux_BdmumuKst_L1BPH-2M9-0DR15-2MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3VF']+Topo2Group),
        ChainProp(name='HLT_2mu4_bBmumux_LbPqKm_L1BPH-2M9-0DR15-2MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3VF']+Topo2Group),
        ChainProp(name='HLT_2mu4_bBmumux_BcmumuDsloose_L1BPH-2M9-0DR15-2MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3VF']+Topo2Group),
        ChainProp(name='HLT_2mu4_bBmumux_BcmumuDploose_L1BPH-2M9-0DR15-2MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3VF']+Topo2Group),
        ChainProp(name='HLT_2mu4_bBmumux_BcmumuD0Xloose_L1BPH-2M9-0DR15-2MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3VF']+Topo2Group),
        ChainProp(name='HLT_2mu4_bBmumux_BcmumuDstarloose_L1BPH-2M9-0DR15-2MU3VF', l1SeedThresholds=['MU3VF'], stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_BPH-2M9-0DR15-2MU3VF']+Topo2Group),

        #-- Bmux EOF triggers
        ChainProp(name='HLT_mu20_bBmux_BpmuD0X_L1MU14FCH', l1SeedThresholds=['MU14FCH'], stream=["BphysDelayed"], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU14FCH']),
        ChainProp(name='HLT_mu20_bBmux_BdmuDpX_L1MU14FCH', l1SeedThresholds=['MU14FCH'], stream=["BphysDelayed"], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU14FCH']),
        ChainProp(name='HLT_mu20_bBmux_BdmuDstarX_L1MU14FCH', l1SeedThresholds=['MU14FCH'], stream=["BphysDelayed"], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU14FCH']),
        ChainProp(name='HLT_mu20_bBmux_BsmuDsX_L1MU14FCH', l1SeedThresholds=['MU14FCH'], stream=["BphysDelayed"], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU14FCH']),
        ChainProp(name='HLT_mu20_bBmux_LbmuLcX_L1MU14FCH', l1SeedThresholds=['MU14FCH'], stream=["BphysDelayed"], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU14FCH']),
        # ATR-25512
        ChainProp(name='HLT_mu23_bBmux_BpmuD0X_L1MU18VFCH', l1SeedThresholds=['MU18VFCH'], stream=["BphysDelayed"], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU18VFCH']),
        ChainProp(name='HLT_mu23_bBmux_BdmuDpX_L1MU18VFCH', l1SeedThresholds=['MU18VFCH'], stream=["BphysDelayed"], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU18VFCH']),
        ChainProp(name='HLT_mu23_bBmux_BdmuDstarX_L1MU18VFCH', l1SeedThresholds=['MU18VFCH'], stream=["BphysDelayed"], groups=BphysicsGroup+EOFBPhysL1MuGroup+['RATE:CPS_MU18VFCH']),
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

        # AFP+dijet backup chains, discussed in ATR-24813
        # TODO: Kept PS:Online for consistency, but move to PS:NoBulkMCProd?
        ChainProp(name='HLT_2j120_mb_afprec_afpdijet_L1AFP_A_AND_C', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream], groups=MinBiasGroup+SupportGroup, monGroups=['mbMon:shifter']),
        ChainProp(name='HLT_2j175_mb_afprec_afpdijet_L1AFP_A_AND_C', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream], groups=MinBiasGroup+SupportGroup, monGroups=['mbMon:t0']),

        ChainProp(name='HLT_2j120_mb_afprec_afpdijet_L1J100', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],groups=MinBiasGroup+SupportLegGroup, monGroups=['mbMon:shifter']),
        ChainProp(name='HLT_2j175_mb_afprec_afpdijet_L1J100', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],groups=MinBiasGroup+SupportLegGroup, monGroups=['mbMon:t0']),
        #ATR-27257
        ChainProp(name='HLT_2j120_mb_afprec_afpdijet_L1jJ160', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:shifter']),
        ChainProp(name='HLT_2j175_mb_afprec_afpdijet_L1jJ160', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),

        ChainProp(name='HLT_2j120_mb_afprec_afpdijet_L1AFP_A_AND_C_TOF_jJ90', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:shifter']),
        ChainProp(name='HLT_2j175_mb_afprec_afpdijet_L1AFP_A_AND_C_TOF_jJ125', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],groups=MinBiasGroup+SupportPhIGroup, monGroups=['mbMon:t0']),

        # Primary e-mu chains
        ChainProp(name='HLT_e17_lhloose_mu14_L1eEM18L_MU8F', l1SeedThresholds=['eEM18L','MU8F'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_e7_lhmedium_mu24_L1MU14FCH',l1SeedThresholds=['EM3','MU14FCH'],  stream=[PhysicsStream], groups=PrimaryLegGroup+EgammaMuonGroup),
        # ATR-25512
        ChainProp(name='HLT_e7_lhmedium_mu24_L1MU18VFCH',l1SeedThresholds=['EM3','MU18VFCH'],  stream=[PhysicsStream], groups=PrimaryLegGroup+EgammaMuonGroup),
        #ChainProp(name='HLT_e12_lhloose_2mu10_L12MU8F', l1SeedThresholds=['EM8VH','MU8F'], stream=[PhysicsStream], groups=PrimaryLegGroup+EgammaMuonGroup), #ATR-27654
        ChainProp(name='HLT_e7_lhmedium_L1eEM5_mu24_L1MU14FCH',l1SeedThresholds=['eEM5','MU14FCH'],  stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),# moved from Dev to Phy
        # ATR-25512
        ChainProp(name='HLT_e7_lhmedium_L1eEM5_mu24_L1MU18VFCH',l1SeedThresholds=['eEM5','MU18VFCH'],  stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),# moved from Dev to Phy
        ChainProp(name='HLT_e12_lhloose_L1eEM10L_2mu10_L12MU8F', l1SeedThresholds=['eEM10L','MU8F'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),# moved from Dev to Phy
        ChainProp(name='HLT_2e12_lhloose_mu10_L12eEM10L_MU8F', l1SeedThresholds=['eEM10L','MU8F'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),

        # Primary g-mu chains
        ChainProp(name='HLT_g25_medium_mu24_L1MU14FCH',l1SeedThresholds=['EM15VH','MU14FCH'], stream=[PhysicsStream], groups=PrimaryLegGroup+EgammaMuonGroup), #ATR-22594
        ChainProp(name='HLT_g25_medium_L1eEM18L_mu24_L1MU14FCH',l1SeedThresholds=['eEM18L','MU14FCH'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup), #ATR-22594 moved from Dev to Phy
        # ATR-25512
        ChainProp(name='HLT_g25_medium_mu24_L1MU18VFCH',l1SeedThresholds=['EM15VH','MU18VFCH'], stream=[PhysicsStream], groups=PrimaryLegGroup+EgammaMuonGroup), #ATR-22594
        ChainProp(name='HLT_g25_medium_L1eEM18L_mu24_L1MU18VFCH',l1SeedThresholds=['eEM18L','MU18VFCH'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup), #ATR-22594 moved from Dev to Phy
        
        ChainProp(name='HLT_g35_loose_mu18_L1eEM28M', l1SeedThresholds=['eEM28M','MU8F'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_2g10_loose_mu20_L1MU14FCH', l1SeedThresholds=['EM7','MU14FCH'], stream=[PhysicsStream], groups=PrimaryLegGroup+EgammaMuonGroup), # unsure what EM seed should be
        ChainProp(name='HLT_2g10_loose_L1eEM9_mu20_L1MU14FCH', l1SeedThresholds=['eEM9','MU14FCH'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup), # unsure what eEM seed should be
        # ATR-25512
        ChainProp(name='HLT_2g10_loose_mu23_L1MU18VFCH', l1SeedThresholds=['EM7','MU18VFCH'], stream=[PhysicsStream], groups=PrimaryLegGroup+EgammaMuonGroup), # unsure what EM seed should be
        ChainProp(name='HLT_2g10_loose_L1eEM9_mu23_L1MU18VFCH', l1SeedThresholds=['eEM9','MU18VFCH'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup), # unsure what eEM seed should be
        
        #---------- support 2m + 1g + ZRad triggers
        ChainProp(name='HLT_2mu14_g20_tight_probe_L12MU8F', l1SeedThresholds=['MU8F','PROBEEM15VHI'],groups=TagAndProbeLegGroup+EgammaMuonGroup),
        ChainProp(name='HLT_2mu14_g22_tight_probe_L12MU8F', l1SeedThresholds=['MU8F','PROBEEM15VHI'],groups=TagAndProbeLegGroup+EgammaMuonGroup),
        ChainProp(name='HLT_2mu14_g25_medium_probe_L12MU8F', l1SeedThresholds=['MU8F','PROBEEM20VH'],groups=TagAndProbeLegGroup+EgammaMuonGroup),
        ChainProp(name='HLT_2mu14_g35_medium_probe_L12MU8F', l1SeedThresholds=['MU8F','PROBEEM20VH'],groups=TagAndProbeLegGroup+EgammaMuonGroup),
        ChainProp(name='HLT_2mu14_g50_loose_probe_L12MU8F', l1SeedThresholds=['MU8F','PROBEEM20VH'],groups=TagAndProbeLegGroup+EgammaMuonGroup),
        ChainProp(name='HLT_2mu14_g22_tight_probe_L1eEM18M_L12MU8F', l1SeedThresholds=['MU8F','PROBEeEM18M'],groups=TagAndProbePhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_2mu14_g25_medium_probe_L1eEM24L_L12MU8F', l1SeedThresholds=['MU8F','PROBEeEM24L'],groups=TagAndProbePhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_2mu14_g35_medium_probe_L1eEM24L_L12MU8F', l1SeedThresholds=['MU8F','PROBEeEM24L'],groups=TagAndProbePhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_2mu14_g50_loose_probe_L1eEM24L_L12MU8F', l1SeedThresholds=['MU8F','PROBEeEM24L'],groups=TagAndProbePhIGroup+EgammaMuonGroup),

        #LLP
        ChainProp(name='HLT_g15_loose_L1eEM10L_2mu10_msonly_L12MU8F', l1SeedThresholds=['eEM10L','MU8F'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),

        #ATR-23732 Displaced Jet Trigger
        ChainProp(name='HLT_j180_2dispjet50_3d2p_L1J100', groups=SingleJetGroup+UnconvTrkGroup+PrimaryLegGroup, l1SeedThresholds=['FSNOSEED']*2),
        ChainProp(name='HLT_j180_dispjet50_3d2p_dispjet50_1p_L1J100', groups=SingleJetGroup+UnconvTrkGroup+PrimaryLegGroup, l1SeedThresholds=['FSNOSEED']*3),
        #ATR-27257, Phase-I
        ChainProp(name='HLT_j180_2dispjet50_3d2p_L1jJ160', groups=SingleJetGroup+UnconvTrkGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED']*2),
        ChainProp(name='HLT_j180_dispjet50_3d2p_dispjet50_1p_L1jJ160', groups=SingleJetGroup+UnconvTrkGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED']*3),
        ChainProp(name='HLT_j180_dispjet90_x3d1p_L1jJ160', groups=SingleJetGroup+UnconvTrkGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED']*2),
        ChainProp(name='HLT_j180_dispjet100_x3d1p_L1jJ160', groups=SingleJetGroup+UnconvTrkGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED']*2),

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
        ChainProp(name='HLT_e9_lhvloose_mu20_mu8noL1_L1MU14FCH', l1SeedThresholds=['EM3','MU14FCH','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryLegGroup+EgammaMuonGroup),

        
        #ATR-27251, Phase-I
        ChainProp(name='HLT_e26_lhmedium_mu8noL1_L1eEM26M', l1SeedThresholds=['eEM26M','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_e28_lhmedium_mu8noL1_L1eEM28M', l1SeedThresholds=['eEM28M','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),

        # ATR-25512
        ChainProp(name='HLT_e9_lhvloose_mu23_mu8noL1_L1MU18VFCH', l1SeedThresholds=['EM3','MU18VFCH','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryLegGroup+EgammaMuonGroup),
        
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

        # tau + muon triggers
        ChainProp(name='HLT_mu20_ivarloose_tau20_mediumRNN_tracktwoMVA_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','TAU8'], stream=[PhysicsStream], groups=PrimaryLegGroup+MuonTauGroup),
        # ATR 25512
        ChainProp(name='HLT_mu23_ivarloose_tau20_mediumRNN_tracktwoMVA_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','TAU8'], stream=[PhysicsStream], groups=PrimaryLegGroup+MuonTauGroup),
        
        ChainProp(name='HLT_mu14_ivarloose_tau35_mediumRNN_tracktwoMVA_03dRAB_L1MU8F_eTAU20M', l1SeedThresholds=['MU8F','eTAU20M'], stream=[PhysicsStream], groups=PrimaryPhIGroup+MuonTauGroup),
        ChainProp(name='HLT_mu14_ivarloose_tau25_mediumRNN_tracktwoMVA_03dRAB_L1MU8F_eTAU20M_3jJ30', l1SeedThresholds=['MU8F','eTAU20M'], stream=[PhysicsStream], groups=PrimaryPhIGroup+MuonTauGroup),
        
        # ATR-27546
        ChainProp(name='HLT_mu26_ivarmedium_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU30M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU30M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU30M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU30M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU30M_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEcTAU30M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU30M_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEcTAU30M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),


        # tau + electron triggers
        ChainProp(name='HLT_e24_lhmedium_ivarloose_tau20_mediumRNN_tracktwoMVA_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','eTAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_e17_lhmedium_ivarloose_tau25_mediumRNN_tracktwoMVA_03dRAB_L1eEM18M_2eTAU20M_4jJ30', l1SeedThresholds=['eEM18M','eTAU20M'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        #ChainProp(name='HLT_e17_lhmedium_ivarloose_tau25_mediumRNN_tracktwoMVA_03dRAB_L1eEM18M_2eTAU20_4jJ30', l1SeedThresholds=['eEM18M','eTAU20'], stream=[PhysicsStream], groups=SupportPhIGroup+EgammaTauGroup), ERROR [checkL1HLTConsistency] chain HLT_e17_lhmedium_ivarloose_tau25_mediumRNN_tracktwoMVA_03dRAB_L1eEM18M_2eTAU20_4jJ30: L1item: L1_eEM18M_2eTAU20_4jJ30, not found in the items list of the L1Menu L1Menu_Dev_pp_run3_v1_22.0.58.json

        # ATR-27373
        ChainProp(name='HLT_e24_lhmedium_ivarloose_tau20_mediumRNN_tracktwoMVA_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','eTAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),

        # tau + met
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_xe50_cell_03dRAB_L1eTAU60_2eTAU20M_jXE80', l1SeedThresholds=['eTAU60','eTAU20M','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+TauMETGroup),  
        ChainProp(name='HLT_e17_lhmedium_tau25_mediumRNN_tracktwoMVA_xe50_cell_03dRAB_L1eEM18M_2eTAU20M_jXE70', l1SeedThresholds=['eEM18M','eTAU20M','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+TauMETGroup),
        ChainProp(name='HLT_mu14_tau25_mediumRNN_tracktwoMVA_xe50_cell_03dRAB_L1MU8F_eTAU20M_jXE70', l1SeedThresholds=['MU8F','eTAU20M','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+TauMETGroup),
        # T&P alignement-based tau chains (ATR-23150)

        # Legacy tau+X chains with muon L1
        ChainProp(name='HLT_mu26_ivarmedium_tau20_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBETAU8'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleMuonGroup,monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau20_mediumRNN_tracktwoMVA_probe_L1TAU12IM_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBETAU12IM'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleMuonGroup,monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau25_idperf_tracktwoMVA_probe_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBETAU12IM'], stream=[PhysicsStream,'express'], groups=TagAndProbeLegGroup+SingleMuonGroup, monGroups=['idMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau25_perf_tracktwoMVA_probe_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBETAU12IM'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau25_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBETAU12IM'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau30_mediumRNN_tracktwoMVA_probe_L1TAU20IM_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBETAU20IM'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleMuonGroup, monGroups=['tauMon:t0']),        
        ChainProp(name='HLT_mu26_ivarmedium_tau35_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBETAU20IM'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau40_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBETAU25IM'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau60_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBETAU40'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau60_mediumRNN_tracktwoLLP_probe_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBETAU40'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleMuonGroup),
        # ATR-25512
        ChainProp(name='HLT_mu26_ivarmedium_tau20_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBETAU8'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleMuonGroup,monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau25_idperf_tracktwoMVA_probe_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBETAU12IM'], stream=[PhysicsStream,'express'], groups=TagAndProbeLegGroup+SingleMuonGroup, monGroups=['idMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau25_perf_tracktwoMVA_probe_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBETAU12IM'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau25_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBETAU12IM'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau35_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBETAU20IM'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau40_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBETAU25IM'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau60_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBETAU40'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau60_mediumRNN_tracktwoLLP_probe_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBETAU40'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleMuonGroup),
        # ATR-25512
        ChainProp(name='HLT_mu24_ivarmedium_tau20_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBETAU8'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleMuonGroup,monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau25_idperf_tracktwoMVA_probe_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBETAU12IM'], stream=[PhysicsStream,'express'], groups=TagAndProbeLegGroup+SingleMuonGroup, monGroups=['idMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau25_perf_tracktwoMVA_probe_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBETAU12IM'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau25_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBETAU12IM'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau35_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBETAU20IM'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau40_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBETAU25IM'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau60_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBETAU40'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau60_mediumRNN_tracktwoLLP_probe_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBETAU40'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau20_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBETAU8'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleMuonGroup,monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau25_idperf_tracktwoMVA_probe_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBETAU12IM'], stream=[PhysicsStream,'express'], groups=TagAndProbeLegGroup+SingleMuonGroup, monGroups=['idMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau25_perf_tracktwoMVA_probe_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBETAU12IM'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau25_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBETAU12IM'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau35_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBETAU20IM'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau40_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBETAU25IM'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau60_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBETAU40'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau60_mediumRNN_tracktwoLLP_probe_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBETAU40'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleMuonGroup),


        # Phase-I tau+X chains with muon L1
        ChainProp(name='HLT_mu26_ivarmedium_tau20_mediumRNN_tracktwoMVA_probe_L1eTAU12_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU12'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU20M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau25_idperf_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU20M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['idMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau25_perf_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU20M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau25_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU20M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU20M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau35_mediumRNN_tracktwoMVA_probe_L1cTAU30M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU30M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau40_mediumRNN_tracktwoMVA_probe_L1cTAU35M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU35M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau60_mediumRNN_tracktwoMVA_probe_L1eTAU60_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU60'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau80_mediumRNN_tracktwoMVA_probe_L1eTAU80_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU80'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau160_mediumRNN_tracktwoMVA_probe_L1eTAU140_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU140'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau60_mediumRNN_tracktwoLLP_probe_L1eTAU60_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU60'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau80_mediumRNN_tracktwoLLP_probe_L1eTAU80_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU80'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau180_mediumRNN_tracktwoLLP_probe_L1eTAU140_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU140'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup),
        # ATR-25512
        ChainProp(name='HLT_mu26_ivarmedium_tau20_mediumRNN_tracktwoMVA_probe_L1eTAU12_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU12'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU20M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau25_idperf_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU20M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['idMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau25_perf_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU20M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau25_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU20M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU20M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau35_mediumRNN_tracktwoMVA_probe_L1cTAU30M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU30M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau40_mediumRNN_tracktwoMVA_probe_L1cTAU35M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU35M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau60_mediumRNN_tracktwoMVA_probe_L1eTAU60_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU60'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau80_mediumRNN_tracktwoMVA_probe_L1eTAU80_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU80'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau160_mediumRNN_tracktwoMVA_probe_L1eTAU140_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU140'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau60_mediumRNN_tracktwoLLP_probe_L1eTAU60_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU60'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau80_mediumRNN_tracktwoLLP_probe_L1eTAU80_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU80'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau180_mediumRNN_tracktwoLLP_probe_L1eTAU140_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU140'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup),
        # ATR-27343
        ChainProp(name='HLT_mu24_ivarmedium_tau20_mediumRNN_tracktwoMVA_probe_L1eTAU12_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU12'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau25_idperf_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU20M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['idMon:t0']),  
        ChainProp(name='HLT_mu24_ivarmedium_tau25_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU20M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau25_perf_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU20M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau35_mediumRNN_tracktwoMVA_probe_L1cTAU30M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU30M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau40_mediumRNN_tracktwoMVA_probe_L1cTAU35M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU35M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau60_mediumRNN_tracktwoMVA_probe_L1eTAU60_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU60'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau80_mediumRNN_tracktwoMVA_probe_L1eTAU80_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU80'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau160_mediumRNN_tracktwoMVA_probe_L1eTAU140_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU140'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau60_mediumRNN_tracktwoLLP_probe_L1eTAU60_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU60'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau80_mediumRNN_tracktwoLLP_probe_L1eTAU80_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU80'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau180_mediumRNN_tracktwoLLP_probe_L1eTAU140_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU140'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup),

        # ATR-25512
        ChainProp(name='HLT_mu24_ivarmedium_tau20_mediumRNN_tracktwoMVA_probe_L1eTAU12_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU12'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau25_idperf_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU20M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['idMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau25_perf_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU20M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau25_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU20M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau35_mediumRNN_tracktwoMVA_probe_L1cTAU30M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU30M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau40_mediumRNN_tracktwoMVA_probe_L1cTAU35M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU35M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau60_mediumRNN_tracktwoMVA_probe_L1eTAU60_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU60'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau80_mediumRNN_tracktwoMVA_probe_L1eTAU80_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU80'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau160_mediumRNN_tracktwoMVA_probe_L1eTAU140_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU140'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau60_mediumRNN_tracktwoLLP_probe_L1eTAU60_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU60'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau80_mediumRNN_tracktwoLLP_probe_L1eTAU80_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU80'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau180_mediumRNN_tracktwoLLP_probe_L1eTAU140_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU140'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup),

        # Phase-I tau+X chains with elec L1
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau20_mediumRNN_tracktwoMVA_probe_L1eTAU12_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeTAU12'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEcTAU20M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau25_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEcTAU20M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEcTAU20M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau35_mediumRNN_tracktwoMVA_probe_L1cTAU30M_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEcTAU30M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau40_mediumRNN_tracktwoMVA_probe_L1cTAU35M_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEcTAU35M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau60_mediumRNN_tracktwoMVA_probe_L1eTAU60_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeTAU60'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau80_mediumRNN_tracktwoMVA_probe_L1eTAU80_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeTAU80'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau160_mediumRNN_tracktwoMVA_probe_L1eTAU140_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeTAU140'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau60_mediumRNN_tracktwoLLP_probe_L1eTAU60_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeTAU60'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau80_mediumRNN_tracktwoLLP_probe_L1eTAU80_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeTAU80'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau180_mediumRNN_tracktwoLLP_probe_L1eTAU140_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeTAU140'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup), 

#
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau20_mediumRNN_tracktwoMVA_probe_L1eTAU12_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeTAU12'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEcTAU20M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau25_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEcTAU20M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEcTAU20M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau35_mediumRNN_tracktwoMVA_probe_L1cTAU30M_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEcTAU30M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau40_mediumRNN_tracktwoMVA_probe_L1cTAU35M_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEcTAU35M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau60_mediumRNN_tracktwoMVA_probe_L1eTAU60_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeTAU60'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau80_mediumRNN_tracktwoMVA_probe_L1eTAU80_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeTAU80'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau160_mediumRNN_tracktwoMVA_probe_L1eTAU140_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeTAU140'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau60_mediumRNN_tracktwoLLP_probe_L1eTAU60_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeTAU60'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau80_mediumRNN_tracktwoLLP_probe_L1eTAU80_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeTAU80'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau180_mediumRNN_tracktwoLLP_probe_L1eTAU140_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeTAU140'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),

        
        #tau + X intermediate Phase-I tag, Legacy probe 
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau20_mediumRNN_tracktwoMVA_probe_L1TAU8_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBETAU8'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau20_mediumRNN_tracktwoMVA_probe_L1TAU12IM_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBETAU12IM'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau25_mediumRNN_tracktwoMVA_probe_L1TAU12IM_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBETAU12IM'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau30_mediumRNN_tracktwoMVA_probe_L1TAU20IM_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBETAU20IM'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau35_mediumRNN_tracktwoMVA_probe_L1TAU20IM_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBETAU20IM'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau40_mediumRNN_tracktwoMVA_probe_L1TAU25IM_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBETAU25IM'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau60_mediumRNN_tracktwoMVA_probe_L1TAU40_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBETAU40'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau60_mediumRNN_tracktwoLLP_probe_L1TAU40_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBETAU40'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),
#
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau20_mediumRNN_tracktwoMVA_probe_L1TAU8_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBETAU8'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau20_mediumRNN_tracktwoMVA_probe_L1TAU12IM_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBETAU12IM'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau25_mediumRNN_tracktwoMVA_probe_L1TAU12IM_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBETAU12IM'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau30_mediumRNN_tracktwoMVA_probe_L1TAU20IM_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBETAU20IM'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau35_mediumRNN_tracktwoMVA_probe_L1TAU20IM_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBETAU20IM'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau40_mediumRNN_tracktwoMVA_probe_L1TAU25IM_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBETAU25IM'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau60_mediumRNN_tracktwoMVA_probe_L1TAU40_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBETAU40'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau60_mediumRNN_tracktwoLLP_probe_L1TAU40_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBETAU40'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup),


        

        # MET + tau tag and probe chains (ATR-23507)
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1XE50', l1SeedThresholds=['PROBETAU8','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1XE50', l1SeedThresholds=['PROBETAU12IM','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1XE50', l1SeedThresholds=['PROBETAU20IM','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1XE50', l1SeedThresholds=['PROBETAU25IM','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1XE50', l1SeedThresholds=['PROBETAU40','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoLLP_probe_xe65_cell_xe90_pfopufit_L1XE50', l1SeedThresholds=['PROBETAU40','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        # ATR-25512
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1XE50', l1SeedThresholds=['PROBETAU8','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1XE50', l1SeedThresholds=['PROBETAU12IM','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1XE50', l1SeedThresholds=['PROBETAU20IM','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1XE50', l1SeedThresholds=['PROBETAU25IM','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1XE50', l1SeedThresholds=['PROBETAU40','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoLLP_probe_xe65_cell_xe100_pfopufit_L1XE50', l1SeedThresholds=['PROBETAU40','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        # ATR-25512 another higher HLT threshold
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1XE50', l1SeedThresholds=['PROBETAU8','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1XE50', l1SeedThresholds=['PROBETAU12IM','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1XE50', l1SeedThresholds=['PROBETAU20IM','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1XE50', l1SeedThresholds=['PROBETAU25IM','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1XE50', l1SeedThresholds=['PROBETAU40','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoLLP_probe_xe75_cell_xe100_pfopufit_L1XE50', l1SeedThresholds=['PROBETAU40','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        # L1 backup
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1XE55', l1SeedThresholds=['PROBETAU8','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1XE55', l1SeedThresholds=['PROBETAU12IM','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1XE55', l1SeedThresholds=['PROBETAU20IM','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1XE55', l1SeedThresholds=['PROBETAU25IM','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1XE55', l1SeedThresholds=['PROBETAU40','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoLLP_probe_xe65_cell_xe90_pfopufit_L1XE55', l1SeedThresholds=['PROBETAU40','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        #
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1XE55', l1SeedThresholds=['PROBETAU8','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1XE55', l1SeedThresholds=['PROBETAU12IM','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1XE55', l1SeedThresholds=['PROBETAU20IM','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1XE55', l1SeedThresholds=['PROBETAU25IM','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1XE55', l1SeedThresholds=['PROBETAU40','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoLLP_probe_xe65_cell_xe100_pfopufit_L1XE55', l1SeedThresholds=['PROBETAU40','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        #
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1XE55', l1SeedThresholds=['PROBETAU8','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1XE55', l1SeedThresholds=['PROBETAU12IM','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1XE55', l1SeedThresholds=['PROBETAU20IM','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1XE55', l1SeedThresholds=['PROBETAU25IM','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1XE55', l1SeedThresholds=['PROBETAU40','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoLLP_probe_xe75_cell_xe100_pfopufit_L1XE55', l1SeedThresholds=['PROBETAU40','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),

        #
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU12','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU20M','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU30M','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU35M','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoLLP_probe_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoLLP_probe_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau180_mediumRNN_tracktwoLLP_probe_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        # ATR-25512
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU12','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU20M','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU30M','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU35M','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoLLP_probe_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoLLP_probe_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau180_mediumRNN_tracktwoLLP_probe_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        # ATR-25512 another higher HLT threshold
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU12','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU20M','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU30M','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU35M','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoLLP_probe_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoLLP_probe_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau180_mediumRNN_tracktwoLLP_probe_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoLLP_probe_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau180_mediumRNN_tracktwoLLP_probe_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoLLP_probe_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau180_mediumRNN_tracktwoLLP_probe_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoLLP_probe_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau180_mediumRNN_tracktwoLLP_probe_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),

        #ATR-28679 -jXE
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU12','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU20M','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU30M','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU35M','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoLLP_probe_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        #
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU12','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU20M','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU30M','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU35M','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoLLP_probe_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        #
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU12','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU20M','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU30M','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU35M','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoLLP_probe_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),

        # ATR-28679 gXE
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU12','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU20M','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU30M','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU35M','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoLLP_probe_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        #
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU12','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU20M','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU30M','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU35M','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoLLP_probe_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        #
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU12','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU20M','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU30M','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU35M','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoLLP_probe_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        #
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoLLP_probe_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau180_mediumRNN_tracktwoLLP_probe_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        #
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoLLP_probe_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau180_mediumRNN_tracktwoLLP_probe_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        #
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoLLP_probe_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau180_mediumRNN_tracktwoLLP_probe_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),

        # jet + tau tag and probe (ATR-24031)
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_L1RD0_FILLED', l1SeedThresholds=['PROBEeTAU12'], groups=TagAndProbeLegGroup+TauJetGroup+['RATE:CPS_RD0_FILLED']),
        # Commented for now because the corresponding jet trigger is out due to fluctuating counts
        #   ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_j15_pf_ftf_03dRAB_L1RD0_FILLED', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=TagAndProbeLegGroup+TauJetGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_j25_pf_ftf_03dRAB_L1RD0_FILLED', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=SupportLegGroup+TauJetGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_j35_pf_ftf_03dRAB_L1RD0_FILLED', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=SupportLegGroup+TauJetGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_j45_pf_ftf_preselj20_03dRAB_L1jJ40', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=SupportPhIGroup+TauJetGroup+['RATE:CPS_jJ40']),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_j60_pf_ftf_preselj50_03dRAB_L1jJ50', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=SupportPhIGroup+TauJetGroup+['RATE:CPS_jJ50']),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_j85_pf_ftf_preselj50_03dRAB_L1jJ50', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=SupportPhIGroup+TauJetGroup+['RATE:CPS_jJ50']),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_j110_pf_ftf_preselj80_03dRAB_L1jJ60', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=SupportPhIGroup+TauJetGroup+['RATE:CPS_jJ60']),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_j175_pf_ftf_preselj140_03dRAB_L1jJ90', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=SupportPhIGroup+TauJetGroup+['RATE:CPS_jJ90']),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_j260_pf_ftf_preselj200_03dRAB_L1jJ125', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=SupportPhIGroup+TauJetGroup+['RATE:CPS_jJ125']),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_j360_pf_ftf_preselj225_03dRAB_L1jJ160', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=SupportPhIGroup+TauJetGroup+['RATE:CPS_jJ160']),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_j420_pf_ftf_preselj225_03dRAB_L1jJ160', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauJetGroup),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_j440_pf_ftf_preselj225_03dRAB_L1jJ160', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauJetGroup),
        # Photon+tau T&P
        #ATR-27251, Phase-I
        ChainProp(name='HLT_g140_loose_tau20_mediumRNN_tracktwoMVA_probe_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBETAU8'], groups=TagAndProbePhIGroup+TauPhotonGroup),
        ChainProp(name='HLT_g140_loose_tau20_mediumRNN_tracktwoMVA_probe_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBETAU8'], groups=TagAndProbePhIGroup+TauPhotonGroup),

        # b-jet trigger calibration chains
        ChainProp(name='HLT_mu26_ivarmedium_2j20_0eta290_020jvt_boffperf_pf_ftf_L1MU14FCH', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=TagAndProbeGroup+SingleMuonGroup, monGroups=['muonMon:online','bJetMon:online']),
        ChainProp(name='HLT_mu24_ivarmedium_2j20_0eta290_020jvt_boffperf_pf_ftf_L1MU14FCH', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=TagAndProbeGroup+SingleMuonGroup, monGroups=['muonMon:online','bJetMon:online']),
        # ATR-25932
        ChainProp(name='HLT_mu26_ivarmedium_2j20_roiftf_preselj20_L1MU14FCH', l1SeedThresholds=['MU14FCH','FSNOSEED'], stream=[PhysicsStream], groups=TagAndProbeGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_2j20_roiftf_preselj20_L1MU14FCH', l1SeedThresholds=['MU14FCH','FSNOSEED'], stream=[PhysicsStream], groups=TagAndProbeGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_2j20_roiftf_preselj20_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','FSNOSEED'], stream=[PhysicsStream], groups=TagAndProbeGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_2j20_roiftf_preselj20_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','FSNOSEED'], stream=[PhysicsStream], groups=TagAndProbeGroup+SingleMuonGroup),
     
        

        # ATR 25512
        ChainProp(name='HLT_mu24_ivarmedium_2j20_0eta290_020jvt_boffperf_pf_ftf_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','FSNOSEED'], groups=TagAndProbeGroup+SingleMuonGroup, monGroups=['muonMon:online','bJetMon:online']),
        ChainProp(name='HLT_mu26_ivarmedium_2j20_0eta290_020jvt_boffperf_pf_ftf_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','FSNOSEED'], groups=TagAndProbeGroup+SingleMuonGroup, monGroups=['muonMon:online','bJetMon:online']),

        #ATR-27251, Phase-I
        ChainProp(name='HLT_e26_lhtight_ivarloose_mu22noL1_2j20_0eta290_020jvt_bgn185_pf_ftf_L1eEM26M', l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'], stream=[PhysicsStream,'express'], groups=TagAndProbePhIGroup+EgammaBjetGroup, monGroups=['bJetMon:shifter','bJetMon:online']),
        ChainProp(name='HLT_e28_lhtight_ivarloose_mu22noL1_2j20_0eta290_020jvt_bgn185_pf_ftf_L1eEM28M', l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'], stream=[PhysicsStream,'express'], groups=TagAndProbePhIGroup+EgammaBjetGroup, monGroups=['bJetMon:shifter','bJetMon:online']),


        # ATR-24698: muon + bjet chains for calibrations
        ChainProp(name='HLT_mu4_j20_0eta290_020jvt_boffperf_pf_ftf_dRAB04_L1MU3V'    ,   l1SeedThresholds=['MU3V' ,'FSNOSEED'], groups=SupportGroup   +MuonBjetGroup, monGroups=['bJetMon:t0','muonMon:online','bJetMon:online'], stream=[PhysicsStream,'express']),
        ChainProp(name='HLT_mu6_j100_0eta290_020jvt_boffperf_pf_ftf_dRAB04_L1MU5VF_jJ90', l1SeedThresholds=['MU5VF','FSNOSEED'], groups=SupportPhIGroup+MuonBjetGroup, monGroups=['bJetMon:t0','muonMon:online','bJetMon:online'], stream=[PhysicsStream,'express']),
        
        # ATR-28761 added some  muon + bjet Phase-1 chains
        ChainProp(name='HLT_mu4_j35_0eta290_020jvt_boffperf_pf_ftf_dRAB04_L1MU3V_jJ30',   l1SeedThresholds=['MU3V' ,'FSNOSEED'], groups=SupportPhIGroup+MuonBjetGroup, monGroups=['muonMon:online','bJetMon:online'], stream=[PhysicsStream,         ]),
        ChainProp(name='HLT_mu4_j45_0eta290_020jvt_boffperf_pf_ftf_dRAB04_L1MU3V_jJ30',   l1SeedThresholds=['MU3V' ,'FSNOSEED'], groups=SupportPhIGroup+MuonBjetGroup, monGroups=['bJetMon:shifter','muonMon:online','bJetMon:online'], stream=[PhysicsStream,'express']),
        ChainProp(name='HLT_mu6_j60_0eta290_020jvt_boffperf_pf_ftf_dRAB04_L1MU3V_jJ30',   l1SeedThresholds=['MU3V' ,'FSNOSEED'], groups=SupportPhIGroup+MuonBjetGroup, monGroups=['muonMon:online','bJetMon:online'], stream=[PhysicsStream,         ]),




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

        ChainProp(name='HLT_e26_lhtight_ivarloose_2j20_0eta290_020jvt_boffperf_pf_ftf_L1eEM26M', l1SeedThresholds=['eEM26M','FSNOSEED'], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['bJetMon:online']),
        ChainProp(name='HLT_e26_lhtight_ivarloose_mu22noL1_j20_0eta290_020jvt_boffperf_pf_ftf_L1eEM26M', l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'], stream=[PhysicsStream,'express'], groups=TagAndProbePhIGroup+EgammaBjetGroup, monGroups=['bJetMon:shifter','bJetMon:online']),

        # ATR-27373
        ChainProp(name='HLT_e28_lhtight_ivarloose_2j20_0eta290_020jvt_boffperf_pf_ftf_L1eEM28M', l1SeedThresholds=['eEM28M','FSNOSEED'], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['bJetMon:online']),
        ChainProp(name='HLT_e28_lhtight_ivarloose_mu22noL1_j20_0eta290_020jvt_boffperf_pf_ftf_L1eEM28M', l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'], stream=[PhysicsStream,'express'], groups=TagAndProbePhIGroup+EgammaBjetGroup, monGroups=['bJetMon:shifter','bJetMon:online']),

        #Isolated High pt Track Trigger
        #Primary
        ChainProp(name='HLT_xe80_tcpufit_isotrk120_medium_iaggrmedium_L1XE50', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream], groups=UnconvTrkGroup+PrimaryLegGroup),
        #Backup for Primary Triggers
        ChainProp(name='HLT_xe80_tcpufit_isotrk140_medium_iaggrmedium_L1XE50', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream], groups=UnconvTrkGroup+PrimaryLegGroup),
        #Support
        ChainProp(name='HLT_xe80_tcpufit_isotrk100_medium_iaggrmedium_L1XE50', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream], groups=UnconvTrkGroup+SupportLegGroup+['RATE:CPS_XE50']),
        ChainProp(name='HLT_xe80_tcpufit_isotrk120_medium_iaggrloose_L1XE50', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],  groups=UnconvTrkGroup+SupportLegGroup+['RATE:CPS_XE50']),
        # L1 backup
        ChainProp(name='HLT_xe80_tcpufit_isotrk120_medium_iaggrmedium_L1XE55', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream], groups=UnconvTrkGroup+PrimaryLegGroup),
        ChainProp(name='HLT_xe80_tcpufit_isotrk140_medium_iaggrmedium_L1XE55', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream], groups=UnconvTrkGroup+PrimaryLegGroup),
        ChainProp(name='HLT_xe80_tcpufit_isotrk100_medium_iaggrmedium_L1XE55', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream], groups=UnconvTrkGroup+SupportLegGroup+['RATE:CPS_XE55']),
        ChainProp(name='HLT_xe80_tcpufit_isotrk120_medium_iaggrloose_L1XE55', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],  groups=UnconvTrkGroup+SupportLegGroup+['RATE:CPS_XE55']),
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

        # electron + MET (ATR-22594)
        ChainProp(name='HLT_e70_lhloose_xe70_cell_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMETGroup),

        # ATR-27373
        ChainProp(name='HLT_e70_lhloose_xe70_cell_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMETGroup),
        #TAU + met ATR-23600
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_xe80_tcpufit_xe50_cell_L1XE50', l1SeedThresholds=['TAU25IM','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_xe80_pfopufit_xe50_cell_L1XE50', l1SeedThresholds=['TAU25IM','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryLegGroup+TauMETGroup),
        # L1 backup
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_xe80_tcpufit_xe50_cell_L1XE55', l1SeedThresholds=['TAU25IM','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_xe80_pfopufit_xe50_cell_L1XE55', l1SeedThresholds=['TAU25IM','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryLegGroup+TauMETGroup),

        #Phase-1 TAU + met 
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_xe80_tcpufit_xe50_cell_L1jXE100', l1SeedThresholds=['cTAU35M','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_xe80_pfopufit_xe50_cell_L1jXE100', l1SeedThresholds=['cTAU35M','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+TauMETGroup),

        #ATR-28679
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_xe80_tcpufit_xe50_cell_L1jXE110', l1SeedThresholds=['cTAU35M','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_xe80_pfopufit_xe50_cell_L1jXE110', l1SeedThresholds=['cTAU35M','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_xe80_tcpufit_xe50_cell_L1gXEJWOJ110', l1SeedThresholds=['cTAU35M','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_xe80_pfopufit_xe50_cell_L1gXEJWOJ110', l1SeedThresholds=['cTAU35M','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+TauMETGroup),

        ChainProp(name='HLT_j75_0eta290_020jvt_bgn160_pf_ftf_xe60_cell_L12jJ90_jXE80', l1SeedThresholds=['FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup),
        
        ChainProp(name='HLT_g25_medium_mu24_ivarmedium_L1MU14FCH', l1SeedThresholds=['EM22VHI','MU14FCH'], stream=[PhysicsStream], groups=PrimaryLegGroup+EgammaMuonGroup),
        # ATR-25512
        ChainProp(name='HLT_g25_medium_mu24_ivarmedium_L1MU18VFCH', l1SeedThresholds=['EM22VHI','MU18VFCH'], stream=[PhysicsStream], groups=PrimaryLegGroup+EgammaMuonGroup),


        # VBF triggers (ATR-22594)
        # Main stream VBF
        ChainProp(name='HLT_e10_lhmedium_ivarloose_j70_j50a_j0_DJMASS900j50_L1MJJ-500-NFF',l1SeedThresholds=['EM8VH','FSNOSEED','FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+EgammaJetGroup+LegacyTopoGroup), #ATR-27654
        ChainProp(name='HLT_mu10_ivarmedium_j70_j50a_j0_DJMASS900j50_L1MJJ-500-NFF',l1SeedThresholds=['MU8F','FSNOSEED','FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MuonJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_j70_j50a_j0_DJMASS900j50_L1MJJ-500-NFF',l1SeedThresholds=['TAU8','TAU8','FSNOSEED','FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+TauJetGroup+LegacyTopoGroup),
        # Delayed stream VBF
        ChainProp(name='HLT_2mu6_2j50a_j0_DJMASS900j50_L1MJJ-500-NFF',l1SeedThresholds=['MU5VF','FSNOSEED','FSNOSEED'],stream=['VBFDelayed'], groups=PrimaryLegGroup+MuonJetGroup+LegacyTopoGroup), # Formerly HLT_2mu6_2j50_0eta490_invm900j50                       
        ChainProp(name='HLT_e5_lhvloose_j70_j50a_j0_DJMASS1000j50_xe50_tcpufit_L1MJJ-500-NFF',l1SeedThresholds=['EM3','FSNOSEED','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['VBFDelayed'], groups=PrimaryLegGroup+EgammaJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_2e5_lhmedium_j70_j50a_j0_DJMASS900j50_L1MJJ-500-NFF',l1SeedThresholds=['EM3','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['VBFDelayed'], groups=PrimaryLegGroup+EgammaJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_g25_medium_4j35a_j0_DJMASS1000j35_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaJetGroup),
        # ATR-27373
        ChainProp(name='HLT_g25_medium_4j35a_j0_DJMASS1000j35_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaJetGroup),

        ChainProp(name='HLT_mu4_j70_j50a_j0_DJMASS1000j50_xe50_tcpufit_L1MJJ-500-NFF',l1SeedThresholds=['MU3V','FSNOSEED','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['VBFDelayed'], groups=PrimaryLegGroup+MuonJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j70_j50a_j0_DJMASS1000j50dphi240_xe90_tcpufit_xe50_cell_L1MJJ-500-NFF',l1SeedThresholds=['FSNOSEED']*5,stream=['VBFDelayed'], groups=PrimaryLegGroup+JetMETGroup+LegacyTopoGroup),
        #ATR-27251, Phase-I
        ChainProp(name='HLT_g35_medium_4j35a_j0_DJMASS1000j35_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaJetGroup),
        ChainProp(name='HLT_g35_tight_4j35a_j0_DJMASS1000j35_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaJetGroup),
        ChainProp(name='HLT_g35_medium_4j35a_j0_DJMASS1000j35_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaJetGroup),
        ChainProp(name='HLT_g35_tight_4j35a_j0_DJMASS1000j35_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaJetGroup),



        # Photon+VBF
        ChainProp(name='HLT_g20_tight_icaloloose_j35_0eta290_020jvt_bgn177_3j35a_j0_DJMASS500j35_pf_ftf_L1EM18VHI_MJJ-300',l1SeedThresholds=['EM18VHI','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryLegGroup+EgammaBjetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_g20_tight_j35_0eta290_020jvt_bgn177_3j35a_j0_DJMASS500j35_pf_ftf_L1EM18VHI_MJJ-300',l1SeedThresholds=['EM18VHI','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportLegGroup+EgammaBjetGroup+LegacyTopoGroup),
        

        # Muon-in-jet
        ChainProp(name='HLT_mu4_j20_0eta290_020jvt_boffperf_pf_ftf_dRAB03_L1MU3V_J15', l1SeedThresholds=['MU3V','FSNOSEED'], groups=SupportLegGroup+SingleBjetGroup),
        # ATR-26032
        ChainProp(name='HLT_mu10_j225_0eta290_020jvt_boffperf_pf_ftf_preselj180_dRAB04_L1J100', l1SeedThresholds=['MU8F','FSNOSEED'], stream=[PhysicsStream], groups=SupportLegGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu10_j300_0eta290_020jvt_boffperf_pf_ftf_preselj225_dRAB04_L1J100', l1SeedThresholds=['MU8F','FSNOSEED'], stream=[PhysicsStream], groups=SupportLegGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu10_j360_0eta290_020jvt_boffperf_pf_ftf_preselj225_dRAB04_L1J100', l1SeedThresholds=['MU8F','FSNOSEED'], stream=[PhysicsStream], groups=SupportLegGroup+SingleBjetGroup),
        #
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
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_j70_j50a_j0_DJMASS900j50_L1jMJJ-500-NFF',l1SeedThresholds=['eTAU12','eTAU12','FSNOSEED','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+TauJetGroup+Topo3Group),
        # Delayed stream
        ChainProp(name='HLT_2mu6_2j50a_j0_DJMASS900j50_L1jMJJ-500-NFF',l1SeedThresholds=['MU5VF','FSNOSEED','FSNOSEED'],stream=['VBFDelayed'], groups=PrimaryPhIGroup+MuonJetGroup+Topo3Group), # Formerly HLT_2mu6_2j50_0eta490_invm900j50                       
        ChainProp(name='HLT_e5_lhvloose_j70_j50a_j0_DJMASS1000j50_xe50_tcpufit_L1jMJJ-500-NFF',l1SeedThresholds=['eEM5','FSNOSEED','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['VBFDelayed'], groups=PrimaryPhIGroup+EgammaJetGroup+Topo3Group),
        ChainProp(name='HLT_2e5_lhmedium_j70_j50a_j0_DJMASS900j50_L1jMJJ-500-NFF',l1SeedThresholds=['eEM5','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['VBFDelayed'], groups=PrimaryPhIGroup+EgammaJetGroup+Topo3Group),
        ChainProp(name='HLT_mu4_j70_j50a_j0_DJMASS1000j50_xe50_tcpufit_L1jMJJ-500-NFF',l1SeedThresholds=['MU3V','FSNOSEED','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['VBFDelayed'], groups=PrimaryPhIGroup+MuonJetGroup+Topo3Group),
        ChainProp(name='HLT_j70_j50a_j0_DJMASS1000j50dphi240_xe90_tcpufit_xe50_cell_L1jMJJ-500-NFF',l1SeedThresholds=['FSNOSEED']*5,stream=['VBFDelayed'], groups=PrimaryPhIGroup+JetMETGroup+Topo3Group),

        # Photon+VBF
        ChainProp(name='HLT_g20_tight_icaloloose_j35_0eta290_020jvt_bgn177_3j35a_j0_DJMASS500j35_pf_ftf_L1eEM22M_jMJJ-300',l1SeedThresholds=['eEM22M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaBjetGroup+Topo2Group),


        # B-jet preselection
        #ATR-27251, Phase-I
        ChainProp(name='HLT_g25_tight_2j35_0eta290_020jvt_bgn177_2j35a_pf_ftf_presel2a20b90XX2a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g25_tight_j35_0eta290_020jvt_bgn177_3j35a_j0_DJMASS700j35_pf_ftf_presela20b85XX3a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g25_tight_2j35_0eta290_020jvt_bgn177_2j35a_pf_ftf_presel2a20b90XX2a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_g25_tight_j35_0eta290_020jvt_bgn177_3j35a_j0_DJMASS700j35_pf_ftf_presela20b85XX3a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),

        #ATR-27373
        ChainProp(name='HLT_g35_medium_2j35_0eta290_020jvt_bgn177_2j35a_pf_ftf_presel2a20b90XX2a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g35_medium_2j35_0eta290_020jvt_bgn177_2j35a_pf_ftf_presel2a20b90XX2a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_g35_medium_j35_0eta290_020jvt_bgn177_3j35a_j0_DJMASS700j35_pf_ftf_presela20b85XX3a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g35_medium_j35_0eta290_020jvt_bgn177_3j35a_j0_DJMASS700j35_pf_ftf_presela20b85XX3a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_g35_tight_icaloloose_2j35_0eta290_020jvt_bgn177_2j35a_pf_ftf_presel2a20b90XX2a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaBjetGroup),
        ChainProp(name='HLT_g35_tight_icaloloose_2j35_0eta290_020jvt_bgn177_2j35a_pf_ftf_presel2a20b90XX2a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaBjetGroup),
        ChainProp(name='HLT_g35_tight_icaloloose_j35_0eta290_020jvt_bgn177_3j35a_j0_DJMASS700j35_pf_ftf_presela20b85XX3a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaBjetGroup),
        ChainProp(name='HLT_g35_tight_icaloloose_j35_0eta290_020jvt_bgn177_3j35a_j0_DJMASS700j35_pf_ftf_presela20b85XX3a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaBjetGroup),

        # photon + VBF Hbb (ATR-23293) GN1
        # B-jet preselection
        ChainProp(name='HLT_g25_tight_icaloloose_2j35_0eta290_020jvt_bgn177_2j35a_pf_ftf_presel2a20b90XX2a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaBjetGroup),
        ChainProp(name='HLT_g25_tight_icaloloose_j35_0eta290_020jvt_bgn177_3j35a_j0_DJMASS700j35_pf_ftf_presela20b85XX3a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaBjetGroup),
        ChainProp(name='HLT_g25_medium_2j35_0eta290_020jvt_bgn177_2j35a_pf_ftf_presel2a20b90XX2a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g25_medium_j35_0eta290_020jvt_bgn177_3j35a_j0_DJMASS700j35_pf_ftf_presela20b85XX3a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g25_tight_icaloloose_2j35_0eta290_020jvt_bgn177_2j35a_pf_ftf_presel2a20b90XX2a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaBjetGroup),
        ChainProp(name='HLT_g25_tight_icaloloose_j35_0eta290_020jvt_bgn177_3j35a_j0_DJMASS700j35_pf_ftf_presela20b85XX3a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaBjetGroup),
        ChainProp(name='HLT_g25_medium_2j35_0eta290_020jvt_bgn177_2j35a_pf_ftf_presel2a20b90XX2a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_g25_medium_j35_0eta290_020jvt_bgn177_3j35a_j0_DJMASS700j35_pf_ftf_presela20b85XX3a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_g35_tight_2j35_0eta290_020jvt_bgn177_2j35a_pf_ftf_presel2a20b90XX2a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g35_tight_j35_0eta290_020jvt_bgn177_3j35a_j0_DJMASS700j35_pf_ftf_presela20b85XX3a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g35_tight_2j35_0eta290_020jvt_bgn177_2j35a_pf_ftf_presel2a20b90XX2a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_g35_tight_j35_0eta290_020jvt_bgn177_3j35a_j0_DJMASS700j35_pf_ftf_presela20b85XX3a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),



        # LLP late stream
        ChainProp(name='HLT_j55c_xe50_cell_L1jJ60_EMPTY', l1SeedThresholds=['FSNOSEED']*2, stream=['Late'], groups=PrimaryPhIGroup+JetMETGroup),
        ChainProp(name='HLT_j55c_xe50_cell_L1jJ60_FIRSTEMPTY', l1SeedThresholds=['FSNOSEED']*2, stream=['Late'], groups=PrimaryPhIGroup+JetMETGroup),

        # Muon-in-jet
        ChainProp(name='HLT_mu4_j20_0eta290_020jvt_boffperf_pf_ftf_dRAB03_L1MU3V_jJ40', l1SeedThresholds=['MU3V','FSNOSEED'], groups=SingleBjetGroup+SupportPhIGroup),

        # ATR-20505
        ChainProp(name='HLT_g40_loose_mu40_msonly_L1MU14FCH', l1SeedThresholds=['EM20VH','MU14FCH'], stream=[PhysicsStream], groups=PrimaryLegGroup+EgammaMuonGroup),
        ChainProp(name='HLT_g40_loose_L1eEM24L_mu40_msonly_L1MU14FCH', l1SeedThresholds=['eEM24L','MU14FCH'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),  
        # ATR-25512
        ChainProp(name='HLT_g40_loose_mu40_msonly_L1MU18VFCH', l1SeedThresholds=['EM20VH','MU18VFCH'], stream=[PhysicsStream], groups=PrimaryLegGroup+EgammaMuonGroup),
        ChainProp(name='HLT_g40_loose_L1eEM24L_mu40_msonly_L1MU18VFCH', l1SeedThresholds=['eEM24L','MU18VFCH'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),  

        ## Unconventional tracking ATR-23797
        # hit-based DV
        ChainProp(name='HLT_xe80_tcpufit_hitdvjet200_tight_L1XE50', groups=PrimaryLegGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']*2),
        ChainProp(name='HLT_xe80_tcpufit_hitdvjet200_medium_L1XE50', groups=SupportLegGroup+UnconvTrkGroup+['RATE:CPS_XE50'], l1SeedThresholds=['FSNOSEED']*2),
        ChainProp(name='HLT_xe80_tcpufit_hitdvjet200_tight_L1XE55', groups=PrimaryLegGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']*2),
        ChainProp(name='HLT_xe80_tcpufit_hitdvjet200_medium_L1XE55', groups=SupportLegGroup+UnconvTrkGroup+['RATE:CPS_XE55'], l1SeedThresholds=['FSNOSEED']*2),
        ChainProp(name='HLT_xe80_tcpufit_hitdvjet200_tight_L1jXE110', groups=PrimaryPhIGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']*2), #ATR-28679
        ChainProp(name='HLT_xe80_tcpufit_hitdvjet200_medium_L1jXE110', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_jXE110'], l1SeedThresholds=['FSNOSEED']*2), #ATR-28679
        ChainProp(name='HLT_xe80_tcpufit_hitdvjet200_tight_L1gXEJWOJ110', groups=PrimaryPhIGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']*2), #ATR-28679
        ChainProp(name='HLT_xe80_tcpufit_hitdvjet200_medium_L1gXEJWOJ110', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_gXEJWOJ110'], l1SeedThresholds=['FSNOSEED']*2), #ATR-28679
        # jet preselection for hit DV
        ChainProp(name='HLT_j180_hitdvjet260_tight_L1J100', groups=PrimaryLegGroup+UnconvTrkGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j180_hitdvjet260_medium_L1J100', groups=SupportLegGroup+UnconvTrkGroup+['RATE:CPS_J100'], l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j180_hitdvjet200_medium_L1XE50', groups=SupportLegGroup+UnconvTrkGroup+['RATE:CPS_XE50'], l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j180_hitdvjet200_medium_L1XE55', groups=SupportLegGroup+UnconvTrkGroup+['RATE:CPS_XE55'], l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j180_hitdvjet200_medium_L1jXE110', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_jXE110'], l1SeedThresholds=2*['FSNOSEED']), #ATR-28679
        ChainProp(name='HLT_j180_hitdvjet200_medium_L1gXEJWOJ110', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_gXEJWOJ110'], l1SeedThresholds=2*['FSNOSEED']), #ATR-28679
        #ATR-27257, Phase-I
        ChainProp(name='HLT_j180_hitdvjet260_tight_L1jJ160', groups=PrimaryPhIGroup+UnconvTrkGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j180_hitdvjet260_medium_L1jJ160', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_jJ160'], l1SeedThresholds=2*['FSNOSEED']),


        # disappearing track trigger
        ChainProp(name='HLT_xe80_tcpufit_distrk20_tight_L1XE50',  groups=PrimaryLegGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']*2),
        ChainProp(name='HLT_xe80_tcpufit_distrk20_medium_L1XE50', groups=PrimaryLegGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']*2),
        ChainProp(name='HLT_xe80_tcpufit_distrk20_tight_L1XE55',  groups=PrimaryLegGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']*2),
        ChainProp(name='HLT_xe80_tcpufit_distrk20_medium_L1XE55', groups=PrimaryLegGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']*2),
        # dEdx triggers
        ChainProp(name='HLT_xe80_tcpufit_dedxtrk25_medium_L1XE50', groups=SupportLegGroup+UnconvTrkGroup+['RATE:CPS_XE50'], l1SeedThresholds=['FSNOSEED']*2),
        ChainProp(name='HLT_xe80_tcpufit_dedxtrk50_medium_L1XE50', groups=PrimaryLegGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']*2),
        ChainProp(name='HLT_xe80_tcpufit_dedxtrk25_medium_L1XE55', groups=SupportLegGroup+UnconvTrkGroup+['RATE:CPS_XE55'], l1SeedThresholds=['FSNOSEED']*2),
        ChainProp(name='HLT_xe80_tcpufit_dedxtrk50_medium_L1XE55', groups=PrimaryLegGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']*2),
        # Phase-I L1Calo
        ChainProp(name='HLT_xe80_tcpufit_hitdvjet200_tight_L1jXE100', groups=PrimaryPhIGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']*2),
        ChainProp(name='HLT_xe80_tcpufit_hitdvjet200_medium_L1jXE100', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_jXE100'], l1SeedThresholds=['FSNOSEED']*2),
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

        # Combined BPhys Bee chains (ATR-19285, ATR-22749) # Remove to save unique L1 rate (ATR-26912)
        #ChainProp(name='HLT_e9_lhvloose_e5_lhvloose_bBeeM6000_mu6_l2io_L1BPH-0M9-EM7-EM5_MU5VF', l1SeedThresholds=['EM7','EM3','MU5VF'], stream=['BphysDelayed'], groups=EOFBeeLegGroup+BphysElectronGroup+LegacyTopoGroup),
        #ChainProp(name='HLT_e9_lhvloose_e5_lhvloose_bBeeM6000_2mu4_l2io_L1BPH-0M9-EM7-EM5_2MU3V', l1SeedThresholds=['EM7','EM3','MU3V'], stream=['BphysDelayed'], groups=PrimaryLegGroup+BphysElectronGroup+LegacyTopoGroup),
        #ChainProp(name='HLT_e5_lhvloose_bBeeM6000_mu6_l2io_L1BPH-0DR3-EM7J15_MU5VF', l1SeedThresholds=['EM7','MU5VF'], stream=['BphysDelayed'], groups=EOFBeeLegGroup+BphysElectronGroup+LegacyTopoGroup),
        #ChainProp(name='HLT_e5_lhvloose_bBeeM6000_2mu4_l2io_L1BPH-0DR3-EM7J15_2MU3V', l1SeedThresholds=['EM7','MU3V'], stream=['BphysDelayed'], groups=EOFBeeLegGroup+BphysElectronGroup+LegacyTopoGroup),


        # [ATR-25500] Switch to dl1d for bjet+met+met triggers, and now to GN1
        ChainProp(name="HLT_j95_0eta290_020jvt_bgn160_pf_ftf_xe50_cell_xe85_tcpufit_L1XE55", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryLegGroup+BjetMETGroup),
        ChainProp(name="HLT_2j45_0eta290_020jvt_bgn160_pf_ftf_xe50_cell_xe85_tcpufit_L12jJ40_jXE110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup),
        ChainProp(name="HLT_3j35_0eta290_020jvt_bgn160_pf_ftf_xe50_cell_xe70_tcpufit_L13jJ40p0ETA25_jXE80", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup),
        ChainProp(name="HLT_j95_0eta290_020jvt_bgn160_pf_ftf_xe50_cell_xe85_pfopufit_L1XE55", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryLegGroup+BjetMETGroup),
        ChainProp(name="HLT_2j45_0eta290_020jvt_bgn160_pf_ftf_xe50_cell_xe85_pfopufit_L12jJ40_jXE110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup),
        ChainProp(name="HLT_3j35_0eta290_020jvt_bgn160_pf_ftf_xe50_cell_xe70_pfopufit_L13jJ40p0ETA25_jXE80", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup),

        #ATR-28679
        ChainProp(name="HLT_j95_0eta290_020jvt_bgn160_pf_ftf_xe50_cell_xe85_tcpufit_L1jXE110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup), 
        ChainProp(name="HLT_j100_0eta290_020jvt_bgn160_pf_ftf_xe50_cell_xe85_tcpufit_L1jXE110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup),
        ChainProp(name="HLT_j95_0eta290_020jvt_bgn160_pf_ftf_xe50_cell_xe85_pfopufit_L1jXE110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup), 
        ChainProp(name="HLT_j100_0eta290_020jvt_bgn160_pf_ftf_xe50_cell_xe85_pfopufit_L1jXE110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup),

        ChainProp(name="HLT_j95_0eta290_020jvt_bgn160_pf_ftf_xe50_cell_xe85_tcpufit_L1gXEJWOJ110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup), 
        ChainProp(name="HLT_j100_0eta290_020jvt_bgn160_pf_ftf_xe50_cell_xe85_tcpufit_L1gXEJWOJ110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup),
        ChainProp(name="HLT_j95_0eta290_020jvt_bgn160_pf_ftf_xe50_cell_xe85_pfopufit_L1gXEJWOJ110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup), 
        ChainProp(name="HLT_j100_0eta290_020jvt_bgn160_pf_ftf_xe50_cell_xe85_pfopufit_L1gXEJWOJ110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup),
                
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
        ChainProp(name='HLT_g35_medium_tau25_dipion3_tracktwoMVA_60invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','TAU8'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),

        #ATR-27373
        ChainProp(name='HLT_g25_medium_tau25_dikaonmass_tracktwoMVA_50invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','eTAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g25_medium_tau25_kaonpi1_tracktwoMVA_50invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','eTAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g25_medium_tau25_kaonpi2_tracktwoMVA_50invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','eTAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g25_medium_tau25_singlepion_tracktwoMVA_50invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','eTAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g25_medium_tau25_dipion1_tracktwoMVA_50invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','eTAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g25_medium_tau25_dipion2_tracktwoMVA_50invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','eTAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_medium_tau25_dipion3_tracktwoMVA_60invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','TAU8'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),

        ## Higher threshold to mitigate high ROS access 
        ChainProp(name='HLT_g35_medium_tau25_dikaonmass_tracktwoMVA_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','TAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_medium_tau25_kaonpi1_tracktwoMVA_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','TAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_medium_tau25_kaonpi2_tracktwoMVA_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','TAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_medium_tau25_singlepion_tracktwoMVA_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','TAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_medium_tau25_dipion1_tracktwoMVA_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','TAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_medium_tau25_dipion2_tracktwoMVA_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','TAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_medium_tau25_dipion3_tracktwoMVA_L1TAU12_60invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','TAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_medium_tau25_dikaonmass_tracktwoMVA_50invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','TAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_medium_tau25_kaonpi1_tracktwoMVA_50invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','TAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_medium_tau25_kaonpi2_tracktwoMVA_50invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','TAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_medium_tau25_singlepion_tracktwoMVA_50invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','TAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_medium_tau25_dipion1_tracktwoMVA_50invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','TAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_medium_tau25_dipion2_tracktwoMVA_50invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','TAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_medium_tau25_dipion3_tracktwoMVA_L1TAU12_60invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','TAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),


        ## 
        ChainProp(name='HLT_g35_tight_tau25_dikaonmass_tracktwoMVA_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','TAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_tight_tau25_kaonpi1_tracktwoMVA_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','TAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_tight_tau25_kaonpi2_tracktwoMVA_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','TAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_tight_tau25_singlepion_tracktwoMVA_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','TAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_tight_tau25_dipion1_tracktwoMVA_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','TAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_tight_tau25_dipion2_tracktwoMVA_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','TAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_tight_tau25_dipion3_tracktwoMVA_60invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','TAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_tight_tau25_dikaonmass_tracktwoMVA_50invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','TAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_tight_tau25_kaonpi1_tracktwoMVA_50invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','TAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_tight_tau25_kaonpi2_tracktwoMVA_50invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','TAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_tight_tau25_singlepion_tracktwoMVA_50invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','TAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_tight_tau25_dipion1_tracktwoMVA_50invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','TAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_tight_tau25_dipion2_tracktwoMVA_50invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','TAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_tight_tau25_dipion3_tracktwoMVA_60invmAB_L1eEM28M', l1SeedThresholds=['eEM28M','TAU12'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),


    
        ChainProp(name='HLT_j180_2dispjet50_2p_L1J100', groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J100'], l1SeedThresholds=['FSNOSEED']*2),
        #ATR-27257
        ChainProp(name='HLT_j180_2dispjet50_2p_L1jJ160', groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ160'], l1SeedThresholds=['FSNOSEED']*2),

        # [ATR-26377] To understand the muvtx rates as compared to Run 2
        ChainProp(name='HLT_j30_mu3vtx_L12MU8F', l1SeedThresholds=['FSNOSEED','MU8F'], groups=PrimaryL1MuGroup+MultiMuonGroup),

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
    ]



    chains['Monitor'] = [
        ChainProp(name='HLT_noalg_CostMonDS_L1All',        l1SeedThresholds=['FSNOSEED'], stream=['CostMonitoring'], groups=['PS:NoBulkMCProd', 'Primary:CostAndRate', 'RATE:Monitoring', 'BW:Other']), # HLT_costmonitor
    ]


    chains['UnconventionalTracking'] = [

        # hit-based DV                                 
        ChainProp(name='HLT_hitdvjet260_tight_L1J100', groups=PrimaryLegGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_hitdvjet260_medium_L1J100', groups=SupportLegGroup+UnconvTrkGroup+['RATE:CPS_J100'], l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_hitdvjet200_medium_L1XE50', groups=SupportLegGroup+UnconvTrkGroup+['RATE:CPS_XE50'], l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_hitdvjet200_medium_L1XE55', groups=SupportLegGroup+UnconvTrkGroup+['RATE:CPS_XE55'], l1SeedThresholds=['FSNOSEED']),

        # Phase I L1Calo inputs
        # hit-based DV                                 
        ChainProp(name='HLT_hitdvjet260_tight_L1jJ160', groups=PrimaryPhIGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_hitdvjet260_medium_L1jJ160', groups=SupportPhIGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_hitdvjet200_medium_L1jXE100', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_jXE100'], l1SeedThresholds=['FSNOSEED']),
         ChainProp(name='HLT_hitdvjet200_medium_L1jXE110', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_jXE110'], l1SeedThresholds=['FSNOSEED']), #ATR-28679
        ChainProp(name='HLT_hitdvjet200_medium_L1gXEJWOJ110', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_gXEJWOJ110'], l1SeedThresholds=['FSNOSEED']), #ATR-28679

        # disappearing track trigger
        ChainProp(name='HLT_distrk20_tight_L1XE50', groups=SupportLegGroup+UnconvTrkGroup+['RATE:CPS_XE50'], l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_distrk20_medium_L1XE50', groups=SupportLegGroup+UnconvTrkGroup+['RATE:CPS_XE50'], l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_distrk20_tight_L1XE55', groups=SupportLegGroup+UnconvTrkGroup+['RATE:CPS_XE55'], l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_distrk20_medium_L1XE55', groups=SupportLegGroup+UnconvTrkGroup+['RATE:CPS_XE55'], l1SeedThresholds=['FSNOSEED']),
        # Phase-I L1Calo
        ChainProp(name='HLT_distrk20_tight_L1jXE100', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_jXE100'], l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_distrk20_medium_L1jXE100', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_jXE100'], l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_distrk20_tight_L1jXE110', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_jXE110'], l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_distrk20_medium_L1jXE110', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_jXE110'], l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_distrk20_tight_L1gXEJWOJ110', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_gXEJWOJ110'], l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_distrk20_medium_L1gXEJWOJ110', groups=SupportPhIGroup+UnconvTrkGroup+['RATE:CPS_gXEJWOJ110'], l1SeedThresholds=['FSNOSEED']),

    ]

    chains['Streaming'] = [
        # Streamers already active in MC for jet/MET monitoring
        ChainProp(name='HLT_noalg_L1J100',    l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+JetStreamersGroup+SupportLegGroup, monGroups=['jetMon:t0']),

        ChainProp(name='HLT_noalg_L1J400',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+PrimaryLegGroup+JetStreamersGroup+['BW:Other'], monGroups=['jetMon:t0']), # catch all high-Et
        ChainProp(name='HLT_noalg_L1XE300', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+PrimaryLegGroup+METStreamersGroup),
        ChainProp(name='HLT_noalg_L1XE30',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportLegGroup+METStreamersGroup),
        ChainProp(name='HLT_noalg_L1XE50',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=['PS:NoBulkMCProd']+SupportLegGroup+METStreamersGroup+['RATE:CPS_XE50'], monGroups=['metMon:t0']),
        ChainProp(name='HLT_noalg_L1XE55',  l1SeedThresholds=['FSNOSEED'], stream=['Main', 'express'], groups=['PS:NoBulkMCProd']+METStreamersGroup+SupportLegGroup+['RATE:CPS_XE55'], monGroups=['metMon:t0']),

        # Phase I jet inputs ATR-24411
        ChainProp(name='HLT_noalg_L1jJ500', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+PrimaryPhIGroup+JetStreamersGroup+['BW:Other']), # catch all high-Et

        # Phase I primary candidates
        ChainProp(name='HLT_noalg_L1jJ160',         l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_noalg_L1jLJ140',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_noalg_L1gJ400p0ETA25',         l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),
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
        ChainProp(name='HLT_noalg_L1MU3V',      l1SeedThresholds=['FSNOSEED'], stream=['Main','express'], groups=['PS:NoBulkMCProd']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU3VF',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU5VF',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU8F',      l1SeedThresholds=['FSNOSEED'], stream=['Main','express'], groups=['PS:NoBulkMCProd']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU8VF',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU8VFC',    l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU4BOM',    l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU10BO',    l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU10BOM',   l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU12BOM',   l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU14FCH',   l1SeedThresholds=['FSNOSEED'], stream=['Main','express'], groups=['PS:NoBulkMCProd']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU18VFCH',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SingleMuonGroup+SupportGroup),

        ChainProp(name='HLT_noalg_L1TAU8',    l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+TauStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TAU40',   l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+TauStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TAU12IM', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+TauStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TAU20IM', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+TauStreamersGroup+SupportLegGroup),

        ChainProp(name='HLT_noalg_L1J15',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+JetStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1J20',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+JetStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1J50',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+JetStreamersGroup+SupportLegGroup),

        #ChainProp(name='HLT_noalg_L1XE60',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+METStreamersGroup+SupportLegGroup), #ATR-28679

        #Phase-I
        ChainProp(name='HLT_noalg_L1eTAU12',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+TauPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eTAU20',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+TauPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jTAU20',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+TauPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jTAU30',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+TauPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jTAU30M',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+TauPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1cTAU20M',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+TauPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eTAU20M',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+TauPhaseIStreamersGroup),
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
        ChainProp(name='HLT_noalg_L1jJ40',          l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ40p30ETA49',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ50',          l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ50p30ETA49',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ60',          l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ60p30ETA49',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ90',          l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ90p30ETA49',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ125',         l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ125p30ETA49', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),

        ChainProp(name='HLT_noalg_L1jLJ80',         l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jLJ120',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),

        ChainProp(name='HLT_noalg_L1gJ20p0ETA25',          l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1gJ20p25ETA49',          l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1gJ20p0ETA25_EMPTY',          l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1gJ50p0ETA25',          l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1gJ100p0ETA25',         l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+JetPhaseIStreamersGroup, monGroups=['jetMon:online']),

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

        ChainProp(name='HLT_noalg_L1CEP-CjJ100', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup+Topo3Group),
        ChainProp(name='HLT_noalg_L1CEP-CjJ90', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup+Topo3Group),
        # TODO add once L1 items/thresholds are in place
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_TOF_T0T1_jJ90', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_TOF_T0T1_jJ125', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_TOF_jJ90', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_TOF_jJ125', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportPhIGroup),

        #Muon streamers for L1Topo validation
        ChainProp(name='HLT_noalg_L1BPH-2M9-0DR15-2MU3VF',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportGroup+MuonXStreamersGroup+Topo2Group),
        ChainProp(name='HLT_noalg_L1BPH-8M15-0DR22-2MU5VF',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportGroup+MuonXStreamersGroup+Topo2Group),
        ChainProp(name='HLT_noalg_L12MU5VF',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportGroup+MuonXStreamersGroup),
    ]

    # check all chains are classified as either primary, support or T&P chains
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
