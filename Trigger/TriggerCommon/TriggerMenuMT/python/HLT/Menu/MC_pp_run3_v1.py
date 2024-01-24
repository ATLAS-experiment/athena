# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# MC_pp_run3_v1.py menu for the long shutdown development
#------------------------------------------------------------------------#

# This defines the input format of the chain and it's properties with the defaults set
# always required are: name, stream and groups
#['name', 'L1chainParts'=[], 'stream', 'groups', 'merging'=[], 'topoStartFrom'=False],
from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp
from .SignatureDicts import ChainStore

import TriggerMenuMT.HLT.Menu.Physics_pp_run3_v1 as physics_menu 
from TriggerMenuMT.HLT.Menu.Physics_pp_run3_v1 import ( SingleElectronGroup,
                                                        SinglePhotonGroup,
                                                        SingleTauGroup,
                                                        MultiTauGroup,
                                                        TauJetGroup,
                                                        TauBJetGroup,
                                                        TauMETGroup,
                                                        TauStreamersGroup,
                                                        BphysicsGroup,
                                                        EOFBPhysL1MuGroup,
                                                        EOFL1MuGroup,
                                                        EgammaMETGroup,
                                                        MultiJetGroup,
                                                        PrimaryLegGroup,
                                                        PrimaryPhIGroup,
                                                        PrimaryL1MuGroup,
                                                        TagAndProbeLegGroup,
                                                        TagAndProbePhIGroup,
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
)

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

        # low threshold single jet support chains with JVT
        ChainProp(name='HLT_j45_020jvt_pf_ftf_preselj20_L1J15', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup),

        ChainProp(name="HLT_j20_roiftf_preselj20_L1RD0_FILLED", l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),

        # Low-threshold calibration Large-R jets
        # Non-commissioning L1 thresholds
        ChainProp(name='HLT_j85_a10sd_cssk_pf_nojcalib_ftf_preselj50_L1jLJ60', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jLJ60']),
        ChainProp(name='HLT_j85_a10sd_cssk_pf_jes_ftf_preselj50_L1jLJ60', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jLJ60']),
        ChainProp(name='HLT_j85_a10t_lcw_nojcalib_L1jLJ60', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jLJ60']),
        ChainProp(name='HLT_j85_a10t_lcw_jes_L1jLJ60',      l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jLJ60']),
        ChainProp(name='HLT_j175_a10sd_cssk_pf_jes_ftf_preselj140_L1jLJ100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jLJ100']),
        ChainProp(name='HLT_j175_a10t_lcw_jes_L1jLJ100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jLJ100']),

        ## ATR-25456 - Emerging jet
        # alternate emerging jet single-jet chain
        ChainProp(name='HLT_j200_0eta200_tracklessdR1p2_a10r_subjesIS_ftf_preselj200_L1J100', groups=SingleJetGroup+PrimaryLegGroup, l1SeedThresholds=['FSNOSEED']),    
        # backup Trackless jet chain
        ChainProp(name='HLT_j260_0eta200_tracklessdR1p2_a10r_subjesIS_ftf_preselj200_L1J100', groups=SingleJetGroup+PrimaryLegGroup, l1SeedThresholds=['FSNOSEED']),    

        ## ATR-25456 - calratio jet chains
        
        #ChainProp(name='HLT_j30_CLEANllp_calratio_L1TAU60', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup),
        #ChainProp(name='HLT_j30_CLEANllp_calratiormbib_L1TAU60', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup),
        #ChainProp(name='HLT_j30_CLEANllp_calratio_L1eTAU80', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        #ChainProp(name='HLT_j30_CLEANllp_calratiormbib_L1eTAU80', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),

        # Calratio Legacy primary chains
        #ChainProp(name='HLT_j30_CLEANllp_calratio_L1TAU100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryLegGroup),
        #ChainProp(name='HLT_j30_CLEANllp_calratiormbib_L1TAU100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j20_calratio_roiftf_preselj20emf24_L1TAU100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup),
        ChainProp(name='HLT_j20_calratio_roiftf_preselj20emf24_L1TAU60', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup),

        # Calratio Legacy support chains
        #ChainProp(name='HLT_j30_CLEANllp_calratio_L1TAU40_EMPTY', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup),
        #ChainProp(name='HLT_j30_CLEANllp_calratiormbib_L1TAU40_EMPTY', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup),
        #ChainProp(name='HLT_j30_CLEANllp_calratio_L1TAU40_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup),
        #ChainProp(name='HLT_j30_CLEANllp_calratiormbib_L1TAU40_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j20_calratio_roiftf_preselj20emf24_L1TAU40_EMPTY', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup),
        ChainProp(name='HLT_j20_calratio_roiftf_preselj20emf24_L1TAU40_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup),
      
        # supporting chains for background studies
      

        ## ATR-25456 - Central Exclusive Production for SM group
        ChainProp(name='HLT_2j100_L1CEP-CjJ90', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),
        ChainProp(name='HLT_2j100_L1CEP-CjJ100', l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+MultiJetGroup+Topo3Group),

        ## ATR-27238 - Wrong L1 jJ thresholds; moving from Physics to MC for compatibility with 2023 menu 
        ChainProp(name='HLT_j60_L1jJ90', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ90']),
        ChainProp(name='HLT_j85_L1jJ90', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ90']),


        ## ATR-27270 - Downshifted chains 
        ChainProp(name='HLT_j420_pf_ftf_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J100']),
        ChainProp(name='HLT_j420_pf_ftf_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup+['RATE:CPS_jJ160']),
        ChainProp(name='HLT_j420_pf_ftf_preselj225_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j420_pf_ftf_preselj225_L1J120', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j420_pf_ftf_preselj225_L1jJ180', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),

        ChainProp(name='HLT_2j250c_j120c_pf_ftf_presel2j180XXj80_L1J100', l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup + PrimaryLegGroup),
        ChainProp(name='HLT_3j200_pf_ftf_presel3j150_L1J100', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryLegGroup),
        ChainProp(name='HLT_4j115_pf_ftf_presel4j85_L13J50', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryLegGroup),
        ChainProp(name='HLT_5j70c_pf_ftf_presel5c50_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryLegGroup),
        ChainProp(name='HLT_5j70c_pf_ftf_presel5c50_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_5j85_pf_ftf_presel5j50_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryLegGroup),
        ChainProp(name='HLT_5j85_pf_ftf_presel5j55_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryLegGroup),
        ChainProp(name='HLT_5j70c_pf_ftf_presel5c55_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryLegGroup),
        ChainProp(name='HLT_5j70c_pf_ftf_presel5c55_L14jJ40', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_6j70_pf_ftf_presel6j40_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryLegGroup),
        ChainProp(name='HLT_6j70_pf_ftf_presel6j45_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryLegGroup),
        ChainProp(name='HLT_10j40_pf_ftf_presel7j30_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryLegGroup),
        
        ChainProp(name='HLT_3j200_pf_ftf_L1J100', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+SupportLegGroup+['RATE:CPS_J100']),
        ChainProp(name='HLT_3j200_pf_ftf_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+SupportPhIGroup+['RATE:CPS_jJ160']),

        ChainProp(name='HLT_j0_HT1000_pf_ftf_preselj180_L1J100', l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleJetGroup),
        ChainProp(name='HLT_j0_HT1000_pf_ftf_preselcHT450_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleJetGroup+LegacyTopoGroup, monGroups=['caloMon:t0']),
        ChainProp(name='HLT_j0_HT1000_pf_ftf_preselj190_L1J100', l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleJetGroup),
        ChainProp(name='HLT_j0_HT1000_pf_ftf_preselj200_L1J100', l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleJetGroup),
        ChainProp(name='HLT_j0_HT1000_pf_ftf_preselj180_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleJetGroup+LegacyTopoGroup),

        ChainProp(name='HLT_j420_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryPhIGroup),

        ChainProp(name='HLT_2j250c_j120c_pf_ftf_presel2j180XXj80_L1jJ160', l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_3j200_pf_ftf_presel3j150_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),
        ChainProp(name='HLT_4j115_pf_ftf_presel4j85_L13jJ90', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup + PrimaryPhIGroup),
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

    ]

    chains['Bjet'] = [
        # B-tagger training
        ChainProp(name='HLT_j20_0eta290_020jvt_boffperf_pf_ftf_L1J15', l1SeedThresholds=['FSNOSEED'], groups=SingleBjetGroup+SupportLegGroup, monGroups=['bJetMon:online']),
        ChainProp(name='HLT_j20_0eta290_020jvt_boffperf_pf_ftf_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleBjetGroup+SupportLegGroup, monGroups=['bJetMon:online']),
        ChainProp(name='HLT_j20_0eta290_020jvt_boffperf_pf_ftf_L1jJ40', l1SeedThresholds=['FSNOSEED'], groups=SingleBjetGroup+SupportPhIGroup, monGroups=['bJetMon:online']),
        # HH4b
        # Muon seeded candidates
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1d82_pf_ftf_presel2c20XX2c20b85_L1MU8F_2J15_J20', l1SeedThresholds=['FSNOSEED']*5, groups=PrimaryLegGroup+MultiBjetGroup),
 # tighter preselection backup - b80
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1d82_pf_ftf_presel2c20XX2c20b80_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=PrimaryLegGroup+MultiBjetGroup), # downshift
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1d82_pf_ftf_presel2c20XX2c20b80_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=PrimaryLegGroup+MultiBjetGroup),        
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d77_pf_ftf_presel2c20XX2c20b80_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=PrimaryLegGroup+MultiBjetGroup), # downshift
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d77_pf_ftf_presel2c20XX2c20b80_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=PrimaryLegGroup+MultiBjetGroup),

    ]

    chains['Egamma'] = [
        ChainProp(name='HLT_e5_lhtight_L1EM3', groups=SingleElectronGroup),
        ChainProp(name='HLT_e5_lhtight_noringer_L1EM3', groups=SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_L1eEM26', groups=SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_L1eEM26L', groups=SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_L1eEM26T', groups=SingleElectronGroup),
        #------------ dnn chains
        ChainProp(name='HLT_e5_dnnloose_L1EM3', groups=SingleElectronGroup),
        ChainProp(name='HLT_e5_dnnmedium_L1EM3', groups=SingleElectronGroup),
        ChainProp(name='HLT_e5_dnntight_L1EM3', groups=SingleElectronGroup),

        # ATR-24268, K*ee chains for rate and acceptance studies
        ChainProp(name='HLT_e5_lhvloose_bBeeM6000_L1All', l1SeedThresholds=['EM3'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        ChainProp(name='HLT_2e5_lhvloose_bBeeM6000_L1All', l1SeedThresholds=['EM3'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        ChainProp(name='HLT_2e5_lhvloose_bBeeM6000_L1eEM26M', l1SeedThresholds=['eEM5'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        ChainProp(name='HLT_e5_lhvloose_bBeeM6000_L1eEM26M', l1SeedThresholds=['eEM5'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        ChainProp(name='HLT_2e5_lhvloose_bBeeM6000_L1BPH-0M9-EM7-EM5_MU5VF', l1SeedThresholds=['EM3'], stream=['BphysDelayed'], groups=BphysElectronGroup+LegacyTopoGroup),
        ChainProp(name='HLT_e5_lhvloose_bBeeM6000_L1BPH-0DR3-EM7J15_MU5VF', l1SeedThresholds=['EM3'], stream=['BphysDelayed'], groups=BphysElectronGroup+LegacyTopoGroup),
        ChainProp(name='HLT_2e5_lhvloose_bBeeM6000_L14J15', l1SeedThresholds=['EM3'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        ChainProp(name='HLT_e5_lhvloose_bBeeM6000_L14J15', l1SeedThresholds=['EM3'], stream=['BphysDelayed'], groups=BphysElectronGroup),        


        # ATR-27780
        ChainProp(name='HLT_g5_nopid_L1EM3', groups=SinglePhotonGroup+['PS:NoBulkMCProd']),
        ChainProp(name='HLT_g5_nopid_L1eEM5', groups=SinglePhotonGroup+['PS:NoBulkMCProd']),

        #ATR-25764 - adding Photon chains with different isolation WPs
        ChainProp(name='HLT_g25_tight_icaloloose_L1eEM26M', groups=SinglePhotonGroup, monGroups=['egammaMon:shifter']),
        ChainProp(name='HLT_g25_tight_icalotight_L1eEM26M', groups=SinglePhotonGroup, monGroups=['egammaMon:shifter']),

        #------------ nopid trigger and etcut from ATR-26311
        # ATR-23723
        ChainProp(name='HLT_e5_nopid_L1eEM5', groups=SingleElectronGroup+['PS:NoBulkMCProd']), #ATR-27264

        #  ATR-26311
        #  Validating/checking eFEX and primary electron trigger
        ChainProp(name='HLT_e26_etcut_L1eEM26M', groups=SingleElectronGroup),

        # Ringer development / validation also ATR-24384
        ChainProp(name='HLT_g20_loose_ringer_L1eEM18M', groups=SinglePhotonGroup,monGroups=['egammaMon:shifter']),
        ChainProp(name='HLT_g20_medium_ringer_L1eEM18M', groups=SinglePhotonGroup,monGroups=['egammaMon:shifter']),
        ChainProp(name='HLT_g20_tight_ringer_L1eEM18M', groups=SinglePhotonGroup,monGroups=['egammaMon:shifter']),
        ChainProp(name='HLT_g120_loose_ringer_L1eEM26M', groups=SinglePhotonGroup,monGroups=['egammaMon:shifter']),
        ChainProp(name='HLT_g20_loose_L1eEM18M',  groups=SinglePhotonGroup,monGroups=['egammaMon:shifter']),

        ChainProp(name='HLT_g20_medium_L1eEM18M', groups=SinglePhotonGroup,monGroups=['egammaMon:shifter']),
        
        #  ATR-27156
        #  Migration of legacy EM seeded items to Phase 1 eEM seeded - EGamma chains
        ChainProp(name='HLT_e26_lhtight_ivarloose_nogsf_L1eEM26M', groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM26M']), #Phase-1
        ChainProp(name='HLT_e60_lhmedium_nogsf_L1eEM26M', groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM26M']), #Phase-1
        ChainProp(name='HLT_g25_tight_icalotight_L1eEM24L', groups=SupportPhIGroup+SinglePhotonGroup+['RATE:CPS_eEM24L']), #Phase-1
        ChainProp(name='HLT_g25_tight_icalomedium_L1eEM24L', groups=SupportPhIGroup+SinglePhotonGroup+['RATE:CPS_eEM24L']),#Phase-1
        
    ]

    chains['Tau'] = [
        # Primary single-tau Legacy chains
        ChainProp(name='HLT_tau160_mediumRNN_tracktwoMVA_L1TAU100', groups=PrimaryLegGroup+SingleTauGroup, monGroups=['tauMon:online','tauMon:t0']),
        ChainProp(name='HLT_tau200_mediumRNN_tracktwoMVA_L1TAU100', groups=PrimaryLegGroup+SingleTauGroup),

        # Support single-tau Legacy chains
        ChainProp(name="HLT_tau160_idperf_tracktwoMVA_L1TAU100", groups=SingleTauGroup+SupportLegGroup+['RATE:CPS_TAU100'], monGroups=['tauMon:online','tauMon:t0','idMon:t0']),
        ChainProp(name="HLT_tau160_perf_tracktwoMVA_L1TAU100", groups=SingleTauGroup+SupportLegGroup+['RATE:CPS_TAU100'], monGroups=['tauMon:online','tauMon:t0']),
        ChainProp(name="HLT_tau160_ptonly_L1TAU100", groups=SingleTauGroup),

        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_L1TAU60', groups=SupportLegGroup+SingleTauGroup+['RATE:CPS_TAU60'], monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau80_idperf_tracktwoMVA_L1TAU60", groups=SingleTauGroup+SupportLegGroup+['RATE:CPS_TAU60'], monGroups=['idMon:t0']),
        ChainProp(name="HLT_tau0_ptonly_L1TAU60", groups=SingleTauGroup+SupportLegGroup+['RATE:CPS_TAU60'], monGroups=['tauMon:online','tauMon:shifter']),

        # Primary displaced single-tau Legacy chains
        ChainProp(name="HLT_tau180_mediumRNN_tracktwoLLP_L1TAU100", groups=PrimaryLegGroup+SingleTauGroup, monGroups=['tauMon:shifter']),
        ChainProp(name="HLT_tau200_mediumRNN_tracktwoLLP_L1TAU100", groups=PrimaryLegGroup+SingleTauGroup, monGroups=['tauMon:online','tauMon:t0']),
        ChainProp(name="HLT_tau180_tightRNN_tracktwoLLP_L1TAU100", groups=PrimaryLegGroup+SingleTauGroup), 
        ChainProp(name="HLT_tau200_tightRNN_tracktwoLLP_L1TAU100", groups=PrimaryLegGroup+SingleTauGroup, monGroups=['tauMon:t0']),

        # LRT displaced single-tau Legacy chains
        ChainProp(name="HLT_tau160_mediumRNN_trackLRT_L1TAU100", groups=PrimaryLegGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau80_mediumRNN_trackLRT_L1TAU60", groups=SupportLegGroup+SingleTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau80_idperf_trackLRT_L1TAU60", groups=SupportLegGroup+SingleTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau160_idperf_trackLRT_L1TAU100", groups=SupportLegGroup+SingleTauGroup, monGroups=['tauMon:t0']),
        
        # Primary di-tau Legacy chains
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_tau60_mediumRNN_tracktwoMVA_03dRAB_L1TAU60_2TAU40', l1SeedThresholds=['TAU60','TAU40'], groups=PrimaryLegGroup+MultiTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_tau35_mediumRNN_tracktwoMVA_03dRAB30_L1TAU60_DR-TAU20ITAU12I', l1SeedThresholds=['TAU60','TAU12IM'],   groups=PrimaryLegGroup+MultiTauGroup+LegacyTopoGroup, monGroups=['tauMon:online','tauMon:shifter']),

        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB30_L1DR-TAU20ITAU12I-J25', l1SeedThresholds=['TAU20IM','TAU12IM'], groups=PrimaryLegGroup+MultiTauGroup+LegacyTopoGroup, monGroups=['tauMon:online','tauMon:shifter']),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_L1DR-TAU20ITAU12I-J25', l1SeedThresholds=['TAU20IM','TAU12IM'], stream=['VBFDelayed'], groups=PrimaryLegGroup+MultiTauGroup+LegacyTopoGroup),

        # Support di-tau Legacy chains
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB30_L1DR-TAU20ITAU12I', l1SeedThresholds=['TAU20IM','TAU12IM'], groups=SupportLegGroup+MultiTauGroup+LegacyTopoGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_L1DR-TAU20ITAU12I', l1SeedThresholds=['TAU20IM','TAU12IM'], stream=['VBFDelayed'], groups=SupportLegGroup+MultiTauGroup+LegacyTopoGroup),

        ChainProp(name="HLT_tau0_mediumRNN_tracktwoMVA_tau0_mediumRNN_tracktwoMVA_03dRAB30_L1DR-TAU20ITAU12I-J25",l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+LegacyTopoGroup),

        # Asymmetric Legacy tau chains for HH->bbtautau
        ChainProp(name="HLT_tau25_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_L1DR-TAU20ITAU12I-J25", l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+LegacyTopoGroup),
        ChainProp(name="HLT_tau35_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_L1DR-TAU20ITAU12I-J25", l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+LegacyTopoGroup),
        ChainProp(name="HLT_tau40_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_L1DR-TAU20ITAU12I-J25", l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+LegacyTopoGroup),
        ChainProp(name="HLT_tau25_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB30_L1DR-TAU20ITAU12I-J25", l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+LegacyTopoGroup),
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB30_L1DR-TAU20ITAU12I-J25", l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+LegacyTopoGroup),
        ChainProp(name="HLT_tau30_idperf_tracktwoMVA_tau20_idperf_tracktwoMVA_03dRAB30_L1DR-TAU20ITAU12I-J25", l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+LegacyTopoGroup),
    ]

    chains['Bphysics'] = [

        #ATR-21566, chains for di-muon TLA, but with HLT selections to test rates. Here streaming into BphysDelayed (not in TLA stream)   
        ChainProp(name='HLT_2mu4_b7invmAB22vtx20_L1BPH-7M22-2MU3VF', l1SeedThresholds=['MU3VF'],stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+Topo3Group),
        ChainProp(name='HLT_mu6_mu4_b7invmAB22vtx20_L1BPH-7M22-MU5VFMU3VF', l1SeedThresholds=['MU5VF','MU3VF'],stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+Topo3Group),
        ChainProp(name='HLT_2mu4_b0dRAB207invmAB22vtx20_L1BPH-7M22-0DR20-2MU3V', l1SeedThresholds=['MU3V'],stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+Topo3Group),
        ChainProp(name='HLT_2mu4_b0dRAB207invmAB22vtx20_L1BPH-7M22-0DR20-2MU3VF', l1SeedThresholds=['MU3VF'],stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+Topo3Group),
        ChainProp(name='HLT_2mu4_b0dRAB127invmAB22vtx20_L1BPH-7M22-0DR12-2MU3V', l1SeedThresholds=['MU3V'],stream=['BphysDelayed'], groups=BphysicsGroup+EOFBPhysL1MuGroup+Topo3Group),
    ]

    chains['Streaming'] += [
        ChainProp(name='HLT_noalg_L1All', l1SeedThresholds=['FSNOSEED'], groups=['Primary:CostAndRate', 'RATE:SeededStreamers', 'BW:Other']), # ATR-22072, for rates in MC.

        ChainProp(name='HLT_noalg_L1TAU60', l1SeedThresholds=['FSNOSEED'], groups=['PS:NoBulkMCProd']+TauStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TAU8_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['CosmicCalo'], groups=['RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportLegGroup),
    ]

    chains['Calib'] += [
        ChainProp(name='HLT_larpsall_L1TAU8_EMPTY', l1SeedThresholds=['TAU8'], stream=['CosmicCalo'], groups=['RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportLegGroup),
        ChainProp(name='HLT_noalg_LArPEBNoise_L1TAU8_EMPTY', l1SeedThresholds=['TAU8'], stream=['LArCellsEmpty'], groups=['RATE:Calibration','BW:Detector']+SupportLegGroup),
    ]

    chains['Combined'] += [

        ## ATR-25456 - Photon+MET reoptimised
        ChainProp(name='HLT_g25_tight_icalotight_xe40_cell_xe50_tcpufit_80mTAC_L1eEM26M',l1SeedThresholds=['eEM26M']+2*['FSNOSEED'], groups=PrimaryPhIGroup+EgammaMETGroup),
        ChainProp(name='HLT_g25_tight_icalotight_xe40_cell_xe40_tcpufit_xe40_pfopufit_80mTAC_L1eEM26M',l1SeedThresholds=['eEM26M']+3*['FSNOSEED'], groups=PrimaryPhIGroup+EgammaMETGroup),

        ## ATR-25456 - B/D to e+mu triggers
        ChainProp(name='HLT_e14_lhtight_mu6_dRAB15_invmAB10_L1LFV-eEM15L-MU5VF', l1SeedThresholds=['eEM12L','MU5VF'], groups=PrimaryPhIGroup+BphysicsGroup+Topo3Group),
        ChainProp(name='HLT_e12_lhtight_mu11_dRAB15_invmAB10_L1LFV-eEM10L-MU8VF', l1SeedThresholds=['eEM10L','MU8VF'], groups=PrimaryPhIGroup+BphysicsGroup+Topo3Group),

        # Primary displaced tau+X Legacy chains
        ChainProp(name="HLT_tau80_mediumRNN_tracktwoLLP_tau60_mediumRNN_tracktwoLLP_03dRAB_L1TAU60_2TAU40", l1SeedThresholds=['TAU60','TAU40'], groups=PrimaryLegGroup+MultiTauGroup, monGroups=['tauMon:shifter']),
        ChainProp(name="HLT_tau80_mediumRNN_tracktwoLLP_tau60_tightRNN_tracktwoLLP_03dRAB_L1TAU60_2TAU40", l1SeedThresholds=['TAU60','TAU40'], groups=PrimaryLegGroup+MultiTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau80_tightRNN_tracktwoLLP_tau60_tightRNN_tracktwoLLP_03dRAB_L1TAU60_2TAU40", l1SeedThresholds=['TAU60','TAU40'], groups=PrimaryLegGroup+TauJetGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau100_mediumRNN_tracktwoLLP_tau80_mediumRNN_tracktwoLLP_03dRAB_L1TAU60_2TAU40", l1SeedThresholds=['TAU60','TAU40'], groups=PrimaryLegGroup+MultiTauGroup, monGroups=['tauMon:t0']),

        # e + tau intermediate Phase-I tag, Legacy probe chains
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau160_mediumRNN_tracktwoMVA_probe_L1TAU100_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBETAU100'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau180_mediumRNN_tracktwoLLP_probe_L1TAU100_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBETAU100'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau80_mediumRNN_tracktwoMVA_probe_L1TAU60_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBETAU60'], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau80_mediumRNN_tracktwoLLP_probe_L1TAU60_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBETAU60'], groups=TagAndProbePhIGroup+SingleElectronGroup),

        ChainProp(name='HLT_e28_lhtight_ivarloose_tau160_mediumRNN_tracktwoMVA_probe_L1TAU100_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBETAU100'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau180_mediumRNN_tracktwoLLP_probe_L1TAU100_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBETAU100'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau80_mediumRNN_tracktwoMVA_probe_L1TAU60_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBETAU60'], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau80_mediumRNN_tracktwoLLP_probe_L1TAU60_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBETAU60'], groups=TagAndProbePhIGroup+SingleElectronGroup),

        # mu + tau Legacy probe chains
        ChainProp(name='HLT_mu26_ivarmedium_tau80_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBETAU60'], groups=TagAndProbeLegGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau160_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBETAU100'], groups=TagAndProbeLegGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau80_mediumRNN_tracktwoLLP_probe_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBETAU60'], groups=TagAndProbeLegGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau180_mediumRNN_tracktwoLLP_probe_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBETAU100'], groups=TagAndProbeLegGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau80_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBETAU60'], groups=TagAndProbeLegGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau160_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBETAU100'], groups=TagAndProbeLegGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau80_mediumRNN_tracktwoLLP_probe_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBETAU60'], groups=TagAndProbeLegGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau180_mediumRNN_tracktwoLLP_probe_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBETAU100'], groups=TagAndProbeLegGroup+SingleMuonGroup),

        ChainProp(name='HLT_mu24_ivarmedium_tau80_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBETAU60'], groups=TagAndProbeLegGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau160_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBETAU100'], groups=TagAndProbeLegGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau80_mediumRNN_tracktwoLLP_probe_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBETAU60'], groups=TagAndProbeLegGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau180_mediumRNN_tracktwoLLP_probe_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBETAU100'], groups=TagAndProbeLegGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau80_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBETAU60'], groups=TagAndProbeLegGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau160_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBETAU100'], groups=TagAndProbeLegGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau80_mediumRNN_tracktwoLLP_probe_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBETAU60'], groups=TagAndProbeLegGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau180_mediumRNN_tracktwoLLP_probe_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH','PROBETAU100'], groups=TagAndProbeLegGroup+SingleMuonGroup),

        ChainProp(name='HLT_mu26_ivarmedium_tau100_mediumRNN_tracktwoLLP_probe_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBETAU60'], groups=TagAndProbeLegGroup+SingleMuonGroup),
        #ChainProp(name='HLT_e26_lhtight_ivarloose_tau100_mediumRNN_tracktwoLLP_probe_03dRAB_L1EM22VHI', l1SeedThresholds=['EM22VHI','PROBETAU60'], groups=TagAndProbeLegGroup+SingleElectronGroup),

        # MET + tau Legacy tag and probe chains
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1XE50', l1SeedThresholds=['PROBETAU60','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1XE50', l1SeedThresholds=['PROBETAU100','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoLLP_probe_xe65_cell_xe90_pfopufit_L1XE50', l1SeedThresholds=['PROBETAU60','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau180_mediumRNN_tracktwoLLP_probe_xe65_cell_xe90_pfopufit_L1XE50', l1SeedThresholds=['PROBETAU100','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1XE50', l1SeedThresholds=['PROBETAU60','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1XE50', l1SeedThresholds=['PROBETAU100','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoLLP_probe_xe65_cell_xe100_pfopufit_L1XE50', l1SeedThresholds=['PROBETAU60','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau180_mediumRNN_tracktwoLLP_probe_xe65_cell_xe100_pfopufit_L1XE50', l1SeedThresholds=['PROBETAU100','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1XE50', l1SeedThresholds=['PROBETAU60','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1XE50', l1SeedThresholds=['PROBETAU100','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoLLP_probe_xe75_cell_xe100_pfopufit_L1XE50', l1SeedThresholds=['PROBETAU60','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau180_mediumRNN_tracktwoLLP_probe_xe75_cell_xe100_pfopufit_L1XE50', l1SeedThresholds=['PROBETAU100','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),

        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1XE55', l1SeedThresholds=['PROBETAU60','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumRNN_tracktwoMVA_probe_xe65_cell_xe90_pfopufit_L1XE55', l1SeedThresholds=['PROBETAU100','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoLLP_probe_xe65_cell_xe90_pfopufit_L1XE55', l1SeedThresholds=['PROBETAU60','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau180_mediumRNN_tracktwoLLP_probe_xe65_cell_xe90_pfopufit_L1XE55', l1SeedThresholds=['PROBETAU100','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1XE55', l1SeedThresholds=['PROBETAU60','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumRNN_tracktwoMVA_probe_xe65_cell_xe100_pfopufit_L1XE55', l1SeedThresholds=['PROBETAU100','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoLLP_probe_xe65_cell_xe100_pfopufit_L1XE55', l1SeedThresholds=['PROBETAU60','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau180_mediumRNN_tracktwoLLP_probe_xe65_cell_xe100_pfopufit_L1XE55', l1SeedThresholds=['PROBETAU100','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1XE55', l1SeedThresholds=['PROBETAU60','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumRNN_tracktwoMVA_probe_xe75_cell_xe100_pfopufit_L1XE55', l1SeedThresholds=['PROBETAU100','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumRNN_tracktwoLLP_probe_xe75_cell_xe100_pfopufit_L1XE55', l1SeedThresholds=['PROBETAU60','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau180_mediumRNN_tracktwoLLP_probe_xe75_cell_xe100_pfopufit_L1XE55', l1SeedThresholds=['PROBETAU100','FSNOSEED','FSNOSEED'],  groups=TagAndProbeLegGroup+TauMETGroup),

        # Asymmetric Legacy tau chains for HH->bbtautau
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_j20c_020jvt_bgn185_pf_ftf_L1DR-TAU20ITAU12I-J25', l1SeedThresholds=['TAU20IM','TAU12IM', 'FSNOSEED'], stream=['VBFDelayed'], groups=TauBJetGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_4j20c_020jvt_SHARED_j20c_020jvt_bgn185_pf_ftf_L1DR-TAU20ITAU12I-J25', l1SeedThresholds=['TAU20IM','TAU12IM']+2*['FSNOSEED'], stream=['VBFDelayed'], groups=TauBJetGroup),
    ]

    return chains

def setupMenu(menu_name):
    
    chains = physics_menu.setupMenu(menu_name)

    log.info('[setupMenu] going to add the MC menu chains now')

    for sig,chainsInSig in getMCSignatures().items():
        chains[sig] += chainsInSig

    return chains
