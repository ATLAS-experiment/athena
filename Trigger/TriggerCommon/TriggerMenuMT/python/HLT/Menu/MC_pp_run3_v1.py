# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# MC_pp_run3_v1.py menu for the long shutdown development
#------------------------------------------------------------------------#

# All chains are represented as ChainProp objects in a ChainStore
from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp
from .SignatureDicts import ChainStore

import TriggerMenuMT.HLT.Menu.Physics_pp_run3_v1 as physics_menu 
from TriggerMenuMT.HLT.Menu.Physics_pp_run3_v1 import ( PhysicsStream,
                                                        METGroup,
                                                        SingleElectronGroup,
                                                        SinglePhotonGroup,
                                                        SingleTauGroup,
                                                        MultiTauGroup,
                                                        TauJetGroup,
                                                        TauBJetGroup,
                                                        TauMETGroup,
                                                        TauPhotonGroup,
                                                        TauStreamersGroup,
                                                        BphysicsGroup,
                                                        EOFBPhysL1MuGroup,
                                                        EOFL1MuGroup,
                                                        EgammaBjetGroup,
                                                        EgammaJetGroup,
                                                        EgammaMETGroup,
                                                        EgammaStreamersGroup,
                                                        EgammaTauGroup,
                                                        EgammaMuonGroup,
                                                        METStreamersGroup,
                                                        MultiJetGroup,
                                                        MultiPhotonGroup,
                                                        SupportGroup,
                                                        SupportPhIGroup,
                                                        SupportLegGroup,
                                                        SingleBjetGroup,
                                                        MultiBjetGroup,
                                                        SingleJetGroup,
                                                        SingleMuonGroup,
                                                        MultiMuonGroup,
                                                        MuonTauGroup,
                                                        BphysElectronGroup,
                                                        Topo2Group,
                                                        Topo3Group,
                                                        LegacyTopoGroup,
                                                        MuonBjetGroup,
                                                        BjetMETGroup,
                                                        JetStreamersGroup,
                                                        JetMETGroup,
                                                        MinBiasGroup,
                                                        MuonJetGroup,
                                                        MultiElectronGroup,
                                                        EOFTLALegGroup,
                                                        UnconvTrkGroup,
)


# For central PHYS and PHYSLITE derivations of MC where we do not yet have a whole-year GRL available
# (e.g. because the derivation is running on a campaign which was prepared before the end of data taking for the current year)
# we use the chain's group's to identify a sub-set of chains from which we determine the sub-set of lowest un-prescaled chains for which we store
# trigger matching data in the DAOD. This is based on chains with a group which contains 'Primary' or 'TagAndProbe'.
# We do NOT want chains in the MC menu resolving as looser primaries as compared to their counterparts in the Physics menu.
# As it would mean that we have matching information available for different chains in MC as compared to data.
# We avoid this here by renaming these groups in the MC menu to avoid either of these substrings. 'Primary' to 'MCPri' and 'TagAndProbe' to 'MCTagProbe' 
# See ATR-30593
PrimaryL1MuGroup = ['MCPri:L1Muon']
PrimaryLegGroup = ['MCPri:Legacy']
PrimaryPhIGroup = ['MCPri:PhaseI']
TagAndProbeLegGroup = ['Support:MCLegacyTagProbe']
TagAndProbePhIGroup = ['Support:MCPhaseITagProbe']

from AthenaCommon.Logging import logging
log = logging.getLogger( __name__ )

def getMCSignatures():
    chains = ChainStore()

    chains['Muon'] = [

        ChainProp(name="HLT_mu8_L1MU5VF", groups=SingleMuonGroup),
        ChainProp(name="HLT_mu10_L1MU8F", groups=SingleMuonGroup),
        ChainProp(name="HLT_mu14_L1MU8F", groups=SingleMuonGroup),
        ChainProp(name="HLT_mu14_L1MU8VFC", groups=SingleMuonGroup),
        ChainProp(name='HLT_2mu4_L12MU3V',  groups=MultiMuonGroup),

        #ATR-29567
        ChainProp(name='HLT_mu16_ivarmedium_L1MU12FCH', l1SeedThresholds=['MU12FCH'], groups=PrimaryPhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu18_ivarmedium_L1MU12FCH', l1SeedThresholds=['MU12FCH'], groups=PrimaryPhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu20_ivarmedium_L1MU12FCH', l1SeedThresholds=['MU12FCH'], groups=PrimaryPhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu18_ivarmedium_L1MU14FCH', l1SeedThresholds=['MU14FCH'], groups=PrimaryPhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu20_ivarmedium_L1MU14FCH', l1SeedThresholds=['MU14FCH'], groups=PrimaryPhIGroup+SingleMuonGroup),

        
        #-- nscan ATR-19376, TODO: to be moved to physics once debugged to a resaonable rate
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan_L1MU14FCH_J40', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryLegGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan_L1MU14FCH_jJ80', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryPhIGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan_L1MU14FCH_XE30', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryLegGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan_L1MU14FCH_jXE70', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryPhIGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan_L110DR-MU14FCH-MU5VF', l1SeedThresholds=['MU14FCH','FSNOSEED'],   groups=PrimaryL1MuGroup+MultiMuonGroup+Topo2Group),



        ## ATR-24198
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan_L1MU14FCH', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=SupportGroup+MultiMuonGroup),

        ## ATR-25456 - dimuon TLA with L1TOPO
        ChainProp(name='HLT_2mu4_PhysicsTLA_L1BPH-7M22-2MU3VF', l1SeedThresholds=['MU3VF'],stream=['TLA'], groups=MultiMuonGroup+EOFL1MuGroup+Topo3Group),
        ChainProp(name='HLT_2mu4_PhysicsTLA_L1BPH-7M22-0DR20-2MU3VF', l1SeedThresholds=['MU3VF'],stream=['TLA'], groups=MultiMuonGroup+EOFL1MuGroup+Topo3Group),
        ChainProp(name='HLT_2mu4_PhysicsTLA_L1BPH-7M22-0DR12-2MU3V', l1SeedThresholds=['MU3V'],stream=['TLA'], groups=MultiMuonGroup+EOFL1MuGroup+Topo3Group),

        ## ATR-25456 - 4mu
        ChainProp(name='HLT_mu6_mu4_L1BPH-7M14-MU5VFMU3VF', l1SeedThresholds=['MU5VF','MU3VF'], stream=["BphysDelayed"], groups=BphysicsGroup+EOFBPhysL1MuGroup+Topo3Group),
        ChainProp(name='HLT_2mu4_L1BPH-7M14-2MU3V', l1SeedThresholds=['MU3V'], stream=["BphysDelayed"], groups=BphysicsGroup+EOFBPhysL1MuGroup+Topo3Group),
        ChainProp(name='HLT_2mu4_L1BPH-7M14-2MU3VF', l1SeedThresholds=['MU3VF'], stream=["BphysDelayed"], groups=BphysicsGroup+EOFBPhysL1MuGroup+Topo3Group),

        ## ATR-25456 - 4mu L1 with DR, for optimization
        ChainProp(name='HLT_2mu4_L1BPH-7M14-0DR25-MU5VFMU3VF', l1SeedThresholds=['MU3VF'], stream=["BphysDelayed"], groups=MultiMuonGroup+EOFBPhysL1MuGroup+Topo3Group),
    ]

    chains['Jet'] = [
        ChainProp(name="HLT_j20_roiftf_preselj20_L1RD0_FILLED", l1SeedThresholds=['FSNOSEED'],  monGroups=['idMon:t0'], groups=SingleJetGroup+SupportGroup),

        # Low-threshold calibration Large-R jets
        # Non-commissioning L1 thresholds
        ChainProp(name='HLT_j85_a10sd_cssk_pf_nojcalib_ftf_preselj50_L1jLJ60', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j85_a10sd_cssk_pf_jes_ftf_preselj50_L1jLJ60', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j85_a10t_lcw_nojcalib_L1jLJ60', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j85_a10t_lcw_jes_L1jLJ60',      l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j175_a10sd_cssk_pf_jes_ftf_preselj140_L1jLJ100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j175_a10t_lcw_jes_L1jLJ100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),

        # Emerging Jets test chains ATR-28771
        ChainProp(name='HLT_j200_0eta180_emergingPTF0p08dR1p2_a10sd_cssk_pf_jes_ftf_preselj160_L1jJ160', groups=SingleJetGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED']),

        ChainProp(name='HLT_j200_0eta180_emergingPTF0p08dR1p2_a10sd_cssk_pf_jes_ftf_preselj160_L1gLJ140p0ETA25', groups=SingleJetGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j200_0eta180_emergingPTF0p08dR1p2_a10sd_cssk_pf_jes_ftf_preselj160_L1SC111-CjJ40', groups=SingleJetGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED']),

        ## ATR-25456 - calratio jet chains
        #ChainProp(name='HLT_j30_CLEANllp_calratio_L1eTAU80', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        #ChainProp(name='HLT_j30_CLEANllp_calratiormbib_L1eTAU80', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),



        ## ATR-25456 - Central Exclusive Production for SM group
        ChainProp(name='HLT_2j100_L1CEP-CjJ90', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),
        ChainProp(name='HLT_2j100_L1CEP-CjJ100', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),

        ## ATR-27238 - Wrong L1 jJ thresholds; moving from Physics to MC for compatibility with 2023 menu 
        ChainProp(name='HLT_j60_L1jJ90', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j85_L1jJ90', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),

        ## ATR-27270 - Downshifted chains 

        ChainProp(name='HLT_3j200_pf_ftf_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j420_pf_ftf_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j420_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j420_pf_ftf_preselj225_L1jJ180', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),

        ChainProp(name='HLT_2j250c_j120c_pf_ftf_presel2j180XXj80_L1jJ160', l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_3j200_pf_ftf_presel3j150_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_4j115_pf_ftf_presel4j85_L13jJ90', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_5j70c_pf_ftf_presel5c50_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_5j70c_pf_ftf_presel5c55_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_5j85_pf_ftf_presel5j50_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_5j85_pf_ftf_presel5j55_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_6j70_pf_ftf_presel6j40_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_6j70_pf_ftf_presel6j45_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_10j40_pf_ftf_presel7j30_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),

        ChainProp(name='HLT_j0_HT1000_pf_ftf_preselj180_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j0_HT1000_pf_ftf_preselj190_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j0_HT1000_pf_ftf_preselj200_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j0_HT1000_pf_ftf_preselj180_L1HT190-jJ40s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup+Topo3Group),
        ChainProp(name='HLT_j0_HT1000_pf_ftf_preselj180_L1gJ400p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleJetGroup),


        # ATR-28795 bjet menu cleanup
        ChainProp(name='HLT_2j20_2j20_pf_ftf_presel2c20XX2c20b82_PhysicsTLA_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*2, stream=['TLA'], groups=MultiJetGroup+SupportPhIGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_2j20_2j20_pf_ftf_presel2c20XX2c20b80_PhysicsTLA_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*2, stream=['TLA'], groups=MultiJetGroup+SupportPhIGroup, monGroups=['tlaMon:shifter']),

        #-- calratio/calratio+VBF ATR-28412 candidates moved to MC for now best candiate moved to physics after validation
        ChainProp(name='HLT_j30_CLEANllp_momemfrac006_calratio_L1jMJJ-500-NFF', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j30_CLEANllp_momemfrac012_calratiormbib_L1jMJJ-500-NFF', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j30_CLEANllp_momemfrac012_calratiovar186_roiftf_preselj20emf12_L1jMJJ-500-NFF', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j0_DJMASS900j50dphi260x200deta_calratiovar186_roiftf_preselj20emf12_L1jMJJ-500-NFF',l1SeedThresholds=['FSNOSEED'],stream=['Main'],groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),
        ChainProp(name='HLT_j0_DJMASS900j50dphi260x200deta_calratiovar165_roiftf_preselj20emf18_L1jMJJ-500-NFF',l1SeedThresholds=['FSNOSEED'],stream=['Main'],groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),
        ChainProp(name='HLT_j0_DJMASS900j50dphi260x200deta_calratiovar150_roiftf_preselj20emf24_L1jMJJ-500-NFF',l1SeedThresholds=['FSNOSEED'],stream=['Main'],groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),
        ChainProp(name='HLT_j30_CLEANllp_momemfrac012_calratiovar186_roiftf_preselj20emf12_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),

    ]

    chains['Bjet'] = [
        #move bjet GN1 tagger from physics to MC

        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b85_L1MU8F_2jJ40_jJ50', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),
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
        # 3b asymmetric b-jet pt for Physics_Main
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn182_pf_ftf_presel2c20XX2c20b85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),
        # 2b asymmetric b-jet pt for VBFDelayed
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], monGroups=['idMon:t0'], groups=PrimaryPhIGroup+MultiBjetGroup),
        # Candidates for allhad ttbar delayed stream
        ChainProp(name='HLT_5j35c_020jvt_j25c_020jvt_SHARED_j25c_020jvt_bgn160_pf_ftf_presel5c25XXc25b85_L14jJ40', l1SeedThresholds=['FSNOSEED']*3, stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_5j45c_020jvt_j25c_020jvt_SHARED_j25c_020jvt_bgn160_pf_ftf_presel5c25XXc25b85_L14jJ40', l1SeedThresholds=['FSNOSEED']*3, stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),

        # B-tagger training
        ChainProp(name='HLT_j20_0eta290_020jvt_boffperf_pf_ftf_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleBjetGroup+SupportLegGroup, monGroups=['bJetMon:online']),
        ChainProp(name='HLT_j20_0eta290_020jvt_boffperf_pf_ftf_L1jJ40', l1SeedThresholds=['FSNOSEED'], groups=SingleBjetGroup+SupportPhIGroup, monGroups=['bJetMon:online']),
        # HH4b
        # Muon+jet legacy seeded, backup for L1Topo muon-in-jet moved by ATR-28761
        
        #L1 legacy Removal
        #ATR-29523
        ChainProp(name='HLT_2j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b85_L13jJ40p0ETA25'    , l1SeedThresholds=['FSNOSEED']*4, stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_2j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*4, stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_2j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*4, stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_2j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn280_pf_ftf_presel2c20XX2c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*4, stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_2j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn280_pf_ftf_presel2c20XX2c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*4, stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),

        ChainProp(name='HLT_j50c_020jvt_2j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*4, stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j40c_020jvt_2j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*4, stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j50c_020jvt_2j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn280_pf_ftf_presel2c20XX2c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*4, stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j40c_020jvt_2j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn280_pf_ftf_presel2c20XX2c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*4, stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),
        
        # Various 2018 multi-b triggers
        ChainProp(name="HLT_3j60_0eta290_020jvt_bgn177_pf_ftf_presel3j45b95_L13J35p0ETA23", l1SeedThresholds=['FSNOSEED'], groups=MultiBjetGroup + PrimaryLegGroup), # downshift
        ChainProp(name="HLT_3j65_0eta290_020jvt_bgn177_pf_ftf_presel3j45b95_L13J35p0ETA23", l1SeedThresholds=['FSNOSEED'], groups=MultiBjetGroup + PrimaryLegGroup),

        ChainProp(name="HLT_4j35_0eta290_020jvt_bgn177_pf_ftf_presel4j25b95_L14J15p0ETA25", l1SeedThresholds=['FSNOSEED'], groups=MultiBjetGroup + PrimaryLegGroup),
        ChainProp(name="HLT_3j35_0eta290_020jvt_bgn170_j35_pf_ftf_presel2j25XX2j25b85_L14J15p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j35_0eta290_020jvt_bgn170_2j35_0eta290_020jvt_bgn185_pf_ftf_presel4j25b95_L14J15p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j50_0eta290_020jvt_bgn160_2j50_pf_ftf_presel2j25XX2j25b85_L14J15p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup), # downshift
        ChainProp(name="HLT_2j55_0eta290_020jvt_bgn160_2j55_pf_ftf_presel2j25XX2j25b85_L14J15p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j45_0eta290_020jvt_bgn160_2j45_pf_ftf_presel2j25XX2j25b85_L14J15p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j35c_020jvt_bgn160_2j35c_020jvt_pf_ftf_presel2j25XX2j25b85_L14J15p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup),

        # Various 2018 multi-b triggers
        ChainProp(name="HLT_2j35_0eta290_020jvt_bgn160_3j35_pf_ftf_presel3j25XX2j25b85_L15J15p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j45_0eta290_020jvt_bgn160_3j45_pf_ftf_presel3j25XX2j25b85_L15J15p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup),

        # downshift after jet calibration
        ChainProp(name='HLT_2j45_0eta290_020jvt_bgn170_j0_HT300_j0_DJMASS700j35_pf_ftf_L1HT150-J20s5pETA31_MJJ-400-CF', l1SeedThresholds=['FSNOSEED']*3, groups=PrimaryLegGroup+MultiBjetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j70a_j50a_2j35a_SHARED_2j35_0eta290_020jvt_bgn170_j0_DJMASS1000j50_pf_ftf_presela60XXa40XX2a25_L1MJJ-500-NFF', l1SeedThresholds=['FSNOSEED']*5,stream=['VBFDelayed'], groups=PrimaryLegGroup+MultiBjetGroup+LegacyTopoGroup),

        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b85_L1MU8F_2jJ40_jJ50', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b82_L1MU8F_2jJ40_jJ50', l1SeedThresholds=['FSNOSEED']*5, stream=['Main'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b80_L1MU8F_2jJ40_jJ50', l1SeedThresholds=['FSNOSEED']*5, stream=['Main'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_boffperf_pf_ftf_presel2c20XX2c20b85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*4, stream=['Main'],  monGroups=['idMon:t0'], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_boffperf_pf_ftf_presel2c20XX2c20b82_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*4, stream=['Main'],  monGroups=['idMon:t0'], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_boffperf_pf_ftf_presel2c20XX2c20b80_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*4, stream=['Main'],  monGroups=['idMon:t0'], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_j225_0eta290_020jvt_bgn170_pf_ftf_preselj180_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup, monGroups=['bJetMon:online']),
        ChainProp(name="HLT_j300_0eta290_020jvt_bgn177_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_j360_0eta290_020jvt_bgn185_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_j225_0eta290_020jvt_bgn170_pf_ftf_preselj190_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup, monGroups=['bJetMon:online']),
        ChainProp(name="HLT_j225_0eta290_020jvt_bgn170_pf_ftf_preselj200_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup, monGroups=['bJetMon:online']),
        ChainProp(name="HLT_j225_0eta290_020jvt_bgn160_pf_ftf_preselj180_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_j275_0eta290_020jvt_bgn170_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_j300_0eta290_020jvt_bgn170_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_j360_0eta290_020jvt_bgn177_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_3j65_0eta290_020jvt_bgn177_pf_ftf_presel3j45b95_L13jJ70p0ETA23", l1SeedThresholds=['FSNOSEED'], groups=MultiBjetGroup + PrimaryPhIGroup),
        ChainProp(name="HLT_2j55_0eta290_020jvt_bgn160_2j55_pf_ftf_presel2j25XX2j25b85_L14jJ40p0ETA25",        l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_j75_0eta290_020jvt_bgn160_3j75_pf_ftf_preselj50b85XX3j50_L14jJ50",           l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_j150_2j55_0eta290_020jvt_bgn170_pf_ftf_preselj80XX2j45b90_L1jJ140_3jJ60", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_j175_0eta290_020jvt_bgn160_j60_0eta290_020jvt_bgn160_pf_ftf_preselj140b85XXj45b85_L1jJ160", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_2j45_0eta290_020jvt_bgn170_j0_HT300_j0_DJMASS700j35_pf_ftf_L1HT150-jJ50s5pETA32_jMJJ-400-CF', l1SeedThresholds=['FSNOSEED']*3, groups=PrimaryPhIGroup+MultiBjetGroup+Topo3Group),
        ChainProp(name='HLT_j80c_j60_j45f_SHARED_2j45_0eta290_020jvt_bgn160_pf_ftf_preselc60XXj45XXf40_L1jJ80p0ETA25_2jJ55_jJ50p30ETA49', l1SeedThresholds=['FSNOSEED']*4, groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_j80_0eta290_020jvt_bgn170_j60_0eta290_020jvt_bgn185_j45f_pf_ftf_preselj60XXj45XXf40_L1jJ80p0ETA25_2jJ55_jJ50p30ETA49", l1SeedThresholds=['FSNOSEED']*3,stream=['Main'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_j55_0eta290_020jvt_bgn170_2j45f_pf_ftf_preselj45XX2f40_L1jJ55p0ETA23_2jJ40p30ETA49",l1SeedThresholds=['FSNOSEED']*2,  stream=['Main'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j70a_j50a_2j35a_SHARED_2j35_0eta290_020jvt_bgn170_j0_DJMASS1000j50_pf_ftf_presela60XXa40XX2a25_L1jMJJ-500-NFF', l1SeedThresholds=['FSNOSEED']*5,stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiBjetGroup+Topo3Group),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn182_pf_ftf_presel2c20XX2c20b85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['Main'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'],  monGroups=['idMon:t0'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn182_pf_ftf_presel2c20XX2c20b82_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['Main'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b82_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn182_pf_ftf_presel2c20XX2c20b80_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['Main'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b80_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiBjetGroup),

        # tighter presel backups
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b82_L1MU8F_2jJ40_jJ50', l1SeedThresholds=['FSNOSEED']*5, stream=['Main'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b80_L1MU8F_2jJ40_jJ50', l1SeedThresholds=['FSNOSEED']*5, stream=['Main'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_boffperf_pf_ftf_presel2c20XX2c20b82_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*4, stream=['Main'],  monGroups=['idMon:t0'], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_boffperf_pf_ftf_presel2c20XX2c20b80_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*4, stream=['Main'],  monGroups=['idMon:t0'], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn182_pf_ftf_presel2c20XX2c20b82_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['Main'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b82_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn182_pf_ftf_presel2c20XX2c20b80_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['Main'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b80_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiBjetGroup),

    ]

    chains['Egamma'] = [

        #ATR-29567
        ChainProp(name='HLT_e24_lhtight_ivarloose_L1eEM24VM', groups=PrimaryPhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_L1eEM24VM', groups=PrimaryPhIGroup+SingleElectronGroup),
        
        
        
        ChainProp(name='HLT_e26_lhtight_L1eEM26', groups=SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_L1eEM26L', groups=SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_L1eEM26T', groups=SingleElectronGroup),
        #------------ dnn chains
        
        
        

        # ATR-24268, K*ee chains for rate and acceptance studies
        ChainProp(name='HLT_e5_lhvloose_L1eEM5_bBeeM6000_L1All', l1SeedThresholds=['eEM5'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        ChainProp(name='HLT_2e5_lhvloose_L1eEM5_bBeeM6000_L1All', l1SeedThresholds=['eEM5'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        ChainProp(name='HLT_e5_lhvloose_L1eEM5_e3_lhvloose_L1eEM5_bBeeM6000_L1All', l1SeedThresholds=['eEM5','eEM5'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        
        #
        ChainProp(name='HLT_2e5_lhvloose_bBeeM6000_L1eEM26M', l1SeedThresholds=['eEM5'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        ChainProp(name='HLT_e5_lhvloose_e3_lhvloose_bBeeM6000_L1eEM26M', l1SeedThresholds=['eEM5','eEM5'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        ChainProp(name='HLT_e5_lhvloose_bBeeM6000_L1eEM26M', l1SeedThresholds=['eEM5'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        #
        ChainProp(name='HLT_2e5_lhvloose_bBeeM6000_L14jJ40', l1SeedThresholds=['eEM5'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        ChainProp(name='HLT_e5_lhvloose_e3_lhvloose_bBeeM6000_L14jJ40', l1SeedThresholds=['eEM5','eEM5'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        ChainProp(name='HLT_e5_lhvloose_bBeeM6000_L14jJ40', l1SeedThresholds=['eEM5'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        
        
        #
        ChainProp(name='HLT_2e5_lhvloose_bBeeM6000_L1BPH-0M9-eEM9-eEM7_MU5VF', l1SeedThresholds=['eEM5'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        ChainProp(name='HLT_e5_lhvloose_e3_lhvloose_bBeeM6000_L1BPH-0M9-eEM9-eEM7_MU5VF', l1SeedThresholds=['eEM5','eEM5'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        ChainProp(name='HLT_e5_lhvloose_bBeeM6000_L1BPH-0DR3-eEM9jJ40_MU5VF', l1SeedThresholds=['eEM5'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        
        

        # ATR-27780
        
        ChainProp(name='HLT_g5_nopid_L1eEM5', groups=SinglePhotonGroup+['PS:NoBulkMCProd']),

        #ATR-25764 - adding Photon chains with different isolation WPs
        ChainProp(name='HLT_g25_tight_icaloloose_L1eEM26M', groups=SinglePhotonGroup, monGroups=['egammaMon:shifter']),
        ChainProp(name='HLT_g25_tight_icalotight_L1eEM26M', groups=SinglePhotonGroup, monGroups=['egammaMon:shifter']),

        #  ATR-26311
        #  Validating/checking eFEX and primary electron trigger
        ChainProp(name='HLT_e26_etcut_L1eEM26M', groups=SingleElectronGroup),

        # Ringer development / validation also ATR-24384
        ChainProp(name='HLT_g20_loose_ringer_L1eEM18M', groups=SinglePhotonGroup,monGroups=['egammaMon:shifter']),
        ChainProp(name='HLT_g20_medium_ringer_L1eEM18M', groups=SinglePhotonGroup,monGroups=['egammaMon:shifter']),
        ChainProp(name='HLT_g120_loose_ringer_L1eEM26M', groups=SinglePhotonGroup,monGroups=['egammaMon:shifter']),
        ChainProp(name='HLT_g20_loose_L1eEM18M',  groups=SinglePhotonGroup,monGroups=['egammaMon:shifter']),

        ChainProp(name='HLT_g20_medium_L1eEM18M', groups=SinglePhotonGroup,monGroups=['egammaMon:shifter']),
        
        #  ATR-27156
        #  Migration of legacy EM seeded items to Phase 1 eEM seeded - EGamma chains
        ChainProp(name='HLT_e26_lhtight_ivarloose_nogsf_L1eEM26M', groups=SupportPhIGroup+SingleElectronGroup), #Phase-1
        ChainProp(name='HLT_e60_lhmedium_nogsf_L1eEM26M', groups=SupportPhIGroup+SingleElectronGroup), #Phase-1
        ChainProp(name='HLT_g25_tight_icalotight_L1eEM24L', groups=SupportPhIGroup+SinglePhotonGroup), #Phase-1
        ChainProp(name='HLT_g25_tight_icalomedium_L1eEM24L', groups=SupportPhIGroup+SinglePhotonGroup),#Phase-1
       
        # ATR-27940
        ChainProp(name='HLT_g35_medium_g25_medium_L12eEM9_EMPTY', l1SeedThresholds=['eEM9']*2, stream=['Late'], groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_2g22_tight_L12eEM9_EMPTY', l1SeedThresholds=['eEM9'], stream=['Late'], groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_2g50_tight_L12eEM9_EMPTY', l1SeedThresholds=['eEM9'], stream=['Late'], groups=PrimaryPhIGroup+MultiPhotonGroup),


        # ATR-23625 HH->bbrr trigger
        ChainProp(name='HLT_g45_medium_g20_medium_L12eEM18M', l1SeedThresholds=['eEM18M','eEM18M'], stream=['Main'], groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_g45_medium_g20_medium_L12eEM18L', l1SeedThresholds=['eEM18L','eEM18L'], stream=['Main'], groups=SupportPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_2g20_medium_L12eEM18L', l1SeedThresholds=['eEM18L'], stream=['Main'], groups=SupportPhIGroup+MultiPhotonGroup),

    ]

    chains['MET'] = []

    chains['Tau'] = [
        #ATR-29439
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ50', l1SeedThresholds=['cTAU30M','cTAU20M'], stream=['VBFDelayed'], groups=SupportPhIGroup+MultiTauGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ40', l1SeedThresholds=['cTAU30M','cTAU20M'], stream=['VBFDelayed'], groups=SupportPhIGroup+MultiTauGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ30', l1SeedThresholds=['cTAU30M','cTAU20M'], stream=['VBFDelayed'], groups=SupportPhIGroup+MultiTauGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ50', l1SeedThresholds=['cTAU30M','cTAU20M'], stream=['VBFDelayed'], groups=SupportPhIGroup+MultiTauGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ40', l1SeedThresholds=['cTAU30M','cTAU20M'], stream=['VBFDelayed'], groups=SupportPhIGroup+MultiTauGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ30', l1SeedThresholds=['cTAU30M','cTAU20M'], stream=['VBFDelayed'], groups=SupportPhIGroup+MultiTauGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB_L1cTAU30M_2cTAU20M_3jJ30p0ETA25'          , l1SeedThresholds=['cTAU30M','cTAU20M'], stream=['VBFDelayed'], groups=SupportPhIGroup+MultiTauGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L1cTAU30M_2cTAU20M_3jJ30p0ETA25'          , l1SeedThresholds=['cTAU30M','cTAU20M'], stream=['VBFDelayed'], groups=SupportPhIGroup+MultiTauGroup),
    ]

    chains['Bphysics'] = [

        #ATR-21566, chains for di-muon TLA, but with HLT selections to test rates. Here streaming into BphysDelayed (not in TLA stream)   
        ChainProp(name='HLT_2mu4_b0dRAB207invmAB22vtx20_L1BPH-7M22-0DR20-2MU3V', l1SeedThresholds=['MU3V'],stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+Topo3Group),
        ChainProp(name='HLT_2mu4_b0dRAB207invmAB22vtx20_L1BPH-7M22-0DR20-2MU3VF', l1SeedThresholds=['MU3VF'],stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+Topo3Group),
        ChainProp(name='HLT_2mu4_b0dRAB127invmAB22vtx20_L1BPH-7M22-0DR12-2MU3V', l1SeedThresholds=['MU3V'],stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+Topo3Group),
    ]

    chains['Streaming'] += [
        ChainProp(name='HLT_noalg_L1All', l1SeedThresholds=['FSNOSEED'], groups=['Primary:CostAndRate', 'RATE:SeededStreamers', 'BW:Other']), # ATR-22072, for rates in MC.
    ]

    chains['Calib'] += [

    ]

    chains['Combined'] += [
        # movebjet GN1 tagger from Physics to MC
        # one remaining bgn1 chain for posterity
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn185_pf_ftf_presel3c20XX1c20bgtwo85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_mu22noL1_2j20_0eta290_020jvt_bgn185_pf_ftf_L1eEM26M', l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'], stream=[PhysicsStream,'express'], groups=TagAndProbePhIGroup+EgammaBjetGroup, monGroups=['bJetMon:shifter','bJetMon:online']),
        ChainProp(name='HLT_e28_lhtight_ivarloose_mu22noL1_2j20_0eta290_020jvt_bgn185_pf_ftf_L1eEM28M', l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'], stream=[PhysicsStream,'express'], groups=TagAndProbePhIGroup+EgammaBjetGroup, monGroups=['bJetMon:shifter','bJetMon:online']),
        ChainProp(name='HLT_j75_0eta290_020jvt_bgn160_pf_ftf_xe60_cell_L12jJ90_jXE80', l1SeedThresholds=['FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup),
        ChainProp(name='HLT_g20_tight_j35_0eta290_020jvt_bgn177_3j35a_j0_DJMASS500j35_pf_ftf_L1eEM22M_jMJJ-300',l1SeedThresholds=['eEM22M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaBjetGroup+Topo2Group),
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
        # [ATR-25500] Switch to dl1d for bjet+met+met triggers, and now to GN1
        ChainProp(name="HLT_2j45_0eta290_020jvt_bgn160_pf_ftf_xe50_cell_xe85_tcpufit_L12jJ40_jXE110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup),
        ChainProp(name="HLT_3j35_0eta290_020jvt_bgn160_pf_ftf_xe50_cell_xe70_tcpufit_L13jJ40p0ETA25_jXE80", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+BjetMETGroup),
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

        ## ATR-25456 - Photon+MET reoptimised
        ChainProp(name='HLT_g25_tight_icalotight_xe40_cell_xe50_tcpufit_80mTAC_L1eEM26M',l1SeedThresholds=['eEM26M']+2*['FSNOSEED'], groups=PrimaryPhIGroup+EgammaMETGroup),
        ChainProp(name='HLT_g25_tight_icalotight_xe40_cell_xe40_tcpufit_xe40_pfopufit_80mTAC_L1eEM26M',l1SeedThresholds=['eEM26M']+3*['FSNOSEED'], groups=PrimaryPhIGroup+EgammaMETGroup),

        ## ATR-25456 - B/D to e+mu triggers
        ChainProp(name='HLT_e14_lhtight_mu6_dRAB15_invmAB10_L1LFV-eEM15L-MU5VF', l1SeedThresholds=['eEM12L','MU5VF'], groups=PrimaryPhIGroup+BphysicsGroup+Topo3Group),
        ChainProp(name='HLT_e12_lhtight_mu11_dRAB15_invmAB10_L1LFV-eEM10L-MU8VF', l1SeedThresholds=['eEM10L','MU8VF'], groups=PrimaryPhIGroup+BphysicsGroup+Topo3Group),

        # ATR-29651 - Tau+X chains using eTAU20M seeds
        ChainProp(name='HLT_e17_lhmedium_ivarloose_tau25_mediumRNN_tracktwoMVA_03dRAB_L1eEM18M_2eTAU20M_4jJ30', l1SeedThresholds=['eEM18M','eTAU20M'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_xe50_cell_03dRAB_L1eTAU60_2eTAU20M_jXE80', l1SeedThresholds=['eTAU60','eTAU20M','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+TauMETGroup),
        ChainProp(name='HLT_e17_lhmedium_tau25_mediumRNN_tracktwoMVA_xe50_cell_03dRAB_L1eEM18M_2eTAU20M_jXE70', l1SeedThresholds=['eEM18M','eTAU20M','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+TauMETGroup),

        #ATR-29523
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_j50c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel3c20XX1c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_L1eTAU12_j50c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel3c20XX1c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_2j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel3c20XX1c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+4*['FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_L1eTAU12_2j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel3c20XX1c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+4*['FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+TauBJetGroup),

        # ATR-28795 bjet menu cleanup
        ChainProp(name='HLT_g60_tight_2j55_0eta200_emergingPTF0p1dR0p4_pf_ftf_L1eEM26M', groups=SupportPhIGroup+EgammaJetGroup, l1SeedThresholds=['eEM26M','FSNOSEED']),
        ChainProp(name='HLT_g45_tight_2j55_0eta200_emergingPTF0p1dR0p4_pf_ftf_L1eEM26M', groups=SupportPhIGroup+EgammaJetGroup, l1SeedThresholds=['eEM26M','FSNOSEED']),
        ChainProp(name='HLT_g60_tight_icaloloose_2j55_0eta200_emergingPTF0p1dR0p4_pf_ftf_L1eEM26M', groups=PrimaryPhIGroup+EgammaJetGroup, l1SeedThresholds=['eEM26M','FSNOSEED']),
        ChainProp(name='HLT_g45_tight_icaloloose_2j55_0eta200_emergingPTF0p1dR0p4_pf_ftf_L1eEM26M', groups=PrimaryPhIGroup+EgammaJetGroup, l1SeedThresholds=['eEM26M','FSNOSEED']),
        ChainProp(name='HLT_g60_tight_2j55_0eta200_emergingPTF0p1dR0p4_pf_ftf_L1eEM28M', groups=SupportPhIGroup+EgammaJetGroup, l1SeedThresholds=['eEM28M','FSNOSEED']),
        ChainProp(name='HLT_g45_tight_2j55_0eta200_emergingPTF0p1dR0p4_pf_ftf_L1eEM28M', groups=SupportPhIGroup+EgammaJetGroup, l1SeedThresholds=['eEM28M','FSNOSEED']),
        ChainProp(name='HLT_g60_tight_icaloloose_2j55_0eta200_emergingPTF0p1dR0p4_pf_ftf_L1eEM28M', groups=PrimaryPhIGroup+EgammaJetGroup, l1SeedThresholds=['eEM28M','FSNOSEED']),
        ChainProp(name='HLT_g45_tight_icaloloose_2j55_0eta200_emergingPTF0p1dR0p4_pf_ftf_L1eEM28M', groups=PrimaryPhIGroup+EgammaJetGroup, l1SeedThresholds=['eEM28M','FSNOSEED']),

        ChainProp(name='HLT_j80_0eta290_020jvt_bgn160_pf_ftf_xe60_cell_L12jJ90_jXE80', l1SeedThresholds=['FSNOSEED','FSNOSEED'], stream=['Main'], groups=PrimaryPhIGroup+BjetMETGroup),

        #-- nscan+VBF ATR-28412 candidates moved to MC for now best candiate moved to physics after validation

        ChainProp(name='HLT_mu6_msonly_j70_j50a_j0_DJMASS900j50dphi260x200deta_L1jMJJ-500-NFF', l1SeedThresholds=['MU5VF','FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream],groups=PrimaryPhIGroup+MuonJetGroup+Topo3Group),
        ChainProp(name='HLT_2mu6noL1_msonly_nscan_j70_j50a_j0_DJMASS900j50dphi260x200deta_L1jMJJ-500-NFF', l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream],groups=PrimaryPhIGroup+MuonJetGroup+Topo3Group),

        ChainProp(name='HLT_mu20_msonly_j70_j50a_j0_DJMASS900j50dphi260x200deta_L1jMJJ-500-NFF', l1SeedThresholds=['MU14FCH','FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream],groups=PrimaryPhIGroup+MuonJetGroup+Topo3Group),
        ChainProp(name='HLT_mu6noL1_msonly_j70_j50a_j0_DJMASS900j50dphi260x200deta_L1jMJJ-500-NFF', l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream],groups=PrimaryPhIGroup+MuonJetGroup+Topo3Group),
 
        #ATR-29567
        ChainProp(name='HLT_e17_lhloose_mu12_L1eEM18L_MU8F', l1SeedThresholds=['eEM18L','MU8F'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_e17_lhloose_mu10_L1eEM18L_MU8F', l1SeedThresholds=['eEM18L','MU8F'], stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_e7_lhmedium_L1eEM5_mu22_L1MU14FCH',l1SeedThresholds=['eEM5','MU14FCH'],  stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_e7_lhmedium_L1eEM5_mu20_L1MU14FCH',l1SeedThresholds=['eEM5','MU14FCH'],  stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_e7_lhmedium_L1eEM5_mu18_L1MU14FCH',l1SeedThresholds=['eEM5','MU14FCH'],  stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_e7_lhmedium_L1eEM5_mu22_L1MU18VFCH',l1SeedThresholds=['eEM5','MU18VFCH'],  stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),
        ChainProp(name='HLT_e7_lhmedium_L1eEM5_mu20_L1MU18VFCH',l1SeedThresholds=['eEM5','MU18VFCH'],  stream=[PhysicsStream], groups=PrimaryPhIGroup+EgammaMuonGroup),

    ]


    return chains

def setupMenu():
    
    chains = physics_menu.setupMenu()

    log.info('[setupMenu] going to add the MC menu chains now')

    for sig,chainsInSig in getMCSignatures().items():
        chains[sig] += chainsInSig

    return chains
