# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# MC_pp_run3_v1.py menu for the long shutdown development
#------------------------------------------------------------------------#

# All chains are represented as ChainProp objects in a ChainStore
from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp
from .SignatureDicts import ChainStore

import TriggerMenuMT.HLT.Menu.Physics_pp_run3_v1 as physics_menu 
from TriggerMenuMT.HLT.Menu.Physics_pp_run3_v1 import ( PhysicsStream,
                                                        SingleElectronGroup,
                                                        SinglePhotonGroup,
                                                        MultiTauGroup,
                                                        TauBJetGroup,
                                                        TauMETGroup,
                                                        TauJetGroup,
                                                        TauPhotonGroup,
                                                        EOFTauBjetGroup,
                                                        EOFTauPhIGroup,
                                                        BphysicsGroup,
                                                        EOFBPhysL1MuGroup,
                                                        EOFL1MuGroup,
                                                        EgammaBjetGroup,
                                                        EgammaJetGroup,
                                                        EgammaMETGroup,
                                                        EgammaTauGroup,
                                                        EgammaMuonGroup,
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
                                                        BphysElectronGroup,
                                                        Topo2Group,
                                                        Topo3Group,
                                                        LegacyTopoGroup,
                                                        BjetMETGroup,
                                                        MuonJetGroup,
                                                        MuonTauGroup,
                                                        SingleTauGroup,
                                                        TauPhaseIStreamersGroup,
                                                        UnconvTrkGroup,
                                                        EOFEgammaPhIGroup,
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

        #ATR-29567 + ATR-30593
        ChainProp(name='HLT_mu16_ivarmedium_L1MU12FCH', l1SeedThresholds=['MU12FCH'], groups=EOFL1MuGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu18_ivarmedium_L1MU12FCH', l1SeedThresholds=['MU12FCH'], groups=EOFL1MuGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu20_ivarmedium_L1MU12FCH', l1SeedThresholds=['MU12FCH'], groups=EOFL1MuGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu18_ivarmedium_L1MU14FCH', l1SeedThresholds=['MU14FCH'], groups=EOFL1MuGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu20_ivarmedium_L1MU14FCH', l1SeedThresholds=['MU14FCH'], groups=EOFL1MuGroup+SingleMuonGroup),

        
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


        # Emerging Jets test chains ATR-28771
        ChainProp(name='HLT_j200_0eta180_emergingPTF0p08dR1p2_a10sd_cssk_pf_jes_ftf_preselj160_L1jJ160', groups=SingleJetGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED']),

        ChainProp(name='HLT_j200_0eta180_emergingPTF0p08dR1p2_a10sd_cssk_pf_jes_ftf_preselj160_L1gLJ140p0ETA25', groups=SingleJetGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j200_0eta180_emergingPTF0p08dR1p2_a10sd_cssk_pf_jes_ftf_preselj160_L1SC175-SCjJ10', groups=SingleJetGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED']),

        ## ATR-25456 - calratio jet chains
        #ChainProp(name='HLT_j30_CLEANllp_calratio_L1eTAU80', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        #ChainProp(name='HLT_j30_CLEANllp_calratiormbib_L1eTAU80', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),


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

        ChainProp(name='HLT_6j20_pf_ftf_presel6c25_PhysicsTLA_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['tlaMon:shifter']),
        
        #-- calratio/calratio+VBF ATR-28412 candidates moved to MC for now best candiate moved to physics after validation
        ChainProp(name='HLT_j30_CLEANllp_momemfrac006_calratio_L1jMJJ-500-NFF', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j30_CLEANllp_momemfrac012_calratiormbib_L1jMJJ-500-NFF', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j30_CLEANllp_momemfrac012_calratiovar186_roiftf_preselj20emf12_L1jMJJ-500-NFF', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j0_DJMASS900j50dphi260x200deta_calratiovar186_roiftf_preselj20emf12_L1jMJJ-500-NFF',l1SeedThresholds=['FSNOSEED'],stream=['Main'],groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),
        ChainProp(name='HLT_j0_DJMASS900j50dphi260x200deta_calratiovar165_roiftf_preselj20emf18_L1jMJJ-500-NFF',l1SeedThresholds=['FSNOSEED'],stream=['Main'],groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),
        ChainProp(name='HLT_j0_DJMASS900j50dphi260x200deta_calratiovar150_roiftf_preselj20emf24_L1jMJJ-500-NFF',l1SeedThresholds=['FSNOSEED'],stream=['Main'],groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),
        ChainProp(name='HLT_j30_CLEANllp_momemfrac012_calratiovar186_roiftf_preselj20emf12_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),
        
        # ATR-31260 moving chains to MC as they are not needed
        ChainProp(name='HLT_j30_CLEANllp_momemfrac006_calratiormbib_L1eTAU60_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j0_HT1000_L1HT190-jJ40s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+SingleJetGroup+Topo3Group, monGroups=['jetMon:shifter']),

    ]

    chains['Bjet'] = [

        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_boffperf_pf_ftf_presel2c20XX2c20b85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*4, stream=[PhysicsStream],  monGroups=['idMon:t0'], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_2j20_2j20_pf_ftf_presel2c20XX2c20b85_PhysicsTLA_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*2, stream=['TLA'], groups=MultiJetGroup+SupportPhIGroup, monGroups=['tlaMon:shifter']),
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

        #ATR_30971
        ChainProp(name='HLT_g35_medium_g25_medium_noringer_L12eEM24L', groups=PrimaryPhIGroup+SinglePhotonGroup, monGroups=['egammaMon:shifter']),
        ChainProp(name='HLT_e26_dnntight_ivarloose_e30_dnnloose_nopix_lrtmedium_probe_L1eEM26M',l1SeedThresholds=['eEM26M','PROBEeEM26M'],groups=TagAndProbePhIGroup+SingleElectronGroup),
       

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
        
        # ATR-27438
        ChainProp(name='HLT_e5_nopid_lrtloose_L1eEM5', l1SeedThresholds=['eEM5'], groups=SingleElectronGroup+['PS:NoBulkMCProd']),

        # ATR-27780
        

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

        # ATR-21637
        ChainProp(name='HLT_2g9_loose_25dphiAA_invmAA80_L1DPHI-M70-2eEM9L', l1SeedThresholds=['eEM9'], groups=EOFEgammaPhIGroup+MultiPhotonGroup+Topo2Group),

        # ATR-23625 HH->bbrr trigger
        ChainProp(name='HLT_g45_medium_g20_medium_L12eEM18M', l1SeedThresholds=['eEM18M','eEM18M'], stream=['Main'], groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_g45_medium_g20_medium_L12eEM18L', l1SeedThresholds=['eEM18L','eEM18L'], stream=['Main'], groups=SupportPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_2g20_medium_L12eEM18L', l1SeedThresholds=['eEM18L'], stream=['Main'], groups=SupportPhIGroup+MultiPhotonGroup),

        #ATR-31145 ---- primary special
        ChainProp(name='HLT_e20_lhtight_ivarloose_L1ZAFB-25DPHI-eEM18M', l1SeedThresholds=['eEM18M'], groups=PrimaryPhIGroup+SingleElectronGroup+Topo3Group),
        ChainProp(name='HLT_e20_lhtight_ivarloose_L1ZAFB-25DPHIM-eEM18M', l1SeedThresholds=['eEM18M'], groups=PrimaryPhIGroup+SingleElectronGroup+Topo3Group),

    ]

    chains['MET'] = []

    chains['Tau'] = [
        # ------------------------------------------------------------------
        # Former Physics menu DeepSet triggers (ATR-31420)
        # ------------------------------------------------------------------
        # Single tau primaries
        ChainProp(name='HLT_tau160_mediumRNN_tracktwoMVA_L1eTAU140', groups=PrimaryPhIGroup+SingleTauGroup),
        ChainProp(name='HLT_tau200_mediumRNN_tracktwoMVA_L1eTAU140', groups=PrimaryPhIGroup+SingleTauGroup),


        # Di-tau primaries and support chains
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55', l1SeedThresholds=['cTAU30M', 'cTAU20M'], groups=PrimaryPhIGroup+MultiTauGroup+Topo2Group),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30LeTAU20L-jJ55', l1SeedThresholds=['cTAU30M', 'cTAU20M'], groups=PrimaryPhIGroup+MultiTauGroup+Topo2Group),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20', l1SeedThresholds=['cTAU30M', 'cTAU20M'], groups=EOFTauPhIGroup+MultiTauGroup+Topo2Group),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30LeTAU20L', l1SeedThresholds=['cTAU30M', 'cTAU20M'], groups=SupportPhIGroup+MultiTauGroup+Topo2Group),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB_L1cTAU30M_2cTAU20M_4jJ30p0ETA25', l1SeedThresholds=['cTAU30M', 'cTAU20M'], groups=PrimaryPhIGroup+MultiTauGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB_L1cTAU30M_2cTAU20M', l1SeedThresholds=['cTAU30M', 'cTAU20M'], groups=SupportPhIGroup+MultiTauGroup),

        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_tau35_mediumRNN_tracktwoMVA_03dRAB_L1cTAU35M_2cTAU30M_2jJ55_3jJ50', l1SeedThresholds=['cTAU35M', 'cTAU30M'], groups=PrimaryPhIGroup+MultiTauGroup),
        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_tau35_mediumRNN_tracktwoMVA_03dRAB_L1cTAU35M_2cTAU30M' , l1SeedThresholds=['cTAU35M', 'cTAU30M'], groups=SupportPhIGroup+MultiTauGroup),

        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_tau35_mediumRNN_tracktwoMVA_03dRAB30_L1eTAU80_2cTAU30M_DR-eTAU30eTAU20', l1SeedThresholds=['eTAU80', 'cTAU30M'], groups=PrimaryPhIGroup+MultiTauGroup+Topo2Group),

        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_tau60_mediumRNN_tracktwoMVA_03dRAB_L1eTAU80_2eTAU60', l1SeedThresholds=['eTAU80', 'eTAU60'], groups=PrimaryPhIGroup+MultiTauGroup),


        # Delayed stream di-tau primaries and support chains
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55', l1SeedThresholds=['cTAU30M', 'cTAU20M'], stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiTauGroup+Topo2Group),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20', l1SeedThresholds=['cTAU30M', 'cTAU20M'], stream=['VBFDelayed'], groups=EOFTauPhIGroup+MultiTauGroup+Topo2Group),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L1cTAU30M_2cTAU20M_4jJ30p0ETA25', l1SeedThresholds=['cTAU30M', 'cTAU20M'], stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiTauGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L1cTAU30M_2cTAU20M', l1SeedThresholds=['cTAU30M', 'cTAU20M'], stream=['VBFDelayed'], groups=SupportPhIGroup+MultiTauGroup),


        # Boosted low-pT di-tau chains
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB10_L1cTAU20M_DR-eTAU20eTAU12-jJ40', l1SeedThresholds=['cTAU20M', 'eTAU12'], groups=MultiTauGroup+SupportPhIGroup+Topo2Group),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_02dRAB10_L1cTAU20M_DR-eTAU20eTAU12-jJ40', l1SeedThresholds=['cTAU20M', 'eTAU12'], groups=MultiTauGroup+SupportPhIGroup+Topo2Group),

        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_02dRAB10_L1cTAU20M_DR-eTAU20eTAU12-jJ40', l1SeedThresholds=['cTAU20M', 'eTAU12'], groups=MultiTauGroup+SupportPhIGroup+Topo2Group),

        
        # HH->2b2tau: di-tau support chains (ATR-28890)
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L14jJ30p0ETA24_0DETA24-eTAU30eTAU12", l1SeedThresholds=['eTAU20', 'eTAU12'], groups=SupportPhIGroup+MultiTauGroup+Topo3Group),
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L1cTAU20M_cTAU12M_4jJ30p0ETA24_0DETA24-eTAU30eTAU12", l1SeedThresholds=['cTAU20M', 'cTAU12M'],  groups=SupportPhIGroup+MultiTauGroup+Topo3Group),


        # yy->tautau: g-2 tau measurement chains (ATR-30638)
        ChainProp(name='HLT_tau70_mediumRNN_tau50_mediumRNN_29dphiAB_L1eTAU70_2cTAU50M_DPHI-2eTAU50', l1SeedThresholds=['eTAU70', 'cTAU50M'], groups=PrimaryPhIGroup+MultiTauGroup+Topo3Group),
        ChainProp(name='HLT_2tau50_mediumRNN_29dphiAA_L12cTAU50M_DPHI-2eTAU50', l1SeedThresholds=['cTAU50M'], groups=EOFTauPhIGroup+MultiTauGroup+Topo3Group),


        # Single tau support chains
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_L1eTAU12', groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_eTAU12']),
        ChainProp(name="HLT_tau25_mediumRNN_tracktwoMVA_L1eTAU20", groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_eTAU20']),
        ChainProp(name="HLT_tau25_mediumRNN_tracktwoMVA_L1eTAU20L", groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_eTAU20L']),
        ChainProp(name="HLT_tau25_mediumRNN_tracktwoMVA_L1cTAU20M", groups=SingleTauGroup+SupportPhIGroup+['RATE:CPS_cTAU20M']),
        ChainProp(name="HLT_tau35_mediumRNN_tracktwoMVA_L1cTAU30M", groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_cTAU30M']),
        ChainProp(name="HLT_tau40_mediumRNN_L1cTAU35M", groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_cTAU35M']),
        ChainProp(name='HLT_tau50_mediumRNN_L1cTAU50M', groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_cTAU50M']),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_L1eTAU60', groups=SupportPhIGroup+SingleTauGroup),
        ChainProp(name='HLT_tau70_mediumRNN_L1eTAU70', groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_eTAU70']),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_L1eTAU80', groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_eTAU80']),



        #ATR-29439
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB_L1cTAU30M_2cTAU20M_3jJ30p0ETA25'          , l1SeedThresholds=['cTAU30M','cTAU20M'], stream=['VBFDelayed'], groups=SupportPhIGroup+MultiTauGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L1cTAU30M_2cTAU20M_3jJ30p0ETA25'          , l1SeedThresholds=['cTAU30M','cTAU20M'], stream=['VBFDelayed'], groups=SupportPhIGroup+MultiTauGroup),
        
        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_tau35_mediumRNN_tracktwoMVA_03dRAB_L1eTAU35M_2eTAU30M', l1SeedThresholds=['eTAU35M','eTAU30M'], groups=SupportPhIGroup+MultiTauGroup),

        ChainProp(name="HLT_tau25_idperf_tracktwoMVA_L1eTAU20M", groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_eTAU20M']), #ATR-27013
        ChainProp(name="HLT_tau25_perf_tracktwoMVA_L1eTAU20M", groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_eTAU20M']), #ATR-27013
        ChainProp(name="HLT_tau25_mediumRNN_tracktwoMVA_L1eTAU20M", groups=SupportPhIGroup+SingleTauGroup+['RATE:CPS_eTAU20M']),

        # ATR-31149
        ChainProp(name="HLT_tau0_ptonly_L1eTAU12", groups=SingleTauGroup+SupportPhIGroup+['RATE:CPS_eTAU12']),
        ChainProp(name="HLT_tau0_ptonly_L1eTAU80", groups=SingleTauGroup+SupportPhIGroup+['RATE:CPS_eTAU80']),
    ]

    chains['Bphysics'] = [

        #ATR-21566, chains for di-muon TLA, but with HLT selections to test rates. Here streaming into BphysDelayed (not in TLA stream)   
        ChainProp(name='HLT_2mu4_b0dRAB207invmAB22vtx20_L1BPH-7M22-0DR20-2MU3V', l1SeedThresholds=['MU3V'],stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+Topo3Group),
        ChainProp(name='HLT_2mu4_b0dRAB207invmAB22vtx20_L1BPH-7M22-0DR20-2MU3VF', l1SeedThresholds=['MU3VF'],stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+Topo3Group),
        ChainProp(name='HLT_2mu4_b0dRAB127invmAB22vtx20_L1BPH-7M22-0DR12-2MU3V', l1SeedThresholds=['MU3V'],stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+Topo3Group),
    ]

    chains['Streaming'] += [
        ChainProp(name='HLT_noalg_L1All', l1SeedThresholds=['FSNOSEED'], groups=['Primary:CostAndRate', 'RATE:SeededStreamers', 'BW:Other']), # ATR-22072, for rates in MC.

        ChainProp(name='HLT_noalg_L1eTAU20M',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportPhIGroup+TauPhaseIStreamersGroup),
    ]

    chains['Calib'] += [

    ]

    chains['Combined'] += [
        # ATR-31537
        ChainProp(name='HLT_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_L1BTAG-MU3VFjJ40_2jJ40p0ETA25'            , l1SeedThresholds=['FSNOSEED']*5,  stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup+Topo2Group),
        ChainProp(name='HLT_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L1BTAG-MU3VFjJ40_2jJ40p0ETA25'            , l1SeedThresholds=['FSNOSEED']*5,  stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup+Topo2Group),
        ChainProp(name='HLT_j55c_nnJvtv1_j40c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_2j20c_nnJvtv1_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L1BTAG-MU3VFjJ40_2jJ40p0ETA25'            , l1SeedThresholds=['FSNOSEED']*5,  stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup+Topo2Group),
        ChainProp(name='HLT_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_L1BTAG-MU5VFjJ40_2jJ40p0ETA25'            , l1SeedThresholds=['FSNOSEED']*5,  stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup+Topo2Group),
        ChainProp(name='HLT_mu6_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_L1BTAG-MU5VFjJ40_2jJ40p0ETA25'                         , l1SeedThresholds=['MU3VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup+Topo2Group),
        ChainProp(name='HLT_mu6_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn277_j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_dRAF04_L1BTAG-MU5VFjJ40_2jJ40p0ETA25', l1SeedThresholds=['MU3VF']+['FSNOSEED']*6,  stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup+Topo2Group),

        ChainProp(name='HLT_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L1BTAG-MU5VFjJ40_2jJ40p0ETA25'            , l1SeedThresholds=['FSNOSEED']*5,  stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup+Topo2Group),
        ChainProp(name='HLT_mu6_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L1BTAG-MU5VFjJ40_2jJ40p0ETA25'                         , l1SeedThresholds=['MU3VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup+Topo2Group),
        ChainProp(name='HLT_mu6_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn277_j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_dRAF04_L1BTAG-MU5VFjJ40_2jJ40p0ETA25', l1SeedThresholds=['MU3VF']+['FSNOSEED']*6,  stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup+Topo2Group),

        ChainProp(name='HLT_j55c_nnJvtv1_j40c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_2j20c_nnJvtv1_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L1BTAG-MU5VFjJ40_2jJ40p0ETA25'            , l1SeedThresholds=['FSNOSEED']*5,  stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup+Topo2Group),
        ChainProp(name='HLT_mu6_j55c_nnJvtv1_j40c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_2j20c_nnJvtv1_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L1BTAG-MU5VFjJ40_2jJ40p0ETA25'                         , l1SeedThresholds=['MU3VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup+Topo2Group),
        ChainProp(name='HLT_mu6_j55c_nnJvtv1_j40c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_j20c_nnJvtv1_bgn277_j20c_nnJvtv1_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_dRAF04_L1BTAG-MU5VFjJ40_2jJ40p0ETA25', l1SeedThresholds=['MU3VF']+['FSNOSEED']*6,  stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup+Topo2Group),
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

        ChainProp(name='HLT_mu14_ivarloose_tau35_mediumRNN_tracktwoMVA_03dRAB_L1MU8F_cTAU30M', l1SeedThresholds=['MU8F','cTAU30M'], stream=[PhysicsStream], groups=PrimaryPhIGroup+MuonTauGroup),
        
        # eTAU20M T&P chains (unused in 2025)
        ChainProp(name='HLT_mu26_ivarmedium_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU20M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU20M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU20M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU20M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU20M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU20M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU20M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU20M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU20M_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeTAU20M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU20M_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeTAU20M'], stream=[PhysicsStream], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU20M_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU20M','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU20M_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU20M','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU20M_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU20M','FSNOSEED','FSNOSEED'],  groups=TagAndProbePhIGroup+TauMETGroup),

        # ------------------------------------------------------------------
        # Former Physics menu DeepSet tau triggers (ATR-31420)
        # ------------------------------------------------------------------
        # Tau + Muon primaries
        ChainProp(name='HLT_mu14_ivarloose_tau25_mediumRNN_tracktwoMVA_03dRAB_L1MU8F_cTAU20M_3jJ30', l1SeedThresholds=['MU8F','cTAU20M'], groups=PrimaryPhIGroup+MuonTauGroup),
        ChainProp(name='HLT_mu14_ivarloose_tau35_mediumRNN_tracktwoMVA_03dRAB_L1cTAU30M_3DR35-MU8F-eTAU30', l1SeedThresholds=['MU8F', 'cTAU30M'], groups=PrimaryPhIGroup+MuonTauGroup+Topo3Group),
        ChainProp(name='HLT_mu20_ivarloose_tau20_mediumRNN_tracktwoMVA_L1eTAU12_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH', 'eTAU12'], groups=PrimaryPhIGroup+MuonTauGroup),
        ChainProp(name='HLT_mu23_ivarloose_tau20_mediumRNN_tracktwoMVA_L1eTAU12_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH', 'eTAU12'], groups=PrimaryPhIGroup+MuonTauGroup),


        # Tau + Electron primaries
        ChainProp(name='HLT_e17_lhmedium_ivarloose_tau25_mediumRNN_tracktwoMVA_03dRAB_L1eEM18M_2cTAU20M_4jJ30', l1SeedThresholds=['eEM18M', 'cTAU20M'], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_e24_lhmedium_ivarloose_tau20_mediumRNN_tracktwoMVA_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M', 'eTAU12'], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_e24_lhmedium_ivarloose_tau20_mediumRNN_tracktwoMVA_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M', 'eTAU12'], groups=PrimaryPhIGroup+EgammaTauGroup),


        # Tau + MET primaries
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_xe80_tcpufit_xe50_cell_L1jXE100', l1SeedThresholds=['cTAU35M','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_xe80_pfopufit_xe50_cell_L1jXE100', l1SeedThresholds=['cTAU35M','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_xe80_tcpufit_xe50_cell_L1gXEJWOJ100', l1SeedThresholds=['cTAU35M','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_xe80_pfopufit_xe50_cell_L1gXEJWOJ100', l1SeedThresholds=['cTAU35M','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_xe80_tcpufit_xe50_cell_L1jXE110', l1SeedThresholds=['cTAU35M','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_xe80_pfopufit_xe50_cell_L1jXE110', l1SeedThresholds=['cTAU35M','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_xe80_tcpufit_xe50_cell_L1gXEJWOJ110', l1SeedThresholds=['cTAU35M','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_xe80_pfopufit_xe50_cell_L1gXEJWOJ110', l1SeedThresholds=['cTAU35M','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),


        # Di-tau (Tau + Muon/Electron/Tau) + MET primaries
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_xe50_cell_03dRAB_L1eTAU60_2cTAU20M_jXE80', l1SeedThresholds=['eTAU60', 'cTAU20M', 'FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),
        ChainProp(name='HLT_e17_lhmedium_tau25_mediumRNN_tracktwoMVA_xe50_cell_03dRAB_L1eEM18M_2cTAU20M_jXE70', l1SeedThresholds=['eEM18M', 'cTAU20M', 'FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),
        ChainProp(name='HLT_mu14_tau25_mediumRNN_tracktwoMVA_xe50_cell_03dRAB_L1MU8F_cTAU20M_jXE70', l1SeedThresholds=['MU8F', 'cTAU20M', 'FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),

        # Mu + Tau T&P chains
        ChainProp(name='HLT_mu26_ivarmedium_tau20_mediumRNN_tracktwoMVA_probe_L1eTAU12_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU12'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU12M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU12M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU12'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU20_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau25_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau30_mediumRNN_tracktwoMVA_probe_L1eTAU20_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU30M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau35_mediumRNN_tracktwoMVA_probe_L1cTAU30M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau40_mediumRNN_tracktwoMVA_probe_L1cTAU35M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU35M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU35M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU35M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU50M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU50M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau60_mediumRNN_tracktwoMVA_probe_L1eTAU60_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU60'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau70_mediumRNN_tracktwoMVA_probe_L1eTAU70_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU70'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau80_mediumRNN_tracktwoMVA_probe_L1eTAU80_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU80'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau160_mediumRNN_tracktwoMVA_probe_L1eTAU140_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU140'], groups=TagAndProbePhIGroup+SingleMuonGroup),

        ChainProp(name='HLT_mu26_ivarmedium_tau20_mediumRNN_tracktwoMVA_probe_L1eTAU12_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU12'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU12M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU12M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU12'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU20_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau25_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau30_mediumRNN_tracktwoMVA_probe_L1eTAU20_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU30M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau35_mediumRNN_tracktwoMVA_probe_L1cTAU30M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau40_mediumRNN_tracktwoMVA_probe_L1cTAU35M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU35M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU35M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU35M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU50M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU50M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau60_mediumRNN_tracktwoMVA_probe_L1eTAU60_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU60'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau70_mediumRNN_tracktwoMVA_probe_L1eTAU70_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU70'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau80_mediumRNN_tracktwoMVA_probe_L1eTAU80_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU80'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau160_mediumRNN_tracktwoMVA_probe_L1eTAU140_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU140'], groups=TagAndProbePhIGroup+SingleMuonGroup),

        ChainProp(name='HLT_mu24_ivarmedium_tau20_mediumRNN_tracktwoMVA_probe_L1eTAU12_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU12'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU12M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU12M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU12'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU20_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau25_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau30_mediumRNN_tracktwoMVA_probe_L1eTAU20_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU30M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau35_mediumRNN_tracktwoMVA_probe_L1cTAU30M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau40_mediumRNN_tracktwoMVA_probe_L1cTAU35M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU35M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU35M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU35M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU50M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEcTAU50M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau60_mediumRNN_tracktwoMVA_probe_L1eTAU60_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU60'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau70_mediumRNN_tracktwoMVA_probe_L1eTAU70_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU70'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau80_mediumRNN_tracktwoMVA_probe_L1eTAU80_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU80'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau160_mediumRNN_tracktwoMVA_probe_L1eTAU140_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBEeTAU140'], groups=TagAndProbePhIGroup+SingleMuonGroup),

        ChainProp(name='HLT_mu24_ivarmedium_tau20_mediumRNN_tracktwoMVA_probe_L1eTAU12_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU12'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU12M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU12M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU12'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU20_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau25_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau30_mediumRNN_tracktwoMVA_probe_L1eTAU20_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU30M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau35_mediumRNN_tracktwoMVA_probe_L1cTAU30M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau40_mediumRNN_tracktwoMVA_probe_L1cTAU35M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU35M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU35M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU35M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU50M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEcTAU50M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau60_mediumRNN_tracktwoMVA_probe_L1eTAU60_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU60'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau70_mediumRNN_tracktwoMVA_probe_L1eTAU70_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU70'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau80_mediumRNN_tracktwoMVA_probe_L1eTAU80_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU80'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau160_mediumRNN_tracktwoMVA_probe_L1eTAU140_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU140'], groups=TagAndProbePhIGroup+SingleMuonGroup),


        # Electron + Tau T&P chains
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau20_mediumRNN_tracktwoMVA_probe_L1eTAU12_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeTAU12'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU12M_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEcTAU12M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeTAU12'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU20_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau25_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau30_mediumRNN_tracktwoMVA_probe_L1eTAU20_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU30M_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau35_mediumRNN_tracktwoMVA_probe_L1cTAU30M_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau40_mediumRNN_tracktwoMVA_probe_L1cTAU35M_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEcTAU35M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU35M_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEcTAU35M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU50M_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEcTAU50M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau60_mediumRNN_tracktwoMVA_probe_L1eTAU60_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeTAU60'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau70_mediumRNN_tracktwoMVA_probe_L1eTAU70_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeTAU70'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau80_mediumRNN_tracktwoMVA_probe_L1eTAU80_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeTAU80'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau160_mediumRNN_tracktwoMVA_probe_L1eTAU140_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeTAU140'], groups=TagAndProbePhIGroup+SingleElectronGroup),

        ChainProp(name='HLT_e28_lhtight_ivarloose_tau20_mediumRNN_tracktwoMVA_probe_L1eTAU12_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeTAU12'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU12M_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEcTAU12M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeTAU12'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU20_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau25_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau30_mediumRNN_tracktwoMVA_probe_L1eTAU20_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU30M_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau35_mediumRNN_tracktwoMVA_probe_L1cTAU30M_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau40_mediumRNN_tracktwoMVA_probe_L1cTAU35M_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEcTAU35M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU35M_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEcTAU35M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU50M_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEcTAU50M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau60_mediumRNN_tracktwoMVA_probe_L1eTAU60_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeTAU60'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau70_mediumRNN_tracktwoMVA_probe_L1eTAU70_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeTAU70'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau80_mediumRNN_tracktwoMVA_probe_L1eTAU80_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeTAU80'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau160_mediumRNN_tracktwoMVA_probe_L1eTAU140_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeTAU140'], groups=TagAndProbePhIGroup+SingleElectronGroup),


        # Photon + Tau T&P chains (ATR-28791)
        ChainProp(name='HLT_g140_loose_tau20_mediumRNN_tracktwoMVA_probe_L1eTAU12_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBEeTAU12'], groups=TagAndProbePhIGroup+TauPhotonGroup),
        ChainProp(name='HLT_g140_loose_tau20_mediumRNN_tracktwoMVA_probe_L1eTAU12_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBEeTAU12'], groups=TagAndProbePhIGroup+TauPhotonGroup),


        # MET + Tau T&P chains (ATR-23507)
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU12','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU12M_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU12M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU20M_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU20M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU12','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU20_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU20','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU20M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1eTAU20_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU20M_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU30M_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU30M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU35M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU35M_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU50M_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU50M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau70_mediumRNN_tracktwoMVA_probe_L1eTAU70_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU70', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU12','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU12M_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU12M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU20M_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU20M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU12','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU20_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU20','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU20M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1eTAU20_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU20M_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU30M_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU30M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU35M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU35M_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU50M_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU50M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau70_mediumRNN_tracktwoMVA_probe_L1eTAU70_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU70', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU12','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU12M_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU12M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU20M_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU20M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU12','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU20_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU20','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU20M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1eTAU20_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU20M_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU30M_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU30M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU35M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU35M_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU50M_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU50M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau70_mediumRNN_tracktwoMVA_probe_L1eTAU70_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU70', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU12','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU12M_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU12M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU20M_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU20M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU12','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU20_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU20','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU20M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1eTAU20_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU20M_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU30M_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU30M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU35M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU35M_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU50M_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU50M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau70_mediumRNN_tracktwoMVA_probe_L1eTAU70_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU70', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU12','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU12M_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU12M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU20M_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU20M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU12','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU20_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU20','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU20M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1eTAU20_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU20M_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU30M_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU30M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU35M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU35M_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU50M_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU50M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau70_mediumRNN_tracktwoMVA_probe_L1eTAU70_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU70', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU12','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU12M_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU12M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU20M_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU20M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU12','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU20_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU20','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU20M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1eTAU20_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU20M_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU30M_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU30M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU35M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU35M_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU50M_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU50M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau70_mediumRNN_tracktwoMVA_probe_L1eTAU70_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU70', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU12','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU12M_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU12M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU20M_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU20M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU12','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU20_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU20','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU20M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1eTAU20_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU20M_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU30M_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU30M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU35M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU35M_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU50M_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU50M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau70_mediumRNN_tracktwoMVA_probe_L1eTAU70_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU70', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU12','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU12M_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU12M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU20M_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU20M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU12','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU20_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU20','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU20M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1eTAU20_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU20M_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU30M_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU30M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU35M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU35M_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU50M_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU50M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau70_mediumRNN_tracktwoMVA_probe_L1eTAU70_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU70', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU12','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU12M_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU12M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU20M_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU20M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU12','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU20_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU20','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU20M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1eTAU20_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU20M_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU30M_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU30M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU35M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU35M_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU50M_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU50M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau70_mediumRNN_tracktwoMVA_probe_L1eTAU70_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU70', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU12','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU12M_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU12M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU20M_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU20M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU12','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU20_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU20','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU20M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1eTAU20_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU20M_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU30M_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU30M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU35M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU35M_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU50M_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU50M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau70_mediumRNN_tracktwoMVA_probe_L1eTAU70_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU70', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU12','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU12M_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU12M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU20M_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU20M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU12','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU20_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU20','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU20M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1eTAU20_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU20M_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU30M_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU30M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU35M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU35M_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU50M_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU50M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau70_mediumRNN_tracktwoMVA_probe_L1eTAU70_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU70', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU12','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU12M_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU12M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU20M_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU20M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU12','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU20_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU20','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU20M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1eTAU20_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU20M_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU30M_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU30M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU35M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU35M_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumRNN_tracktwoMVA_probe_L1cTAU50M_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU50M','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU60','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau70_mediumRNN_tracktwoMVA_probe_L1eTAU70_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU70', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU80','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU140','FSNOSEED','FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),

        
        # Jet + Tau T&P chains (ATR-24031)
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_L1RD0_FILLED', l1SeedThresholds=['eTAU12'], groups=SupportPhIGroup+TauJetGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_j25_pf_ftf_03dRAB_L1RD0_FILLED', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=SupportPhIGroup+TauJetGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_j35_pf_ftf_03dRAB_L1RD0_FILLED', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=SupportPhIGroup+TauJetGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_j45_pf_ftf_preselj20_03dRAB_L1jJ40', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=SupportPhIGroup+TauJetGroup+['RATE:CPS_jJ40']),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_j60_pf_ftf_preselj50_03dRAB_L1jJ50', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=SupportPhIGroup+TauJetGroup+['RATE:CPS_jJ50']),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_j85_pf_ftf_preselj50_03dRAB_L1jJ50', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=SupportPhIGroup+TauJetGroup+['RATE:CPS_jJ50']),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_j110_pf_ftf_preselj80_03dRAB_L1jJ60', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=SupportPhIGroup+TauJetGroup+['RATE:CPS_jJ60']),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_j175_pf_ftf_preselj140_03dRAB_L1jJ90', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=SupportPhIGroup+TauJetGroup+['RATE:CPS_jJ90']),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_j260_pf_ftf_preselj200_03dRAB_L1jJ125', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=SupportPhIGroup+TauJetGroup+['RATE:CPS_jJ125']),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_j360_pf_ftf_preselj225_03dRAB_L1jJ160', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=SupportPhIGroup+TauJetGroup+['RATE:CPS_jJ160']),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_j420_pf_ftf_preselj225_03dRAB_L1jJ160', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauJetGroup),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_j440_pf_ftf_preselj225_03dRAB_L1jJ160', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauJetGroup),
        

        # HH->2b2tau: Bjet + Tau primaries, including backups for additional CPU savings (ATR-28198)
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_j65c_nnJvtv1_j40c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_j20c_nnJvtv1_bgn285_pf_ftf_presel3c20XX1c20bgtwo85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_j65c_nnJvtv1_j40c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_j20c_nnJvtv1_bgn285_pf_ftf_presel2c20XX1c20bgtwo85XX1c20gntau90_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_j65c_nnJvtv1_j40c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_j20c_nnJvtv1_bgn285_pf_ftf_presel2c20XX1c20bgtwo85XX1c20gntau85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_j65c_nnJvtv1_j40c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_j20c_nnJvtv1_bgn285_pf_ftf_presel2c20XX1c20bgtwo82XX1c20gntau85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_j65c_nnJvtv1_j40c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_j20c_nnJvtv1_bgn285_pf_ftf_presel2c20XX1c20bgtwo82XX1c20gntau80_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_j65c_nnJvtv1_j40c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_j20c_nnJvtv1_bgn285_pf_ftf_presel2c20XX1c20bgtwo80XX1c20gntau80_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], groups=PrimaryPhIGroup+TauBJetGroup),

        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel3c20XX1c20bgtwo85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo85XX1c20gntau90_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo85XX1c20gntau85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo82XX1c20gntau85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo82XX1c20gntau80_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo80XX1c20gntau80_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], groups=PrimaryPhIGroup+TauBJetGroup),

        # EoF chains (ATR-29523)
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_j50c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel3c20XX1c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], groups=EOFTauBjetGroup+TauBJetGroup),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_L1eTAU12_j50c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel3c20XX1c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], groups=EOFTauBjetGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_probe_L1eTAU12_2j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel3c20XX1c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+4*['FSNOSEED'], groups=EOFTauBjetGroup+TauBJetGroup),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_L1eTAU12_2j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel3c20XX1c20bgtwo85_L13jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+4*['FSNOSEED'], groups=EOFTauBjetGroup+TauBJetGroup),
        

        # VBF triggers (ATR-22594)
        # Main stream
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_j70_j50a_j0_DJMASS900j50_L1jMJJ-500-NFF', l1SeedThresholds=['eTAU12','eTAU12','FSNOSEED','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+TauJetGroup+Topo3Group),

    ]

    chains['UnconventionalTracking'] = [
        # Phase I L1Calo inputs
        # hit-based DV  
        #ATR-31122
        ChainProp(name='HLT_hitdvjet260_tight_L1jJ160', groups=PrimaryPhIGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_hitdvjet260_medium_L1jJ160', groups=SupportPhIGroup+UnconvTrkGroup, l1SeedThresholds=['FSNOSEED']),
    ]


    return chains

def setupMenu():
    
    chains = physics_menu.setupMenu()

    log.info('[setupMenu] going to add the MC menu chains now')

    for sig,chainsInSig in getMCSignatures().items():
        chains[sig] += chainsInSig

    return chains
