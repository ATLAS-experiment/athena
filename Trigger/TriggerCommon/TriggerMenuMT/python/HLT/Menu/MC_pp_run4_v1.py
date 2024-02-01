# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

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
    PhysicsStream, ExpressStream,
    SingleMuonGroup, MultiMuonGroup, 
    MultiElectronGroup,MultiPhotonGroup,
    SingleElectronGroup, SinglePhotonGroup,
    SingleTauGroup, MultiTauGroup,
    SingleJetGroup, MultiJetGroup,
    METGroup, 
    JetMETGroup,
    EgammaMuonGroup, EgammaTauGroup, EgammaMETGroup, EgammaJetGroup, 
    #EgammaBjetGroup,
    #UnconvTrkGroup,
    #SingleBjetGroup, MultiBjetGroup,
    PrimaryLegGroup, PrimaryPhIGroup, PrimaryL1MuGroup,
    SupportPhIGroup, SupportLegGroup, SupportGroup,
    TagAndProbePhIGroup, TagAndProbeLegGroup,
    Topo2Group, Topo3Group, 
    BphysElectronGroup,
    BphysicsGroup,
    TauMETGroup,
    MuonJetGroup,
    TauJetGroup,
)

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
        ChainProp(name='HLT_mu22_mu8noL1_L1MU14FCH',  l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryL1MuGroup+MultiMuonGroup, monGroups=['muonMon:online','muonMon:shifter']),        
        ChainProp(name='HLT_mu20_ivarmedium_mu8noL1_L1MU14FCH', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryL1MuGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_2mu4noL1_L1MU14FCH',  l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryL1MuGroup+MultiMuonGroup, monGroups=['muonMon:online','muonMon:shifter']),        
        ChainProp(name='HLT_3mu6_L13MU5VF',  l1SeedThresholds=['MU5VF'], groups=PrimaryL1MuGroup+MultiMuonGroup, monGroups=['muonMon:online','muonMon:shifter']),        
        ChainProp(name='HLT_3mu6_msonly_L13MU5VF',  l1SeedThresholds=['MU5VF'], groups=PrimaryL1MuGroup+MultiMuonGroup, monGroups=['muonMon:online','muonMon:shifter']),        
        ChainProp(name='HLT_4mu4_L14MU3V',  l1SeedThresholds=['MU3V'], groups=PrimaryL1MuGroup+MultiMuonGroup, monGroups=['muonMon:online','muonMon:shifter']),        
        
        # Multi muon with mass cut Run-3 primaries
        ChainProp(name='HLT_mu10_ivarmedium_mu10_10invmAB70_L12MU8F', groups=PrimaryL1MuGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_ivarmedium_mu4noL1_10invmAB70_L1MU14FCH', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryL1MuGroup+MultiMuonGroup),

        # Muon LRT chain
        ChainProp(name='HLT_mu20_LRT_d0loose_L1MU14FCH',  groups=PrimaryL1MuGroup+SingleMuonGroup, monGroups=['muonMon:online']),

        # Support
        ChainProp(name='HLT_mu24_idperf_L1MU14FCH', stream=[PhysicsStream,ExpressStream], groups=SupportGroup+SingleMuonGroup, monGroups=['idMon:shifter']),
        ChainProp(name='HLT_mu6_idperf_L1MU5VF', stream=[PhysicsStream,ExpressStream], groups=SupportGroup+SingleMuonGroup, monGroups=['idMon:t0']),
        ChainProp(name='HLT_mu26_ivarperf_L1MU14FCH', stream=[PhysicsStream,ExpressStream], groups=SupportGroup+SingleMuonGroup, monGroups=['idMon:shifter']), # ATR-21905

        #From TDR studies 
        ChainProp(name='HLT_mu18_ivarmedium_L1MU5VF', groups=PrimaryL1MuGroup+SingleMuonGroup, monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu20_ivarmedium_L1MU5VF', groups=PrimaryL1MuGroup+SingleMuonGroup, monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu40_ivarmedium_L1MU8VF', groups=PrimaryL1MuGroup+SingleMuonGroup, monGroups=['muonMon:shifter','muonMon:online']),

        ChainProp(name='HLT_mu20_mu8noL1_L1MU3VF',  l1SeedThresholds=['MU3VF','FSNOSEED'], groups=PrimaryL1MuGroup+MultiMuonGroup, monGroups=['muonMon:online','muonMon:shifter']),        
        ChainProp(name='HLT_2mu10_L12MU3VF', groups=PrimaryL1MuGroup+MultiMuonGroup, monGroups=['muonMon:online','muonMon:shifter']),
    ]

    chainsMC['Egamma'] = [

        # single electron Run-3 primaries
        ChainProp(name='HLT_e26_lhtight_ivarloose_L1eEM26M', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),
        ChainProp(name='HLT_e26_lhtight_ivarloose_L1eEM26L', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),
        ChainProp(name='HLT_e60_lhmedium_L1eEM26M', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),
        ChainProp(name='HLT_e140_lhloose_L1eEM26M', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),
        ChainProp(name='HLT_e300_etcut_L1eEM26M', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0']),
        # electron LRT
        ChainProp(name='HLT_e30_lhloose_nopix_lrtmedium_L1eEM26M', groups=PrimaryPhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e35_lhloose_nopix_lrtmedium_L1eEM26M', groups=PrimaryPhIGroup+SingleElectronGroup),
        # electron ringer
        ChainProp(name='HLT_e140_lhloose_noringer_L1eEM26M', groups=SingleElectronGroup,monGroups=['egammaMon:shifter_tp']),

        # multi electron
        ChainProp(name='HLT_2e17_lhvloose_L12eEM18M', groups=PrimaryPhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_e26_lhtight_e14_etcut_probe_50invmAB130_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeEM9'], groups=PrimaryPhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_e24_lhvloose_2e12_lhvloose_L1eEM24L_3eEM12L',l1SeedThresholds=['eEM24L','eEM12L'], groups=PrimaryPhIGroup+MultiElectronGroup),
        # single photon
        ChainProp(name='HLT_g140_loose_L1eEM26M', groups=PrimaryPhIGroup+SinglePhotonGroup),
        ChainProp(name='HLT_g300_etcut_L1eEM26M', groups=PrimaryPhIGroup+SinglePhotonGroup),

        # multi photon
        ChainProp(name='HLT_2g20_tight_icaloloose_L12eEM18M', groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_2g22_tight_L12eEM18M', groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_g35_medium_g25_medium_L12eEM24L', l1SeedThresholds=['eEM24L','eEM24L'], groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_2g50_loose_L12eEM24L', groups=PrimaryPhIGroup+MultiPhotonGroup),

        # support chains for displaced electrons
        ChainProp(name='HLT_e5_idperf_loose_lrtloose_probe_g25_medium_L1eEM24L',l1SeedThresholds=['PROBEeEM5','eEM24L'],groups=SupportPhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_e30_lhloose_nopix_lrtmedium_probe_L1eEM26M',l1SeedThresholds=['eEM26M','PROBEeEM26M'],groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_e30_lhloose_nopix_probe_L1eEM26M',l1SeedThresholds=['eEM26M','PROBEeEM26M'],groups=TagAndProbePhIGroup+SingleElectronGroup),

        # Electron + Photon triggers
        ChainProp(name='HLT_e24_lhmedium_g25_medium_02dRAB_L12eEM24L', l1SeedThresholds=['eEM24L','eEM24L'], groups=PrimaryPhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_e24_lhmedium_g12_loose_g12_loose_02dRAB_02dRAC_02dRBC_L1eEM24L_3eEM12L', l1SeedThresholds=['eEM24L','eEM12L','eEM12L'], groups=PrimaryPhIGroup+MultiElectronGroup),

        # T&P chains for displaced electrons
        ChainProp(name='HLT_e25_mergedtight_g35_medium_90invmAB_02dRAB_L12eEM24L', l1SeedThresholds=['eEM24L','eEM24L'], groups=PrimaryPhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_e26_lhtight_e14_idperf_tight_probe_50invmAB130_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeEM9'], groups=PrimaryPhIGroup+MultiElectronGroup, monGroups=['idMon:shifter']),
        ChainProp(name='HLT_e26_lhtight_e14_idperf_tight_nogsf_probe_50invmAB130_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeEM9'], groups=PrimaryPhIGroup+MultiElectronGroup, monGroups=['idMon:t0']),
        # B->K*ee chains
        ChainProp(name='HLT_e5_lhvloose_bBeeM6000_L1BKeePrimary', l1SeedThresholds=['eEM5'], stream=['BphysDelayed',ExpressStream], groups=PrimaryLegGroup+BphysElectronGroup, monGroups=['bphysMon:online','bphysMon:shifter']),
        ChainProp(name='HLT_2e5_lhvloose_bBeeM6000_L1BKeePrimary', l1SeedThresholds=['eEM5'], stream=['BphysDelayed',ExpressStream], groups=PrimaryLegGroup+BphysElectronGroup, monGroups=['bphysMon:online','bphysMon:shifter']),

        # Support
        ChainProp(name='HLT_e26_idperf_loose_L1eEM26M', groups=SingleElectronGroup+SupportPhIGroup, monGroups=['idMon:t0']),
        ChainProp(name='HLT_e5_idperf_tight_L1eEM5', groups=SingleElectronGroup+SupportPhIGroup, monGroups=['idMon:t0']),
        # TEMP Support (remove once supercell RDOs are availble)
        ChainProp(name='HLT_e5_idperf_tight_L1EM3', groups=SingleElectronGroup+SupportLegGroup, monGroups=['idMon:t0']),

        # From TDR studies
        ChainProp(name='HLT_e20_lhmedium_ivarloose_L1eEM12L', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),
        ChainProp(name='HLT_e20_lhtight_ivarloose_L1eEM12L',  groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),
        ChainProp(name='HLT_e22_lhmedium_ivarloose_L1eEM12L', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),
        ChainProp(name='HLT_e22_lhtight_ivarloose_L1eEM12L',  groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),
        ChainProp(name='HLT_e45_lhmedium_L1eEM12L',  groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),
        ChainProp(name='HLT_e60_lhloose_L1eEM12L', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),
        # ringer
        ChainProp(name='HLT_e26_lhtight_ivarloose_noringer_L1eEM12L', groups=PrimaryPhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_noringer_L1eEM12L', groups=PrimaryPhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e60_lhmedium_noringer_L1eEM12L', groups=PrimaryPhIGroup+SingleElectronGroup),

        ChainProp(name='HLT_g100_loose_L1eEM12L', groups=PrimaryPhIGroup+SinglePhotonGroup),
        ChainProp(name='HLT_g100_medium_L1eEM12L', groups=PrimaryPhIGroup+SinglePhotonGroup),
        ChainProp(name='HLT_g120_loose_L1eEM12L', groups=PrimaryPhIGroup+SinglePhotonGroup),
        ChainProp(name='HLT_g120_medium_L1eEM12L', groups=PrimaryPhIGroup+SinglePhotonGroup),
        ChainProp(name='HLT_g180_loose_L1eEM12L', groups=PrimaryPhIGroup+SinglePhotonGroup),

        ChainProp(name='HLT_g25_loose_g20_loose_L12eEM18L', l1SeedThresholds=['eEM24L','eEM18L'], groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_g25_medium_g20_medium_L12eEM18L', l1SeedThresholds=['eEM24L','eEM18L'], groups=PrimaryPhIGroup+MultiPhotonGroup),

        ChainProp(name='HLT_2g20_loose_L12eEM18L', groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_2g20_medium_L12eEM18L', groups=PrimaryPhIGroup+MultiPhotonGroup),

        ChainProp(name='HLT_2g25_loose_L12eEM18L', groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_2g25_medium_L12eEM18L', groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_2g25_tight_L12eEM18L', groups=PrimaryPhIGroup+MultiPhotonGroup),
    ]

    chainsMC['Tau'] = [
        ChainProp(name="HLT_tau160_mediumRNN_tracktwoMVA_L1eTAU140", groups=PrimaryPhIGroup+SingleTauGroup),
        ChainProp(name='HLT_tau200_mediumRNN_tracktwoMVA_L1eTAU140', groups=PrimaryPhIGroup+SingleTauGroup),

        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55', l1SeedThresholds=['cTAU30M','cTAU20M'], groups=PrimaryPhIGroup+MultiTauGroup+Topo2Group), 
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB_L1cTAU30M_2cTAU20M_4jJ30p0ETA25', l1SeedThresholds=['cTAU30M','cTAU20M'], groups=PrimaryPhIGroup+MultiTauGroup), 
        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_tau35_mediumRNN_tracktwoMVA_03dRAB_L1cTAU35M_2cTAU30M_2jJ55_3jJ50', l1SeedThresholds=['cTAU35M','cTAU30M'], groups=PrimaryPhIGroup+MultiTauGroup), 
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_tau35_mediumRNN_tracktwoMVA_03dRAB30_L1eTAU80_2cTAU30M_DR-eTAU30eTAU20', l1SeedThresholds=['eTAU80','cTAU30M'], groups=PrimaryPhIGroup+MultiTauGroup+Topo2Group, monGroups=['tauMon:online','tauMon:shifter']), 
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_tau60_mediumRNN_tracktwoMVA_03dRAB_L1eTAU80_2eTAU60', l1SeedThresholds=['eTAU80','eTAU60'], groups=PrimaryPhIGroup+MultiTauGroup),

        # tau LLP
        ChainProp(name="HLT_tau80_mediumRNN_tracktwoLLP_tau60_mediumRNN_tracktwoLLP_03dRAB_L1eTAU80_2eTAU60", l1SeedThresholds=['eTAU80','eTAU60'], groups=PrimaryPhIGroup+MultiTauGroup),
        ChainProp(name='HLT_tau180_mediumRNN_tracktwoLLP_L1eTAU140', groups=PrimaryPhIGroup+SingleTauGroup, monGroups=['tauMon:shifter']),
        ChainProp(name="HLT_tau200_mediumRNN_tracktwoLLP_L1eTAU140", groups=PrimaryPhIGroup+SingleTauGroup, monGroups=['tauMon:online','tauMon:t0']),
        ChainProp(name="HLT_tau180_tightRNN_tracktwoLLP_L1eTAU140", groups=PrimaryPhIGroup+SingleTauGroup),
        ChainProp(name="HLT_tau200_tightRNN_tracktwoLLP_L1eTAU140", groups=PrimaryPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau100_mediumRNN_tracktwoLLP_tau80_mediumRNN_tracktwoLLP_03dRAB_L1eTAU80_2eTAU60", l1SeedThresholds=['eTAU80','eTAU60'], groups=PrimaryPhIGroup+MultiTauGroup, monGroups=['tauMon:t0']),

        # Support
        ChainProp(name="HLT_tau25_idperf_tracktwoMVA_L1TAU12IM", groups=SingleTauGroup+SupportLegGroup, monGroups=['tauMon:online','tauMon:shifter','idMon:shifter']), #ATR-27013

    ]

    chainsMC['Jet'] = [
        ChainProp(name='HLT_j420_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'],  groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j440_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j220f_L1jJ125p30ETA49', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j230f_L1jJ125p30ETA49', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j240f_L1jJ125p30ETA49', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j260f_L1jJ125p30ETA49', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j460_a10sd_cssk_pf_jes_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'],  groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j460_a10sd_cssk_pf_jes_ftf_preselj225_L1SC111-CjJ40', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup+Topo3Group),
        ChainProp(name='HLT_j460_a10t_lcw_jes_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j460_a10t_lcw_jes_L1SC111-CjJ40', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup+Topo3Group),
        ChainProp(name='HLT_j460_a10r_L1jJ160', l1SeedThresholds=['FSNOSEED'],  groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j460_a10r_L1SC111-CjJ40', l1SeedThresholds=['FSNOSEED'],  groups=PrimaryPhIGroup+SingleJetGroup+Topo3Group),
        ChainProp(name='HLT_j460_a10_lcw_subjes_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j460_a10_lcw_subjes_L1SC111-CjJ40',         l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup+Topo3Group),
        ChainProp(name='HLT_j420_35smcINF_a10sd_cssk_pf_jes_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j420_35smcINF_a10sd_cssk_pf_jes_ftf_preselj225_L1SC111-CjJ40', l1SeedThresholds=['FSNOSEED'],  groups=SingleJetGroup+PrimaryPhIGroup+Topo3Group),
        ChainProp(name='HLT_j420_35smcINF_a10t_lcw_jes_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j420_35smcINF_a10t_lcw_jes_L1SC111-CjJ40', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup+Topo3Group),
        
        #HT chains
        ChainProp(name='HLT_j0_HT1000_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j0_HT1000_L1HT190-jJ40s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+SingleJetGroup+Topo3Group),
        ChainProp(name='HLT_j0_HT1000_pf_ftf_preselj180_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j0_HT1000_pf_ftf_preselj190_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j0_HT1000_pf_ftf_preselj200_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j0_HT1000_pf_ftf_preselj180_L1HT190-jJ40s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup+Topo3Group),

        # multi jets small R
        ChainProp(name='HLT_2j250c_j120c_pf_ftf_presel2j180XXj80_L1jJ160', l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_3j200_pf_ftf_presel3j150_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_4j115_pf_ftf_presel4j85_L13jJ90', l1SeedThresholds=['FSNOSEED'],  groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_5j70c_pf_ftf_presel5j50_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_5j70c_pf_ftf_presel5j55_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_5j85_pf_ftf_presel5j50_L14jJ40', l1SeedThresholds=['FSNOSEED'],  groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_5j85_pf_ftf_presel5j55_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_6j55c_pf_ftf_presel6j40_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_6j55c_pf_ftf_presel6c45_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_6j70_pf_ftf_presel6j40_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_6j70_pf_ftf_presel6j45_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_7j45_pf_ftf_presel7j30_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_10j40_pf_ftf_presel7j30_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),

        #multijet large R with mass cut
        ChainProp(name='HLT_2j330_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_2j330_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC111-CjJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+PrimaryPhIGroup+Topo3Group),
        ChainProp(name='HLT_2j330_35smcINF_a10t_lcw_jes_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+MultiJetGroup),
        ChainProp(name='HLT_2j330_35smcINF_a10t_lcw_jes_L1SC111-CjJ40', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),
        ChainProp(name='HLT_j360_60smcINF_j360_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC111-CjJ40', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),
        ChainProp(name='HLT_j360_60smcINF_j360_a10t_lcw_jes_L1SC111-CjJ40', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),
        ChainProp(name='HLT_j370_35smcINF_j370_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC111-CjJ40', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),
        ChainProp(name='HLT_j370_35smcINF_j370_a10t_lcw_jes_L1SC111-CjJ40', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),

        # Large R primary jet chains with gLJ L1 items (L1J100->L1gLJ140p0ETA25)
        ChainProp(name='HLT_j460_a10sd_cssk_pf_jes_ftf_preselj225_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:shifter', 'jetMon:online']),
        ChainProp(name='HLT_j460_a10t_lcw_jes_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j460_a10r_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'],  groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j460_a10_lcw_subjes_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j420_35smcINF_a10t_lcw_jes_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_2j330_35smcINF_a10t_lcw_jes_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j420_35smcINF_a10sd_cssk_pf_jes_ftf_preselj225_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_2j330_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1gLJ140p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:t0']),

        # Support
        ChainProp(name='HLT_j45_pf_ftf_preselj20_L1J15', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,ExpressStream], groups=SingleJetGroup+SupportLegGroup, monGroups=['jetMon:t0','jetMon:online','idMon:shifter','caloMon:t0']),
       
    ]

    chainsMC['Bjet'] = [
        # Jan 2024: bjet chains do not configure, mention of TRT and then crash in pixel geo.
        # GeoModelSvc                                                       INFO GeoModelSvc.TRT_DetectorTool     SZ= 2372Kb      Time = 0.04S
        # Followed by crash
        # PixelDetectorTool::create() /build/atnight/localbuilds/nightlies/Athena/main/athena/InnerDetector/InDetDetDescr/PixelGeoModel/src/PixelDetectorTool.cxx:103:25 

        # ChainProp(name="HLT_j225_0eta290_020jvt_bdl1d70_pf_ftf_preselj180_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup, monGroups=['bJetMon:online']),
        # ChainProp(name="HLT_j300_0eta290_020jvt_bdl1d77_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        # ChainProp(name="HLT_j360_0eta290_020jvt_bdl1d85_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        # ChainProp(name="HLT_j225_0eta290_020jvt_bdl1d70_pf_ftf_preselj190_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup, monGroups=['bJetMon:online']),
        # ChainProp(name="HLT_j225_0eta290_020jvt_bdl1d70_pf_ftf_preselj200_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup, monGroups=['bJetMon:online']),
        # ChainProp(name="HLT_j225_0eta290_020jvt_bdl1d60_pf_ftf_preselj180_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        # ChainProp(name="HLT_j275_0eta290_020jvt_bdl1d70_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        # ChainProp(name="HLT_j300_0eta290_020jvt_bdl1d70_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        # ChainProp(name="HLT_j360_0eta290_020jvt_bdl1d77_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        # ChainProp(name="HLT_3j65_0eta290_020jvt_bdl1d77_pf_ftf_presel3j45b95_L13jJ70p0ETA23", l1SeedThresholds=['FSNOSEED'], groups=MultiBjetGroup + PrimaryPhIGroup),
        # ChainProp(name="HLT_4j35_0eta290_020jvt_bdl1d77_pf_ftf_presel4j25b95_L14jJ40p0ETA25", l1SeedThresholds=['FSNOSEED'], groups=MultiBjetGroup + PrimaryPhIGroup),
        # ChainProp(name="HLT_3j35_0eta290_020jvt_bdl1d70_j35_pf_ftf_presel2j25XX2j25b85_L14jJ40p0ETA25",      l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        # ChainProp(name="HLT_2j35_0eta290_020jvt_bdl1d70_2j35_0eta290_020jvt_bdl1d85_pf_ftf_presel4j25b95_L14jJ40p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        # ChainProp(name="HLT_2j55_0eta290_020jvt_bdl1d60_2j55_pf_ftf_presel2j25XX2j25b85_L14jJ40p0ETA25",        l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        # ChainProp(name="HLT_2j35_0eta290_020jvt_bdl1d60_3j35_pf_ftf_presel3j25XX2j25b85_L15jJ40p0ETA25",  l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        # ChainProp(name="HLT_2j45_0eta290_020jvt_bdl1d60_3j45_pf_ftf_presel3j25XX2j25b85_L15jJ40p0ETA25",  l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        # ChainProp(name="HLT_j75_0eta290_020jvt_bdl1d60_3j75_pf_ftf_preselj50b85XX3j50_L14jJ50",           l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        # ChainProp(name="HLT_2j45_0eta290_020jvt_bdl1d60_2j45_pf_ftf_presel2j25XX2j25b85_L14jJ40p0ETA25",  l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        # ChainProp(name="HLT_j150_2j55_0eta290_020jvt_bdl1d70_pf_ftf_preselj80XX2j45b90_L1jJ140_3jJ60", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        # ChainProp(name="HLT_j175_0eta290_020jvt_bdl1d60_j60_0eta290_020jvt_bdl1d60_pf_ftf_preselj140b85XXj45b85_L1jJ160", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        # ChainProp(name="HLT_2j35c_020jvt_bdl1d60_2j35c_020jvt_pf_ftf_presel2j25XX2j25b85_L14jJ40p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        # ChainProp(name='HLT_2j45_0eta290_020jvt_bdl1d70_j0_HT300_j0_DJMASS700j35_pf_ftf_L1HT150-jJ50s5pETA32_jMJJ-400-CF', l1SeedThresholds=['FSNOSEED']*3, groups=PrimaryPhIGroup+MultiBjetGroup+Topo3Group),
        
        # VBF chains
        # ChainProp(name='HLT_j80c_j60_j45f_SHARED_2j45_0eta290_020jvt_bdl1d60_pf_ftf_preselc60XXj45XXf40_L1jJ80p0ETA25_2jJ55_jJ50p30ETA49', l1SeedThresholds=['FSNOSEED']*4, groups=PrimaryPhIGroup+MultiBjetGroup),
        # ChainProp(name="HLT_j80_0eta290_020jvt_bdl1d70_j60_0eta290_020jvt_bdl1d85_j45f_pf_ftf_preselj60XXj45XXf40_L1jJ80p0ETA25_2jJ55_jJ50p30ETA49", l1SeedThresholds=['FSNOSEED']*3, groups=PrimaryPhIGroup+MultiBjetGroup),
        # ChainProp(name="HLT_j55_0eta290_020jvt_bdl1d70_2j45f_pf_ftf_preselj45XX2f40_L1jJ55p0ETA23_2jJ40p30ETA49",l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+MultiBjetGroup),
        # ChainProp(name='HLT_j70a_j50a_2j35a_SHARED_2j35_0eta290_020jvt_bdl1d70_j0_DJMASS1000j50_pf_ftf_presela60XXa40XX2a25_L1jMJJ-500-NFF', l1SeedThresholds=['FSNOSEED']*5,stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiBjetGroup+Topo3Group),
        # ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1d82_pf_ftf_presel2c20XX2c20b85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, groups=PrimaryPhIGroup+MultiBjetGroup),
        # ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d77_pf_ftf_presel2c20XX2c20b85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiBjetGroup),
        # ChainProp(name='HLT_5j35c_020jvt_j25c_020jvt_SHARED_j25c_020jvt_bdl1d60_pf_ftf_presel5c25XXc25b85_L14jJ40', l1SeedThresholds=['FSNOSEED']*3, stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiBjetGroup),
        # ChainProp(name='HLT_5j45c_020jvt_j25c_020jvt_SHARED_j25c_020jvt_bdl1d60_pf_ftf_presel5c25XXc25b85_L14jJ40', l1SeedThresholds=['FSNOSEED']*3, stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiBjetGroup),
    ]

    chainsMC['MET'] = [
        ChainProp(name='HLT_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe80_cell_xe115_tcpufit_L1jXE100',l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe90_pfopufit_L1gXERHO100', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe90_pfopufit_L1gXENC100', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe100_mhtpufit_pf_L1gXEJWOJ100', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe100_mhtpufit_pf_L1gXERHO100', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe65_cell_xe100_mhtpufit_pf_L1gXENC100', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe80_cell_xe115_tcpufit_L1gXEJWOJ100',l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe80_cell_xe115_tcpufit_L1gXERHO100',l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
        ChainProp(name='HLT_xe80_cell_xe115_tcpufit_L1gXENC100',l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryPhIGroup+METGroup, monGroups=['metMon:t0']),
    ]

    chainsMC['Bphysics'] = [ 
        #-- dimuon primary triggers
        ChainProp(name='HLT_2mu10_bJpsimumu_L12MU8F', stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_2mu10_bUpsimumu_L12MU8F', stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),

        #-- RCP multiple candidate
        ChainProp(name='HLT_mu10_l2mt_mu4_l2mt_bJpsimumu_L1MU10BOM', l1SeedThresholds=['MU10BOM']*2, stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu10_l2mt_mu4_l2mt_bJpsimumu_L1MU12BOM', l1SeedThresholds=['MU12BOM']*2, stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),

        #-- mu11_mu6 chains
        ChainProp(name='HLT_mu11_mu6_bJpsimumu_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu11_mu6_bJpsimumu_Lxy0_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu11_mu6_bUpsimumu_L1MU8VF_2MU5VF', l1SeedThresholds=['MU8VF','MU5VF'], stream=['BphysDelayed'], groups=BphysicsGroup+PrimaryL1MuGroup),
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
        ChainProp(name='HLT_g25_medium_L1eEM18L_mu24_L1MU18VFCH',l1SeedThresholds=['eEM18L','MU18VFCH'], groups=PrimaryPhIGroup+EgammaMuonGroup), #ATR-22594 moved from Dev to Phy
        ChainProp(name='HLT_2g10_loose_L1eEM9_mu20_L1MU14FCH', l1SeedThresholds=['eEM9','MU14FCH'], groups=PrimaryPhIGroup+EgammaMuonGroup), # unsure what eEM seed should be
        ChainProp(name='HLT_2g10_loose_L1eEM9_mu20_L1MU18VFCH', l1SeedThresholds=['eEM9','MU18VFCH'], groups=PrimaryPhIGroup+EgammaMuonGroup), # unsure what eEM seed should be
        ChainProp(name='HLT_g15_loose_L1eEM10L_2mu10_msonly_L12MU8F', l1SeedThresholds=['eEM10L','MU8F'], groups=PrimaryPhIGroup+EgammaMuonGroup),

        # #Electron+Muon
        ChainProp(name='HLT_e7_lhmedium_L1eEM5_mu24_L1MU14FCH',l1SeedThresholds=['eEM5','MU14FCH'], groups=PrimaryPhIGroup+EgammaMuonGroup), 
        ChainProp(name='HLT_e17_lhloose_mu14_L1eEM18L_MU8F', l1SeedThresholds=['eEM18L','MU8F'], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_2e12_lhloose_mu10_L12eEM10L_MU8F', l1SeedThresholds=['eEM10L','MU8F'], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_e7_lhmedium_L1eEM5_mu24_L1MU18VFCH',l1SeedThresholds=['eEM5','MU18VFCH'],  groups=PrimaryPhIGroup+EgammaMuonGroup),# moved from Dev to Phy
        ChainProp(name='HLT_e12_lhloose_L1eEM10L_2mu10_L12MU8F', l1SeedThresholds=['eEM10L','MU8F'], groups=PrimaryPhIGroup+EgammaMuonGroup),# moved from Dev to Phy
        ChainProp(name='HLT_g40_loose_L1eEM24L_mu40_msonly_L1MU14FCH', l1SeedThresholds=['eEM24L','MU14FCH'], groups=PrimaryPhIGroup+EgammaMuonGroup),  
        ChainProp(name='HLT_g40_loose_L1eEM24L_mu40_msonly_L1MU18VFCH', l1SeedThresholds=['eEM24L','MU18VFCH'], groups=PrimaryPhIGroup+EgammaMuonGroup),  

        # Late stream for LLP
        ChainProp(name='HLT_g15_loose_L1eEM10L_2mu10_msonly_L1MU3V_EMPTY', l1SeedThresholds=['eEM10L','MU3V'], stream=['Late'], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_g15_loose_L1eEM10L_2mu10_msonly_L12MU5VF_EMPTY', l1SeedThresholds=['eEM10L','MU5VF'], stream=['Late'], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_g15_loose_L1eEM10L_2mu10_msonly_L1MU3V_UNPAIRED_ISO', l1SeedThresholds=['eEM10L','MU3V'], stream=['Late'], groups=PrimaryPhIGroup+EgammaMuonGroup),
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

        # Egamma+jet
        ChainProp(name='HLT_g25_medium_4j35a_j0_DJMASS1000j35_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],groups=PrimaryPhIGroup+EgammaJetGroup),
        ChainProp(name='HLT_g45_loose_6j45c_L14jJ40p0ETA25',l1SeedThresholds=['eEM18','FSNOSEED'],groups=PrimaryPhIGroup+EgammaJetGroup),
        ChainProp(name='HLT_e10_lhmedium_ivarloose_j70_j50a_j0_DJMASS900j50_L1jMJJ-500-NFF',l1SeedThresholds=['eEM10L','FSNOSEED','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+EgammaJetGroup+Topo3Group),
        ChainProp(name='HLT_2e5_lhmedium_j70_j50a_j0_DJMASS900j50_L1jMJJ-500-NFF',l1SeedThresholds=['eEM5','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['VBFDelayed'], groups=PrimaryPhIGroup+EgammaJetGroup+Topo3Group),
        ChainProp(name='HLT_g85_tight_3j50_pf_ftf_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED'],groups=PrimaryPhIGroup+EgammaJetGroup),

        # Egamma+bjet
        # Bjet not working (see above)
        # ChainProp(name='HLT_g25_tight_icaloloose_2j35_0eta290_020jvt_bdl1d77_2j35a_pf_ftf_presel2a20b90XX2a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],groups=PrimaryPhIGroup+EgammaBjetGroup),
        # ChainProp(name='HLT_g25_tight_icaloloose_j35_0eta290_020jvt_bdl1d77_3j35a_j0_DJMASS700j35_pf_ftf_presela20b85XX3a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED','FSNOSEED'],groups=PrimaryPhIGroup+EgammaBjetGroup),

        # Muon+jet
        ChainProp(name='HLT_mu10_ivarmedium_j70_j50a_j0_DJMASS900j50_L1jMJJ-500-NFF',l1SeedThresholds=['MU8F','FSNOSEED','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MuonJetGroup+Topo3Group),
        ChainProp(name='HLT_2mu6_2j50a_j0_DJMASS900j50_L1jMJJ-500-NFF',l1SeedThresholds=['MU5VF','FSNOSEED','FSNOSEED'],stream=['VBFDelayed'], groups=PrimaryPhIGroup+MuonJetGroup+Topo3Group), # Formerly HLT_2mu6_2j50_0eta490_invm900j50                       

        # Tau+jet
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_j70_j50a_j0_DJMASS900j50_L1jMJJ-500-NFF',l1SeedThresholds=['eTAU12','eTAU12','FSNOSEED','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+TauJetGroup+Topo3Group),
        
        # Jet+MET
        ChainProp(name='HLT_j70_j50a_j0_DJMASS1000j50dphi240_xe90_tcpufit_xe50_cell_L1jMJJ-500-NFF',l1SeedThresholds=['FSNOSEED']*5,stream=['VBFDelayed'], groups=PrimaryPhIGroup+JetMETGroup+Topo3Group),
        ChainProp(name='HLT_j55c_xe50_cell_L1jJ60_EMPTY', l1SeedThresholds=['FSNOSEED']*2, stream=['Late'], groups=PrimaryPhIGroup+JetMETGroup),
        ChainProp(name='HLT_j55c_xe50_cell_L1jJ60_FIRSTEMPTY', l1SeedThresholds=['FSNOSEED']*2, stream=['Late'], groups=PrimaryPhIGroup+JetMETGroup),

        # Egamma+Jet+MET
        ChainProp(name='HLT_e5_lhvloose_j70_j50a_j0_DJMASS1000j50_xe50_tcpufit_L1jMJJ-500-NFF',l1SeedThresholds=['eEM5','FSNOSEED','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['VBFDelayed'], groups=PrimaryPhIGroup+EgammaJetGroup+Topo3Group),

        # Muon+jet+MET
        ChainProp(name='HLT_mu4_j70_j50a_j0_DJMASS1000j50_xe50_tcpufit_L1jMJJ-500-NFF',l1SeedThresholds=['MU3V','FSNOSEED','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['VBFDelayed'], groups=PrimaryPhIGroup+MuonJetGroup+Topo3Group),

        # Egamma+tau
        ChainProp(name='HLT_g25_medium_tau25_dikaonmass_tracktwoMVA_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','eTAU12'], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g25_medium_tau25_kaonpi1_tracktwoMVA_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','eTAU12'], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g25_medium_tau25_kaonpi2_tracktwoMVA_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','eTAU12'], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g25_medium_tau25_singlepion_tracktwoMVA_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','eTAU12'], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g25_medium_tau25_dipion1_tracktwoMVA_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','eTAU12'], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g25_medium_tau25_dipion2_tracktwoMVA_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','eTAU12'], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_g25_medium_tau25_dipion4_tracktwoMVA_50invmAB_L1eEM26M', l1SeedThresholds=['eEM26M','eTAU12'], groups=PrimaryPhIGroup+EgammaTauGroup),
        #L1 not in menu
        #ChainProp(name='HLT_e17_lhmedium_ivarloose_tau25_mediumRNN_tracktwoMVA_03dRAB_L1eEM18M_2eTAU20_4jJ30', l1SeedThresholds=['eEM18M','eTAU20'], stream=[PhysicsStream], groups=SupportPhIGroup+EgammaTauGroup),

        # Support
        ChainProp(name='HLT_mu24_ivarmedium_tau25_idperf_tracktwoMVA_probe_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBETAU12IM'], stream=[PhysicsStream,ExpressStream], groups=TagAndProbeLegGroup+SingleMuonGroup, monGroups=['idMon:t0']),
    ]

    chainsMC['Streaming'] += []

    chains['Beamspot'] += []

    for sig in chainsMC:
        chains[sig] += chainsMC[sig]

def setupMenu(menu_name):

    from AthenaCommon.Logging import logging
    log = logging.getLogger( __name__ )
    log.info('[setupMenu] going to add the MC menu chains now')
    
    chains = physics_menu.setupMenu(menu_name)

    addMCSignatures(chains)

    return chains
