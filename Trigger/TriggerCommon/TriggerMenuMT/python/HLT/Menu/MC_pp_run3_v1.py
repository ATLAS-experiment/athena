# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# MC_pp_run3_v1.py menu for the long shutdown development
#------------------------------------------------------------------------#

# This defines the input format of the chain and it's properties with the defaults set
# always required are: name, stream and groups
#['name', 'L1chainParts'=[], 'stream', 'groups', 'merging'=[], 'topoStartFrom'=False],
from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp
from .SignatureDicts import ChainStore

import TriggerMenuMT.HLT.Menu.Physics_pp_run3_v1 as physics_menu 
from TriggerMenuMT.HLT.Menu.Physics_pp_run3_v1 import ( PhysicsStream,
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
                                                        EOFTLALegGroup
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

        ChainProp(name="HLT_j20_roiftf_preselj20_L1RD0_FILLED", l1SeedThresholds=['FSNOSEED'],  monGroups=['idMon:t0'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),

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
        
        ChainProp(name='HLT_j30_CLEANllp_calratio_L1TAU60', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j30_CLEANllp_calratiormbib_L1TAU60', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup),
        #ChainProp(name='HLT_j30_CLEANllp_calratio_L1eTAU80', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),
        #ChainProp(name='HLT_j30_CLEANllp_calratiormbib_L1eTAU80', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportPhIGroup),

        # Calratio Legacy primary chains
        ChainProp(name='HLT_j30_CLEANllp_calratio_L1TAU100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j30_CLEANllp_calratiormbib_L1TAU100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryLegGroup),

      
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

        #L1 legacy Removal
        ChainProp(name='HLT_j110_pf_ftf_preselj80_L1J30', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J30']),
        ChainProp(name='HLT_j110_L1J30', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J30']),
        ChainProp(name='HLT_j110_a10sd_cssk_pf_jes_ftf_preselj80_L1J30', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J30']),
        ChainProp(name='HLT_j110_a10t_lcw_jes_L1J30', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J30']),

        ChainProp(name='HLT_j260_pf_ftf_preselj200_L1J75', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J75']),
        ChainProp(name='HLT_j260_L1J75', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J75']),
        ChainProp(name='HLT_j260_a10sd_cssk_pf_jes_ftf_preselj200_L1J75', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J75']),
        ChainProp(name='HLT_j260_a10t_lcw_jes_L1J75', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J75']),

        ChainProp(name='HLT_j400_pf_ftf_preselj225_L1J120', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryLegGroup), 
        ChainProp(name='HLT_j420_L1J120', l1SeedThresholds=['FSNOSEED'], groups=SupportLegGroup+SingleJetGroup),
        ChainProp(name='HLT_j420_ftf_preselj225_L1J120', l1SeedThresholds=['FSNOSEED'], groups=SupportLegGroup+SingleJetGroup),

        ChainProp(name='HLT_j60f_L1J20p31ETA49', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J20p31ETA49']),
        ChainProp(name='HLT_j85f_L1J20p31ETA49', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J20p31ETA49']),

        ChainProp(name='HLT_j110f_L1J30p31ETA49', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j175f_L1J50p31ETA49', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j45f_L1J15p31ETA49', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup,  monGroups=['caloMon:t0']),

        ChainProp(name='HLT_2j20_2j20_pf_ftf_presel2c20XX2c20b85_PhysicsTLA_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*2, stream=['TLA'], groups=MultiJetGroup+SupportLegGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_j60_j45_j25_j20_pf_ftf_preselc60XXc45XXc25XXc20_PhysicsTLA_L1J45p0ETA21_3J15p0ETA25',l1SeedThresholds=['FSNOSEED']*4, stream=['TLA'], groups=MultiJetGroup+SupportLegGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_2j20_2j20_pf_ftf_presel2c20XX2c20b82_PhysicsTLA_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*2, stream=['TLA'], groups=MultiJetGroup+SupportLegGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_2j20_2j20_pf_ftf_presel2c20XX2c20b80_PhysicsTLA_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*2, stream=['TLA'], groups=MultiJetGroup+SupportLegGroup, monGroups=['tlaMon:shifter']),

        ChainProp(name='HLT_j0_perf_L1J12_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SingleJetGroup+SupportLegGroup, monGroups=['jetMon:online']),

        # ATR-28795 bjet menu cleanup
        ChainProp(name='HLT_2j20_2j20_pf_ftf_presel2c20XX2c20b82_PhysicsTLA_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*2, stream=['TLA'], groups=MultiJetGroup+SupportPhIGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_2j20_2j20_pf_ftf_presel2c20XX2c20b80_PhysicsTLA_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*2, stream=['TLA'], groups=MultiJetGroup+SupportPhIGroup, monGroups=['tlaMon:shifter']),
        
        
        ChainProp(name='HLT_2j330_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=MultiJetGroup+PrimaryLegGroup+LegacyTopoGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_2j330_35smcINF_a10t_lcw_jes_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_2j330_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+SupportLegGroup+LegacyTopoGroup+['RATE:CPS_SC111-CJ15'], monGroups=['jetMon:t0']),
        ChainProp(name='HLT_j0_HT1000_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SupportLegGroup+SingleJetGroup+LegacyTopoGroup, monGroups=['jetMon:shifter', 'jetMon:online']),
        ChainProp(name='HLT_j0_HT650XX0eta240_pf_ftf_preselcHT450_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=PrimaryLegGroup+SingleJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT940_pf_ftf_preselcHT450_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], stream=['Main','express'], groups=PrimaryLegGroup+SingleJetGroup+LegacyTopoGroup, monGroups=['jetMon:online','jetMon:shifter','caloMon:t0']),
        ChainProp(name='HLT_j0_HT940_pf_ftf_preselj180_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=PrimaryLegGroup+SingleJetGroup+LegacyTopoGroup), 
        ChainProp(name='HLT_j20_pf_ftf_preselcHT450_PhysicsTLA_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=MultiJetGroup+PrimaryLegGroup+LegacyTopoGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_j20_PhysicsTLA_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryLegGroup+SingleJetGroup+LegacyTopoGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_j20_PhysicsTLA_L1J50_DETA20-J50J', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=EOFTLALegGroup+SingleJetGroup+LegacyTopoGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_j360_60smcINF_j360_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryLegGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j360_a10sd_cssk_pf_jes_ftf_preselj225_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup+LegacyTopoGroup+['RATE:CPS_SC111-CJ15']),
        ChainProp(name='HLT_j360_a10t_lcw_jes_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportLegGroup+LegacyTopoGroup+['RATE:CPS_SC111-CJ15']),
        ChainProp(name='HLT_j370_35smcINF_j370_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryLegGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j370_35smcINF_j370_a10t_lcw_jes_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryLegGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j420_35smcINF_a10sd_cssk_pf_jes_ftf_preselj225_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED'], stream=['Main','express'], groups=SingleJetGroup+PrimaryLegGroup+LegacyTopoGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_j420_35smcINF_a10t_lcw_jes_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j460_a10_lcw_subjes_L1SC111-CJ15',         l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j460_a10r_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED'],  groups=PrimaryLegGroup+SingleJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j460_a10sd_cssk_pf_jes_ftf_preselj225_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryLegGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j460_a10t_lcw_jes_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryLegGroup+LegacyTopoGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_j480_a10sd_cssk_pf_jes_ftf_preselj225_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryLegGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j480_a10t_lcw_jes_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+PrimaryLegGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j70_j50a_j0_DJMASS1000j50dphi200x400deta_L1MJJ-500-NFF', l1SeedThresholds=['FSNOSEED']*3,stream=['VBFDelayed'],groups=PrimaryLegGroup+MultiJetGroup+LegacyTopoGroup), # previously HLT_j70_j50_0eta490_invm1000j70_dphi20_deta40_L1MJJ-500-NFF
        ChainProp(name='HLT_j360_60smcINF_j360_a10t_lcw_jes_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryLegGroup+MultiJetGroup+LegacyTopoGroup),
    ]

    chains['Bjet'] = [
        # B-tagger training
        ChainProp(name='HLT_j20_0eta290_020jvt_boffperf_pf_ftf_L1J15', l1SeedThresholds=['FSNOSEED'], groups=SingleBjetGroup+SupportLegGroup, monGroups=['bJetMon:online']),
        ChainProp(name='HLT_j20_0eta290_020jvt_boffperf_pf_ftf_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleBjetGroup+SupportLegGroup, monGroups=['bJetMon:online']),
        ChainProp(name='HLT_j20_0eta290_020jvt_boffperf_pf_ftf_L1jJ40', l1SeedThresholds=['FSNOSEED'], groups=SingleBjetGroup+SupportPhIGroup, monGroups=['bJetMon:online']),
        # HH4b
        # Muon seeded candidates
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1d82_pf_ftf_presel2c20XX2c20b85_L1MU8F_2J15_J20', l1SeedThresholds=['FSNOSEED']*5, groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1d82_pf_ftf_presel2c20XX2c20b85_L1MU8F_2jJ40_jJ50', l1SeedThresholds=['FSNOSEED']*5, groups=PrimaryPhIGroup+MultiBjetGroup), # Phase-1 version with ATR-28761

        # Muon+jet legacy seeded, backup for L1Topo muon-in-jet moved by ATR-28761
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d77_pf_ftf_presel2c20XX2c20b85_L1MU8F_2J15_J20', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=PrimaryLegGroup+MultiBjetGroup), # downshift
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d77_pf_ftf_presel2c20XX2c20b85_L1MU8F_2J15_J20', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b85_L1MU8F_2J15_J20', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=PrimaryLegGroup+MultiBjetGroup), # downshift
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b85_L1MU8F_2J15_J20', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=PrimaryLegGroup+MultiBjetGroup),

        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b82_L1MU8F_2J15_J20', l1SeedThresholds=['FSNOSEED']*5, stream=['Main'], groups=PrimaryLegGroup+MultiBjetGroup), # downshift
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b80_L1MU8F_2J15_J20', l1SeedThresholds=['FSNOSEED']*5, stream=['Main'], groups=PrimaryLegGroup+MultiBjetGroup), # downshift
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b82_L1MU8F_2J15_J20', l1SeedThresholds=['FSNOSEED']*5, stream=['Main'], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b80_L1MU8F_2J15_J20', l1SeedThresholds=['FSNOSEED']*5, stream=['Main'], groups=PrimaryLegGroup+MultiBjetGroup),
        
        # tighter preselection backup - b80
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1d82_pf_ftf_presel2c20XX2c20b80_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=PrimaryLegGroup+MultiBjetGroup), # downshift
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1d82_pf_ftf_presel2c20XX2c20b80_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'],  monGroups=['idMon:t0'], groups=PrimaryLegGroup+MultiBjetGroup),        
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d77_pf_ftf_presel2c20XX2c20b80_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'],  monGroups=['idMon:t0'], groups=PrimaryLegGroup+MultiBjetGroup), # downshift
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d77_pf_ftf_presel2c20XX2c20b80_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'],  monGroups=['idMon:t0'], groups=PrimaryLegGroup+MultiBjetGroup),

        #L1 legacy Removal
        # HH4b chains with b-jet preselections
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn182_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, groups=PrimaryLegGroup+MultiBjetGroup), # downshift
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn182_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, groups=PrimaryLegGroup+MultiBjetGroup),
        # Extra GN1 b-tagger chains
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5,  monGroups=['idMon:t0'], groups=PrimaryLegGroup+MultiBjetGroup), # downshift
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5,  monGroups=['idMon:t0'], groups=PrimaryLegGroup+MultiBjetGroup),
        # tighter preselection backup - b82
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn182_pf_ftf_presel2c20XX2c20b82_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, groups=PrimaryLegGroup+MultiBjetGroup), # downshift
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn182_pf_ftf_presel2c20XX2c20b82_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b82_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5,  monGroups=['idMon:t0'], groups=PrimaryLegGroup+MultiBjetGroup), # downshift
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b82_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5,  monGroups=['idMon:t0'], groups=PrimaryLegGroup+MultiBjetGroup),
        # tighter preselection backup - b82
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn182_pf_ftf_presel2c20XX2c20b80_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, groups=PrimaryLegGroup+MultiBjetGroup), # downshift
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn182_pf_ftf_presel2c20XX2c20b80_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b80_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, groups=PrimaryLegGroup+MultiBjetGroup), # downshift
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b80_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, groups=PrimaryLegGroup+MultiBjetGroup),

        ChainProp(name="HLT_j70_0eta290_020jvt_bdl1d60_3j70_pf_ftf_preselj50b85XX3j50_L14J20", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup), # downshift
        ChainProp(name="HLT_j75_0eta290_020jvt_bdl1d60_3j75_pf_ftf_preselj50b85XX3j50_L14J20", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name="HLT_j70_0eta290_020jvt_bgn160_3j70_pf_ftf_preselj50b85XX3j50_L14J20", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup), # downshift
        ChainProp(name="HLT_j75_0eta290_020jvt_bgn160_3j75_pf_ftf_preselj50b85XX3j50_L14J20", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup),

        # Asymmetric, 1j + 2b
        ChainProp(name="HLT_j140_2j50_0eta290_020jvt_bdl1d70_pf_ftf_preselj80XX2j45b90_L1J85_3J30", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup), # downshift
        ChainProp(name="HLT_j150_2j55_0eta290_020jvt_bdl1d70_pf_ftf_preselj80XX2j45b90_L1J85_3J30", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup),
        # Asymmetric, 1j + 2b
        ChainProp(name="HLT_j140_2j50_0eta290_020jvt_bgn170_pf_ftf_preselj80XX2j45b90_L1J85_3J30", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup), # downshift
        ChainProp(name="HLT_j150_2j55_0eta290_020jvt_bgn170_pf_ftf_preselj80XX2j45b90_L1J85_3J30", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup),

        # VBF chains
        ChainProp(name="HLT_j50_0eta290_020jvt_bdl1d70_2j45f_pf_ftf_preselj45XX2f40_L1J25p0ETA23_2J15p31ETA49",l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryLegGroup+MultiBjetGroup), # downshift
        ChainProp(name="HLT_j55_0eta290_020jvt_bdl1d70_2j45f_pf_ftf_preselj45XX2f40_L1J25p0ETA23_2J15p31ETA49",l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name="HLT_j50_0eta290_020jvt_bgn170_2j45f_pf_ftf_preselj45XX2f40_L1J25p0ETA23_2J15p31ETA49",l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryLegGroup+MultiBjetGroup), # downshift
        ChainProp(name="HLT_j55_0eta290_020jvt_bgn170_2j45f_pf_ftf_preselj45XX2f40_L1J25p0ETA23_2J15p31ETA49",l1SeedThresholds=['FSNOSEED']*2, groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_j55_j45f_SHARED_2j45_0eta290_020jvt_bdl1d60_pf_ftf_preselc60XXj45XXf40_L1J40p0ETA25_2J25_J20p31ETA49', l1SeedThresholds=['FSNOSEED']*4, groups=PrimaryLegGroup+MultiBjetGroup), # downshift
        ChainProp(name='HLT_j80c_j60_j45f_SHARED_2j45_0eta290_020jvt_bdl1d60_pf_ftf_preselc60XXj45XXf40_L1J40p0ETA25_2J25_J20p31ETA49', l1SeedThresholds=['FSNOSEED']*4, groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name="HLT_j75_0eta290_020jvt_bdl1d70_j55_0eta290_020jvt_bdl1d85_j45f_pf_ftf_preselj60XXj45XXf40_L1J40p0ETA25_2J25_J20p31ETA49", l1SeedThresholds=['FSNOSEED']*3, groups=PrimaryLegGroup+MultiBjetGroup), # downshift
        ChainProp(name="HLT_j80_0eta290_020jvt_bdl1d70_j60_0eta290_020jvt_bdl1d85_j45f_pf_ftf_preselj60XXj45XXf40_L1J40p0ETA25_2J25_J20p31ETA49", l1SeedThresholds=['FSNOSEED']*3, groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_j55_j45f_SHARED_2j45_0eta290_020jvt_bgn160_pf_ftf_preselc60XXj45XXf40_L1J40p0ETA25_2J25_J20p31ETA49', l1SeedThresholds=['FSNOSEED']*4, groups=PrimaryLegGroup+MultiBjetGroup), # downshift
        ChainProp(name='HLT_j80c_j60_j45f_SHARED_2j45_0eta290_020jvt_bgn160_pf_ftf_preselc60XXj45XXf40_L1J40p0ETA25_2J25_J20p31ETA49', l1SeedThresholds=['FSNOSEED']*4, groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name="HLT_j75_0eta290_020jvt_bgn170_j55_0eta290_020jvt_bgn185_j45f_pf_ftf_preselj60XXj45XXf40_L1J40p0ETA25_2J25_J20p31ETA49", l1SeedThresholds=['FSNOSEED']*3, groups=PrimaryLegGroup+MultiBjetGroup), # downshift
        ChainProp(name="HLT_j80_0eta290_020jvt_bgn170_j60_0eta290_020jvt_bgn185_j45f_pf_ftf_preselj60XXj45XXf40_L1J40p0ETA25_2J25_J20p31ETA49", l1SeedThresholds=['FSNOSEED']*3, groups=PrimaryLegGroup+MultiBjetGroup),

        # Various 2018 multi-b triggers
        ChainProp(name="HLT_3j60_0eta290_020jvt_bdl1d77_pf_ftf_presel3j45b95_L13J35p0ETA23", l1SeedThresholds=['FSNOSEED'], groups=MultiBjetGroup + PrimaryLegGroup), # downshift
        ChainProp(name="HLT_3j65_0eta290_020jvt_bdl1d77_pf_ftf_presel3j45b95_L13J35p0ETA23", l1SeedThresholds=['FSNOSEED'], groups=MultiBjetGroup + PrimaryLegGroup),
        ChainProp(name="HLT_3j60_0eta290_020jvt_bgn177_pf_ftf_presel3j45b95_L13J35p0ETA23", l1SeedThresholds=['FSNOSEED'], groups=MultiBjetGroup + PrimaryLegGroup), # downshift
        ChainProp(name="HLT_3j65_0eta290_020jvt_bgn177_pf_ftf_presel3j45b95_L13J35p0ETA23", l1SeedThresholds=['FSNOSEED'], groups=MultiBjetGroup + PrimaryLegGroup),

        ChainProp(name="HLT_4j35_0eta290_020jvt_bdl1d77_pf_ftf_presel4j25b95_L14J15p0ETA25", l1SeedThresholds=['FSNOSEED'], groups=MultiBjetGroup + PrimaryLegGroup),
        ChainProp(name="HLT_3j35_0eta290_020jvt_bdl1d70_j35_pf_ftf_presel2j25XX2j25b85_L14J15p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j35_0eta290_020jvt_bdl1d70_2j35_0eta290_020jvt_bdl1d85_pf_ftf_presel4j25b95_L14J15p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j50_0eta290_020jvt_bdl1d60_2j50_pf_ftf_presel2j25XX2j25b85_L14J15p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup), # downshift
        ChainProp(name="HLT_2j55_0eta290_020jvt_bdl1d60_2j55_pf_ftf_presel2j25XX2j25b85_L14J15p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j45_0eta290_020jvt_bdl1d60_2j45_pf_ftf_presel2j25XX2j25b85_L14J15p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup),

        ChainProp(name="HLT_2j35_0eta290_020jvt_bdl1d60_3j35_pf_ftf_presel3j25XX2j25b85_L15J15p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j45_0eta290_020jvt_bdl1d60_3j45_pf_ftf_presel3j25XX2j25b85_L15J15p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup),

        ChainProp(name="HLT_4j35_0eta290_020jvt_bgn177_pf_ftf_presel4j25b95_L14J15p0ETA25", l1SeedThresholds=['FSNOSEED'], groups=MultiBjetGroup + PrimaryLegGroup),
        ChainProp(name="HLT_3j35_0eta290_020jvt_bgn170_j35_pf_ftf_presel2j25XX2j25b85_L14J15p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j35_0eta290_020jvt_bgn170_2j35_0eta290_020jvt_bgn185_pf_ftf_presel4j25b95_L14J15p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j50_0eta290_020jvt_bgn160_2j50_pf_ftf_presel2j25XX2j25b85_L14J15p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup), # downshift
        ChainProp(name="HLT_2j55_0eta290_020jvt_bgn160_2j55_pf_ftf_presel2j25XX2j25b85_L14J15p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j45_0eta290_020jvt_bgn160_2j45_pf_ftf_presel2j25XX2j25b85_L14J15p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j35c_020jvt_bgn160_2j35c_020jvt_pf_ftf_presel2j25XX2j25b85_L14J15p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup),

        # Run 2 HH4b low-threshold chain
        ChainProp(name="HLT_2j35c_020jvt_bdl1d60_2j35c_020jvt_pf_ftf_presel2j25XX2j25b85_L14J15p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup),

        # Various 2018 multi-b triggers
        ChainProp(name="HLT_2j35_0eta290_020jvt_bgn160_3j35_pf_ftf_presel3j25XX2j25b85_L15J15p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j45_0eta290_020jvt_bgn160_3j45_pf_ftf_presel3j25XX2j25b85_L15J15p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup),

        # 3b symmetric b-jet pt for Physics_Main
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1d82_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, groups=PrimaryLegGroup+MultiBjetGroup), # downshift
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1d82_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5,  monGroups=['idMon:t0'], groups=PrimaryLegGroup+MultiBjetGroup),
        # 2b symmetric b-jet pt for VBFDelayed
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d77_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'],  monGroups=['idMon:t0'], groups=PrimaryLegGroup+MultiBjetGroup), # downshift
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d77_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'],  monGroups=['idMon:t0'], groups=PrimaryLegGroup+MultiBjetGroup),
        # tighter preselection backup - b82
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1d82_pf_ftf_presel2c20XX2c20b82_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, groups=PrimaryLegGroup+MultiBjetGroup), # downshift
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1d82_pf_ftf_presel2c20XX2c20b82_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5,  monGroups=['idMon:t0'], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d77_pf_ftf_presel2c20XX2c20b82_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'],  monGroups=['idMon:t0'], groups=PrimaryLegGroup+MultiBjetGroup), # downshift
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d77_pf_ftf_presel2c20XX2c20b82_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'],  monGroups=['idMon:t0'], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_boffperf_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*4,  monGroups=['idMon:t0'], groups=SupportLegGroup+MultiBjetGroup), # downshift
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_boffperf_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*4,  monGroups=['idMon:t0'], groups=SupportLegGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_boffperf_pf_ftf_presel2c20XX2c20b82_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*4,  monGroups=['idMon:t0'], groups=SupportLegGroup+MultiBjetGroup), # downshift
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_boffperf_pf_ftf_presel2c20XX2c20b82_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*4,  monGroups=['idMon:t0'], groups=SupportLegGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_boffperf_pf_ftf_presel2c20XX2c20b80_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*4,  monGroups=['idMon:t0'], groups=SupportLegGroup+MultiBjetGroup), # downshift
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_boffperf_pf_ftf_presel2c20XX2c20b80_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*4,  monGroups=['idMon:t0'], groups=SupportLegGroup+MultiBjetGroup),

        # 2024 spring cleaning of Physics menu ATR-28795
        # Dl1d
        ChainProp(name="HLT_j210_0eta290_020jvt_bdl1d70_pf_ftf_preselj180_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup, monGroups=['bJetMon:online']),
        ChainProp(name="HLT_j225_0eta290_020jvt_bdl1d70_pf_ftf_preselj180_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup, monGroups=['bJetMon:online']),
        ChainProp(name="HLT_j280_0eta290_020jvt_bdl1d77_pf_ftf_preselj225_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup),
        ChainProp(name="HLT_j300_0eta290_020jvt_bdl1d77_pf_ftf_preselj225_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup),
        ChainProp(name="HLT_j340_0eta290_020jvt_bdl1d85_pf_ftf_preselj225_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup),
        ChainProp(name="HLT_j360_0eta290_020jvt_bdl1d85_pf_ftf_preselj225_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup),
        ChainProp(name="HLT_j210_0eta290_020jvt_bdl1d70_pf_ftf_preselj190_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup, monGroups=['bJetMon:online']),
        ChainProp(name="HLT_j225_0eta290_020jvt_bdl1d70_pf_ftf_preselj190_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup, monGroups=['bJetMon:online']),
        ChainProp(name="HLT_j210_0eta290_020jvt_bdl1d70_pf_ftf_preselj200_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup, monGroups=['bJetMon:online']),
        ChainProp(name="HLT_j225_0eta290_020jvt_bdl1d70_pf_ftf_preselj200_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup, monGroups=['bJetMon:online']),
        ChainProp(name="HLT_j210_0eta290_020jvt_bdl1d60_pf_ftf_preselj180_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup),
        ChainProp(name="HLT_j225_0eta290_020jvt_bdl1d60_pf_ftf_preselj180_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup),
        ChainProp(name="HLT_j255_0eta290_020jvt_bdl1d70_pf_ftf_preselj225_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup),
        ChainProp(name="HLT_j275_0eta290_020jvt_bdl1d70_pf_ftf_preselj225_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup),
        ChainProp(name="HLT_j280_0eta290_020jvt_bdl1d70_pf_ftf_preselj225_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup),
        ChainProp(name="HLT_j300_0eta290_020jvt_bdl1d70_pf_ftf_preselj225_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup),
        ChainProp(name="HLT_j340_0eta290_020jvt_bdl1d77_pf_ftf_preselj225_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup),
        ChainProp(name="HLT_j360_0eta290_020jvt_bdl1d77_pf_ftf_preselj225_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup),
        ChainProp(name="HLT_j225_0eta290_020jvt_bdl1d77_pf_ftf_preselj180_L1J100", l1SeedThresholds=['FSNOSEED'], groups=SupportLegGroup+SingleBjetGroup),
        ChainProp(name='HLT_j275_0eta290_020jvt_bdl1d85_pf_ftf_preselj225_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SupportLegGroup+SingleBjetGroup),
        ChainProp(name='HLT_j300_0eta290_020jvt_bdl1d85_pf_ftf_preselj225_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SupportLegGroup+SingleBjetGroup),
        ChainProp(name="HLT_j165_0eta290_020jvt_bdl1d60_j55_0eta290_020jvt_bdl1d60_pf_ftf_preselj140b85XXj45b85_L1J100", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name="HLT_j175_0eta290_020jvt_bdl1d60_j60_0eta290_020jvt_bdl1d60_pf_ftf_preselj140b85XXj45b85_L1J100", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name='HLT_2j45_0eta290_020jvt_bdl1d70_j0_HT290_j0_DJMASS700j35_pf_ftf_L1HT150-J20s5pETA31_MJJ-400-CF', l1SeedThresholds=['FSNOSEED']*3, groups=PrimaryLegGroup+MultiBjetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_2j45_0eta290_020jvt_bdl1d70_j0_HT300_j0_DJMASS700j35_pf_ftf_L1HT150-J20s5pETA31_MJJ-400-CF', l1SeedThresholds=['FSNOSEED']*3, groups=PrimaryLegGroup+MultiBjetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j65a_j45a_2j35a_SHARED_2j35_0eta290_020jvt_bdl1d70_j0_DJMASS1000j50_pf_ftf_presela60XXa40XX2a25_L1MJJ-500-NFF', l1SeedThresholds=['FSNOSEED']*5,stream=['VBFDelayed'], groups=PrimaryLegGroup+MultiBjetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j70a_j50a_2j35a_SHARED_2j35_0eta290_020jvt_bdl1d70_j0_DJMASS1000j50_pf_ftf_presela60XXa40XX2a25_L1MJJ-500-NFF', l1SeedThresholds=['FSNOSEED']*5,stream=['VBFDelayed'], groups=PrimaryLegGroup+MultiBjetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d77_pf_ftf_presel2c20XX2c20b85_L1MU8F_2jJ40_jJ50', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d77_pf_ftf_presel2c20XX2c20b85_L1MU8F_2jJ40_jJ50', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_5j35c_020jvt_j25c_020jvt_SHARED_j25c_020jvt_bdl1d60_pf_ftf_presel5c25XXc25b85_L14J15', l1SeedThresholds=['FSNOSEED']*3, stream=['VBFDelayed'], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name='HLT_5j45c_020jvt_j25c_020jvt_SHARED_j25c_020jvt_bdl1d60_pf_ftf_presel5c25XXc25b85_L14J15', l1SeedThresholds=['FSNOSEED']*3, stream=['VBFDelayed'], groups=PrimaryLegGroup+MultiBjetGroup),
        ChainProp(name="HLT_j210_0eta290_020jvt_bdl1d70_pf_ftf_preselj180_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup, monGroups=['bJetMon:online']),
        ChainProp(name="HLT_j225_0eta290_020jvt_bdl1d70_pf_ftf_preselj180_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup, monGroups=['bJetMon:online']),
        ChainProp(name="HLT_j280_0eta290_020jvt_bdl1d77_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_j300_0eta290_020jvt_bdl1d77_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_j340_0eta290_020jvt_bdl1d85_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_j360_0eta290_020jvt_bdl1d85_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_j210_0eta290_020jvt_bdl1d70_pf_ftf_preselj190_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup, monGroups=['bJetMon:online']),
        ChainProp(name="HLT_j225_0eta290_020jvt_bdl1d70_pf_ftf_preselj190_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup, monGroups=['bJetMon:online']),
        ChainProp(name="HLT_j210_0eta290_020jvt_bdl1d70_pf_ftf_preselj200_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup, monGroups=['bJetMon:online']),
        ChainProp(name="HLT_j225_0eta290_020jvt_bdl1d70_pf_ftf_preselj200_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup, monGroups=['bJetMon:online']),
        ChainProp(name="HLT_j210_0eta290_020jvt_bdl1d60_pf_ftf_preselj180_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_j225_0eta290_020jvt_bdl1d60_pf_ftf_preselj180_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_j255_0eta290_020jvt_bdl1d70_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_j275_0eta290_020jvt_bdl1d70_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_j280_0eta290_020jvt_bdl1d70_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_j300_0eta290_020jvt_bdl1d70_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_j340_0eta290_020jvt_bdl1d77_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_j360_0eta290_020jvt_bdl1d77_pf_ftf_preselj225_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_j225_0eta290_020jvt_bdl1d77_pf_ftf_preselj180_L1jJ160", l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+SingleBjetGroup),
        ChainProp(name='HLT_j275_0eta290_020jvt_bdl1d85_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+SingleBjetGroup),
        ChainProp(name='HLT_j300_0eta290_020jvt_bdl1d85_pf_ftf_preselj225_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SupportPhIGroup+SingleBjetGroup),
        ChainProp(name="HLT_3j60_0eta290_020jvt_bdl1d77_pf_ftf_presel3j45b95_L13jJ70p0ETA23", l1SeedThresholds=['FSNOSEED'], groups=MultiBjetGroup + PrimaryPhIGroup),
        ChainProp(name="HLT_3j65_0eta290_020jvt_bdl1d77_pf_ftf_presel3j45b95_L13jJ70p0ETA23", l1SeedThresholds=['FSNOSEED'], groups=MultiBjetGroup + PrimaryPhIGroup),
        ChainProp(name="HLT_4j35_0eta290_020jvt_bdl1d77_pf_ftf_presel4j25b95_L14jJ40p0ETA25", l1SeedThresholds=['FSNOSEED'], groups=MultiBjetGroup + PrimaryPhIGroup),
        ChainProp(name="HLT_3j35_0eta290_020jvt_bdl1d70_j35_pf_ftf_presel2j25XX2j25b85_L14jJ40p0ETA25",      l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j35_0eta290_020jvt_bdl1d70_2j35_0eta290_020jvt_bdl1d85_pf_ftf_presel4j25b95_L14jJ40p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j50_0eta290_020jvt_bdl1d60_2j50_pf_ftf_presel2j25XX2j25b85_L14jJ40p0ETA25",        l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j55_0eta290_020jvt_bdl1d60_2j55_pf_ftf_presel2j25XX2j25b85_L14jJ40p0ETA25",        l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j35_0eta290_020jvt_bdl1d60_3j35_pf_ftf_presel3j25XX2j25b85_L15jJ40p0ETA25",  l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j45_0eta290_020jvt_bdl1d60_3j45_pf_ftf_presel3j25XX2j25b85_L15jJ40p0ETA25",  l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_j70_0eta290_020jvt_bdl1d60_3j70_pf_ftf_preselj50b85XX3j50_L14jJ50",           l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_j75_0eta290_020jvt_bdl1d60_3j75_pf_ftf_preselj50b85XX3j50_L14jJ50",           l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j45_0eta290_020jvt_bdl1d60_2j45_pf_ftf_presel2j25XX2j25b85_L14jJ40p0ETA25",  l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_j140_2j50_0eta290_020jvt_bdl1d70_pf_ftf_preselj80XX2j45b90_L1jJ140_3jJ60", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_j150_2j55_0eta290_020jvt_bdl1d70_pf_ftf_preselj80XX2j45b90_L1jJ140_3jJ60", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_j165_0eta290_020jvt_bdl1d60_j55_0eta290_020jvt_bdl1d60_pf_ftf_preselj140b85XXj45b85_L1jJ160", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_j175_0eta290_020jvt_bdl1d60_j60_0eta290_020jvt_bdl1d60_pf_ftf_preselj140b85XXj45b85_L1jJ160", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_2j35c_020jvt_bdl1d60_2j35c_020jvt_pf_ftf_presel2j25XX2j25b85_L14jJ40p0ETA25", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_2j45_0eta290_020jvt_bdl1d70_j0_HT290_j0_DJMASS700j35_pf_ftf_L1HT150-jJ50s5pETA32_jMJJ-400-CF', l1SeedThresholds=['FSNOSEED']*3, groups=PrimaryPhIGroup+MultiBjetGroup+Topo3Group),
        ChainProp(name='HLT_2j45_0eta290_020jvt_bdl1d70_j0_HT300_j0_DJMASS700j35_pf_ftf_L1HT150-jJ50s5pETA32_jMJJ-400-CF', l1SeedThresholds=['FSNOSEED']*3, groups=PrimaryPhIGroup+MultiBjetGroup+Topo3Group),
        ChainProp(name='HLT_j75c_j55_j45f_SHARED_2j45_0eta290_020jvt_bdl1d60_pf_ftf_preselc60XXj45XXf40_L1jJ80p0ETA25_2jJ55_jJ50p30ETA49', l1SeedThresholds=['FSNOSEED']*4, groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_j60_j45f_SHARED_2j45_0eta290_020jvt_bdl1d60_pf_ftf_preselc60XXj45XXf40_L1jJ80p0ETA25_2jJ55_jJ50p30ETA49', l1SeedThresholds=['FSNOSEED']*4, groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_j75_0eta290_020jvt_bdl1d70_j55_0eta290_020jvt_bdl1d85_j45f_pf_ftf_preselj60XXj45XXf40_L1jJ80p0ETA25_2jJ55_jJ50p30ETA49", l1SeedThresholds=['FSNOSEED']*3,stream=['Main'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_j80_0eta290_020jvt_bdl1d70_j60_0eta290_020jvt_bdl1d85_j45f_pf_ftf_preselj60XXj45XXf40_L1jJ80p0ETA25_2jJ55_jJ50p30ETA49", l1SeedThresholds=['FSNOSEED']*3,stream=['Main'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_j50_0eta290_020jvt_bdl1d70_2j45f_pf_ftf_preselj45XX2f40_L1jJ55p0ETA23_2jJ40p30ETA49",l1SeedThresholds=['FSNOSEED']*2,  stream=['Main'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_j55_0eta290_020jvt_bdl1d70_2j45f_pf_ftf_preselj45XX2f40_L1jJ55p0ETA23_2jJ40p30ETA49",l1SeedThresholds=['FSNOSEED']*2,  stream=['Main'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j65a_j45a_2j35a_SHARED_2j35_0eta290_020jvt_bdl1d70_j0_DJMASS1000j50_pf_ftf_presela60XXa40XX2a25_L1jMJJ-500-NFF', l1SeedThresholds=['FSNOSEED']*5,stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiBjetGroup+Topo3Group),
        ChainProp(name='HLT_j70a_j50a_2j35a_SHARED_2j35_0eta290_020jvt_bdl1d70_j0_DJMASS1000j50_pf_ftf_presela60XXa40XX2a25_L1jMJJ-500-NFF', l1SeedThresholds=['FSNOSEED']*5,stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiBjetGroup+Topo3Group),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1d82_pf_ftf_presel2c20XX2c20b85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['Main'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1d82_pf_ftf_presel2c20XX2c20b85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['Main'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d77_pf_ftf_presel2c20XX2c20b85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d77_pf_ftf_presel2c20XX2c20b85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1d82_pf_ftf_presel2c20XX2c20b82_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['Main'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1d82_pf_ftf_presel2c20XX2c20b82_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['Main'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d77_pf_ftf_presel2c20XX2c20b82_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d77_pf_ftf_presel2c20XX2c20b82_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1d82_pf_ftf_presel2c20XX2c20b80_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['Main'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1d82_pf_ftf_presel2c20XX2c20b80_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['Main'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d77_pf_ftf_presel2c20XX2c20b80_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d77_pf_ftf_presel2c20XX2c20b80_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_5j35c_020jvt_j25c_020jvt_SHARED_j25c_020jvt_bdl1d60_pf_ftf_presel5c25XXc25b85_L14jJ40', l1SeedThresholds=['FSNOSEED']*3, stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_5j45c_020jvt_j25c_020jvt_SHARED_j25c_020jvt_bdl1d60_pf_ftf_presel5c25XXc25b85_L14jJ40', l1SeedThresholds=['FSNOSEED']*3, stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiBjetGroup),

        # downshift after jet calibration
        ChainProp(name="HLT_j225_0eta290_020jvt_bgn170_pf_ftf_preselj180_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup, monGroups=['bJetMon:online']),
        ChainProp(name="HLT_j300_0eta290_020jvt_bgn177_pf_ftf_preselj225_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup),
        ChainProp(name="HLT_j360_0eta290_020jvt_bgn185_pf_ftf_preselj225_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup),
        ChainProp(name="HLT_j225_0eta290_020jvt_bgn170_pf_ftf_preselj190_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup, monGroups=['bJetMon:online']),
        ChainProp(name="HLT_j225_0eta290_020jvt_bgn170_pf_ftf_preselj200_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup, monGroups=['bJetMon:online']),
        ChainProp(name="HLT_j225_0eta290_020jvt_bgn160_pf_ftf_preselj180_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup),
        ChainProp(name="HLT_j275_0eta290_020jvt_bgn170_pf_ftf_preselj225_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup),
        ChainProp(name="HLT_j300_0eta290_020jvt_bgn170_pf_ftf_preselj225_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup),
        ChainProp(name="HLT_j360_0eta290_020jvt_bgn177_pf_ftf_preselj225_L1J100", l1SeedThresholds=['FSNOSEED'], groups=PrimaryLegGroup+SingleBjetGroup),
        ChainProp(name="HLT_j175_0eta290_020jvt_bgn160_j60_0eta290_020jvt_bgn160_pf_ftf_preselj140b85XXj45b85_L1J100", l1SeedThresholds=['FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MultiBjetGroup),
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
        # HT-seeded
        ChainProp(name='HLT_2j45_0eta290_020jvt_bgn170_j0_HT290_j0_DJMASS700j35_pf_ftf_L1HT150-J20s5pETA31_MJJ-400-CF', l1SeedThresholds=['FSNOSEED']*3, groups=PrimaryLegGroup+MultiBjetGroup+LegacyTopoGroup),
        
        # VBF chains
        ChainProp(name='HLT_j65a_j45a_2j35a_SHARED_2j35_0eta290_020jvt_bgn170_j0_DJMASS1000j50_pf_ftf_presela60XXa40XX2a25_L1MJJ-500-NFF', l1SeedThresholds=['FSNOSEED']*5,stream=['VBFDelayed'], groups=PrimaryLegGroup+MultiBjetGroup+LegacyTopoGroup),

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
        ChainProp(name='HLT_e5_lhvloose_L1eEM5_bBeeM6000_L1All', l1SeedThresholds=['eEM5'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        ChainProp(name='HLT_2e5_lhvloose_L1eEM5_bBeeM6000_L1All', l1SeedThresholds=['eEM5'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        ChainProp(name='HLT_e5_lhvloose_L1eEM5_e3_lhvloose_L1eEM5_bBeeM6000_L1All', l1SeedThresholds=['eEM5','eEM5'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        ChainProp(name='HLT_e5_lhvloose_L1EM3_bBeeM6000_L1All', l1SeedThresholds=['EM3'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        ChainProp(name='HLT_2e5_lhvloose_L1EM3_bBeeM6000_L1All', l1SeedThresholds=['EM3'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        ChainProp(name='HLT_e5_lhvloose_L1EM3_e3_lhvloose_L1EM3_bBeeM6000_L1All', l1SeedThresholds=['EM3','EM3'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        #
        ChainProp(name='HLT_e5_lhvloose_bBeeM6000_L1BKeePrimaryLegacy', l1SeedThresholds=['EM3'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        ChainProp(name='HLT_2e5_lhvloose_bBeeM6000_L1BKeePrimaryLegacy', l1SeedThresholds=['EM3'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        ChainProp(name='HLT_e5_lhvloose_bBeeM6000_L1BKeePrescaledLegacy', l1SeedThresholds=['EM3'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        ChainProp(name='HLT_2e5_lhvloose_bBeeM6000_L1BKeePrescaledLegacy', l1SeedThresholds=['EM3'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        #
        ChainProp(name='HLT_2e5_lhvloose_bBeeM6000_L1eEM26M', l1SeedThresholds=['eEM5'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        ChainProp(name='HLT_e5_lhvloose_e3_lhvloose_bBeeM6000_L1eEM26M', l1SeedThresholds=['eEM5','eEM5'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        ChainProp(name='HLT_e5_lhvloose_bBeeM6000_L1eEM26M', l1SeedThresholds=['eEM5'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        #
        ChainProp(name='HLT_2e5_lhvloose_bBeeM6000_L14jJ40', l1SeedThresholds=['eEM5'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        ChainProp(name='HLT_e5_lhvloose_e3_lhvloose_bBeeM6000_L14jJ40', l1SeedThresholds=['eEM5','eEM5'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        ChainProp(name='HLT_e5_lhvloose_bBeeM6000_L14jJ40', l1SeedThresholds=['eEM5'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        ChainProp(name='HLT_2e5_lhvloose_bBeeM6000_L14J15', l1SeedThresholds=['EM3'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        ChainProp(name='HLT_e5_lhvloose_bBeeM6000_L14J15', l1SeedThresholds=['EM3'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        #
        ChainProp(name='HLT_2e5_lhvloose_bBeeM6000_L1BPH-0M9-eEM9-eEM7_MU5VF', l1SeedThresholds=['eEM5'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        ChainProp(name='HLT_e5_lhvloose_e3_lhvloose_bBeeM6000_L1BPH-0M9-eEM9-eEM7_MU5VF', l1SeedThresholds=['eEM5','eEM5'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        ChainProp(name='HLT_e5_lhvloose_bBeeM6000_L1BPH-0DR3-eEM9jJ40_MU5VF', l1SeedThresholds=['eEM5'], stream=['BphysDelayed'], groups=BphysElectronGroup),
        ChainProp(name='HLT_2e5_lhvloose_bBeeM6000_L1BPH-0M9-EM7-EM5_MU5VF', l1SeedThresholds=['EM3'], stream=['BphysDelayed'], groups=BphysElectronGroup+LegacyTopoGroup),
        ChainProp(name='HLT_e5_lhvloose_bBeeM6000_L1BPH-0DR3-EM7J15_MU5VF', l1SeedThresholds=['EM3'], stream=['BphysDelayed'], groups=BphysElectronGroup+LegacyTopoGroup),

        # ATR-27780
        ChainProp(name='HLT_g5_nopid_L1EM3', groups=SinglePhotonGroup+['PS:NoBulkMCProd']),
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
       
        #  ATR-28612 - Removal of legacy EM items to MC menu
        # From Physics_pp_run3_v1
        ChainProp(name='HLT_e5_etcut_L1EM3' , groups=SingleElectronGroup+SupportLegGroup+['RATE:CPS_EM3']+['PS:NoBulkMCProd']),
        ChainProp(name='HLT_e14_lhvloose_L1EM10VH', groups=SupportLegGroup+SingleElectronGroup+['RATE:CPS_EM10VH']),
        ChainProp(name='HLT_g15_tight_L1EM10VH', groups=SupportLegGroup+SinglePhotonGroup+['RATE:CPS_EM10VH']),
        ChainProp(name='HLT_g15_loose_L1EM10VH', monGroups=['egammaMon:shifter'], groups=SupportLegGroup+SinglePhotonGroup+['RATE:CPS_EM10VH']),
        ChainProp(name='HLT_e5_idperf_tight_L1EM3', groups=SingleElectronGroup+SupportLegGroup+['RATE:CPS_EM3'], monGroups=['idMon:t0']),
        ChainProp(name='HLT_e5_idperf_tight_nogsf_L1EM3', groups=SingleElectronGroup+SupportLegGroup+['RATE:CPS_EM3']),
        ChainProp(name='HLT_g35_medium_g25_medium_L1EM7_EMPTY', l1SeedThresholds=['EM7']*2, stream=['Late'], groups=PrimaryLegGroup+MultiPhotonGroup),
        ChainProp(name='HLT_2g22_tight_L1EM7_EMPTY', l1SeedThresholds=['EM7'], stream=['Late'], groups=PrimaryLegGroup+MultiPhotonGroup),
        ChainProp(name='HLT_2g50_tight_L1EM7_EMPTY', l1SeedThresholds=['EM7'], stream=['Late'], groups=PrimaryLegGroup+MultiPhotonGroup),  
        ChainProp(name='HLT_noalg_L1EM3',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+EgammaStreamersGroup+SupportLegGroup),
        #ChainProp(name='HLT_noalg_L1EM7',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+EgammaStreamersGroup+SupportLegGroup),
        #ChainProp(name='HLT_noalg_L1EM12',    l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+EgammaStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1EM15',    l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+EgammaStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1EM10VH',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+EgammaStreamersGroup+SupportLegGroup),
        #ChainProp(name='HLT_noalg_L1EM15VH',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+EgammaStreamersGroup+SupportLegGroup),
        #ChainProp(name='HLT_noalg_L1EM20VH',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+EgammaStreamersGroup+SupportLegGroup),
        #ChainProp(name='HLT_noalg_L1EM22VHI', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+EgammaStreamersGroup+SupportLegGroup),

        ChainProp(name='HLT_g12_loose_LArPEBHLT_L1EM10VH', stream=['LArCells'], groups=SinglePhotonGroup+SupportLegGroup),
        ChainProp(name='HLT_g20_loose_LArPEBHLT_L1EM15', stream=['LArCells'], groups=SinglePhotonGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_LArPEBCalib_L1EM10VH', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=['RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_noalg_LArPEBCalib_L1EM15', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=['RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_g3_loose_LArPEBHLT_L1EM3', stream=['LArCells'], groups=SinglePhotonGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_LArPEBCalib_L1EM3', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=['RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_noalg_LArPEBNoise_L1EM3_EMPTY',  l1SeedThresholds=['EM3'],  stream=['LArCellsEmpty'],groups=['RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_noalg_LArPEBNoise_L1EM7_EMPTY',  l1SeedThresholds=['EM7'], stream=['LArCellsEmpty'],groups=['RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_noalg_LArPEBNoise_L1EM7_FIRSTEMPTY',  l1SeedThresholds=['EM7'],  stream=['LArCellsEmpty'],groups=['RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1EM3_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['CosmicCalo'],groups=['RATE:Cosmic_Calo','BW:MinBias','RATE:Calibration']+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1EM7_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['CosmicCalo','express'],groups=['RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportLegGroup),
        
        # ATR-27940
        ChainProp(name='HLT_g35_medium_g25_medium_L12eEM9_EMPTY', l1SeedThresholds=['eEM9']*2, stream=['Late'], groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_2g22_tight_L12eEM9_EMPTY', l1SeedThresholds=['eEM9'], stream=['Late'], groups=PrimaryPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_2g50_tight_L12eEM9_EMPTY', l1SeedThresholds=['eEM9'], stream=['Late'], groups=PrimaryPhIGroup+MultiPhotonGroup),

        # From Cosmic_run3_v1.py
        ChainProp(name='HLT_g3_etcut_LArPEB_L1EM3', stream=['LArCells'], groups=['RATE:SinglePhoton', 'BW:Egamma']),
        
        # ATR-28822
        ChainProp(name='HLT_e14_lhtight_e4_etcut_1invmAB5_L1JPSI-1M5-EM12', stream=['Main'], l1SeedThresholds=['EM12','EM3'], groups=SupportLegGroup+MultiElectronGroup+['RATE:CPS_JPSI-1M5-EM12']+LegacyTopoGroup),
        ChainProp(name='HLT_e14_lhtight_e4_idperf_tight_nogsf_probe_1invmAB5_L1JPSI-1M5-EM12', l1SeedThresholds=['EM12','PROBEEM3'], groups=SupportLegGroup+MultiElectronGroup+['RATE:CPS_JPSI-1M5-EM12']+LegacyTopoGroup),
        ChainProp(name='HLT_e14_lhtight_e4_idperf_tight_probe_1invmAB5_L1JPSI-1M5-EM12', stream=['Main','express'], l1SeedThresholds=['EM12','PROBEEM3'], groups=SupportLegGroup+MultiElectronGroup+['RATE:CPS_JPSI-1M5-EM12']+LegacyTopoGroup, monGroups=['idMon:t0']),
        ChainProp(name='HLT_e5_lhtight_e14_etcut_1invmAB5_L1JPSI-1M5-EM12', stream=['Main'], l1SeedThresholds=['EM3','EM12'], groups=SupportLegGroup+MultiElectronGroup+['RATE:CPS_JPSI-1M5-EM12']+LegacyTopoGroup),
        ChainProp(name='HLT_e5_lhtight_e9_etcut_1invmAB5_L1JPSI-1M5-EM7',  stream=['Main'], l1SeedThresholds=['EM3','EM7'], groups=SupportLegGroup+MultiElectronGroup+['RATE:CPS_JPSI-1M5-EM7']+LegacyTopoGroup),
        #Jpsiee        
        ChainProp(name='HLT_e9_lhtight_e4_etcut_1invmAB5_L1JPSI-1M5-EM7',  stream=['Main'], l1SeedThresholds=['EM7','EM3'], groups=SupportLegGroup+MultiElectronGroup+['RATE:CPS_JPSI-1M5-EM7']+LegacyTopoGroup),
        # LRT idperf
        ChainProp(name='HLT_e9_lhtight_e4_idperf_tight_probe_1invmAB5_L1JPSI-1M5-EM7', stream=['Main'], l1SeedThresholds=['EM7','PROBEEM3'], groups=SupportLegGroup+MultiElectronGroup+['RATE:CPS_JPSI-1M5-EM7']+LegacyTopoGroup),
        ChainProp(name='HLT_e9_lhtight_e4_idperf_tight_nogsf_probe_1invmAB5_L1JPSI-1M5-EM7', stream=['Main'], l1SeedThresholds=['EM7','PROBEEM3'], groups=SupportLegGroup+MultiElectronGroup+['RATE:CPS_JPSI-1M5-EM7']+LegacyTopoGroup),

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
        #ATR-28692
        ChainProp(name='HLT_tau40_mediumRNN_tracktwoMVA_tau35_mediumRNN_tracktwoMVA_03dRAB_L1TAU25IM_2TAU20IM',   l1SeedThresholds=['TAU25IM','TAU20IM'], groups=SupportLegGroup+MultiTauGroup),
        #ATR-28842
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB_L1TAU20IM_2TAU12IM', l1SeedThresholds=['TAU20IM','TAU12IM'], groups=SupportLegGroup+MultiTauGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L1TAU20IM_2TAU12IM', l1SeedThresholds=['TAU20IM','TAU12IM'], stream=['VBFDelayed'], groups=SupportLegGroup+MultiTauGroup),
        ChainProp(name='HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB_L1TAU20IM_2TAU12IM_4J12p0ETA25', l1SeedThresholds=['TAU20IM','TAU12IM'], groups=PrimaryLegGroup+MultiTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L1TAU20IM_2TAU12IM_4J12p0ETA25', l1SeedThresholds=['TAU20IM','TAU12IM'], stream=['VBFDelayed'], groups=PrimaryLegGroup+MultiTauGroup),

        
        # Asymmetric Legacy tau chains for HH->bbtautau
        ChainProp(name="HLT_tau25_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_L1DR-TAU20ITAU12I-J25", l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+LegacyTopoGroup),
        ChainProp(name="HLT_tau35_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_L1DR-TAU20ITAU12I-J25", l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+LegacyTopoGroup),
        ChainProp(name="HLT_tau40_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_L1DR-TAU20ITAU12I-J25", l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+LegacyTopoGroup),
        ChainProp(name="HLT_tau25_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB30_L1DR-TAU20ITAU12I-J25", l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+LegacyTopoGroup),
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB30_L1DR-TAU20ITAU12I-J25", l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+LegacyTopoGroup),
        ChainProp(name="HLT_tau30_idperf_tracktwoMVA_tau20_idperf_tracktwoMVA_03dRAB30_L1DR-TAU20ITAU12I-J25", l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+LegacyTopoGroup),
    ]

    chains['Calib'] = [
        ChainProp(name='HLT_larpsallem_L1EM3_EMPTY', l1SeedThresholds=['EM3'], stream=['CosmicCalo'],groups=['RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportLegGroup),
        ChainProp(name='HLT_larpsallem_L1EM7_FIRSTEMPTY', l1SeedThresholds=['EM7'], stream=['LArNoiseBurst'], groups=['RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_larpsallem_L1EM7_EMPTY', l1SeedThresholds=['EM7'], stream=['CosmicCalo'],groups=['RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportLegGroup),
        ChainProp(name='HLT_larnoiseburst_L1J40_XE50', l1SeedThresholds=['FSNOSEED'], stream=['LArNoiseBurst'], groups=['RATE:Calibration','BW:Detector']+SupportLegGroup), # ATR-28761 moved
        ChainProp(name='HLT_larnoiseburst_L1J40_XE60', l1SeedThresholds=['FSNOSEED'], stream=['LArNoiseBurst'], groups=['RATE:Calibration','BW:Detector']+SupportLegGroup), # ATR-28761 moved

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

        #ATR-28679
        ChainProp(name='HLT_noalg_L1XE35',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportLegGroup+METStreamersGroup),
        ChainProp(name='HLT_noalg_L1XE40',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportLegGroup+METStreamersGroup),
        ChainProp(name='HLT_noalg_L1XE45',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportLegGroup+METStreamersGroup),
        ChainProp(name='HLT_noalg_L1XE60',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+SupportLegGroup+METStreamersGroup),
        #L1 Legacy Removal
        ChainProp(name='HLT_noalg_L1J25',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=JetStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1J30',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+JetStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1J40',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+JetStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1J75',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+JetStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1J85',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=JetStreamersGroup+SupportLegGroup),

        # TODO add once L1 items/thresholds are in place
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_TOF_T0T1_J50', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_TOF_T0T1_J75', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_TOF_J50', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_TOF_J75', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:NoBulkMCProd']+MinBiasGroup+SupportLegGroup),


    ]

    chains['Calib'] += [
        ChainProp(name='HLT_larpsall_L1TAU8_EMPTY', l1SeedThresholds=['TAU8'], stream=['CosmicCalo'], groups=['RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportLegGroup),
        ChainProp(name='HLT_noalg_LArPEBNoise_L1TAU8_EMPTY', l1SeedThresholds=['TAU8'], stream=['LArCellsEmpty'], groups=['RATE:Calibration','BW:Detector']+SupportLegGroup),
    ]

    chains['Combined'] += [
        # Primary e+mu
        ChainProp(name='HLT_e7_lhmedium_mu24_L1MU14FCH',l1SeedThresholds=['EM3','MU14FCH'],  stream=[PhysicsStream], groups=PrimaryLegGroup+EgammaMuonGroup),
        ChainProp(name='HLT_e7_lhmedium_mu24_L1MU18VFCH',l1SeedThresholds=['EM3','MU18VFCH'],  stream=[PhysicsStream], groups=PrimaryLegGroup+EgammaMuonGroup),
        #ATR-22107
        ChainProp(name='HLT_e9_lhvloose_mu20_mu8noL1_L1MU14FCH', l1SeedThresholds=['EM3','MU14FCH','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryLegGroup+EgammaMuonGroup),
        # ATR-25512
        ChainProp(name='HLT_e9_lhvloose_mu23_mu8noL1_L1MU18VFCH', l1SeedThresholds=['EM3','MU18VFCH','FSNOSEED'], stream=[PhysicsStream], groups=PrimaryLegGroup+EgammaMuonGroup),

        # Primary g-mu chains
        ChainProp(name='HLT_g25_medium_mu24_L1MU14FCH',l1SeedThresholds=['EM15VH','MU14FCH'], stream=[PhysicsStream], groups=PrimaryLegGroup+EgammaMuonGroup), #ATR-22594
         # ATR-25512
        ChainProp(name='HLT_g25_medium_mu24_L1MU18VFCH',l1SeedThresholds=['EM15VH','MU18VFCH'], stream=[PhysicsStream], groups=PrimaryLegGroup+EgammaMuonGroup), #ATR-22594
        ChainProp(name='HLT_2g10_loose_mu20_L1MU14FCH', l1SeedThresholds=['EM7','MU14FCH'], stream=[PhysicsStream], groups=PrimaryLegGroup+EgammaMuonGroup), # unsure what EM seed should be
        ChainProp(name='HLT_2g10_loose_mu23_L1MU18VFCH', l1SeedThresholds=['EM7','MU18VFCH'], stream=[PhysicsStream], groups=PrimaryLegGroup+EgammaMuonGroup), # unsure what EM seed should be
        ChainProp(name='HLT_g25_medium_mu24_ivarmedium_L1MU14FCH', l1SeedThresholds=['EM22VHI','MU14FCH'], stream=[PhysicsStream], groups=PrimaryLegGroup+EgammaMuonGroup),
        ChainProp(name='HLT_g25_medium_mu24_ivarmedium_L1MU18VFCH', l1SeedThresholds=['EM22VHI','MU18VFCH'], stream=[PhysicsStream], groups=PrimaryLegGroup+EgammaMuonGroup),
        # ATR-20505
        ChainProp(name='HLT_g40_loose_mu40_msonly_L1MU14FCH', l1SeedThresholds=['EM20VH','MU14FCH'], stream=[PhysicsStream], groups=PrimaryLegGroup+EgammaMuonGroup),
        ChainProp(name='HLT_g40_loose_mu40_msonly_L1MU18VFCH', l1SeedThresholds=['EM20VH','MU18VFCH'], stream=[PhysicsStream], groups=PrimaryLegGroup+EgammaMuonGroup),

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

        # ATR-28761 L1_3J15p0ETA25_XE40 moved to MC
        ChainProp(name="HLT_3j35_0eta290_020jvt_bdl1d60_pf_ftf_xe50_cell_xe70_tcpufit_L13J15p0ETA25_XE40", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=['Main'], groups=PrimaryLegGroup+BjetMETGroup),
        ChainProp(name="HLT_3j35_0eta290_020jvt_bdl1d60_pf_ftf_xe50_cell_xe70_pfopufit_L13J15p0ETA25_XE40", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=['Main'], groups=PrimaryLegGroup+BjetMETGroup),
        ChainProp(name="HLT_3j35_0eta290_020jvt_bgn160_pf_ftf_xe50_cell_xe70_tcpufit_L13J15p0ETA25_XE40", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=['Main'], groups=PrimaryLegGroup+BjetMETGroup),
        ChainProp(name="HLT_3j35_0eta290_020jvt_bgn160_pf_ftf_xe50_cell_xe70_pfopufit_L13J15p0ETA25_XE40", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=['Main'], groups=PrimaryLegGroup+BjetMETGroup),
        # ATR-28761 L1_2J15_XE55 moved to MC
        ChainProp(name="HLT_2j45_0eta290_020jvt_bdl1d60_pf_ftf_xe50_cell_xe85_tcpufit_L12J15_XE55", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=['Main'], groups=PrimaryLegGroup+BjetMETGroup),
        ChainProp(name="HLT_2j45_0eta290_020jvt_bdl1d60_pf_ftf_xe50_cell_xe85_pfopufit_L12J15_XE55", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=['Main'], groups=PrimaryLegGroup+BjetMETGroup),
        ChainProp(name="HLT_2j45_0eta290_020jvt_bgn160_pf_ftf_xe50_cell_xe85_tcpufit_L12J15_XE55", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=['Main'], groups=PrimaryLegGroup+BjetMETGroup),
        ChainProp(name="HLT_2j45_0eta290_020jvt_bgn160_pf_ftf_xe50_cell_xe85_pfopufit_L12J15_XE55", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=['Main'], groups=PrimaryLegGroup+BjetMETGroup),

        ChainProp(name='HLT_j75_0eta290_020jvt_bdl1d60_pf_ftf_xe60_cell_L12J50_XE40', l1SeedThresholds=['FSNOSEED','FSNOSEED'], stream=['Main'], groups=PrimaryLegGroup+BjetMETGroup), # downshift
        ChainProp(name='HLT_j80_0eta290_020jvt_bdl1d60_pf_ftf_xe60_cell_L12J50_XE40', l1SeedThresholds=['FSNOSEED','FSNOSEED'], stream=['Main'], groups=PrimaryLegGroup+BjetMETGroup),
        ChainProp(name='HLT_j75_0eta290_020jvt_bgn160_pf_ftf_xe60_cell_L12J50_XE40', l1SeedThresholds=['FSNOSEED','FSNOSEED'], stream=['Main'], groups=PrimaryLegGroup+BjetMETGroup), # downshift
        ChainProp(name='HLT_j80_0eta290_020jvt_bgn160_pf_ftf_xe60_cell_L12J50_XE40', l1SeedThresholds=['FSNOSEED','FSNOSEED'], stream=['Main'], groups=PrimaryLegGroup+BjetMETGroup),


        # e + tau intermediate Phase-I tag, Legacy probe chains       
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau160_mediumRNN_tracktwoMVA_probe_L1TAU100_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBETAU100'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau180_mediumRNN_tracktwoLLP_probe_L1TAU100_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBETAU100'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau80_mediumRNN_tracktwoMVA_probe_L1TAU60_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBETAU60'], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau80_mediumRNN_tracktwoLLP_probe_L1TAU60_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBETAU60'], groups=TagAndProbePhIGroup+SingleElectronGroup),

        ChainProp(name='HLT_e28_lhtight_ivarloose_tau160_mediumRNN_tracktwoMVA_probe_L1TAU100_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBETAU100'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau180_mediumRNN_tracktwoLLP_probe_L1TAU100_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBETAU100'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau80_mediumRNN_tracktwoMVA_probe_L1TAU60_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBETAU60'], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau80_mediumRNN_tracktwoLLP_probe_L1TAU60_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBETAU60'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        #ATR-28692
        ChainProp(name='HLT_e17_lhmedium_ivarloose_tau25_mediumRNN_tracktwoMVA_03dRAB_L1EM15VHI_2TAU12IM_4J12', l1SeedThresholds=['EM15VHI','TAU12IM'], groups=PrimaryLegGroup+EgammaTauGroup),

        # photon + tau
        #ATR-27251, Phase-I
        ChainProp(name='HLT_g140_loose_tau20_mediumRNN_tracktwoMVA_probe_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M','PROBETAU8'], groups=TagAndProbePhIGroup+TauPhotonGroup),
        ChainProp(name='HLT_g140_loose_tau20_mediumRNN_tracktwoMVA_probe_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M','PROBETAU8'], groups=TagAndProbePhIGroup+TauPhotonGroup),

        # mu + tau Legacy probe chains
        # Legacy tau+X chains with muon L1
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
        #ATR-28692
        ChainProp(name='HLT_mu14_ivarloose_tau35_mediumRNN_tracktwoMVA_03dRAB_L1MU8F_TAU20IM', l1SeedThresholds=['MU8F','TAU20IM'], groups=PrimaryLegGroup+MuonTauGroup),
        ChainProp(name='HLT_mu14_ivarloose_tau25_mediumRNN_tracktwoMVA_03dRAB_L1MU8F_TAU12IM_3J12', l1SeedThresholds=['MU8F','TAU12IM'], groups=PrimaryLegGroup+MuonTauGroup),

        
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


        #ATR-28692
        ChainProp(name='HLT_e17_lhmedium_tau25_mediumRNN_tracktwoMVA_xe50_cell_03dRAB_L1EM15VHI_2TAU12IM_XE35', l1SeedThresholds=['EM15VHI','TAU12IM','FSNOSEED'], groups=PrimaryLegGroup+TauMETGroup),
        ChainProp(name='HLT_mu14_tau25_mediumRNN_tracktwoMVA_xe50_cell_03dRAB_L1MU8F_TAU12IM_XE35', l1SeedThresholds=['MU8F','TAU12IM','FSNOSEED'], groups=PrimaryLegGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_xe50_cell_03dRAB_L1TAU40_2TAU12IM_XE40', l1SeedThresholds=['TAU40','TAU12IM','FSNOSEED'], groups=PrimaryLegGroup+TauMETGroup),  # ATR-22966

        #ATR-28784
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_j45_pf_ftf_preselj20_03dRAB_L1J15', l1SeedThresholds=['PROBETAU8', 'FSNOSEED'], groups=SupportLegGroup+TauJetGroup+['RATE:CPS_J15']),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_j60_pf_ftf_preselj50_03dRAB_L1J20', l1SeedThresholds=['PROBETAU8', 'FSNOSEED'], groups=SupportLegGroup+TauJetGroup+['RATE:CPS_J20']),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_j85_pf_ftf_preselj50_03dRAB_L1J20', l1SeedThresholds=['PROBETAU8', 'FSNOSEED'], groups=SupportLegGroup+TauJetGroup+['RATE:CPS_J20']),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_j175_pf_ftf_preselj140_03dRAB_L1J50', l1SeedThresholds=['PROBETAU8', 'FSNOSEED'], groups=SupportLegGroup+TauJetGroup+['RATE:CPS_J50']),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_j360_pf_ftf_preselj225_03dRAB_L1J100', l1SeedThresholds=['PROBETAU8', 'FSNOSEED'], groups=SupportLegGroup+TauJetGroup+['RATE:CPS_J100']),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_j420_pf_ftf_preselj225_03dRAB_L1J100', l1SeedThresholds=['PROBETAU8', 'FSNOSEED'], groups=TagAndProbeLegGroup+TauJetGroup),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_j440_pf_ftf_preselj225_03dRAB_L1J100', l1SeedThresholds=['PROBETAU8', 'FSNOSEED'], groups=TagAndProbeLegGroup+TauJetGroup),         
        
        #ATR-28761 Legacy mu + jet moved
        ChainProp(name='HLT_mu4_j35_0eta290_020jvt_boffperf_pf_ftf_dRAB04_L1MU3V_J15',   l1SeedThresholds=['MU3V' ,'FSNOSEED'], groups=SupportLegGroup+MuonBjetGroup, monGroups=['muonMon:online','bJetMon:online'], stream=['Main',         ]),
        ChainProp(name='HLT_mu6_j60_0eta290_020jvt_boffperf_pf_ftf_dRAB04_L1MU3V_J15',   l1SeedThresholds=['MU3V' ,'FSNOSEED'], groups=SupportLegGroup+MuonBjetGroup, monGroups=['muonMon:online','bJetMon:online'], stream=['Main',         ]),


        # Asymmetric Legacy tau chains for HH->bbtautau
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_j20c_020jvt_bgn185_pf_ftf_L1DR-TAU20ITAU12I-J25', l1SeedThresholds=['TAU20IM','TAU12IM', 'FSNOSEED'], stream=['VBFDelayed'], groups=TauBJetGroup),
        ChainProp(name='HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_4j20c_020jvt_SHARED_j20c_020jvt_bgn185_pf_ftf_L1DR-TAU20ITAU12I-J25', l1SeedThresholds=['TAU20IM','TAU12IM']+2*['FSNOSEED'], stream=['VBFDelayed'], groups=TauBJetGroup),

        #L1 Legacy Removal
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_j110_pf_ftf_preselj80_03dRAB_L1J30', l1SeedThresholds=['PROBETAU8', 'FSNOSEED'], groups=SupportLegGroup+TauJetGroup+['RATE:CPS_J30']),
        ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_j260_pf_ftf_preselj200_03dRAB_L1J75', l1SeedThresholds=['PROBETAU8', 'FSNOSEED'], groups=SupportLegGroup+TauJetGroup+['RATE:CPS_J75']),

        # LLP late stream
        ChainProp(name='HLT_j55c_xe50_cell_L1J30_EMPTY', l1SeedThresholds=['FSNOSEED']*2, stream=['Late'], groups=PrimaryLegGroup+JetMETGroup),
        ChainProp(name='HLT_j55c_xe50_cell_L1J30_FIRSTEMPTY', l1SeedThresholds=['FSNOSEED']*2, stream=['Late'], groups=PrimaryLegGroup+JetMETGroup),

        # SUSY
        ChainProp(name='HLT_g45_loose_6j45c_L14J15p0ETA25',l1SeedThresholds=['EM15','FSNOSEED'], groups=PrimaryLegGroup+EgammaJetGroup),
        
        # ATR-28761
        ChainProp(name='HLT_2j120_mb_afprec_afpdijet_L1AFP_A_AND_C_TOF_J50', l1SeedThresholds=['FSNOSEED']*2, stream=['Main'],groups=MinBiasGroup+SupportLegGroup, monGroups=['mbMon:shifter']),
        ChainProp(name='HLT_2j175_mb_afprec_afpdijet_L1AFP_A_AND_C_TOF_J75', l1SeedThresholds=['FSNOSEED']*2, stream=['Main'],groups=MinBiasGroup+SupportLegGroup, monGroups=['mbMon:t0']),

        # ATR-28795 bjet menu cleanup
        ChainProp(name='HLT_g60_tight_2j55_0eta200_emergingPTF0p1dR0p4_pf_ftf_L1eEM26M', groups=SupportPhIGroup+EgammaJetGroup+['RATE:CPS_eEM26M'], l1SeedThresholds=['eEM26M','FSNOSEED']),
        ChainProp(name='HLT_g45_tight_2j55_0eta200_emergingPTF0p1dR0p4_pf_ftf_L1eEM26M', groups=SupportPhIGroup+EgammaJetGroup+['RATE:CPS_eEM26M'], l1SeedThresholds=['eEM26M','FSNOSEED']),
        ChainProp(name='HLT_g60_tight_icaloloose_2j55_0eta200_emergingPTF0p1dR0p4_pf_ftf_L1eEM26M', groups=PrimaryPhIGroup+EgammaJetGroup, l1SeedThresholds=['eEM26M','FSNOSEED']),
        ChainProp(name='HLT_g45_tight_icaloloose_2j55_0eta200_emergingPTF0p1dR0p4_pf_ftf_L1eEM26M', groups=PrimaryPhIGroup+EgammaJetGroup, l1SeedThresholds=['eEM26M','FSNOSEED']),
        ChainProp(name='HLT_g60_tight_2j55_0eta200_emergingPTF0p1dR0p4_pf_ftf_L1eEM28M', groups=SupportPhIGroup+EgammaJetGroup+['RATE:CPS_eEM28M'], l1SeedThresholds=['eEM28M','FSNOSEED']),
        ChainProp(name='HLT_g45_tight_2j55_0eta200_emergingPTF0p1dR0p4_pf_ftf_L1eEM28M', groups=SupportPhIGroup+EgammaJetGroup+['RATE:CPS_eEM28M'], l1SeedThresholds=['eEM28M','FSNOSEED']),
        ChainProp(name='HLT_g60_tight_icaloloose_2j55_0eta200_emergingPTF0p1dR0p4_pf_ftf_L1eEM28M', groups=PrimaryPhIGroup+EgammaJetGroup, l1SeedThresholds=['eEM28M','FSNOSEED']),
        ChainProp(name='HLT_g45_tight_icaloloose_2j55_0eta200_emergingPTF0p1dR0p4_pf_ftf_L1eEM28M', groups=PrimaryPhIGroup+EgammaJetGroup, l1SeedThresholds=['eEM28M','FSNOSEED']),

        ChainProp(name='HLT_e26_lhtight_ivarloose_mu22noL1_2j20_0eta290_020jvt_bdl1d85_pf_ftf_L1eEM26M', l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'], stream=['Main','express'], groups=TagAndProbePhIGroup+EgammaBjetGroup, monGroups=['bJetMon:shifter','bJetMon:online']),
        ChainProp(name='HLT_e28_lhtight_ivarloose_mu22noL1_2j20_0eta290_020jvt_bdl1d85_pf_ftf_L1eEM28M', l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'], stream=['Main','express'], groups=TagAndProbePhIGroup+EgammaBjetGroup, monGroups=['bJetMon:shifter','bJetMon:online']),
        ChainProp(name='HLT_g20_tight_icaloloose_j35_0eta290_020jvt_bdl1d77_3j35a_j0_DJMASS500j35_pf_ftf_L1EM18VHI_MJJ-300',l1SeedThresholds=['EM18VHI','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['Main'], groups=PrimaryLegGroup+EgammaBjetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_g20_tight_j35_0eta290_020jvt_bdl1d77_3j35a_j0_DJMASS500j35_pf_ftf_L1EM18VHI_MJJ-300',l1SeedThresholds=['EM18VHI','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['Main'], groups=SupportLegGroup+EgammaBjetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_g20_tight_icaloloose_j35_0eta290_020jvt_bdl1d77_3j35a_j0_DJMASS500j35_pf_ftf_L1eEM22M_jMJJ-300',l1SeedThresholds=['eEM22M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['Main'], groups=PrimaryPhIGroup+EgammaBjetGroup+Topo2Group),
        ChainProp(name='HLT_g20_tight_j35_0eta290_020jvt_bgn177_3j35a_j0_DJMASS500j35_pf_ftf_L1EM18VHI_MJJ-300',l1SeedThresholds=['EM18VHI','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['Main'], groups=SupportLegGroup+EgammaBjetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_g25_medium_2j35_0eta290_020jvt_bdl1d77_2j35a_pf_ftf_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=['Main'], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g25_medium_j35_0eta290_020jvt_bdl1d77_3j35a_j0_DJMASS700j35_pf_ftf_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['Main'], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g25_tight_2j35_0eta290_020jvt_bdl1d77_2j35a_pf_ftf_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=['Main'], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g25_tight_icaloloose_2j35_0eta290_020jvt_bdl1d77_2j35a_pf_ftf_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=['Main'], groups=PrimaryPhIGroup+EgammaBjetGroup),
        ChainProp(name='HLT_g25_tight_icaloloose_j35_0eta290_020jvt_bdl1d77_3j35a_j0_DJMASS700j35_pf_ftf_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['Main'], groups=PrimaryPhIGroup+EgammaBjetGroup),
        ChainProp(name='HLT_g25_tight_j35_0eta290_020jvt_bdl1d77_3j35a_j0_DJMASS700j35_pf_ftf_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['Main'], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g25_medium_2j35_0eta290_020jvt_bdl1d77_2j35a_pf_ftf_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=['Main'], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_g25_medium_j35_0eta290_020jvt_bdl1d77_3j35a_j0_DJMASS700j35_pf_ftf_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['Main'], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_g25_tight_2j35_0eta290_020jvt_bdl1d77_2j35a_pf_ftf_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=['Main'], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_g25_tight_icaloloose_2j35_0eta290_020jvt_bdl1d77_2j35a_pf_ftf_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=['Main'], groups=PrimaryPhIGroup+EgammaBjetGroup),
        ChainProp(name='HLT_g25_tight_icaloloose_j35_0eta290_020jvt_bdl1d77_3j35a_j0_DJMASS700j35_pf_ftf_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['Main'], groups=PrimaryPhIGroup+EgammaBjetGroup),
        ChainProp(name='HLT_g25_tight_j35_0eta290_020jvt_bdl1d77_3j35a_j0_DJMASS700j35_pf_ftf_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['Main'], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_g25_tight_2j35_0eta290_020jvt_bdl1d77_2j35a_pf_ftf_presel2a20b90XX2a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=['Main'], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g25_tight_j35_0eta290_020jvt_bdl1d77_3j35a_j0_DJMASS700j35_pf_ftf_presela20b85XX3a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['Main'], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g25_tight_2j35_0eta290_020jvt_bdl1d77_2j35a_pf_ftf_presel2a20b90XX2a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=['Main'], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_g25_tight_j35_0eta290_020jvt_bdl1d77_3j35a_j0_DJMASS700j35_pf_ftf_presela20b85XX3a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['Main'], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_g25_tight_icaloloose_2j35_0eta290_020jvt_bdl1d77_2j35a_pf_ftf_presel2a20b90XX2a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=['Main'], groups=PrimaryPhIGroup+EgammaBjetGroup),
        ChainProp(name='HLT_g25_tight_icaloloose_2j35_0eta290_020jvt_bdl1d77_2j35a_pf_ftf_presel2a20b90XX2a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=['Main'], groups=PrimaryPhIGroup+EgammaBjetGroup),
        ChainProp(name='HLT_g25_tight_icaloloose_j35_0eta290_020jvt_bdl1d77_3j35a_j0_DJMASS700j35_pf_ftf_presela20b85XX3a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['Main'], groups=PrimaryPhIGroup+EgammaBjetGroup),
        ChainProp(name='HLT_g25_tight_icaloloose_j35_0eta290_020jvt_bdl1d77_3j35a_j0_DJMASS700j35_pf_ftf_presela20b85XX3a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['Main'], groups=PrimaryPhIGroup+EgammaBjetGroup),
        ChainProp(name='HLT_g25_medium_2j35_0eta290_020jvt_bdl1d77_2j35a_pf_ftf_presel2a20b90XX2a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=['Main'], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g25_medium_2j35_0eta290_020jvt_bdl1d77_2j35a_pf_ftf_presel2a20b90XX2a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=['Main'], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_g25_medium_j35_0eta290_020jvt_bdl1d77_3j35a_j0_DJMASS700j35_pf_ftf_presela20b85XX3a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['Main'], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g25_medium_j35_0eta290_020jvt_bdl1d77_3j35a_j0_DJMASS700j35_pf_ftf_presela20b85XX3a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['Main'], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_g35_medium_2j35_0eta290_020jvt_bdl1d77_2j35a_pf_ftf_presel2a20b90XX2a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=['Main'], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g35_medium_2j35_0eta290_020jvt_bdl1d77_2j35a_pf_ftf_presel2a20b90XX2a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=['Main'], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_g35_medium_j35_0eta290_020jvt_bdl1d77_3j35a_j0_DJMASS700j35_pf_ftf_presela20b85XX3a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['Main'], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g35_medium_j35_0eta290_020jvt_bdl1d77_3j35a_j0_DJMASS700j35_pf_ftf_presela20b85XX3a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['Main'], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_g35_tight_2j35_0eta290_020jvt_bdl1d77_2j35a_pf_ftf_presel2a20b90XX2a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=['Main'], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g35_tight_2j35_0eta290_020jvt_bdl1d77_2j35a_pf_ftf_presel2a20b90XX2a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=['Main'], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_g35_tight_icaloloose_2j35_0eta290_020jvt_bdl1d77_2j35a_pf_ftf_presel2a20b90XX2a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=['Main'], groups=PrimaryPhIGroup+EgammaBjetGroup),
        ChainProp(name='HLT_g35_tight_icaloloose_2j35_0eta290_020jvt_bdl1d77_2j35a_pf_ftf_presel2a20b90XX2a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED'],stream=['Main'], groups=PrimaryPhIGroup+EgammaBjetGroup),
        ChainProp(name='HLT_g35_tight_icaloloose_j35_0eta290_020jvt_bdl1d77_3j35a_j0_DJMASS700j35_pf_ftf_presela20b85XX3a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['Main'], groups=PrimaryPhIGroup+EgammaBjetGroup),
        ChainProp(name='HLT_g35_tight_j35_0eta290_020jvt_bdl1d77_3j35a_j0_DJMASS700j35_pf_ftf_presela20b85XX3a20_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['Main'], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM26M']),
        ChainProp(name='HLT_g35_tight_j35_0eta290_020jvt_bdl1d77_3j35a_j0_DJMASS700j35_pf_ftf_presela20b85XX3a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['Main'], groups=SupportPhIGroup+EgammaBjetGroup+['RATE:CPS_eEM28M']),
        ChainProp(name='HLT_g35_tight_icaloloose_j35_0eta290_020jvt_bdl1d77_3j35a_j0_DJMASS700j35_pf_ftf_presela20b85XX3a20_L1eEM28M',l1SeedThresholds=['eEM28M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['Main'], groups=PrimaryPhIGroup+EgammaBjetGroup),

        ChainProp(name='HLT_j75_0eta290_020jvt_bdl1d60_pf_ftf_xe60_cell_L12jJ90_jXE80', l1SeedThresholds=['FSNOSEED','FSNOSEED'], stream=['Main'], groups=PrimaryPhIGroup+BjetMETGroup),
        ChainProp(name='HLT_j80_0eta290_020jvt_bdl1d60_pf_ftf_xe60_cell_L12jJ90_jXE80', l1SeedThresholds=['FSNOSEED','FSNOSEED'], stream=['Main'], groups=PrimaryPhIGroup+BjetMETGroup),
        ChainProp(name="HLT_j95_0eta290_020jvt_bdl1d60_pf_ftf_xe50_cell_xe85_tcpufit_L1XE55", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=['Main'], groups=PrimaryLegGroup+BjetMETGroup),
        ChainProp(name="HLT_j100_0eta290_020jvt_bdl1d60_pf_ftf_xe50_cell_xe85_tcpufit_L1XE55", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=['Main'], groups=PrimaryLegGroup+BjetMETGroup),
        ChainProp(name="HLT_2j45_0eta290_020jvt_bdl1d60_pf_ftf_xe50_cell_xe85_tcpufit_L12jJ40_jXE110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=['Main'], groups=PrimaryPhIGroup+BjetMETGroup),
        ChainProp(name="HLT_3j35_0eta290_020jvt_bdl1d60_pf_ftf_xe50_cell_xe70_tcpufit_L13jJ40p0ETA25_jXE80", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=['Main'], groups=PrimaryPhIGroup+BjetMETGroup),
        ChainProp(name="HLT_j95_0eta290_020jvt_bdl1d60_pf_ftf_xe50_cell_xe85_pfopufit_L1XE55", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=['Main'], groups=PrimaryLegGroup+BjetMETGroup),
        ChainProp(name="HLT_j100_0eta290_020jvt_bdl1d60_pf_ftf_xe50_cell_xe85_pfopufit_L1XE55", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=['Main'], groups=PrimaryLegGroup+BjetMETGroup),
        ChainProp(name="HLT_2j45_0eta290_020jvt_bdl1d60_pf_ftf_xe50_cell_xe85_pfopufit_L12jJ40_jXE110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=['Main'], groups=PrimaryPhIGroup+BjetMETGroup),
        ChainProp(name="HLT_3j35_0eta290_020jvt_bdl1d60_pf_ftf_xe50_cell_xe70_pfopufit_L13jJ40p0ETA25_jXE80", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=['Main'], groups=PrimaryPhIGroup+BjetMETGroup),
        ChainProp(name="HLT_j95_0eta290_020jvt_bdl1d60_pf_ftf_xe50_cell_xe85_tcpufit_L1jXE110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=['Main'], groups=PrimaryPhIGroup+BjetMETGroup),
        ChainProp(name="HLT_j100_0eta290_020jvt_bdl1d60_pf_ftf_xe50_cell_xe85_tcpufit_L1jXE110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=['Main'], groups=PrimaryPhIGroup+BjetMETGroup),
        ChainProp(name="HLT_j95_0eta290_020jvt_bdl1d60_pf_ftf_xe50_cell_xe85_pfopufit_L1jXE110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=['Main'], groups=PrimaryPhIGroup+BjetMETGroup),
        ChainProp(name="HLT_j100_0eta290_020jvt_bdl1d60_pf_ftf_xe50_cell_xe85_pfopufit_L1jXE110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=['Main'], groups=PrimaryPhIGroup+BjetMETGroup),
        ChainProp(name="HLT_j95_0eta290_020jvt_bdl1d60_pf_ftf_xe50_cell_xe85_tcpufit_L1gXEJWOJ110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=['Main'], groups=PrimaryPhIGroup+BjetMETGroup),
        ChainProp(name="HLT_j100_0eta290_020jvt_bdl1d60_pf_ftf_xe50_cell_xe85_tcpufit_L1gXEJWOJ110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=['Main'], groups=PrimaryPhIGroup+BjetMETGroup),
        ChainProp(name="HLT_j95_0eta290_020jvt_bdl1d60_pf_ftf_xe50_cell_xe85_pfopufit_L1gXEJWOJ110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=['Main'], groups=PrimaryPhIGroup+BjetMETGroup),
        ChainProp(name="HLT_j100_0eta290_020jvt_bdl1d60_pf_ftf_xe50_cell_xe85_pfopufit_L1gXEJWOJ110", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=['Main'], groups=PrimaryPhIGroup+BjetMETGroup),
        ChainProp(name='HLT_j80_0eta290_020jvt_bgn160_pf_ftf_xe60_cell_L12jJ90_jXE80', l1SeedThresholds=['FSNOSEED','FSNOSEED'], stream=['Main'], groups=PrimaryPhIGroup+BjetMETGroup),
        ChainProp(name="HLT_j100_0eta290_020jvt_bgn160_pf_ftf_xe50_cell_xe85_tcpufit_L1XE55", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=['Main'], groups=PrimaryLegGroup+BjetMETGroup),
        ChainProp(name="HLT_j100_0eta290_020jvt_bgn160_pf_ftf_xe50_cell_xe85_pfopufit_L1XE55", l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED'], stream=['Main'], groups=PrimaryLegGroup+BjetMETGroup),

        ChainProp(name='HLT_2e5_lhmedium_j70_j50a_j0_DJMASS900j50_L1MJJ-500-NFF',l1SeedThresholds=['EM3','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['VBFDelayed'], groups=PrimaryLegGroup+EgammaJetGroup+LegacyTopoGroup),
        # Delayed stream VBF
        ChainProp(name='HLT_2mu6_2j50a_j0_DJMASS900j50_L1MJJ-500-NFF',l1SeedThresholds=['MU5VF','FSNOSEED','FSNOSEED'],stream=['VBFDelayed'], groups=PrimaryLegGroup+MuonJetGroup+LegacyTopoGroup), # Formerly HLT_2mu6_2j50_0eta490_invm900j50                       
        # Main stream VBF
        ChainProp(name='HLT_e10_lhmedium_ivarloose_j70_j50a_j0_DJMASS900j50_L1MJJ-500-NFF',l1SeedThresholds=['EM8VH','FSNOSEED','FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+EgammaJetGroup+LegacyTopoGroup), 
        ChainProp(name='HLT_e5_lhvloose_j70_j50a_j0_DJMASS1000j50_xe50_tcpufit_L1MJJ-500-NFF',l1SeedThresholds=['EM3','FSNOSEED','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['VBFDelayed'], groups=PrimaryLegGroup+EgammaJetGroup+LegacyTopoGroup),
        # Photon+VBF
        ChainProp(name='HLT_g20_tight_icaloloose_j35_0eta290_020jvt_bgn177_3j35a_j0_DJMASS500j35_pf_ftf_L1EM18VHI_MJJ-300',l1SeedThresholds=['EM18VHI','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['Main'], groups=PrimaryLegGroup+EgammaBjetGroup+LegacyTopoGroup),
        
        ChainProp(name='HLT_j70_j50a_j0_DJMASS1000j50dphi240_xe90_tcpufit_xe50_cell_L1MJJ-500-NFF',l1SeedThresholds=['FSNOSEED']*5,stream=['VBFDelayed'], groups=PrimaryLegGroup+JetMETGroup+LegacyTopoGroup),
        ChainProp(name='HLT_mu10_ivarmedium_j70_j50a_j0_DJMASS900j50_L1MJJ-500-NFF',l1SeedThresholds=['MU8F','FSNOSEED','FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+MuonJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_mu4_j70_j50a_j0_DJMASS1000j50_xe50_tcpufit_L1MJJ-500-NFF',l1SeedThresholds=['MU3V','FSNOSEED','FSNOSEED','FSNOSEED','FSNOSEED'],stream=['VBFDelayed'], groups=PrimaryLegGroup+MuonJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_j70_j50a_j0_DJMASS900j50_L1MJJ-500-NFF',l1SeedThresholds=['TAU8','TAU8','FSNOSEED','FSNOSEED','FSNOSEED'], groups=PrimaryLegGroup+TauJetGroup+LegacyTopoGroup),

    ]

    return chains

def setupMenu(menu_name):
    
    chains = physics_menu.setupMenu(menu_name)

    log.info('[setupMenu] going to add the MC menu chains now')

    for sig,chainsInSig in getMCSignatures().items():
        chains[sig] += chainsInSig

    return chains
