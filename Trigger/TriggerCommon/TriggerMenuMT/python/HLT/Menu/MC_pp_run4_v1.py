# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# MC_pp_run4_v1.py menu for Phase-II developments 
#------------------------------------------------------------------------#

# This defines the input format of the chain and its properties with the defaults set
# always required are: name and groups
# ['name', 'groups', 'l1SeedThresholds'=[], 'stream'=["Main""], 'monGroups'=[]],

from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp 
from .SignatureDicts import ChainStore

import TriggerMenuMT.HLT.Menu.Physics_pp_run4_v1 as physics_menu 
from TriggerMenuMT.HLT.Menu.Physics_pp_run4_v1 import ( 
    #PhysicsStream,
    #ExpressStream,
    SingleMuonGroup,
    MultiMuonGroup,
    MultiElectronGroup,
    MultiPhotonGroup,
    SingleElectronGroup,
    SinglePhotonGroup,
    SingleTauGroup,
    MultiTauGroup,
    SingleJetGroup,
    MultiJetGroup,
    METGroup,
    JetMETGroup,
    EgammaMuonGroup,
    EgammaTauGroup,
    EgammaMETGroup,
    EgammaJetGroup,
    EgammaBjetGroup,
    #UnconvTrkGroup,
    SingleBjetGroup,
    MultiBjetGroup,
    BjetMETGroup,
    #PrimaryLegGroup,
    PrimaryPhIGroup,
    PrimaryL1MuGroup,
    SupportPhIGroup,
    SupportGroup,
    TagAndProbePhIGroup,
    Topo2Group,
    Topo3Group,
    BphysElectronGroup,
    BphysicsGroup,
    EOFBPhysL1MuGroup,
    TauMETGroup,
    MuonTauGroup,
    MuonJetGroup,
    TauJetGroup,
    TauBJetGroup
)

# For NGT-related studies
NGTGroup = ["NGT"]

def addMCSignatures(chains):
    from AthenaCommon.Logging import logging
    log = logging.getLogger( __name__ )
    log.info('[setupMenu] going to add the MC menu chains now')

    chainsMC = ChainStore()

    chainsMC['Muon'] = [
        # Single Muon Run-3 primaries
        ChainProp(name='HLT_mu24_ivarmedium_L1MU14FCH', groups=PrimaryL1MuGroup+SingleMuonGroup, monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu50_L1MU14FCH', groups=PrimaryL1MuGroup+SingleMuonGroup, monGroups=['muonMon:online','muonMon:shifter']),
        ChainProp(name='HLT_mu60_0eta105_msonly_L1MU14FCH', groups=PrimaryL1MuGroup+SingleMuonGroup, monGroups=['muonMon:shifter']),
        ChainProp(name='HLT_mu60_L1MU14FCH', groups=PrimaryL1MuGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu80_L1MU14FCH', groups=PrimaryL1MuGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu80_msonly_3layersEC_L1MU14FCH', groups=PrimaryL1MuGroup+SingleMuonGroup),

        # Multi muon Run-3 primaries
        ChainProp(name='HLT_2mu14_L12MU8F', groups=PrimaryL1MuGroup+MultiMuonGroup, monGroups=['muonMon:online','muonMon:shifter']),
        ChainProp(name='HLT_2mu10_l2mt_L1MU10BOM', groups=MultiMuonGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu22_mu8noL1_L1MU14FCH', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryL1MuGroup+MultiMuonGroup, monGroups=['muonMon:online','muonMon:shifter']),
        ChainProp(name='HLT_mu20_ivarmedium_mu8noL1_L1MU14FCH', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryL1MuGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_2mu4noL1_L1MU14FCH', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryL1MuGroup+MultiMuonGroup),
        ChainProp(name='HLT_3mu6_L13MU5VF', l1SeedThresholds=['MU5VF'], groups=PrimaryL1MuGroup+MultiMuonGroup, monGroups=['muonMon:online']),
        ChainProp(name='HLT_3mu6_msonly_L13MU5VF', l1SeedThresholds=['MU5VF'], groups=PrimaryL1MuGroup+MultiMuonGroup, monGroups=['muonMon:online']),
        ChainProp(name='HLT_4mu4_L14MU3V', l1SeedThresholds=['MU3V'], groups=PrimaryL1MuGroup+MultiMuonGroup, monGroups=['muonMon:online']),
        ChainProp(name='HLT_2mu50_msonly_L1MU14FCH', groups=PrimaryL1MuGroup+MultiMuonGroup),
        
        # Multi muon with mass cut Run-3 primaries
        ChainProp(name='HLT_mu10_ivarmedium_mu10_10invmAB70_L12MU8F', groups=PrimaryL1MuGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_ivarmedium_mu4noL1_10invmAB70_L1MU14FCH', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryL1MuGroup+MultiMuonGroup),

        # Muon LRT chain
        ChainProp(name='HLT_mu20_LRT_d0loose_L1MU14FCH', groups=PrimaryL1MuGroup+SingleMuonGroup, monGroups=['muonMon:online']),

        # Support
        ChainProp(name='HLT_mu6_idperf_L1MU5VF', groups=SupportGroup+SingleMuonGroup, monGroups=['idMon:t0']),
        ChainProp(name='HLT_mu20_LRT_idperf_L1MU14FCH', groups=SupportGroup+SingleMuonGroup, monGroups=['idMon:shifter']),
        ChainProp(name='HLT_mu24_idperf_L1MU14FCH', groups=SupportGroup+SingleMuonGroup, monGroups=['idMon:shifter']),
        ChainProp(name='HLT_mu26_ivarperf_L1MU14FCH', groups=SupportGroup+SingleMuonGroup, monGroups=['idMon:shifter']),
        ChainProp(name='HLT_mu4_mu4_idperf_1invmAB5_L12MU3VF', l1SeedThresholds=['MU3VF','MU3VF'], groups=MultiMuonGroup+SupportGroup, monGroups=['idMon:t0']),
        ChainProp(name='HLT_mu14_mu14_idtp_idZmumu_L12MU8F', l1SeedThresholds=['MU8F','MU8F'], groups=MultiMuonGroup+SupportGroup, monGroups=['idMon:shifter']),

        # Phase-II Single muon
        ChainProp(name='HLT_mu20_ivarmedium_L1MU12FCH', groups=PrimaryL1MuGroup+SingleMuonGroup, monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu18_ivarmedium_L1MU14FCH', l1SeedThresholds=['MU14FCH'], groups=PrimaryPhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu20_ivarmedium_L1MU14FCH', l1SeedThresholds=['MU14FCH'], groups=PrimaryPhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu22_ivarmedium_L1MU14FCH', l1SeedThresholds=['MU14FCH'], groups=PrimaryPhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_L1MU14FCH', l1SeedThresholds=['MU14FCH'], groups=PrimaryPhIGroup+SingleMuonGroup),

        # Phase-II Multi muon
        ChainProp(name='HLT_2mu12_L12MU5VF', groups=PrimaryL1MuGroup+MultiMuonGroup, monGroups=['muonMon:online','muonMon:shifter']),
        ChainProp(name='HLT_2mu10_L12MU8F', groups=PrimaryL1MuGroup+MultiMuonGroup),
        ChainProp(name='HLT_2mu12_L12MU8F', groups=PrimaryL1MuGroup+MultiMuonGroup),

        # Phase-II Support
        ChainProp(name='HLT_mu20_idperf_L1MU12FCH', groups=SupportGroup+SingleMuonGroup, monGroups=['idMon:shifter']),
        ChainProp(name='HLT_mu20_ivarperf_L1MU12FCH', groups=SupportGroup+SingleMuonGroup, monGroups=['idMon:shifter']),

        # HL-LHC TDR inspired chains
        ChainProp(name="HLT_mu3vtx_L12MU8F", groups=PrimaryPhIGroup+SingleTauGroup),
    ]

    chainsMC['Egamma'] = [
        # single electron Run-3 primaries
        ChainProp(name='HLT_e26_lhtight_ivarloose_L1eEM26M', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:online','egammaMon:shifter_tp','caloMon:t0']),
        ChainProp(name='HLT_e28_lhtight_ivarloose_L1eEM28M', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:online','egammaMon:shifter_tp','caloMon:t0']),
        ChainProp(name='HLT_e60_lhmedium_L1eEM26M', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:online','egammaMon:shifter_tp']),
        ChainProp(name='HLT_e140_lhloose_L1eEM26M', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:shifter_tp','caloMon:t0']),
        ChainProp(name='HLT_e300_etcut_L1eEM26M', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:shifter','caloMon:t0']),

        # electron LRT
        ChainProp(name='HLT_e30_lhloose_nopix_lrtmedium_L1eEM26M', groups=PrimaryPhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e35_lhloose_nopix_lrtmedium_L1eEM26M', groups=PrimaryPhIGroup+SingleElectronGroup),

        # electron ringer
        ChainProp(name='HLT_e140_lhloose_noringer_L1eEM26M', groups=SupportPhIGroup+SingleElectronGroup),

        # primary special
        ChainProp(name='HLT_e20_lhtight_ivarloose_L1ZAFB-25DPHI-eEM18M', l1SeedThresholds=['eEM18M'], groups=PrimaryPhIGroup+SingleElectronGroup+Topo3Group),

        # multi electron
        ChainProp(name='HLT_2e17_lhvloose_L12eEM18M', groups=PrimaryPhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_2e18_lhvloose_L12eEM18M', groups=PrimaryPhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_2e20_lhvloose_L12eEM18M', groups=PrimaryPhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_2e22_lhvloose_L12eEM18M', groups=PrimaryPhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_2e24_lhvloose_L12eEM18M', groups=PrimaryPhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_e26_lhtight_e14_etcut_probe_50invmAB130_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeEM9'], groups=PrimaryPhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_e24_lhvloose_2e12_lhvloose_L1eEM24L_3eEM12L',l1SeedThresholds=['eEM24L','eEM12L'], groups=PrimaryPhIGroup+MultiElectronGroup),

        # single photon
        ChainProp(name='HLT_g100_loose_L1eEM26M', groups=PrimaryPhIGroup+SinglePhotonGroup),
        ChainProp(name='HLT_g120_loose_L1eEM26M', groups=PrimaryPhIGroup+SinglePhotonGroup),
        ChainProp(name='HLT_g140_loose_L1eEM26M', groups=PrimaryPhIGroup+SinglePhotonGroup),
        ChainProp(name='HLT_g160_loose_L1eEM26M', groups=PrimaryPhIGroup+SinglePhotonGroup),
        ChainProp(name='HLT_g180_loose_L1eEM26M', groups=PrimaryPhIGroup+SinglePhotonGroup),
        ChainProp(name='HLT_g300_etcut_L1eEM26M', groups=PrimaryPhIGroup+SinglePhotonGroup),

        # multi photon
        ChainProp(name='HLT_2g20_tight_icaloloose_L12eEM18M', groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_2g22_tight_L12eEM18M', groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_g35_medium_g25_medium_L12eEM24L', l1SeedThresholds=['eEM24L','eEM24L'], groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_g45_medium_g20_medium_L1eEM40L_2eEM18L', l1SeedThresholds=['eEM40L', 'eEM18L'], groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_2g50_loose_L12eEM24L', groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_2g25_loose_g15_loose_L12eEM24L', l1SeedThresholds=['eEM24L','eEM12L'], groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_2g25_medium_L12eEM24L', l1SeedThresholds=['eEM24L'], groups=PrimaryPhIGroup+MultiPhotonGroup),
        # low-mass diphoton
        ChainProp(name='HLT_2g15_tight_25dphiAA_invmAA80_L1DPHI-M70-2eEM15M', l1SeedThresholds=['eEM15'], groups=PrimaryPhIGroup+MultiPhotonGroup+Topo2Group),

        # No PID e/gamma chains for Ringer NN studies # ATR-30937 
        ChainProp(name='HLT_e5_nopid_L1eEM5', l1SeedThresholds=['eEM5'], groups=SingleElectronGroup+SupportPhIGroup+['RATE:CPS_eEM5']+['PS:NoBulkMCProd']),
        ChainProp(name='HLT_g5_nopid_L1eEM5', l1SeedThresholds=['eEM5'], groups=SinglePhotonGroup+SupportPhIGroup+['RATE:CPS_eEM5']+['PS:NoBulkMCProd']),

        # T&P chains for displaced electrons
        ChainProp(name='HLT_e5_idperf_loose_lrtloose_probe_g25_medium_L1eEM24L',l1SeedThresholds=['PROBEeEM5','eEM24L'],groups=SupportPhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_e5_idperf_loose_lrtloose_probe_L1eEM26M',l1SeedThresholds=['eEM26M','PROBEeEM5'],groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_e30_lhloose_nopix_lrtmedium_probe_L1eEM26M',l1SeedThresholds=['eEM26M','PROBEeEM26M'],groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_e30_lhloose_nopix_probe_L1eEM26M',l1SeedThresholds=['eEM26M','PROBEeEM26M'],groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e30_lhloose_nopix_lrtmedium_probe_g25_medium_L1eEM24L',l1SeedThresholds=['PROBEeEM26M','eEM24L'],groups=SupportPhIGroup+SingleElectronGroup),

        # Electron + Photon triggers
        ChainProp(name='HLT_e24_lhmedium_g25_medium_02dRAB_L12eEM24L', l1SeedThresholds=['eEM24L','eEM24L'], groups=PrimaryPhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_e24_lhmedium_g12_loose_g12_loose_02dRAB_02dRAC_02dRBC_L1eEM24L_3eEM12L', l1SeedThresholds=['eEM24L','eEM12L','eEM12L'], groups=PrimaryPhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_e25_mergedtight_g35_medium_90invmAB_02dRAB_L12eEM24L', l1SeedThresholds=['eEM24L','eEM24L'], groups=PrimaryPhIGroup+MultiElectronGroup),

        # B->K*ee chains
        ChainProp(name='HLT_e5_lhvloose_e3_lhvloose_bBeeM6000_L1BKeePrimary', l1SeedThresholds=['eEM5','eEM5'], stream=['BphysDelayed'], groups=SupportPhIGroup+BphysElectronGroup, monGroups=['bphysMon:online','bphysMon:shifter']),
        ChainProp(name='HLT_e5_lhvloose_bBeeM6000_L1BKeePrimary', l1SeedThresholds=['eEM5'], stream=['BphysDelayed','express'], groups=SupportPhIGroup+BphysElectronGroup, monGroups=['bphysMon:online']),
        ChainProp(name='HLT_2e5_lhvloose_bBeeM6000_L1BKeePrimary', l1SeedThresholds=['eEM5'], stream=['BphysDelayed','express'], groups=SupportPhIGroup+BphysElectronGroup, monGroups=['bphysMon:online','bphysMon:shifter']),

        # Support
        ChainProp(name='HLT_e5_idperf_tight_L1eEM5', groups=SingleElectronGroup+SupportPhIGroup, monGroups=['idMon:t0']),
        ChainProp(name='HLT_e5_idperf_tight_nogsf_L1eEM5', groups=SingleElectronGroup+SupportPhIGroup),
        ChainProp(name='HLT_e26_idperf_loose_L1eEM26M', groups=SingleElectronGroup+SupportPhIGroup),
        ChainProp(name='HLT_2e17_idperf_loose_L12eEM18M', groups=MultiElectronGroup+SupportPhIGroup),
        ChainProp(name='HLT_2e17_idperf_loose_nogsf_L12eEM18M', groups=MultiElectronGroup+SupportPhIGroup),
        ChainProp(name='HLT_e30_idperf_loose_lrtloose_L1eEM26M', groups=SupportPhIGroup+SingleElectronGroup, monGroups=['idMon:shifter']),
        ChainProp(name='HLT_e26_idperf_tight_L1eEM26M', groups=SingleElectronGroup+SupportPhIGroup, monGroups=['idMon:shifter']),
        ChainProp(name='HLT_e28_idperf_loose_L1eEM28M', groups=SingleElectronGroup+SupportPhIGroup, monGroups=['idMon:shifter']),
        ChainProp(name='HLT_e26_lhtight_e14_idperf_tight_nogsf_probe_50invmAB130_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeEM9'], groups=PrimaryPhIGroup+MultiElectronGroup, monGroups=['idMon:t0']),
        ChainProp(name='HLT_e26_lhtight_e14_idperf_tight_probe_50invmAB130_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeEM9'], groups=PrimaryPhIGroup+MultiElectronGroup, monGroups=['idMon:shifter']),
        ChainProp(name='HLT_e20_idperf_loose_lrtloose_L1eEM18L', groups=SingleElectronGroup+SupportPhIGroup, monGroups=['idMon:t0']),

        # Phase-II Single egamma
        ChainProp(name='HLT_e20_lhtight_ivarloose_L1eEM20M', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:online','egammaMon:shifter_tp','caloMon:t0']),
        ChainProp(name='HLT_e22_lhtight_ivarloose_L1eEM22M', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:online','egammaMon:shifter_tp','caloMon:t0']),
        ChainProp(name='HLT_e24_lhtight_ivarloose_L1eEM24M', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:online','egammaMon:shifter_tp','caloMon:t0']),
        ChainProp(name='HLT_g110_loose_L1eEM22M', groups=PrimaryPhIGroup+SinglePhotonGroup),
        # Phase-II Multi egamma
        ChainProp(name='HLT_2e12_lhvloose_L12eEM10L', groups=PrimaryPhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_2g23_tight_icaloloose_L12eEM10L', groups=PrimaryPhIGroup+MultiPhotonGroup),
        # Phase-II Support
        ChainProp(name='HLT_e22_lhtight_ivarloose_L1eEM18M', groups=SupportPhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e22_lhtight_ivarloose_e5_idperf_loose_lrtloose_probe_L1eEM22M',l1SeedThresholds=['eEM22M','PROBEeEM5'],groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e22_idperf_loose_L1eEM22M', groups=SupportPhIGroup+SingleElectronGroup),

        # HL-LHC TDR inspired chains
        ChainProp(name='HLT_2g20_tight_icaloloose_L12eEM10L', groups=PrimaryPhIGroup+MultiPhotonGroup),
    ]

    chainsMC['Tau'] = [
        ChainProp(name='HLT_tau140_mediumRNN_tracktwoMVA_L1eTAU120', groups=PrimaryPhIGroup+SingleTauGroup),
        ChainProp(name="HLT_tau160_mediumRNN_tracktwoMVA_L1eTAU120", groups=PrimaryPhIGroup+SingleTauGroup, monGroups=['tauMon:online','tauMon:t0']),
        ChainProp(name='HLT_tau180_mediumRNN_tracktwoMVA_L1eTAU120', groups=PrimaryPhIGroup+SingleTauGroup),
        ChainProp(name='HLT_tau200_mediumRNN_tracktwoMVA_L1eTAU120', groups=PrimaryPhIGroup+SingleTauGroup),

        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55', l1SeedThresholds=['cTAU30M','cTAU20M'], groups=PrimaryPhIGroup+MultiTauGroup+Topo2Group, monGroups=['tauMon:online','tauMon:shifter']), 
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB_L1cTAU30M_2cTAU20M_4jJ30p0ETA25', l1SeedThresholds=['cTAU30M','cTAU20M'], groups=PrimaryPhIGroup+MultiTauGroup, monGroups=['tauMon:t0']), 
        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_tau35_mediumRNN_tracktwoMVA_03dRAB_L1cTAU35M_2cTAU30M_2jJ55_3jJ50', l1SeedThresholds=['cTAU35M','cTAU30M'], groups=PrimaryPhIGroup+MultiTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_tau35_mediumRNN_tracktwoMVA_03dRAB30_L1eTAU80_2cTAU30M_DR-eTAU30eTAU20', l1SeedThresholds=['eTAU80','cTAU30M'], groups=PrimaryPhIGroup+MultiTauGroup+Topo2Group, monGroups=['tauMon:online','tauMon:shifter']), 
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_tau60_mediumRNN_tracktwoMVA_03dRAB_L1eTAU80_2eTAU60', l1SeedThresholds=['eTAU80', 'eTAU60'], groups=PrimaryPhIGroup+MultiTauGroup, monGroups=['tauMon:t0']),

        # tau LLP
        ChainProp(name='HLT_tau180_mediumRNN_tracktwoLLP_L1eTAU120', groups=PrimaryPhIGroup+SingleTauGroup, monGroups=['tauMon:shifter']),
        ChainProp(name="HLT_tau200_mediumRNN_tracktwoLLP_L1eTAU120", groups=PrimaryPhIGroup+SingleTauGroup),
        ChainProp(name="HLT_tau80_mediumRNN_tracktwoLLP_tau60_mediumRNN_tracktwoLLP_03dRAB_L1eTAU80_2eTAU60", l1SeedThresholds=['eTAU80','eTAU60'], groups=PrimaryPhIGroup+MultiTauGroup, monGroups=['tauMon:shifter']),
        ChainProp(name="HLT_tau100_mediumRNN_tracktwoLLP_tau80_mediumRNN_tracktwoLLP_03dRAB_L1eTAU80_2eTAU60", l1SeedThresholds=['eTAU80','eTAU60'], groups=PrimaryPhIGroup+MultiTauGroup, monGroups=['tauMon:t0']),

        # Support
        ChainProp(name="HLT_tau20_idperf_tracktwoMVA_L1eTAU12", groups=SingleTauGroup+SupportPhIGroup, monGroups=['tauMon:online','tauMon:shifter','idMon:shifter']),
        ChainProp(name="HLT_tau25_idperf_tracktwoMVA_L1cTAU20M", groups=SingleTauGroup+SupportPhIGroup, monGroups=['tauMon:online','tauMon:shifter','idMon:shifter']), #ATR-27013

        # Phase-II di-tau
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M', l1SeedThresholds=['cTAU30M', 'cTAU20M'], groups=PrimaryPhIGroup+MultiTauGroup, monGroups=['tauMon:online','tauMon:shifter']),

        # HL-LHC TDR inspired chains
        ChainProp(name='HLT_tau140_mediumGNTau_L1eTAU120', groups=PrimaryPhIGroup+SingleTauGroup),
        ChainProp(name='HLT_tau35_mediumGNTau_tau25_mediumGNTau_03dRAB_L1eTAU28M_2eTAU20M', l1SeedThresholds=['eTAU28M', 'eTAU20M'], groups=PrimaryPhIGroup+MultiTauGroup),
    ]

    chainsMC['Jet'] = [
        ChainProp(name='HLT_j360_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j380_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j400_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:shifter','jetMon:online', 'caloMon:t0']),
        ChainProp(name='HLT_j420_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j440_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j450_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j460_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j480_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j500_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j520_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j220f_L1jJ125p30ETA49', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup, monGroups=['jetMon:shifter', 'jetMon:online', 'caloMon:t0']),

        ChainProp(name='HLT_j350_pf_ftf_L1jJ140',l1SeedThresholds=['FSNOSEED'],groups=PrimaryPhIGroup+SingleJetGroup+NGTGroup),
        ChainProp(name='HLT_j350_pf_ftf_L1jJ40',l1SeedThresholds=['FSNOSEED'],groups=PrimaryPhIGroup+SingleJetGroup+NGTGroup),
        ChainProp(name='HLT_j300_pf_ftf_L1jJ40',l1SeedThresholds=['FSNOSEED'],groups=PrimaryPhIGroup+SingleJetGroup+NGTGroup),
        ChainProp(name='HLT_j100_pf_ftf_L1jJ40',l1SeedThresholds=['FSNOSEED'],groups=PrimaryPhIGroup+SingleJetGroup+NGTGroup),
        ChainProp(name='HLT_j50_pf_ftf_L1jJ40',l1SeedThresholds=['FSNOSEED'],groups=PrimaryPhIGroup+SingleJetGroup+NGTGroup),

        ChainProp(name='HLT_j460_a10sd_cssk_pf_jes_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'],groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:shifter','jetMon:online']),
        ChainProp(name='HLT_j460_a10sd_cssk_pf_jes_ftf_preselj225_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup+Topo3Group),
        ChainProp(name='HLT_j460_a10t_lcw_jes_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_j460_a10t_lcw_jes_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup+Topo3Group),
        ChainProp(name='HLT_j460_a10r_L1jJ160', l1SeedThresholds=['FSNOSEED'],  groups=PrimaryPhIGroup+SingleJetGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_j460_a10r_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'],  groups=PrimaryPhIGroup+SingleJetGroup+Topo3Group),
        ChainProp(name='HLT_j460_a10_lcw_subjes_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_j460_a10_lcw_subjes_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup+Topo3Group),
        ChainProp(name='HLT_j420_35smcINF_a10sd_cssk_pf_jes_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j420_35smcINF_a10sd_cssk_pf_jes_ftf_preselj225_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup+Topo3Group, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_j420_35smcINF_a10t_lcw_jes_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup, monGroups=['jetMon:shifter']),
        ChainProp(name='HLT_j420_35smcINF_a10t_lcw_jes_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup+Topo3Group),

        ChainProp(name='HLT_j400_a10sd_cssk_pf_jes_ftf_L1jJ140',l1SeedThresholds=['FSNOSEED'],groups=SingleJetGroup+PrimaryPhIGroup+NGTGroup),
        ChainProp(name='HLT_j300_a10sd_cssk_pf_jes_ftf_L1jJ140',l1SeedThresholds=['FSNOSEED'],groups=SingleJetGroup+PrimaryPhIGroup+NGTGroup),
        ChainProp(name='HLT_j100_a10sd_cssk_pf_jes_ftf_L1jJ140',l1SeedThresholds=['FSNOSEED'],groups=SingleJetGroup+PrimaryPhIGroup+NGTGroup),
        ChainProp(name='HLT_j200_a10sd_cssk_pf_jes_ftf_L1jJ40',l1SeedThresholds=['FSNOSEED'],groups=SingleJetGroup+PrimaryPhIGroup+NGTGroup),
        ChainProp(name='HLT_j100_a10sd_cssk_pf_jes_ftf_L1jJ40',l1SeedThresholds=['FSNOSEED'],groups=SingleJetGroup+PrimaryPhIGroup+NGTGroup),

        # HT chains
        ChainProp(name='HLT_j0_HT940_pf_ftf_preselj180_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j0_HT940_pf_ftf_preselj190_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j0_HT940_pf_ftf_preselj200_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j0_HT940_pf_ftf_preselj180_L1HT190-jJ40s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup+Topo3Group, monGroups=['jetMon:online', 'jetMon:shifter']),
        ChainProp(name='HLT_j0_HT940_pf_ftf_preselcHT450_L1HT190-jJ40s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup+Topo3Group, monGroups=['jetMon:online', 'jetMon:shifter']), 
        
        # HT with JVT
        ChainProp(name='HLT_j0_HT1000XX0eta240XX020jvt_pf_ftf_presel4c40_L1HT190-jJ40s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+Topo3Group),

        # multi jets small R
        ChainProp(name='HLT_2j235c_j115c_pf_ftf_presel2j180XXj80_L1jJ160', l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup + PrimaryPhIGroup), 
        ChainProp(name='HLT_3j190_pf_ftf_presel3j150_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup), 
        ChainProp(name='HLT_4j110_pf_ftf_presel4j85_L13jJ70', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_5j65c_pf_ftf_presel5c50_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_5j70c_pf_ftf_presel5j50_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_5j80_pf_ftf_presel5j50_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_6j55c_pf_ftf_presel6j40_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_6j55c_pf_ftf_presel6c45_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_6j65_pf_ftf_presel6j40_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup), 
        ChainProp(name='HLT_7j45_pf_ftf_presel7j30_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_10j35_pf_ftf_presel7j30_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup), 

        # - Three jets
        ChainProp(name='HLT_3j160_pf_ftf_L1jJ140',l1SeedThresholds=['FSNOSEED'],groups=MultiJetGroup+PrimaryPhIGroup+NGTGroup),
        ChainProp(name='HLT_3j160_pf_ftf_L13jJ40',l1SeedThresholds=['FSNOSEED'],groups=MultiJetGroup+PrimaryPhIGroup+NGTGroup),
        ChainProp(name='HLT_3j160_pf_ftf_L1jJ40',l1SeedThresholds=['FSNOSEED'],groups=MultiJetGroup+PrimaryPhIGroup+NGTGroup),
        ChainProp(name='HLT_3j100_pf_ftf_L1jJ40',l1SeedThresholds=['FSNOSEED'],groups=MultiJetGroup+PrimaryPhIGroup+NGTGroup),
        # - Four jets
        ChainProp(name='HLT_4j100_pf_ftf_L13jJ70',l1SeedThresholds=['FSNOSEED'],groups=MultiJetGroup+PrimaryPhIGroup+NGTGroup),
        ChainProp(name='HLT_4j100_pf_ftf_L1jJ40',l1SeedThresholds=['FSNOSEED'],groups=MultiJetGroup+PrimaryPhIGroup+NGTGroup),
        ChainProp(name='HLT_4j50_pf_ftf_L1jJ40',l1SeedThresholds=['FSNOSEED'],groups=MultiJetGroup+PrimaryPhIGroup+NGTGroup),
        ChainProp(name='HLT_4j100_pf_ftf_L13jJ40',l1SeedThresholds=['FSNOSEED'],groups=MultiJetGroup+PrimaryPhIGroup+NGTGroup),
        # - Five jets
        ChainProp(name='HLT_5j50_pf_ftf_L14jJ40',l1SeedThresholds=['FSNOSEED'],groups=MultiJetGroup+PrimaryPhIGroup+NGTGroup),
        ChainProp(name='HLT_5j50_pf_ftf_L1jJ40',l1SeedThresholds=['FSNOSEED'],groups=MultiJetGroup+PrimaryPhIGroup+NGTGroup),
        # - Six jets
        ChainProp(name='HLT_6j40c_pf_ftf_L14jJ40',l1SeedThresholds=['FSNOSEED'],groups=MultiJetGroup+PrimaryPhIGroup),

        # multijet large R with mass cut
        ChainProp(name='HLT_2j330_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+PrimaryPhIGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_2j330_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+PrimaryPhIGroup+Topo3Group, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_2j330_35smcINF_a10t_lcw_jes_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_2j330_35smcINF_a10t_lcw_jes_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),
        ChainProp(name='HLT_j360_60smcINF_j360_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),
        ChainProp(name='HLT_j360_60smcINF_j360_a10t_lcw_jes_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),
        ChainProp(name='HLT_j370_35smcINF_j370_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),
        ChainProp(name='HLT_j370_35smcINF_j370_a10t_lcw_jes_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),

        # Large R primary jet chains with gLJ L1 items (L1J100->L1gLJ140p0ETA25)
        ChainProp(name='HLT_j460_a10sd_cssk_pf_jes_ftf_preselj225_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:shifter', 'jetMon:online']),
        ChainProp(name='HLT_j460_a10t_lcw_jes_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j460_a10r_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'],  groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j460_a10_lcw_subjes_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j420_35smcINF_a10t_lcw_jes_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_2j330_35smcINF_a10t_lcw_jes_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j420_35smcINF_a10sd_cssk_pf_jes_ftf_preselj225_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_2j330_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:t0']),

        # Inclusive VBF jet trigger
        ChainProp(name='HLT_j70_j50a_j0_DJMASS1000j50dphi200x400deta_L1jMJJ-500-NFF', l1SeedThresholds=['FSNOSEED']*3,stream=['VBFDelayed'],groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),
        ChainProp(name='HLT_j70_j50a_j0_DJMASS1000j50dphi200x400deta_pf_ftf_L1jMJJ-500-NFF', l1SeedThresholds=['FSNOSEED']*3,stream=['VBFDelayed'],groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),

        # Support
        ChainProp(name='HLT_j45_pf_ftf_preselj20_L1jJ40', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup, monGroups=['idMon:shifter','jetMon:t0','jetMon:online']),
        ChainProp(name="HLT_j20_roiftf_preselj20_L1RD0_FILLED", l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup, monGroups=['idMon:t0']),

        # Phase-II single small R jet
        ChainProp(name='HLT_j380_pf_ftf_preselj200_L1jJ140', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        # Phase-II HT
        ChainProp(name='HLT_j0_HT300_pf_ftf_preselj180_L1jJ140', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j0_HT300_pf_ftf_preselj140_L1HT190-jJ40s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup+Topo3Group, monGroups=['jetMon:online', 'jetMon:shifter']),
        # Phase-II multi small R jet
        ChainProp(name='HLT_4j90_pf_ftf_presel4j50_L13jJ40', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['jetMon:t0']),
        # Phase-II single large R jet
        ChainProp(name='HLT_j380_a10sd_cssk_pf_jes_ftf_preselj180_L1gLJ100p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup, monGroups=['jetMon:shifter', 'jetMon:online']),

        # HL-LHC TDR inspired chains
        ChainProp(name='HLT_j300_35smcINF_a10sd_cssk_pf_jes_ftf_preselj245_L1SC175-SCjJ10', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_4j90_pf_ftf_presel4j65_L13jJ70', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_j0_HT650XX0eta240_pf_ftf_preselcHT450_L1HT150-jJ50s5pETA32', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j90_j90a_j0_DJMASS1000j50dphi200x400deta_L1jMJJ-500-NFF', l1SeedThresholds=['FSNOSEED']*3,stream=['VBFDelayed'],groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),
    ]

    chainsMC['Bjet'] = [
        # b-jet primaries
        ChainProp(name="HLT_j210_0eta290_020jvt_bgn270_pf_ftf_preselj180_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup, monGroups=['bJetMon:online']),
        ChainProp(name="HLT_j280_0eta290_020jvt_bgn277_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_j340_0eta290_020jvt_bgn285_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),

        ChainProp(name='HLT_j180_0eta290_020jvt_bgn270_pf_ftf_L1jJ140',l1SeedThresholds=['FSNOSEED'],groups=PrimaryPhIGroup+SingleBjetGroup+NGTGroup),
        ChainProp(name='HLT_j180_020jvt_bgn270_pf_ftf_L1jJ40',l1SeedThresholds=['FSNOSEED'],groups=PrimaryPhIGroup+SingleBjetGroup+NGTGroup),
        ChainProp(name='HLT_j100_0eta290_020jvt_bgn270_pf_ftf_L1jJ40',l1SeedThresholds=['FSNOSEED'],groups=PrimaryPhIGroup+SingleBjetGroup+NGTGroup),
        ChainProp(name='HLT_j100_020jvt_bgn270_pf_ftf_L1jJ40',l1SeedThresholds=['FSNOSEED'],groups=PrimaryPhIGroup+SingleBjetGroup+NGTGroup),

        # loose b-tagging
        ChainProp(name="HLT_j225_0eta290_020jvt_bgn277_pf_ftf_preselj180_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+SingleBjetGroup),
        ChainProp(name='HLT_j275_0eta290_020jvt_bgn285_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+SingleBjetGroup),
        ChainProp(name='HLT_j300_0eta290_020jvt_bgn285_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+SingleBjetGroup),

        ChainProp(name="HLT_3j60_0eta290_020jvt_bgn277_pf_ftf_presel3j45bgtwo95_L13jJ70p0ETA23", l1SeedThresholds=['FSNOSEED'], groups=MultiBjetGroup + PrimaryPhIGroup),
        ChainProp(name="HLT_4j35_0eta290_020jvt_bgn277_pf_ftf_presel4j25bgtwo95_L14jJ40p0ETA25", l1SeedThresholds=['FSNOSEED'], groups=MultiBjetGroup + PrimaryPhIGroup),
        ChainProp(name="HLT_3j35_0eta290_020jvt_bgn270_j35_pf_ftf_presel2j25XX2j25bgtwo85_L14jJ40p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j35_0eta290_020jvt_bgn270_2j35_0eta290_020jvt_bgn285_pf_ftf_presel4j25bgtwo95_L14jJ40p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j50_0eta290_020jvt_bgn260_2j50_pf_ftf_presel2j25XX2j25bgtwo85_L14jJ40p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j35_0eta290_020jvt_bgn260_3j35_pf_ftf_presel3j25XX2j25bgtwo85_L15jJ40p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j45_0eta290_020jvt_bgn260_3j45_pf_ftf_presel3j25XX2j25bgtwo85_L15jJ40p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_j70_0eta290_020jvt_bgn260_3j70_pf_ftf_preselj50bgtwo85XX3j50_L14jJ50", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j45_0eta290_020jvt_bgn260_2j45_pf_ftf_presel2j25XX2j25bgtwo85_L14jJ40p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        # Asymmetric, 1j + 2b
        ChainProp(name="HLT_j140_2j50_0eta290_020jvt_bgn270_pf_ftf_preselj80XX2j45bgtwo90_L1jJ140_3jJ60", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        # Asymmetric 2b
        ChainProp(name="HLT_j165_0eta290_020jvt_bgn260_j55_0eta290_020jvt_bgn260_pf_ftf_preselj140bgtwo85XXj45bgtwo85_L1jJ160", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        # Run 2 HH4b low-threshold chain
        ChainProp(name="HLT_2j35c_020jvt_bgn260_2j35c_020jvt_pf_ftf_presel2j25XX2j25bgtwo85_L14jJ40p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        # Run 2 HH4b low-threshold chain, without presel R4 TDR inpired menu
        ChainProp(name="HLT_2j35c_020jvt_bgn260_2j35c_020jvt_pf_ftf_L14jJ40p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        
        # HT-seeded
        ChainProp(name='HLT_2j45_0eta290_020jvt_bgn270_j0_HT290_j0_DJMASS700j35_pf_ftf_L1HT150-jJ50s5pETA32_jMJJ-400-CF', l1SeedThresholds=['FSNOSEED']*3, groups=PrimaryPhIGroup+MultiBjetGroup+Topo3Group),
        
        # VBF chains
        ChainProp(name="HLT_j50_0eta290_020jvt_bgn270_2j45f_pf_ftf_preselj45XX2f40_L1jJ55p0ETA23_2jJ40p30ETA49", l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j65a_j45a_2j35a_SHARED_2j35_0eta290_020jvt_bgn270_j0_DJMASS1000j50_pf_ftf_presela60XXa40XX2a25_L1jMJJ-500-NFF', l1SeedThresholds=['FSNOSEED']*5, groups=PrimaryPhIGroup+MultiBjetGroup+Topo3Group),

        ChainProp(name='HLT_j55c_j50_j40f_SHARED_2j40_0eta290_020jvt_bgn260_pf_ftf_L1jJ80p0ETA25_2jJ55_jJ50p30ETA49',l1SeedThresholds=['FSNOSEED']*4,groups=PrimaryPhIGroup+MultiBjetGroup+NGTGroup),
        ChainProp(name='HLT_j55c_j50_j40f_SHARED_2j40_0eta290_020jvt_bgn260_pf_ftf_L1jJ40',l1SeedThresholds=['FSNOSEED']*4,groups=PrimaryPhIGroup+MultiBjetGroup+NGTGroup),

        # HH4b primary triggers
        # 3b asymmetric b-jet pt for Physics_Main
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn282_pf_ftf_presel2c20XX2c20bgtwo85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, groups=PrimaryPhIGroup+MultiBjetGroup),
        # 2b asymmetric b-jet pt for Physics_Main
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, groups=PrimaryPhIGroup+MultiBjetGroup, monGroups=['idMon:t0']),
        # 2b asymmetric b-jet pt for with looser b-tagging (ATR-28870)
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn280_pf_ftf_presel2c20XX2c20bgtwo85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, groups=PrimaryPhIGroup+MultiBjetGroup, monGroups=['idMon:t0']),
        # 2b asymmetric b-jet pt for with looser b-tagging without presel - R4 TDR inspired menu
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn280_pf_ftf_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, groups=PrimaryPhIGroup+MultiBjetGroup, monGroups=['idMon:t0']),
        # Candidates for allhad ttbar delayed stream
        ChainProp(name='HLT_5j35c_020jvt_j25c_020jvt_SHARED_j25c_020jvt_bgn260_pf_ftf_presel5c25XXc25bgtwo85_L14jJ40', l1SeedThresholds=['FSNOSEED']*3, groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_5j45c_020jvt_j25c_020jvt_SHARED_j25c_020jvt_bgn260_pf_ftf_presel5c25XXc25bgtwo85_L14jJ40', l1SeedThresholds=['FSNOSEED']*3, groups=PrimaryPhIGroup+MultiBjetGroup),
 
        ChainProp(name='HLT_j55c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn282_pf_ftf_L1jJ85p0ETA21_3jJ40p0ETA25',l1SeedThresholds=['FSNOSEED']*5,groups=PrimaryPhIGroup+MultiBjetGroup+NGTGroup),
        ChainProp(name='HLT_j65c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn282_pf_ftf_L14jJ40',l1SeedThresholds=['FSNOSEED']*5,groups=PrimaryPhIGroup+MultiBjetGroup+NGTGroup),
        ChainProp(name='HLT_j65c_j50c_j25c_j20c_SHARED_3j20c_bgn282_pf_ftf_L14jJ40',l1SeedThresholds=['FSNOSEED']*5,groups=PrimaryPhIGroup+MultiBjetGroup+NGTGroup),
        ChainProp(name='HLT_j65_020jvt_j50_020jvt_j25_020jvt_j20_020jvt_SHARED_3j20_020jvt_bgn282_pf_ftf_L14jJ40',l1SeedThresholds=['FSNOSEED']*5,groups=PrimaryPhIGroup+MultiBjetGroup+NGTGroup),
        ChainProp(name='HLT_j50c_020jvt_j40c_020jvt_j20c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn282_pf_ftf_L14jJ40',l1SeedThresholds=['FSNOSEED']*5,groups=PrimaryPhIGroup+MultiBjetGroup+NGTGroup),

        # Phase-II single b-jet
        ChainProp(name='HLT_j180_0eta290_020jvt_bgn285_pf_ftf_preselj140_L1jJ140', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup, monGroups=['bJetMon:online']),
        # Phase-II multi b-jet
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn282_pf_ftf_presel2c20XX2c20bgtwo85_L13jJ40', l1SeedThresholds=['FSNOSEED']*5, groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_L13jJ40', l1SeedThresholds=['FSNOSEED']*5, groups=PrimaryPhIGroup+MultiBjetGroup, monGroups=['idMon:t0']),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn280_pf_ftf_presel2c20XX2c20bgtwo85_L13jJ40', l1SeedThresholds=['FSNOSEED']*5, groups=PrimaryPhIGroup+MultiBjetGroup, monGroups=['idMon:t0']),

        # Support
        ChainProp(name='HLT_j45_0eta290_020jvt_boffperf_pf_ftf_L1jJ50', l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+SingleBjetGroup, monGroups=['bJetMon:shifter','idMon:shifter']),
        ChainProp(name='HLT_j80_0eta290_020jvt_boffperf_pf_ftf_L1jJ70', l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+SingleBjetGroup, monGroups=['bJetMon:t0','idMon:t0']),
        
        # HL-LHC TDR inspired chains
        ChainProp(name="HLT_j180_0eta290_020jvt_bgn277_pf_ftf_preselj120_L1jJ140", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        
    ]

    chainsMC['MET'] = [
        ChainProp(name='HLT_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe80_cell_xe115_tcpufit_L1jXE100',l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe80_cell_xe115_tcpufit_L1gXEJWOJ100',l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe55_cell_xe105_nn_L1jXE100', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:shifter']),
        ChainProp(name='HLT_xe55_cell_xe105_nn_L1gXEJWOJ100', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:shifter']),
        ChainProp(name='HLT_xe65_cell_xe105_nn_L1jXE100', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:shifter']),
        ChainProp(name='HLT_xe65_cell_xe105_nn_L1gXEJWOJ100', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:shifter']),

        # Phase-II MET
        ChainProp(name='HLT_xe55_cell_xe115_nn_L1jXE100', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup),
        ChainProp(name='HLT_xe55_cell_xe95_nn_L1jXE90', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:shifter']),
        ChainProp(name='HLT_xe55_cell_xe95_nn_L1jXE100', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup),

        # HL-LHC TDR inspired chains
        ChainProp(name='HLT_xe60_cell_xe100_nn_L1jXE90', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup),
    ]

    chainsMC['Bphysics'] = [ 
        #-- dimuon primary triggers
        ChainProp(name='HLT_2mu10_bJpsimumu_L12MU8F', groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_2mu10_bUpsimumu_L12MU8F', groups=BphysicsGroup+PrimaryL1MuGroup),

        #-- RCP multiple candidate
        ChainProp(name='HLT_mu10_l2mt_mu4_l2mt_bJpsimumu_L1MU10BOM', l1SeedThresholds=['MU10BOM']*2, groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu10_l2mt_mu4_l2mt_bJpsimumu_L1MU12BOM', l1SeedThresholds=['MU12BOM']*2, groups=BphysicsGroup+PrimaryL1MuGroup),

        #-- mu11_mu6 chains
        ChainProp(name='HLT_mu11_mu6_bJpsimumu_L1MU8VF_2MU8F', l1SeedThresholds=['MU8VF','MU8F'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu11_mu6_bJpsimumu_Lxy0_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu11_mu6_bUpsimumu_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], groups=BphysicsGroup+PrimaryL1MuGroup),

        #-- support
        ChainProp(name='HLT_2mu4_bBmumux_BsmumuPhi_L12MU3V', l1SeedThresholds=['MU3V'], groups=BphysicsGroup+EOFBPhysL1MuGroup, monGroups=['idMon:shifter']), # Note, delayed in R3
        ChainProp(name='HLT_mu11_mu6_bBmumux_Bidperf_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], groups=BphysicsGroup+SupportGroup, monGroups=['idMon:shifter']), # Note, delayed in R3

        # HL-LHC TDR inspired chains
        ChainProp(name='HLT_mu11_mu6_bBmumux_BpmumuKp_L12MU8F', l1SeedThresholds=['MU8VF', 'MU8F'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu11_mu6_bUpsimumu_L12MU5VF', l1SeedThresholds=['MU8VF', 'MU5VF'], groups=BphysicsGroup+PrimaryL1MuGroup),
    ]

    chains['Monitor'] = [
        ChainProp(name='HLT_noalg_CostMonDS_L1All',        l1SeedThresholds=['FSNOSEED'], stream=['CostMonitoring'], groups=['PS:NoBulkMCProd', 'Primary:CostAndRate', 'RATE:Monitoring', 'BW:Other']), # HLT_costmonitor
    ]


    chainsMC['UnconventionalTracking'] = [
        # hit-based DV 
        # Jan 2024: crash at InnerDetector/InDetDetDescr/PixelGeoModel/src/PixelDetectorTool.cxx:103:25                                
        # ChainProp(name='HLT_hitdvjet260_tight_L1jJ160', groups=PrimaryPhIGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']),
        # ChainProp(name='HLT_hitdvjet260_medium_L1jJ160', groups=SupportPhIGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']),
        # ChainProp(name='HLT_hitdvjet200_medium_L1jXE100', groups=SupportPhIGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']),

        # disappearing track trigger
        # Jan 2024: Scheduler FATAL ( 'xAOD::TrigCompositeContainer' , 'StoreGateSvc+HLT_DisTrkCand' ) required by Algorithm: DisTrkTrack
        # ChainProp(name='HLT_distrk20_tight_L1jXE100', groups=SupportPhIGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']),
        # ChainProp(name='HLT_distrk20_medium_L1jXE100', groups=SupportPhIGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']),
    ]

    chainsMC['Combined'] += [
        # Photon+Muon
        ChainProp(name='HLT_g35_loose_mu18_L1eEM28M', l1SeedThresholds=['eEM28M','MU8F'], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_g25_medium_L1eEM18L_mu24_L1MU14FCH',l1SeedThresholds=['eEM18L','MU14FCH'], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_2g10_loose_L1eEM9_mu20_L1MU14FCH', l1SeedThresholds=['eEM9','MU14FCH'], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_g35_loose_mu15_mu2noL1_L1eEM28M', l1SeedThresholds=['eEM28M','MU5VF','FSNOSEED'], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_g35_tight_icalotight_mu15noL1_mu2noL1_L1eEM28M', l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_g35_tight_icalotight_mu18noL1_L1eEM28M', l1SeedThresholds=['eEM28M','FSNOSEED'], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_g40_loose_L1eEM24L_mu40_msonly_L1MU14FCH', l1SeedThresholds=['eEM24L','MU14FCH'], groups=PrimaryPhIGroup+EgammaMuonGroup),  
        ChainProp(name='HLT_g40_loose_L1eEM24L_mu40_msonly_L1MU18VFCH', l1SeedThresholds=['eEM24L','MU18VFCH'], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_g15_loose_L1eEM10L_2mu10_msonly_L12MU8F', l1SeedThresholds=['eEM10L','MU8F'], groups=PrimaryPhIGroup+EgammaMuonGroup),

        # Electron+Muon
        ChainProp(name='HLT_e7_lhmedium_L1eEM5_mu24_L1MU14FCH',l1SeedThresholds=['eEM5','MU14FCH'], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_e17_lhloose_mu14_L1eEM18L_MU8F', l1SeedThresholds=['eEM18L','MU8F'], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_e12_lhloose_L1eEM10L_2mu10_L12MU8F', l1SeedThresholds=['eEM10L','MU8F'], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_2e12_lhloose_mu10_L12eEM10L_MU8F', l1SeedThresholds=['eEM10L','MU8F'], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_e26_lhmedium_mu8noL1_L1eEM26M', l1SeedThresholds=['eEM26M','FSNOSEED'], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_e9_lhvloose_L1eEM5_mu20_mu8noL1_L1MU14FCH', l1SeedThresholds=['eEM5','MU14FCH','FSNOSEED'], groups=PrimaryPhIGroup+EgammaMuonGroup),

        # Late stream for LLP
        ChainProp(name='HLT_g15_loose_L1eEM10L_2mu10_msonly_L1MU3V_EMPTY', l1SeedThresholds=['eEM10L','MU3V'], stream=['Late'], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_g15_loose_L1eEM10L_2mu10_msonly_L12MU5VF_EMPTY', l1SeedThresholds=['eEM10L','MU5VF'], stream=['Late'], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_g15_loose_L1eEM10L_2mu10_msonly_L1MU3V_UNPAIRED_ISO', l1SeedThresholds=['eEM10L','MU3V'], stream=['Late'], groups=PrimaryPhIGroup+EgammaMuonGroup),

        # Muon+Tau
        ChainProp(name='HLT_mu14_ivarloose_tau25_mediumRNN_tracktwoMVA_03dRAB_L1MU8F_cTAU20M_3jJ30', l1SeedThresholds=['MU8F','cTAU20M'], groups=PrimaryPhIGroup+MuonTauGroup),
        ChainProp(name='HLT_mu14_ivarloose_tau35_mediumRNN_tracktwoMVA_03dRAB_L1MU8F_cTAU30M', l1SeedThresholds=['MU8F','cTAU30M'], groups=PrimaryPhIGroup+MuonTauGroup),

        # Electron+Tau
        ChainProp(name='HLT_e17_lhmedium_ivarloose_tau25_mediumRNN_tracktwoMVA_03dRAB_L1eEM18M_2cTAU20M_4jJ30', l1SeedThresholds=['eEM18M','cTAU20M'], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_e24_lhmedium_ivarloose_tau20_mediumRNN_tracktwoMVA_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','eTAU12'], groups=PrimaryPhIGroup+EgammaTauGroup),

        # unconventional track triggers + X
        # HitDV not currently working (see UnconventionalTracking section above)
        # ChainProp(name='HLT_xe80_tcpufit_hitdvjet200_medium_L1jXE100', groups=SupportPhIGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']*2),
        # ChainProp(name='HLT_xe80_tcpufit_hitdvjet200_tight_L1jXE100', groups=PrimaryPhIGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']*2),

        # distrk not currently working (see UnconventionalTracking section above)
        # ChainProp(name='HLT_xe80_tcpufit_distrk20_tight_L1jXE100',  groups=PrimaryPhIGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']*2),
        # ChainProp(name='HLT_xe80_tcpufit_distrk20_medium_L1jXE100', groups=PrimaryPhIGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']*2),
        # ChainProp(name='HLT_xe80_tcpufit_dedxtrk25_medium_L1jXE100', groups=SupportPhIGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']*2),
        # ChainProp(name='HLT_xe80_tcpufit_dedxtrk50_medium_L1jXE100', groups=PrimaryPhIGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']*2),
        # ChainProp(name='HLT_xe80_tcpufit_isotrk120_medium_iaggrmedium_L1jXE100', l1SeedThresholds=['FSNOSEED']*2, groups=UnconvTrkGroup+PrimaryPhIGroup),
        # ChainProp(name='HLT_xe80_tcpufit_isotrk140_medium_iaggrmedium_L1jXE100', l1SeedThresholds=['FSNOSEED']*2, groups=UnconvTrkGroup+PrimaryPhIGroup),
        # ChainProp(name='HLT_xe80_tcpufit_isotrk100_medium_iaggrmedium_L1jXE100', l1SeedThresholds=['FSNOSEED']*2, groups=UnconvTrkGroup+SupportPhIGroup),
        # ChainProp(name='HLT_xe80_tcpufit_isotrk120_medium_iaggrloose_L1jXE100', l1SeedThresholds=['FSNOSEED']*2, groups=UnconvTrkGroup+SupportPhIGroup),

        # Egamma+MET
        ChainProp(name='HLT_e70_lhloose_xe70_cell_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED'], groups=PrimaryPhIGroup+EgammaMETGroup),
        ChainProp(name='HLT_g90_loose_xe90_cell_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED'], groups=PrimaryPhIGroup+EgammaMETGroup),

        # Tau+MET
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_xe80_tcpufit_xe50_cell_L1jXE100', l1SeedThresholds=['cTAU35M','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_xe80_pfopufit_xe50_cell_L1jXE100', l1SeedThresholds=['cTAU35M','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_xe80_tcpufit_xe50_cell_L1gXEJWOJ100', l1SeedThresholds=['cTAU35M','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_xe80_pfopufit_xe50_cell_L1gXEJWOJ100', l1SeedThresholds=['cTAU35M','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_xe50_cell_03dRAB_L1eTAU60_2cTAU20M_jXE80', l1SeedThresholds=['eTAU60','cTAU20M','FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),  
        ChainProp(name='HLT_e17_lhmedium_tau25_mediumRNN_tracktwoMVA_xe50_cell_03dRAB_L1eEM18M_2cTAU20M_jXE70', l1SeedThresholds=['eEM18M','cTAU20M','FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),
        ChainProp(name='HLT_mu14_tau25_mediumRNN_tracktwoMVA_xe50_cell_03dRAB_L1MU8F_cTAU20M_jXE70', l1SeedThresholds=['MU8F','cTAU20M','FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),

        # Tau+B chains for HH->bbtautau
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel3c20XX1c20bgtwo85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo85XX1c20gntau90_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo85XX1c20gntau85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo82XX1c20gntau85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo82XX1c20gntau80_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo80XX1c20gntau80_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], groups=PrimaryPhIGroup+TauBJetGroup),

        # Egamma+jet
        ChainProp(name='HLT_e10_lhmedium_ivarloose_j70_j50a_j0_DJMASS900j50_L1jMJJ-500-NFF',l1SeedThresholds=['eEM10L','FSNOSEED','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+EgammaJetGroup+Topo3Group),
        ChainProp(name='HLT_g25_medium_4j35a_j0_DJMASS1000j35_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],groups=PrimaryPhIGroup+EgammaJetGroup),
        ChainProp(name='HLT_g45_loose_6j45c_L14jJ40p0ETA25',l1SeedThresholds=['eEM18','FSNOSEED'],groups=PrimaryPhIGroup+EgammaJetGroup),
        ChainProp(name='HLT_g85_tight_3j50_pf_ftf_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED'],groups=PrimaryPhIGroup+EgammaJetGroup),

        # Egamma+bjet
        ChainProp(name='HLT_g20_tight_j35_0eta290_020jvt_bgn277_3j35a_j0_DJMASS500j35_pf_ftf_L1eEM22M_jMJJ-300',l1SeedThresholds=['eEM22M','FSNOSEED','FSNOSEED','FSNOSEED'],groups=PrimaryPhIGroup+EgammaBjetGroup+Topo2Group),
        ChainProp(name='HLT_g20_tight_icaloloose_j35_0eta290_020jvt_bgn277_3j35a_j0_DJMASS500j35_pf_ftf_L1eEM22M_jMJJ-300',l1SeedThresholds=['eEM22M','FSNOSEED','FSNOSEED','FSNOSEED'],groups=PrimaryPhIGroup+EgammaBjetGroup+Topo2Group),
        ChainProp(name='HLT_g25_tight_icaloloose_2j35_0eta290_020jvt_bgn277_2j35a_pf_ftf_presel2a20bgtwo90XX2a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],groups=PrimaryPhIGroup+EgammaBjetGroup),
        ChainProp(name='HLT_g25_tight_icaloloose_j35_0eta290_020jvt_bgn277_3j35a_j0_DJMASS700j35_pf_ftf_presela20bgtwo85XX3a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED','FSNOSEED'],groups=PrimaryPhIGroup+EgammaBjetGroup),
        ChainProp(name='HLT_g25_medium_2j35_0eta290_020jvt_bgn277_2j35a_pf_ftf_presel2a20bgtwo90XX2a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],groups=SupportPhIGroup+EgammaBjetGroup),
        ChainProp(name='HLT_g25_medium_j35_0eta290_020jvt_bgn277_3j35a_j0_DJMASS700j35_pf_ftf_presela20bgtwo85XX3a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED','FSNOSEED'],groups=SupportPhIGroup+EgammaBjetGroup),

        # Muon+jet
        ChainProp(name='HLT_mu10_ivarmedium_j70_j50a_j0_DJMASS900j50_L1jMJJ-500-NFF',l1SeedThresholds=['MU8F','FSNOSEED','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MuonJetGroup+Topo3Group),
        ChainProp(name='HLT_2mu6_2j50a_j0_DJMASS900j50_L1jMJJ-500-NFF',l1SeedThresholds=['MU5VF','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MuonJetGroup+Topo3Group),

        # Tau+jet
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_j70_j50a_j0_DJMASS900j50_L1jMJJ-500-NFF',l1SeedThresholds=['eTAU12','eTAU12','FSNOSEED','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+TauJetGroup+Topo3Group),
        
        # Jet+MET
        ChainProp(name='HLT_j70_j50a_j0_DJMASS1000j50dphi240_xe90_tcpufit_xe50_cell_L1jMJJ-500-NFF',l1SeedThresholds=['FSNOSEED']*5, groups=PrimaryPhIGroup+JetMETGroup+Topo3Group),
        ChainProp(name='HLT_j55c_xe50_cell_L1jJ60_EMPTY', l1SeedThresholds=['FSNOSEED']*2, stream=['Late'], groups=PrimaryPhIGroup+JetMETGroup),
        ChainProp(name='HLT_j55c_xe50_cell_L1jJ60_FIRSTEMPTY', l1SeedThresholds=['FSNOSEED']*2, stream=['Late'], groups=PrimaryPhIGroup+JetMETGroup),

        # Bjet+MET
        ChainProp(name='HLT_j75_0eta290_020jvt_bgn260_pf_ftf_xe60_cell_L12jJ70_jXE80', l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+BjetMETGroup),

        # Egamma+Jet+MET
        ChainProp(name='HLT_e5_lhvloose_j70_j50a_j0_DJMASS1000j50_xe50_tcpufit_L1jMJJ-500-NFF',l1SeedThresholds=['eEM5','FSNOSEED','FSNOSEED','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+EgammaJetGroup+Topo3Group),

        # Muon+jet+MET
        ChainProp(name='HLT_mu4_j70_j50a_j0_DJMASS1000j50_xe50_tcpufit_L1jMJJ-500-NFF',l1SeedThresholds=['MU3V','FSNOSEED','FSNOSEED','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MuonJetGroup+Topo3Group),

        # meson+photon
        ChainProp(name='HLT_g25_medium_tau25_dikaonmass_tracktwoMVA_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','eTAU12'], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g25_medium_tau25_kaonpi1_tracktwoMVA_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','eTAU12'], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g25_medium_tau25_kaonpi2_tracktwoMVA_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','eTAU12'], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g25_medium_tau25_singlepion_tracktwoMVA_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','eTAU12'], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g25_medium_tau25_dipion1_tracktwoMVA_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','eTAU12'], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g25_medium_tau25_dipion2_tracktwoMVA_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','eTAU12'], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g35_medium_tau25_dipion3_tracktwoMVA_60invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','TAU8'], groups=PrimaryPhIGroup+EgammaTauGroup),

        # Support
        ChainProp(name='HLT_mu24_ivarmedium_tau25_idperf_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['idMon:t0']),

        # Phase-II e+mu
        ChainProp(name='HLT_e10_lhmedium_mu10_L1eEM10L_MU8F', l1SeedThresholds=['eEM10L','MU8F'], groups=PrimaryPhIGroup+EgammaMuonGroup),
        # Phase-II e+tau
        ChainProp(name='HLT_e22_lhmedium_ivarloose_tau20_mediumRNN_tracktwoMVA_03dRAB_L1eEM22M', l1SeedThresholds=['eEM22M', 'eTAU12'], groups=PrimaryPhIGroup+EgammaTauGroup),
        # Phase-II mu+tau
        ChainProp(name='HLT_mu10_ivarloose_tau35_mediumRNN_tracktwoMVA_03dRAB_L1MU5VF_cTAU30M', l1SeedThresholds=['MU5VF', 'cTAU30M'], groups=PrimaryPhIGroup+MuonTauGroup),
        # Phase-II Tau+B chains for HH->bbtautau seeded by 3jJ40
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel3c20XX1c20bgtwo85_L13jJ40', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo85XX1c20gntau90_L13jJ40', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo85XX1c20gntau85_L13jJ40', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo82XX1c20gntau85_L13jJ40', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo82XX1c20gntau80_L13jJ40', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo80XX1c20gntau80_L13jJ40', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], groups=PrimaryPhIGroup+TauBJetGroup),
        # Phase-II Support
        ChainProp(name='HLT_e20_lhmedium_ivarloose_tau20_mediumRNN_tracktwoMVA_03dRAB_L1eEM18M', l1SeedThresholds=['eEM18M', 'eTAU12'], groups=SupportPhIGroup+EgammaTauGroup),

        # HL-LHC TDR inspired chains
        ChainProp(name='HLT_e10_lhloose_mu12_L1eEM10L_MU5VF', l1SeedThresholds=['eEM10L','MU5VF'], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_e7_lhloose_mu20_L1MU12FCH', l1SeedThresholds=['eEM7','MU12FCH'], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU12_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo85XX1c20gntau90_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], groups=PrimaryPhIGroup+MuonTauGroup),
        # without presel
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU12_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], groups=PrimaryPhIGroup+MuonTauGroup),
    ]

    chainsMC['Streaming'] += [
        ChainProp(name='HLT_noalg_L1All', l1SeedThresholds=['FSNOSEED'], groups=['Primary:CostAndRate', 'RATE:SeededStreamers', 'BW:Other']), # ATR-22072, for rates in MC.
        ]

    chains['Beamspot'] += []

    for sig in chainsMC:
        chains[sig] += chainsMC[sig]

def setupMenu():

    from AthenaCommon.Logging import logging
    log = logging.getLogger( __name__ )
    log.info('[setupMenu] going to add the MC menu chains now')
    
    chains = physics_menu.setupMenu()

    addMCSignatures(chains)

    return chains
