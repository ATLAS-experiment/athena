# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# Dev_pp_run3_v1.py menu for the long shutdown development
#------------------------------------------------------------------------#

# All chains are represented as ChainProp objects in a ChainStore
from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp
from .SignatureDicts import ChainStore

from . import MC_pp_run3_v1 as mc_menu

from .Physics_pp_run3_v1 import (PhysicsStream,
                                 SingleMuonGroup,
                                 MultiMuonGroup,
                                 METGroup,
                                 SingleJetGroup,
                                 MultiJetGroup,
                                 SingleBjetGroup,
                                 MultiBjetGroup,
                                 SingleTauGroup,
                                 DiTauGroup,
                                 MultiTauGroup,
                                 MultiPhotonGroup,
                                 TauBJetGroup,
                                 BphysicsGroup,
                                 EgammaMETGroup,
                                 EgammaJetGroup,
                                 MuonJetGroup,
                                 MinBiasGroup,
                                 SupportGroup,
                                 SupportLegGroup,
                                 SupportPhIGroup,
                                 UnconvTrkGroup,
                                 METPhaseIStreamersGroup,
                                 EOFTLALegGroup,
                                 LegacyTopoGroup,
                                 Topo2Group,
                                 Topo3Group,
                                 EOFL1MuGroup,
                                 EOFBPhysL1MuGroup,
                                 )

# Some of the group names are modified for MC and Dev, see the MC menu or ATR-30593 for more info.
from .MC_pp_run3_v1 import (PrimaryLegGroup,
                            PrimaryPhIGroup,
                            PrimaryL1MuGroup,
                            TagAndProbeLegGroup,
                            TagAndProbePhIGroup,
                            )

DevGroup = ['Development']

def getDevSignatures():
    chains = ChainStore()
    chains['Muon'] = [
        # ATR-28412 muonDPJ+VBF
        ChainProp(name='HLT_mu15_msonly_L1jMJJ-500-NFF', l1SeedThresholds=['MU5VF'], groups=PrimaryPhIGroup+SingleMuonGroup+Topo3Group),
        ChainProp(name='HLT_mu12_msonly_L1jMJJ-500-NFF', l1SeedThresholds=['MU5VF'], groups=PrimaryPhIGroup+SingleMuonGroup+Topo3Group),
        ChainProp(name='HLT_mu20_msonly_L1jMJJ-500-NFF', l1SeedThresholds=['MU14FCH'], groups=PrimaryPhIGroup+SingleMuonGroup+Topo3Group),
        ChainProp(name='HLT_mu6_msonly_L1jMJJ-500-NFF', l1SeedThresholds=['MU3V'], groups=PrimaryPhIGroup+SingleMuonGroup+Topo3Group),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan40_L1jMJJ-500-NFF', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryPhIGroup+MultiMuonGroup+Topo3Group),
        ChainProp(name='HLT_mu10_msonly_iloosems_mu6noL1_msonly_nscan40_L1jMJJ-500-NFF', l1SeedThresholds=['MU5VF','FSNOSEED'], groups=PrimaryPhIGroup+MultiMuonGroup+Topo3Group),

        #-- nscan ATR-19376, TODO: to be moved to physics once rated
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan20_L1MU14FCH_J40', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryLegGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan30_L1MU14FCH_J40', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryLegGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan40_L1MU14FCH_J40', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryLegGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan20_L1MU14FCH_J50', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryLegGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan30_L1MU14FCH_J50', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryLegGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan20_L1MU14FCH_jJ90', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryPhIGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan30_L1MU14FCH_jJ90', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryPhIGroup+MultiMuonGroup),        
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan20_L1MU14FCH_XE30', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryLegGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan30_L1MU14FCH_XE30', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryLegGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan40_L1MU14FCH_XE30', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryLegGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan20_L1MU14FCH_XE40', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryLegGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan30_L1MU14FCH_XE40', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryLegGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan20_L1MU14FCH_jXE80', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryPhIGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan30_L1MU14FCH_jXE80', l1SeedThresholds=['MU14FCH','FSNOSEED'], groups=PrimaryPhIGroup+MultiMuonGroup),        
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan10_L110DR-MU14FCH-MU5VF', l1SeedThresholds=['MU14FCH','FSNOSEED'],   groups=PrimaryL1MuGroup+MultiMuonGroup+Topo2Group),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan20_L110DR-MU14FCH-MU5VF', l1SeedThresholds=['MU14FCH','FSNOSEED'],   groups=PrimaryL1MuGroup+MultiMuonGroup+Topo2Group),
        ChainProp(name='HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan30_L110DR-MU14FCH-MU5VF', l1SeedThresholds=['MU14FCH','FSNOSEED'],   groups=PrimaryL1MuGroup+MultiMuonGroup+Topo2Group),

        #ATR-26727 - low mass Drell-Yan triggers
        ChainProp(name='HLT_2mu4_7invmAA9_L12MU3VF', l1SeedThresholds=['MU3VF'], groups=MultiMuonGroup+SupportGroup+['RATE:CPS_2MU3VF']),
        ChainProp(name='HLT_2mu4_11invmAA60_L12MU3VF', l1SeedThresholds=['MU3VF'], groups=MultiMuonGroup+SupportGroup+['RATE:CPS_2MU3VF']),


        ChainProp(name='HLT_mu6_ivarmedium_L1MU5VF', groups=DevGroup+SingleMuonGroup),


        # Test ID T&P
        ChainProp(name='HLT_mu14_idtp_L1MU8F', groups=SingleMuonGroup+SupportGroup, monGroups=['idMon:shifter','idMon:t0']),

        # ATR-19452
        ChainProp(name='HLT_2mu4_muonqual_L12MU3V',  groups=DevGroup+MultiMuonGroup),
        ChainProp(name='HLT_2mu6_muonqual_L12MU5VF', groups=DevGroup+MultiMuonGroup),

        #ATR-21003
        ChainProp(name='HLT_2mu14_l2io_L12MU8F', groups=DevGroup+MultiMuonGroup),
        ChainProp(name='HLT_2mu6_l2io_L12MU5VF', groups=DevGroup+MultiMuonGroup),
        
        # Test T&P dimuon
        ChainProp(name='HLT_mu24_mu6_L1MU14FCH', l1SeedThresholds=['MU14FCH','MU3V'], groups=DevGroup+MultiMuonGroup),
        ChainProp(name='HLT_mu24_mu6_probe_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEMU3V'], groups=DevGroup+MultiMuonGroup),

        # ATR-22782, ATR-28868, 4mu analysis
        ChainProp(name='HLT_mu4_ivarloose_mu4_L1BPH-7M14-0DR25-MU5VFMU3VF', l1SeedThresholds=['MU3VF','MU3VF'], stream=['BphysDelayed'], groups=MultiMuonGroup+EOFBPhysL1MuGroup+Topo3Group),
  
        #ATR-31457 - HLT chains for muon AD BDT (NOMAD)
        ChainProp(name='HLT_2mu4_L1ADBDTL',               l1SeedThresholds=['MU3V'], stream=['Main'], groups=Topo3Group+ MultiMuonGroup+EOFL1MuGroup),
        ChainProp(name='HLT_2mu4_l2io_invmDimu_L1ADBDTL', l1SeedThresholds=['MU3V'], stream=['Main'], groups=Topo3Group+ MultiMuonGroup+EOFL1MuGroup),
    ]

    chains['Egamma'] = [
        # test chain for EgammaPEBTLA building type
        ChainProp(name='HLT_g7_loose_EgammaPEBTLA_L1eEM5',l1SeedThresholds=['eEM5'], stream=['EgammaPEBTLA'], groups=SupportPhIGroup+DevGroup),
        ChainProp(name='HLT_g7_loose_L1eEM5',l1SeedThresholds=['eEM5'], groups=SupportPhIGroup+DevGroup),

        # ATR-23625
        ChainProp(name='HLT_g50_medium_g20_medium_L12eEM18M', l1SeedThresholds=['eEM18M','eEM18M'], groups=SupportPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_g50_medium_g20_medium_L12eEM18L', l1SeedThresholds=['eEM18L','eEM18L'], groups=SupportPhIGroup+MultiPhotonGroup),
    ]

    chains['MET'] = [

        #ATR-28679
        ChainProp(name='HLT_xe30_cell_L1jXE60',       l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_mht_L1jXE60',        l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_tcpufit_L1jXE60',    l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_trkmht_L1jXE60',     l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_pfsum_L1jXE60',      l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_pfsum_cssk_L1jXE60', l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_pfsum_vssk_L1jXE60', l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_pfopufit_L1jXE60',   l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_cvfpufit_L1jXE60',   l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_mhtpufit_em_L1jXE60', l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_mhtpufit_pf_L1jXE60', l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),

        ChainProp(name='HLT_xe30_cell_L1gXEJWOJ60',       l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_mht_L1gXEJWOJ60',        l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_tcpufit_L1gXEJWOJ60',    l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_trkmht_L1gXEJWOJ60',     l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_pfsum_L1gXEJWOJ60',      l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_pfsum_cssk_L1gXEJWOJ60', l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_pfsum_vssk_L1gXEJWOJ60', l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_pfopufit_L1gXEJWOJ60',   l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_cvfpufit_L1gXEJWOJ60',   l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_mhtpufit_em_L1gXEJWOJ60', l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_mhtpufit_pf_L1gXEJWOJ60', l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
    ]

    chains['Jet'] = [
        # new calratio chain  fo comparison only
        ChainProp(name='HLT_j20_CLEANllp_momemfrac006_calratiovar_roiftf_preselj20emf6_L1eTAU80HL', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
        ChainProp(name='HLT_j20_CLEANllp_momemfrac006_calratiovar_roiftf_preselj20emf6_L1eTAU60HL', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
        # new calratio chain  
        ChainProp(name='HLT_j20_CLEANllp_momemfrac006_calratiovar_roiftf_preselj20emf6_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
        ChainProp(name='HLT_j20_CLEANllp_momemfrac006_calratiovarrmbib_roiftf_preselj20emf6_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
        ChainProp(name='HLT_j20_CLEANllp_momemfrac006_calratiovar_roiftf_preselj20emf6_L1eTAU60_EMPTY', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
        ChainProp(name='HLT_j20_CLEANllp_momemfrac006_calratiovarrmbib_roiftf_preselj20emf6_L1eTAU60_EMPTY', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
        ChainProp(name='HLT_j20_CLEANllp_momemfrac006_calratiovar_roiftf_preselj20emf6_L1eTAU60_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
        ChainProp(name='HLT_j20_CLEANllp_momemfrac006_calratiovarrmbib_roiftf_preselj20emf6_L1eTAU60_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
        ChainProp(name='HLT_j20_CLEANllp_momemfrac006_calratiovar_roiftf_preselj20emf6_L1eTAU40HT', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
        ChainProp(name='HLT_j20_CLEANllp_momemfrac006_calratiovarrmbib_roiftf_preselj20emf6_L1eTAU40HT', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
        ChainProp(name='HLT_j20_CLEANllp_momemfrac006_calratiovar_roiftf_preselj20emf6_L1eTAU60HM', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
        ChainProp(name='HLT_j20_CLEANllp_momemfrac006_calratiovarrmbib_roiftf_preselj20emf6_L1eTAU60HM', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
        # Phase I duplicates for primary calratio TAU
        ChainProp(name='HLT_j20_CLEANllp_momemfrac006_calratiovar_roiftf_preselj20emf6_L1eTAU140', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
        ChainProp(name='HLT_j20_CLEANllp_momemfrac006_calratiovarrmbib_roiftf_preselj20emf6_L1eTAU140', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),

        # ATR-28412 test lower than VBF inclusive
 
        # ATR-28412 test caloratio with VBF

        ChainProp(name='HLT_j20_CLEANllp_momemfrac072_calratiovar59_roiftf_preselj20emf72_L1eTAU80', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
        ChainProp(name='HLT_j20_CLEANllp_momemfrac072_calratiovar59_roiftf_preselj20emf72_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),

        # pflow jet chains without pile-up residual correction for calibration derivations and calibration cross-checks ATR-26827
        ChainProp(name='HLT_j0_perf_pf_subjesgscIS_ftf_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_j25_pf_subjesgscIS_ftf_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),

        # candidate jet TLA chains ATR-20395
        ChainProp(name='HLT_4j25_PhysicsTLA_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=DevGroup+MultiJetGroup), # adding for study of EMTopo TLA fast b-tagging.
        ## with calo fast-tag presel - so actually btag TLA ATR-23002
        ChainProp(name='HLT_2j20_2j20_pf_ftf_presel2j25XX2j25b85_PhysicsTLA_L14jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*2, stream=['TLA'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_5j20_j20_pf_ftf_presel5c25XXc25b85_PhysicsTLA_L14jJ40', l1SeedThresholds=['FSNOSEED']*2, stream=['TLA'], groups=MultiJetGroup+DevGroup),

        # RPV SUSY TLA DIPZ test chains aiming to improve the benchmark physics menu chain in ATR-28985
        ChainProp(name='HLT_j0_pf_ftf_presel6c25_PhysicsTLA_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['tlaMon:shifter']), # Benchmark chain without main selection
        ChainProp(name='HLT_j0_pf_ftf_presel5c20_PhysicsTLA_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['tlaMon:shifter']), 
        ChainProp(name='HLT_j0_pf_ftf_presel6c20_PhysicsTLA_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['tlaMon:shifter']), 
        ChainProp(name='HLT_6j20c_020jvt_pf_ftf_preselZ219XX6c20_PhysicsTLA_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_6j20_pf_ftf_preselZ219XX6c20_PhysicsTLA_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['tlaMon:shifter']), 
        ChainProp(name='HLT_j0_pf_ftf_preselZ219XX6c20_PhysicsTLA_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_6j20_pf_ftf_preselZ197XX6c20_PhysicsTLA_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_j0_pf_ftf_preselZ197XX6c20_PhysicsTLA_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_6j20_pf_ftf_preselZ182XX6c20_PhysicsTLA_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_j0_pf_ftf_preselZ182XX6c20_PhysicsTLA_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_6j20_pf_ftf_preselZ142XX5c20_PhysicsTLA_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['tlaMon:shifter']), 
        ChainProp(name='HLT_j0_pf_ftf_preselZ142XX5c20_PhysicsTLA_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_6j20c_020jvt_pf_ftf_preselZ134XX5c20_PhysicsTLA_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['tlaMon:shifter']), 
        ChainProp(name='HLT_6j20_pf_ftf_preselZ134XX5c20_PhysicsTLA_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['tlaMon:shifter']), 
        ChainProp(name='HLT_j0_pf_ftf_preselZ134XX5c20_PhysicsTLA_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_6j20c_020jvt_pf_ftf_preselZ124XX5c20_PhysicsTLA_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['tlaMon:shifter']), 
        ChainProp(name='HLT_6j20_pf_ftf_preselZ124XX5c20_PhysicsTLA_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['tlaMon:shifter']), 
        ChainProp(name='HLT_j0_pf_ftf_preselZ124XX5c20_PhysicsTLA_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['tlaMon:shifter']),


        #ATR-29775
        ChainProp(name='HLT_2j20_pf_ftf_presel3c45_PhysicsTLA_L1jJ85p0ETA21_3jJ40p0ETA25'                , l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=EOFTLALegGroup+MultiJetGroup),
        ChainProp(name='HLT_j20_j20_pf_ftf_presel3c20XX1c20bgtwo85_PhysicsTLA_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED', 'FSNOSEED'], stream=['TLA'], groups=EOFTLALegGroup+MultiJetGroup),

        ChainProp(name='HLT_2j20_pf_ftf_presel3c45_PhysicsTLA_L13jJ40p0ETA25'                , l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=EOFTLALegGroup+MultiJetGroup),
        ChainProp(name='HLT_j20_j20_pf_ftf_preselj140XX2j45_PhysicsTLA_L13jJ40p0ETA25'       , l1SeedThresholds=['FSNOSEED', 'FSNOSEED'], stream=['TLA'], groups=EOFTLALegGroup+MultiJetGroup),
        ChainProp(name='HLT_j20_j20_pf_ftf_preselj80XX2j45_PhysicsTLA_L13jJ40p0ETA25'        , l1SeedThresholds=['FSNOSEED', 'FSNOSEED'], stream=['TLA'], groups=EOFTLALegGroup+MultiJetGroup),
        ChainProp(name='HLT_j20_j20_pf_ftf_presel3c20XX1c20bgtwo85_PhysicsTLA_L13jJ40p0ETA25', l1SeedThresholds=['FSNOSEED', 'FSNOSEED'], stream=['TLA'], groups=EOFTLALegGroup+MultiJetGroup),

            ## Without pf_ftf part 
        ChainProp(name='HLT_j0_roiftf_presel6c25_L14jJ40', groups=MultiJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j0_roiftf_presel6c20_L14jJ40', groups=MultiJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j0_roiftf_presel5c20_L14jJ40', groups=MultiJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j0_Z219XX6c20_roiftf_presel6c20_L14jJ40', groups=MultiJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j0_Z197XX6c20_roiftf_presel6c20_L14jJ40', groups=MultiJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j0_Z182XX6c20_roiftf_presel6c20_L14jJ40', groups=MultiJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j0_Z142XX5c20_roiftf_presel5c20_L14jJ40', groups=MultiJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j0_Z134XX5c20_roiftf_presel5c20_L14jJ40', groups=MultiJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j0_Z124XX5c20_roiftf_presel5c20_L14jJ40', groups=MultiJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),



        #TLA+PEB test for jets ATR-21596, matching "multijet+PFlow" TLA chain in physics menu for cross-check of event size
        ChainProp(name='HLT_j20_pf_ftf_preselcHT450_DarkJetPEBTLA_L1HT190-jJ40s5pETA21', l1SeedThresholds=['FSNOSEED'], stream=['DarkJetPEBTLA'], groups=DevGroup+MultiJetGroup+Topo3Group),
        ChainProp(name='HLT_j20_DarkJetPEBTLA_L1HT190-jJ40s5pETA21', l1SeedThresholds=['FSNOSEED'], stream=['DarkJetPEBTLA'], groups=DevGroup+MultiJetGroup+Topo3Group),
        # Multijet TLA support
        ChainProp(name='HLT_2j20_2j20_pf_ftf_presel2c20XX2c20b85_DarkJetPEBTLA_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*2, stream=['DarkJetPEBTLA'], groups=DevGroup+MultiJetGroup, monGroups=['tlaMon:shifter']),
        # PEB for HH4b
        ChainProp(name='HLT_2j20_2j20_pf_ftf_presel2c20XX2c20b85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*2, stream=['Main'], groups=MultiJetGroup+DevGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_3j20_1j20_pf_ftf_presel3c20XX1c20bgtwo85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*2, stream=['Main'], groups=MultiJetGroup+DevGroup, monGroups=['tlaMon:shifter']),

        #
        ChainProp(name='HLT_4j20c_L14jJ40p0ETA25', l1SeedThresholds=['FSNOSEED'],     groups=MultiJetGroup+DevGroup), #ATR-26012
        #

        ### PURE TEST CHAINS


        # Emerging Jets test chains ATR-21593
        # alternate emerging jet single-jet chain

        #PANTELIS Emerging Jets test chains low pT threshold with restricted eta range of 1.4
        ChainProp(name='HLT_j200_0eta140_emergingPTF0p08dR1p2_a10sd_cssk_pf_jes_ftf_preselj160_L1jJ160', groups=SingleJetGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED']),

        # backup emerging jets chains to be used for rate refinement in enhanced bias reprocessing


        # end of emerging jets chains

        #####

        # Primary jet chains w/o preselection, for comparison


        # CSSKPFlow

        ##### End no-preselection

        # ATR-24720 Testing additions to Run 3 baseline menu
        # HT preselection studies
        ###


         #TLA+PEB test for jets ATR-21596, matching "multijet+PFlow" TLA chain in physics menu for cross-check of event size
        # HT preseleection tests
        # jet preselection
        # with HT leg at the HLT
        # + ht preselection
        # + jet preselection
        # no preselection

        # ATR-28103 Test chains for delayed jets, based on significance of delay
        ChainProp(name='HLT_3j45_j45_2timeSig_roiftf_presel4c35_L14jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup+DevGroup), 

        # ATR-28836 additional delayed jets more delay significance thresholds
        ChainProp(name='HLT_3j45_j45_2timeSig_L14jJ40', l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_j220_j150_2timeSig_L1jJ160', l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_3j45_j45_3timeSig_L14jJ40', l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_j220_j150_3timeSig_L1jJ160', l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_3j45_j45_2timing_L14jJ40', l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_j220_j150_2timing_L1jJ160', l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_2j45_2j45_2timeSig_L14jJ40', l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_2j45_2j45_3timeSig_L14jJ40', l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_2j100_2timeSig_L1jJ90', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),

        #ATR-28836 Test chains for delayed jets with upper limit
        ChainProp(name='HLT_3j45_j45_3timeSig15_L14jJ40', l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_j220_j150_3timeSig15_L1jJ160', l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_2j45_2j45_2timeSig15_L14jJ40', l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_2j45_2j45_3timeSig15_L14jJ40', l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup+DevGroup),

        ###High-PT single delayed jet chain
        ChainProp(name='HLT_j300_3timeSig15_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_j200_3timeSig_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_j250_3timeSig_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_j300_3timeSig_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),

        ChainProp(name='HLT_j300_2timing15_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_j200_2timing_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_j250_2timing_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_j300_2timing_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),

        # ATR-28836 Copies of delayed jets chains without timing hypo for reference
        ChainProp(name='HLT_3j45_j45_L14jJ40', l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_j220_j150_L1jJ160', l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup+DevGroup),

        ### END PURE TEST CHAINS

        
        # ATR-28352: Test chains for multijet DIPZ 
        ]


    chains['Bjet'] = [
        
        # Test chain for X to bb tagging with retrained tagger
        # 60% WP
        ChainProp(name='HLT_j210_35smcINF_a10sd_cssk_60bgntwoxt_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j210_35smcINF_a10sd_cssk_60bgntwoxt_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j210_35smcINF_a10sd_cssk_60bgntwoxt_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_a10sd_cssk_60bgntwoxt_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_a10sd_cssk_60bgntwoxt_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_a10sd_cssk_60bgntwoxt_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_60bgntwoxt_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_60bgntwoxt_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_60bgntwoxt_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_60bgntwoxt_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_60bgntwoxt_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_60bgntwoxt_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),

        # 65% WP
        ChainProp(name='HLT_j210_35smcINF_a10sd_cssk_65bgntwoxt_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j210_35smcINF_a10sd_cssk_65bgntwoxt_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j210_35smcINF_a10sd_cssk_65bgntwoxt_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_a10sd_cssk_65bgntwoxt_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_a10sd_cssk_65bgntwoxt_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_a10sd_cssk_65bgntwoxt_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_65bgntwoxt_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_65bgntwoxt_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_65bgntwoxt_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_65bgntwoxt_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_65bgntwoxt_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_65bgntwoxt_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),

        # 70% WP
        ChainProp(name='HLT_j210_35smcINF_a10sd_cssk_70bgntwoxt_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j210_35smcINF_a10sd_cssk_70bgntwoxt_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j210_35smcINF_a10sd_cssk_70bgntwoxt_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_a10sd_cssk_70bgntwoxt_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_a10sd_cssk_70bgntwoxt_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_a10sd_cssk_70bgntwoxt_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_70bgntwoxt_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_70bgntwoxt_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_70bgntwoxt_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_70bgntwoxt_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_70bgntwoxt_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_70bgntwoxt_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),

        # 75% WP
        ChainProp(name='HLT_j210_35smcINF_a10sd_cssk_75bgntwoxt_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j210_35smcINF_a10sd_cssk_75bgntwoxt_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j210_35smcINF_a10sd_cssk_75bgntwoxt_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_a10sd_cssk_75bgntwoxt_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_a10sd_cssk_75bgntwoxt_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_a10sd_cssk_75bgntwoxt_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_75bgntwoxt_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_75bgntwoxt_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_75bgntwoxt_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_75bgntwoxt_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_75bgntwoxt_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_75bgntwoxt_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),

        # 80% WP
        ChainProp(name='HLT_j210_35smcINF_a10sd_cssk_80bgntwoxt_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j210_35smcINF_a10sd_cssk_80bgntwoxt_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j210_35smcINF_a10sd_cssk_80bgntwoxt_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_a10sd_cssk_80bgntwoxt_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_a10sd_cssk_80bgntwoxt_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_a10sd_cssk_80bgntwoxt_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_80bgntwoxt_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_80bgntwoxt_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_80bgntwoxt_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_80bgntwoxt_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_80bgntwoxt_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_80bgntwoxt_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),

        # 85% WP
        ChainProp(name='HLT_j210_35smcINF_a10sd_cssk_85bgntwoxt_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j210_35smcINF_a10sd_cssk_85bgntwoxt_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j210_35smcINF_a10sd_cssk_85bgntwoxt_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_a10sd_cssk_85bgntwoxt_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_a10sd_cssk_85bgntwoxt_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_a10sd_cssk_85bgntwoxt_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_85bgntwoxt_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_85bgntwoxt_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_85bgntwoxt_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_85bgntwoxt_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_85bgntwoxt_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_85bgntwoxt_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),

        # 90% WP
        ChainProp(name='HLT_j210_35smcINF_a10sd_cssk_90bgntwoxt_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j210_35smcINF_a10sd_cssk_90bgntwoxt_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j210_35smcINF_a10sd_cssk_90bgntwoxt_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_a10sd_cssk_90bgntwoxt_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_a10sd_cssk_90bgntwoxt_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_a10sd_cssk_90bgntwoxt_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_90bgntwoxt_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_90bgntwoxt_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_90bgntwoxt_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_90bgntwoxt_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_90bgntwoxt_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_90bgntwoxt_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),

        ChainProp(name='HLT_j175C_35smcINF_90bgntwoxt_j260C_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260C_35smcINF_90bgntwoxt_j175C_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175C_35smcINF_90bgntwoxt_j260C_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260C_35smcINF_90bgntwoxt_j175C_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175C_35smcINF_90bgntwoxt_j260C_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260C_35smcINF_90bgntwoxt_j175C_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),

        # Monitoring
        ChainProp(name='HLT_j260C_35smcINF_a10sd_cssk_90bgntwoxt_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED'], monGroups=['jetMon:online', 'bJetMon:t0']),
        ChainProp(name='HLT_j260C_35smcINF_a10sd_cssk_90bgntwoxt_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED'], monGroups=['jetMon:online', 'bJetMon:t0']),
        ChainProp(name='HLT_j260C_35smcINF_a10sd_cssk_90bgntwoxt_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED'], monGroups=['jetMon:online', 'bJetMon:t0']),


        # 95% WP
        ChainProp(name='HLT_j210_35smcINF_a10sd_cssk_95bgntwoxt_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j210_35smcINF_a10sd_cssk_95bgntwoxt_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j210_35smcINF_a10sd_cssk_95bgntwoxt_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_a10sd_cssk_95bgntwoxt_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_a10sd_cssk_95bgntwoxt_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_a10sd_cssk_95bgntwoxt_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_95bgntwoxt_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_95bgntwoxt_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_95bgntwoxt_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_95bgntwoxt_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_95bgntwoxt_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_95bgntwoxt_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),

        # Test chain for X to bb tagging with offline GN2Xv01 tagger
        ChainProp(name='HLT_j110_a10sd_cssk_79bgntwox_pf_jes_ftf_preselj80_L1jJ60', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_79bgntwox_pf_jes_ftf_preselj140_L1jJ90', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_79bgntwox_pf_jes_ftf_preselj140_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_79bgntwox_pf_jes_ftf_preselj140_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_79bgntwox_pf_jes_ftf_preselj140_L1SC175-SCjJ10', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_79bgntwox_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj140_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_79bgntwox_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj140_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_79bgntwox_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj140_L1SC175-SCjJ10', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_79bgntwox_pf_jes_ftf_preselj200_L1jJ125', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_79bgntwox_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_79bgntwox_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_79bgntwox_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_79bgntwox_j260_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_79bgntwox_j260_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_79bgntwox_j260_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC175-SCjJ10', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j360_a10sd_cssk_79bgntwox_pf_jes_ftf_preselj225_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j420_a10sd_cssk_79bgntwox_pf_jes_ftf_preselj225_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j460_a10sd_cssk_79bgntwox_pf_jes_ftf_preselj225_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),

        ChainProp(name='HLT_j110_a10sd_cssk_86bgntwox_pf_jes_ftf_preselj80_L1jJ60', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_86bgntwox_pf_jes_ftf_preselj140_L1jJ90', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_86bgntwox_pf_jes_ftf_preselj140_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_a10sd_cssk_86bgntwox_pf_jes_ftf_preselj140_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_86bgntwox_pf_jes_ftf_preselj140_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_a10sd_cssk_86bgntwox_pf_jes_ftf_preselj140_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_86bgntwox_pf_jes_ftf_preselj140_L1SC175-SCjJ10', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_a10sd_cssk_86bgntwox_pf_jes_ftf_preselj140_L1SC175-SCjJ10', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_86bgntwox_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj140_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_86bgntwox_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj140_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_86bgntwox_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj140_L1SC175-SCjJ10', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_86bgntwox_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj140_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_86bgntwox_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj140_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_86bgntwox_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj140_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_86bgntwox_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj160_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_86bgntwox_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj160_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_86bgntwox_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj160_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_86bgntwox_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj160_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_86bgntwox_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj180_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_86bgntwox_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj180_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_86bgntwox_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj180_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_86bgntwox_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj180_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_86bgntwox_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_86bgntwox_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED'], monGroups=['jetMon:online']),
        ChainProp(name='HLT_j260_a10sd_cssk_86bgntwox_pf_jes_ftf_preselj200_L1jJ125', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_86bgntwox_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_2j260_a10sd_cssk_86bgntwox_pf_jes_ftf_presel2j225_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_86bgntwox_j260_a10sd_cssk_pf_jes_ftf_presel2j225_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_86bgntwox_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_2j260_a10sd_cssk_86bgntwox_pf_jes_ftf_presel2j225_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_86bgntwox_j260_a10sd_cssk_pf_jes_ftf_presel2j225_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=['FSNOSEED', 'FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_86bgntwox_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_2j260_a10sd_cssk_86bgntwox_pf_jes_ftf_presel2j225_L1SC175-SCjJ10', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_86bgntwox_j260_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC175-SCjJ10', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_86bgntwox_j260_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_86bgntwox_j260_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_86bgntwox_j260_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC175-SCjJ10', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j360_a10sd_cssk_86bgntwox_pf_jes_ftf_preselj225_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j420_a10sd_cssk_86bgntwox_pf_jes_ftf_preselj225_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j460_a10sd_cssk_86bgntwox_pf_jes_ftf_preselj225_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),

        ChainProp(name='HLT_j110_a10sd_cssk_91bgntwox_pf_jes_ftf_preselj80_L1jJ60', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_91bgntwox_pf_jes_ftf_preselj140_L1jJ90', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_91bgntwox_pf_jes_ftf_preselj140_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_91bgntwox_pf_jes_ftf_preselj140_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_91bgntwox_pf_jes_ftf_preselj140_L1SC175-SCjJ10', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_91bgntwox_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj140_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_91bgntwox_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj140_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_91bgntwox_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj140_L1SC175-SCjJ10', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_91bgntwox_pf_jes_ftf_preselj200_L1jJ125', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_91bgntwox_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_91bgntwox_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_91bgntwox_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_91bgntwox_j260_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_91bgntwox_j260_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_91bgntwox_j260_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC175-SCjJ10', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j360_a10sd_cssk_91bgntwox_pf_jes_ftf_preselj225_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j420_a10sd_cssk_91bgntwox_pf_jes_ftf_preselj225_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j460_a10sd_cssk_91bgntwox_pf_jes_ftf_preselj225_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),

        ChainProp(name='HLT_j110_a10sd_cssk_96bgntwox_pf_jes_ftf_preselj80_L1jJ60', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_96bgntwox_pf_jes_ftf_preselj140_L1jJ90', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_96bgntwox_pf_jes_ftf_preselj140_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_96bgntwox_pf_jes_ftf_preselj140_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_96bgntwox_pf_jes_ftf_preselj140_L1SC175-SCjJ10', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_96bgntwox_pf_jes_ftf_preselj200_L1jJ125', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_96bgntwox_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_96bgntwox_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_96bgntwox_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j360_a10sd_cssk_96bgntwox_pf_jes_ftf_preselj225_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j420_a10sd_cssk_96bgntwox_pf_jes_ftf_preselj225_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j460_a10sd_cssk_96bgntwox_pf_jes_ftf_preselj225_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),

        ######################################################################################################################################################################################################################################################
        #HH->bbbb
        # Phase-1 added ATR-28761
        ChainProp(name="HLT_j55c_020jvt_j40c_020jvt_j20c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b85_L1MU8F_2jJ40_jJ50", l1SeedThresholds=['FSNOSEED']*5, groups=MultiBjetGroup + DevGroup),
        ChainProp(name="HLT_j40c_020jvt_j40c_020jvt_j20c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b85_L1MU8F_2jJ40_jJ50", l1SeedThresholds=['FSNOSEED']*5, groups=MultiBjetGroup + DevGroup),
        ChainProp(name="HLT_j55c_020jvt_j28c_020jvt_j20c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b85_L1MU8F_2jJ40_jJ50", l1SeedThresholds=['FSNOSEED']*5, groups=MultiBjetGroup + DevGroup),
        ChainProp(name="HLT_j40c_020jvt_j28c_020jvt_j20c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b85_L1MU8F_2jJ40_jJ50", l1SeedThresholds=['FSNOSEED']*5, groups=MultiBjetGroup + DevGroup),
        ChainProp(name="HLT_j75c_020jvt_j50c_020jvt_j20c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b85_L1MU8F_2jJ40_jJ50", l1SeedThresholds=['FSNOSEED']*5, groups=MultiBjetGroup + DevGroup),
        ChainProp(name="HLT_j75c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b85_L1MU8F_2jJ40_jJ50", l1SeedThresholds=['FSNOSEED']*5, groups=MultiBjetGroup + DevGroup),
        
        # TEST CHAINS WITH ROIFTF PRESEL

        # ATR-28352: HH4b test chains with DIPZ
            # Test chains with full main selection (DIPZ in presel)
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup), #BaselineChain
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_preselZ120XX4c20_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_preselZ116MAXMULT20cXX4c20_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_preselZ138XX4c20_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),
            # Test chains with (DIPZ+b-jet in presel)   
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_preselZ128XX2c20XX2c20b85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*5, stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiBjetGroup),
            # Test chains with just DIPZ in preselection and no main selection:
        ChainProp(name='HLT_j0_pf_ftf_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),  #Baseline
        ChainProp(name='HLT_j0_pf_ftf_preselZ120XX4c20_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j0_pf_ftf_preselZ138MAXMULT5cXX4c20_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j0_pf_ftf_preselZ84XX3c20_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),
            # Test chains with b-tagging in preselection but no main selection at all:
        ChainProp(name='HLT_j0_j0_pf_ftf_presel2c20XX2c20b85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),  #Baseline
        ChainProp(name='HLT_j0_j0_pf_ftf_preselZ120XX2c20XX2c20b85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream], groups=DevGroup+MultiBjetGroup),

        # Tests of potential TLA chains for cost/rate
        # ATR-23002 - b-jets
        # Potential impovements of the FTagPEB stream (mid 2025)
        ChainProp(name='HLT_j20c_2j20c_pf_ftf_presel1c100XX2c20bgtwo85_FTagPEBTLA_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*2, stream=['FTagPEBTLA'], groups=MultiJetGroup+SupportPhIGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_j20c_2j20c_pf_ftf_presel1c120XX2c20bgtwo90_FTagPEBTLA_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*2, stream=['FTagPEBTLA'], groups=MultiJetGroup+SupportPhIGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_j20c_j20c_j20c_pf_ftf_presel1c120CXX1c20XX1c20bgtwo85_FTagPEBTLA_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*3, stream=['FTagPEBTLA'], groups=MultiJetGroup+SupportPhIGroup, monGroups=['tlaMon:shifter']),
        # using single jet L1
        ChainProp(name='HLT_j180c_j20c_pf_ftf_presel1c160XX1c20bgtwo90_FTagPEBTLA_L1jJ160', l1SeedThresholds=['FSNOSEED']*2, stream=['FTagPEBTLA'], groups=MultiJetGroup+SupportPhIGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_j180c_j20c_pf_ftf_presel1c160XX1c20bgtwo85_FTagPEBTLA_L1jJ160', l1SeedThresholds=['FSNOSEED']*2, stream=['FTagPEBTLA'], groups=MultiJetGroup+SupportPhIGroup, monGroups=['tlaMon:shifter']),


        # ATR-29016: EOF TLA chains aiming for H->cc+ISRJet signature
        ChainProp(name='HLT_3j20c_pf_ftf_presel3c30_PhysicsTLA_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=EOFTLALegGroup+MultiJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_3j20c_pf_ftf_presel3c40_PhysicsTLA_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=EOFTLALegGroup+MultiJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_3j20c_pf_ftf_presel3c45_PhysicsTLA_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=EOFTLALegGroup+MultiJetGroup, monGroups=['tlaMon:shifter']),
            ## Without pf_ftf part 
        ChainProp(name='HLT_j0_roiftf_presel3c30_L1jJ85p0ETA21_3jJ40p0ETA25', groups=MultiJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j0_roiftf_presel3c40_L1jJ85p0ETA21_3jJ40p0ETA25', groups=MultiJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j0_roiftf_presel3c45_L1jJ85p0ETA21_3jJ40p0ETA25', groups=MultiJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),

    ]

    chains['Tau'] = [
        # Single tau Loose and Tight variations
        ChainProp(name='HLT_tau20_mediumGNTau_L1cTAU20M', groups=SingleTauGroup+DevGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau25_looseGNTau_L1cTAU20M', groups=SingleTauGroup+DevGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau25_looseRNN_tracktwoLLP_L1cTAU20M', groups=SingleTauGroup+DevGroup),
        ChainProp(name='HLT_tau25_tightGNTau_L1cTAU20M', groups=SingleTauGroup+DevGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau25_tightRNN_tracktwoLLP_L1cTAU20M', groups=SingleTauGroup+DevGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_L1cTAU30M', groups=SingleTauGroup+DevGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau35_looseGNTau_L1cTAU30M', groups=SingleTauGroup+DevGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau35_tightGNTau_L1cTAU30M', groups=SingleTauGroup+DevGroup, monGroups=['tauMon:t0']),


        # TES calibration triggers
        ChainProp(name='HLT_tau160_ptonly_L1eTAU140', groups=SingleTauGroup+SupportPhIGroup),
        ChainProp(name='HLT_tau0_mediumGNTau_tau0_mediumGNTau_03dRAB_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55', l1SeedThresholds=['cTAU30M', 'cTAU20M'], groups=MultiTauGroup+DevGroup+Topo2Group), 
        ChainProp(name='HLT_tau0_mediumGNTau_tau0_mediumGNTau_03dRAB_L1cTAU30M_2cTAU20M_4jJ30p0ETA25', l1SeedThresholds=['cTAU30M', 'cTAU20M'], groups=MultiTauGroup+DevGroup),


        # HH->2b2tau: asymmetric di-tau triggers
        # ATR-22230
        ChainProp(name="HLT_tau25_mediumGNTau_tau20_mediumGNTau_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup+Topo2Group), 
        ChainProp(name="HLT_tau35_mediumGNTau_tau20_mediumGNTau_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup+Topo2Group), 
        ChainProp(name="HLT_tau40_mediumGNTau_tau20_mediumGNTau_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup+Topo2Group), 
        ChainProp(name="HLT_tau25_mediumGNTau_tau25_mediumGNTau_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup+Topo2Group), 
        ChainProp(name="HLT_tau30_mediumGNTau_tau25_mediumGNTau_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup+Topo2Group), 

        ChainProp(name="HLT_tau25_mediumGNTau_tau20_mediumGNTau_03dRAB_L1cTAU30M_2cTAU20M_4jJ30p0ETA25", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup),
        ChainProp(name="HLT_tau35_mediumGNTau_tau20_mediumGNTau_03dRAB_L1cTAU30M_2cTAU20M_4jJ30p0ETA25", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup),
        ChainProp(name="HLT_tau40_mediumGNTau_tau20_mediumGNTau_03dRAB_L1cTAU30M_2cTAU20M_4jJ30p0ETA25", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup),
        ChainProp(name="HLT_tau25_mediumGNTau_tau25_mediumGNTau_03dRAB_L1cTAU30M_2cTAU20M_4jJ30p0ETA25", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup),
        ChainProp(name="HLT_tau30_mediumGNTau_tau25_mediumGNTau_03dRAB_L1cTAU30M_2cTAU20M_4jJ30p0ETA25", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup),

        # ATR-26852
        ChainProp(name="HLT_tau30_idperf_tracktwoMVA_tau20_idperf_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup+Topo2Group),
        ChainProp(name="HLT_tau30_idperf_tracktwoMVA_tau20_idperf_tracktwoMVA_03dRAB_L1cTAU30M_2cTAU20M_4jJ30p0ETA25", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup+Topo2Group),

        # ATR-27121, ATR-27132
        ChainProp(name="HLT_tau30_mediumGNTau_tau20_mediumGNTau_03dRAB_L1cTAU20M_cTAU12M_4jJ30p0ETA24_0DETA24_4DPHI99-eTAU30eTAU20", l1SeedThresholds=['cTAU20M','cTAU12M'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau30_mediumGNTau_tau20_mediumGNTau_03dRAB_L1cTAU20M_cTAU12M_4jJ30p0ETA24_0DETA24_4DPHI99-eTAU30eTAU12", l1SeedThresholds=['cTAU20M','cTAU12M'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau30_mediumGNTau_tau20_mediumGNTau_03dRAB_L1cTAU20M_cTAU12M_4jJ30p0ETA24_0DETA24_10DPHI99-eTAU30eTAU12", l1SeedThresholds=['cTAU20M','cTAU12M'], groups=MultiTauGroup+DevGroup), 

        ChainProp(name="HLT_tau30_mediumGNTau_tau20_mediumGNTau_03dRAB_L14jJ30p0ETA24_0DETA24_4DPHI99-eTAU30eTAU20", l1SeedThresholds=['eTAU20','eTAU12'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau30_mediumGNTau_tau20_mediumGNTau_03dRAB_L14jJ30p0ETA24_0DETA24_4DPHI99-eTAU30eTAU12", l1SeedThresholds=['eTAU20','eTAU12'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau30_mediumGNTau_tau20_mediumGNTau_03dRAB_L14jJ30p0ETA24_0DETA24_10DPHI99-eTAU30eTAU12", l1SeedThresholds=['eTAU20','eTAU12'], groups=MultiTauGroup+DevGroup), 

        ChainProp(name="HLT_tau30_mediumGNTau_tau20_mediumGNTau_03dRAB30_L1jJ85p0ETA21_3jJ40p0ETA25", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup),
        ChainProp(name="HLT_tau30_mediumGNTau_tau20_mediumGNTau_03dRAB_L1jJ85p0ETA21_3jJ40p0ETA25", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup+Topo2Group),
        
        ChainProp(name="HLT_tau0_mediumGNTau_tau0_mediumGNTau_03dRAB_L14jJ30p0ETA24_0DETA24-eTAU30eTAU12", l1SeedThresholds=['eTAU20','eTAU12'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau0_mediumGNTau_tau0_mediumGNTau_03dRAB_L14jJ30p0ETA24_0DETA24_4DPHI99-eTAU30eTAU20", l1SeedThresholds=['eTAU20','eTAU20'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau0_mediumGNTau_tau0_mediumGNTau_03dRAB_L14jJ30p0ETA24_0DETA24_4DPHI99-eTAU30eTAU12", l1SeedThresholds=['eTAU20','eTAU12'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau0_mediumGNTau_tau0_mediumGNTau_03dRAB_L14jJ30p0ETA24_0DETA24_10DPHI99-eTAU30eTAU12", l1SeedThresholds=['eTAU20','eTAU12'], groups=MultiTauGroup+DevGroup), 


        # jTAU- and eTAU-seeded chains to investigate cTAU performance
        ChainProp(name="HLT_tau25_idperf_tracktwoMVA_L1jTAU20",   groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau25_perf_tracktwoMVA_L1jTAU20",     groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau25_mediumGNTau_L1jTAU20', groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),

        ChainProp(name="HLT_tau35_idperf_tracktwoMVA_L1eTAU30",   groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau35_perf_tracktwoMVA_L1eTAU30",   groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau35_mediumGNTau_L1eTAU30', groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),


        # LRT tau chains (ATR-23787)
        ChainProp(name="HLT_tau25_idperf_tracktwoLLP_L1cTAU20M", groups=DevGroup),
        ChainProp(name="HLT_tau25_idperf_trackLRT_L1cTAU20M", groups=DevGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau25_looseRNN_trackLRT_L1cTAU20M", groups=DevGroup),
        ChainProp(name="HLT_tau25_mediumRNN_trackLRT_L1cTAU20M", groups=DevGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau25_tightRNN_trackLRT_L1cTAU20M", groups=DevGroup),
        ChainProp(name="HLT_tau25_mediumRNN_trackLRT_L1eTAU20", groups=DevGroup),

        ChainProp(name="HLT_tau80_idperf_trackLRT_L1eTAU80", groups=DevGroup),
        ChainProp(name="HLT_tau80_mediumRNN_trackLRT_L1eTAU80", groups=DevGroup),

        ChainProp(name="HLT_tau160_idperf_trackLRT_L1eTAU140", groups=DevGroup, monGroups=['tauMon:t0']), 
        ChainProp(name="HLT_tau160_mediumRNN_trackLRT_L1eTAU140", groups=DevGroup, monGroups=['tauMon:t0']),

        # Boosted high-pT di-tau chains (ATR-30999)
        # ntrk <= 9
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni0Trk9_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni1Trk9_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni2Trk9_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni3Trk9_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni4Trk9_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni5Trk9_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni6Trk9_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni7Trk9_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni8Trk9_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni9Trk9_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        # ntrk <= 5
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni0Trk5_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni2Trk5_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni3Trk5_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni4Trk5_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni5Trk5_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni6Trk5_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni7Trk5_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni8Trk5_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni9Trk5_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        # ntrk <= 3
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni0Trk3_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni2Trk3_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni3Trk3_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni4Trk3_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni5Trk3_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni6Trk3_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni7Trk3_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni8Trk3_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni9Trk3_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        # Alternative L1 seed
        # ntrk <= 9
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni0Trk9_pf_jes_ftf_preselj200_L1gLJ140p0ETA25',      l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni1Trk9_pf_jes_ftf_preselj200_L1gLJ140p0ETA25',      l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni2Trk9_pf_jes_ftf_preselj200_L1gLJ140p0ETA25',      l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni3Trk9_pf_jes_ftf_preselj200_L1gLJ140p0ETA25',      l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni4Trk9_pf_jes_ftf_preselj200_L1gLJ140p0ETA25',      l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni5Trk9_pf_jes_ftf_preselj200_L1gLJ140p0ETA25',      l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni6Trk9_pf_jes_ftf_preselj200_L1gLJ140p0ETA25',      l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni7Trk9_pf_jes_ftf_preselj200_L1gLJ140p0ETA25',      l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni8Trk9_pf_jes_ftf_preselj200_L1gLJ140p0ETA25',      l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni9Trk9_pf_jes_ftf_preselj200_L1gLJ140p0ETA25',      l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        # ntrk <= 5
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni0Trk5_pf_jes_ftf_preselj200_L1gLJ140p0ETA25',      l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni2Trk5_pf_jes_ftf_preselj200_L1gLJ140p0ETA25',      l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni3Trk5_pf_jes_ftf_preselj200_L1gLJ140p0ETA25',      l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni4Trk5_pf_jes_ftf_preselj200_L1gLJ140p0ETA25',      l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni5Trk5_pf_jes_ftf_preselj200_L1gLJ140p0ETA25',      l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni6Trk5_pf_jes_ftf_preselj200_L1gLJ140p0ETA25',      l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni7Trk5_pf_jes_ftf_preselj200_L1gLJ140p0ETA25',      l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni8Trk5_pf_jes_ftf_preselj200_L1gLJ140p0ETA25',      l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni9Trk5_pf_jes_ftf_preselj200_L1gLJ140p0ETA25',      l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        # ntrk <= 3
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni0Trk3_pf_jes_ftf_preselj200_L1gLJ140p0ETA25',      l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni2Trk3_pf_jes_ftf_preselj200_L1gLJ140p0ETA25',      l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni3Trk3_pf_jes_ftf_preselj200_L1gLJ140p0ETA25',      l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni4Trk3_pf_jes_ftf_preselj200_L1gLJ140p0ETA25',      l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni5Trk3_pf_jes_ftf_preselj200_L1gLJ140p0ETA25',      l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni6Trk3_pf_jes_ftf_preselj200_L1gLJ140p0ETA25',      l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni7Trk3_pf_jes_ftf_preselj200_L1gLJ140p0ETA25',      l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni8Trk3_pf_jes_ftf_preselj200_L1gLJ140p0ETA25',      l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni9Trk3_pf_jes_ftf_preselj200_L1gLJ140p0ETA25',      l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        # Looser ditauOmni cuts Trk4 chains
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni0Trk4_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni01Trk4_pf_jes_ftf_preselj200_L1jJ160',             l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni02Trk4_pf_jes_ftf_preselj200_L1jJ160',             l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni03Trk4_pf_jes_ftf_preselj200_L1jJ160',             l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni04Trk4_pf_jes_ftf_preselj200_L1jJ160',             l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni05Trk4_pf_jes_ftf_preselj200_L1jJ160',             l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni06Trk4_pf_jes_ftf_preselj200_L1jJ160',             l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni07Trk4_pf_jes_ftf_preselj200_L1jJ160',             l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni08Trk4_pf_jes_ftf_preselj200_L1jJ160',             l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni09Trk4_pf_jes_ftf_preselj200_L1jJ160',             l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni1Trk4_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        # Looser ditauOmni cuts Trk5 chains
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni01Trk5_pf_jes_ftf_preselj200_L1jJ160',             l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni02Trk5_pf_jes_ftf_preselj200_L1jJ160',             l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni03Trk5_pf_jes_ftf_preselj200_L1jJ160',             l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni04Trk5_pf_jes_ftf_preselj200_L1jJ160',             l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni05Trk5_pf_jes_ftf_preselj200_L1jJ160',             l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni06Trk5_pf_jes_ftf_preselj200_L1jJ160',             l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni07Trk5_pf_jes_ftf_preselj200_L1jJ160',             l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni08Trk5_pf_jes_ftf_preselj200_L1jJ160',             l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j250_a10sd_cssk_ditauOmni09Trk5_pf_jes_ftf_preselj200_L1jJ160',             l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),

        # ATR-31631 - test chains for boosted ditauOmni0Trk4 chains with elevated pT
        ChainProp(name='HLT_j260_a10sd_cssk_ditauOmni0Trk4_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j270_a10sd_cssk_ditauOmni0Trk4_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j280_a10sd_cssk_ditauOmni0Trk4_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j290_a10sd_cssk_ditauOmni0Trk4_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j300_a10sd_cssk_ditauOmni0Trk4_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j310_a10sd_cssk_ditauOmni0Trk4_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j320_a10sd_cssk_ditauOmni0Trk4_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j330_a10sd_cssk_ditauOmni0Trk4_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j340_a10sd_cssk_ditauOmni0Trk4_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
        ChainProp(name='HLT_j350_a10sd_cssk_ditauOmni0Trk4_pf_jes_ftf_preselj200_L1jJ160',              l1SeedThresholds=['FSNOSEED'], groups=DiTauGroup+DevGroup),
    ]

    chains['Bphysics'] = [
        #ATR-21003; default dimuon and Bmumux chains from Run2; l2io validation; should not be moved to Physics
        ChainProp(name='HLT_2mu4_noL2Comb_bJpsimumu_L12MU3V', stream=["BphysDelayed"], groups=BphysicsGroup+DevGroup),
        ChainProp(name='HLT_mu6_noL2Comb_mu4_noL2Comb_bJpsimumu_L1MU5VF_2MU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=["BphysDelayed"], groups=BphysicsGroup+DevGroup),
        ChainProp(name='HLT_2mu4_noL2Comb_bBmumux_BpmumuKp_L12MU3V', stream=["BphysDelayed"], groups=BphysicsGroup+DevGroup),
        ChainProp(name='HLT_2mu4_noL2Comb_bBmumux_BsmumuPhi_L12MU3V', stream=["BphysDelayed"], groups=BphysicsGroup+DevGroup),
        ChainProp(name='HLT_2mu4_noL2Comb_bBmumux_LbPqKm_L12MU3V', stream=["BphysDelayed"], groups=BphysicsGroup+DevGroup),

        #ATR-31457 - HLT chains for muon AD BDT (NOMAD)
        ChainProp(name='HLT_2mu4_bDimu2700_L1ADBDTL',     l1SeedThresholds=['MU3V'], stream=['Main'], groups=Topo3Group+BphysicsGroup+EOFBPhysL1MuGroup),
        ChainProp(name='HLT_2mu4_bDimu_L1ADBDTL',         l1SeedThresholds=['MU3V'], stream=['Main'], groups=Topo3Group+BphysicsGroup+EOFBPhysL1MuGroup),
    ]

    chains['Combined'] = [
        # Test chains for muon msonly + VBF for ATR-28412

        # mu-tag & tau-probe triggers for LLP (ATR-23150)
        ChainProp(name='HLT_mu26_ivarmedium_tau100_mediumRNN_tracktwoLLP_probe_L1eTAU80_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU80'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleMuonGroup),

        # tau + jet and tau + photon tag and probe (ATR-24031)
        # *** Temporarily commented because counts are fluctuating in CI and causing confusion ***
        ChainProp(name='HLT_g25_loose_xe40_cell_xe50_tcpufit_18dphiAB_18dphiAC_80mTAC_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaMETGroup),
        ChainProp(name='HLT_g25_tight_icalotight_xe40_cell_xe50_tcpufit_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaMETGroup),

        # ATR-28443, test H to yjj trigger
        ChainProp(name='HLT_g24_tight_icaloloose_j50c_j30c_j24c_03dRAB35_03dRAC35_15dRBC45_50invmBC130_pf_ftf_L1eEM26M', groups=PrimaryLegGroup+EgammaJetGroup, l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream]),
        ChainProp(name='HLT_g24_tight_icaloloose_j40c_j30c_j24c_03dRAB35_03dRAC35_15dRBC45_50invmBC130_pf_ftf_L1eEM26M', groups=PrimaryLegGroup+EgammaJetGroup, l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream]),

        # high-mu AFP
        ChainProp(name='HLT_2j20_mb_afprec_afpdijet_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],groups=MinBiasGroup+SupportLegGroup),

        # AFP ToF Vertex Delta Z: ATR-15719
        ChainProp(name='HLT_2j20_ftf_mb_afprec_afpdz5_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_2j20_ftf_mb_afprec_afpdz10_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream], groups=MinBiasGroup+SupportGroup),

        # Test PEB chains for AFP (single/di-lepton-seeded, can be prescaled)
        # ATR-23946
        # ChainProp(name='HLT_noalg_AFPPEB_L1EM22VHI', l1SeedThresholds=['FSNOSEED'], stream=['AFPPEB'], groups=MinBiasGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1MU14FCH', l1SeedThresholds=['FSNOSEED'], stream=['AFPPEB'], groups=['PS:NoBulkMCProd']+MinBiasGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L12MU5VF', l1SeedThresholds=['FSNOSEED'], stream=['AFPPEB'], groups=['PS:NoBulkMCProd']+MinBiasGroup),

        # Maintain consistency with old naming conventions for validation

        ChainProp(name='HLT_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel3c20XX1c20bgtwo85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=5*['FSNOSEED'], stream=['VBFDelayed'], groups=TagAndProbePhIGroup+TauBJetGroup),
        ChainProp(name='HLT_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo85XX1c20gntau90_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=5*['FSNOSEED'], stream=['VBFDelayed'], groups=TagAndProbePhIGroup+TauBJetGroup),
        ChainProp(name='HLT_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo85XX1c20gntau85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=5*['FSNOSEED'], stream=['VBFDelayed'], groups=TagAndProbePhIGroup+TauBJetGroup),
        ChainProp(name='HLT_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo82XX1c20gntau85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=5*['FSNOSEED'], stream=['VBFDelayed'], groups=TagAndProbePhIGroup+TauBJetGroup),
        ChainProp(name='HLT_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo82XX1c20gntau80_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=5*['FSNOSEED'], stream=['VBFDelayed'], groups=TagAndProbePhIGroup+TauBJetGroup),
        ChainProp(name='HLT_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo80XX1c20gntau80_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=5*['FSNOSEED'], stream=['VBFDelayed'], groups=TagAndProbePhIGroup+TauBJetGroup),

        # Displaced jet trigger additional chains related to ATR-28691
        # Phase 1
        ChainProp(name='HLT_j180_dispjet120_x3d1p_L1jJ160', groups=SingleJetGroup+UnconvTrkGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED']*2),
        ChainProp(name='HLT_j180_dispjet140_x3d1p_L1jJ160', groups=SingleJetGroup+UnconvTrkGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED']*2),

        #ATR-31327
        ChainProp(name='HLT_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_L1BTAG-MU3VFjJ20_2jJ40p0ETA25'            , l1SeedThresholds=['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_L1BTAG-MU5VFjJ20_2jJ40p0ETA25'            , l1SeedThresholds=['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_L1BTAG-MU5VFjJ20_2jJ40p0ETA25'            , l1SeedThresholds=['MU5VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn277_j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_dRAF04_L1BTAG-MU5VFjJ20_2jJ40p0ETA25'            , l1SeedThresholds=['MU5VF']+['FSNOSEED']*6,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
      
        ChainProp(name='HLT_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L1BTAG-MU3VFjJ20_2jJ40p0ETA25'            , l1SeedThresholds=['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L1BTAG-MU5VFjJ20_2jJ40p0ETA25'            , l1SeedThresholds=['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L1BTAG-MU5VFjJ20_2jJ40p0ETA25'            , l1SeedThresholds=['MU5VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn277_j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_dRAF04_L1BTAG-MU5VFjJ20_2jJ40p0ETA25'            , l1SeedThresholds=['MU5VF']+['FSNOSEED']*6,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),

        ChainProp(name='HLT_j55c_nnJvtv1_j40c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_2j20c_nnJvtv1_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L1BTAG-MU3VFjJ20_2jJ40p0ETA25'            , l1SeedThresholds=['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j55c_nnJvtv1_j40c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_2j20c_nnJvtv1_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L1BTAG-MU5VFjJ20_2jJ40p0ETA25'            , l1SeedThresholds=['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j55c_nnJvtv1_j40c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_2j20c_nnJvtv1_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L1BTAG-MU5VFjJ20_2jJ40p0ETA25'            , l1SeedThresholds=['MU5VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j55c_nnJvtv1_j40c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_j20c_nnJvtv1_bgn277_j20c_nnJvtv1_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_dRAF04_L1BTAG-MU5VFjJ20_2jJ40p0ETA25'            , l1SeedThresholds=['MU5VF']+['FSNOSEED']*6,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),

        ChainProp(name='HLT_j40c_020jvt_j35c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_L1BTAG-MU3VFjJ20_2jJ40p0ETA25'            , l1SeedThresholds=['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j40c_020jvt_j35c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_L1BTAG-MU5VFjJ20_2jJ40p0ETA25'            , l1SeedThresholds=['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j40c_020jvt_j35c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_L1BTAG-MU5VFjJ20_2jJ40p0ETA25'            , l1SeedThresholds=['MU5VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j40c_020jvt_j35c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn277_j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_dRAF04_L1BTAG-MU5VFjJ20_2jJ40p0ETA25'            , l1SeedThresholds=['MU5VF']+['FSNOSEED']*6,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),

        ChainProp(name='HLT_j40c_020jvt_j35c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L1BTAG-MU3VFjJ20_2jJ40p0ETA25'            , l1SeedThresholds=['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j40c_020jvt_j35c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L1BTAG-MU5VFjJ20_2jJ40p0ETA25'            , l1SeedThresholds=['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j40c_020jvt_j35c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L1BTAG-MU5VFjJ20_2jJ40p0ETA25'            , l1SeedThresholds=['MU5VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j40c_020jvt_j35c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn277_j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_dRAF04_L1BTAG-MU5VFjJ20_2jJ40p0ETA25'            , l1SeedThresholds=['MU5VF']+['FSNOSEED']*6,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),

        ChainProp(name='HLT_j40c_nnJvtv1_j35c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_2j20c_nnJvtv1_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L1BTAG-MU3VFjJ20_2jJ40p0ETA25'            , l1SeedThresholds=['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j40c_nnJvtv1_j35c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_2j20c_nnJvtv1_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L1BTAG-MU5VFjJ20_2jJ40p0ETA25'            , l1SeedThresholds=['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j40c_nnJvtv1_j35c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_2j20c_nnJvtv1_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L1BTAG-MU5VFjJ20_2jJ40p0ETA25'            , l1SeedThresholds=['MU5VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j40c_nnJvtv1_j35c_nnJvtv1_j25c_nnJvtv1_j20c_nnJvtv1_SHARED_j20c_nnJvtv1_bgn277_j20c_nnJvtv1_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_dRAF04_L1BTAG-MU5VFjJ20_2jJ40p0ETA25'            , l1SeedThresholds=['MU5VF']+['FSNOSEED']*6,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),

        #ATR-30378
        ChainProp(name='HLT_mu6_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_dRAB04_L1BTAG-MU5VFjJ40_2jJ40p0ETA25'            , l1SeedThresholds=['MU5VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        #
        ChainProp(name='HLT_mu6_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn280_pf_ftf_presel2c20XX2c20bgtwo85_dRAB04_L1BTAG-MU5VFjJ40_2jJ40p0ETA25'            , l1SeedThresholds=['MU5VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),

        #same versions as above with presel3c20XX1c20bgtwo85 instead of presel2c20XX2c20bgtwo85
        ChainProp(name='HLT_mu6_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_dRAB04_L1BTAG-MU5VFjJ40_2jJ40p0ETA25'            , l1SeedThresholds=['MU5VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        #
        ChainProp(name='HLT_mu6_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn280_pf_ftf_presel3c20XX1c20bgtwo85_dRAB04_L1BTAG-MU5VFjJ40_2jJ40p0ETA25'            , l1SeedThresholds=['MU5VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),

        #addind a chain to enable optimization studies
        ChainProp(name='HLT_mu6_j20c_020jvt_3j20c_020jvt_SHARED_j20c_020jvt_bgn280_pf_ftf_presel3c20XX1c20bgtwo85_dRAB04_L1BTAG-MU5VFjJ40_2jJ40p0ETA25'            , l1SeedThresholds=['MU5VF']+['FSNOSEED']*3,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),

        ChainProp(name='HLT_mu6_j20c_020jvt_3j20c_020jvt_SHARED_j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_dRAB04_L1BTAG-MU5VFjJ40_2jJ40p0ETA25'            , l1SeedThresholds=['MU5VF']+['FSNOSEED']*3,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
            
        # ATR-21596 
        # Muon+HT Test chains for PEB
        ChainProp(name='HLT_mu6_probe_j20_pf_ftf_DarkJetPEBTLA_L1HT190-J15s5pETA21', l1SeedThresholds=['PROBEMU5VF','FSNOSEED'], stream=['DarkJetPEBTLA'], groups=DevGroup+MuonJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_mu10_probe_j20_pf_ftf_DarkJetPEBTLA_L1HT190-J15s5pETA21', l1SeedThresholds=['PROBEMU8F','FSNOSEED'], stream=['DarkJetPEBTLA'], groups=DevGroup+MuonJetGroup+LegacyTopoGroup),

        #ATR-30824
        ChainProp(name='HLT_mu10_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn277_j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_dRAF04_L1BTAG-MU3VFjJ40_2jJ30p0ETA25'            , l1SeedThresholds=['MU3VF']+['FSNOSEED']*6,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu10_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn277_j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_dRAD04_L1BTAG-MU3VFjJ40_2jJ30p0ETA25_jJ50p0ETA25', l1SeedThresholds=['MU3VF']+['FSNOSEED']*6,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu10_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn277_j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_dRAD04_L1BTAG-MU3VFjJ40_2jJ40p0ETA25'            , l1SeedThresholds=['MU3VF']+['FSNOSEED']*6,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),

        ChainProp(name='HLT_mu10_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn277_j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_dRAD04_L1BTAG-MU5VFjJ40_2jJ30p0ETA25'            , l1SeedThresholds=['MU5VF']+['FSNOSEED']*6,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu10_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn277_j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_dRAD04_L1BTAG-MU5VFjJ40_2jJ30p0ETA25_jJ50p0ETA25', l1SeedThresholds=['MU5VF']+['FSNOSEED']*6,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu10_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn277_j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_dRAD04_L1BTAG-MU5VFjJ40_2jJ40p0ETA25'            , l1SeedThresholds=['MU5VF']+['FSNOSEED']*6,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),

        ChainProp(name='HLT_mu6_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_L1BTAG-MU3VFjJ40_2jJ30p0ETA25'                         , l1SeedThresholds=['MU3VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn277_j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_dRAF04_L1BTAG-MU3VFjJ40_2jJ30p0ETA25', l1SeedThresholds=['MU3VF']+['FSNOSEED']*6,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L1BTAG-MU3VFjJ40_2jJ30p0ETA25'                          , l1SeedThresholds=['MU3VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_dRAF04_L1BTAG-MU3VFjJ40_2jJ30p0ETA25'                   , l1SeedThresholds=['MU3VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),

        ChainProp(name='HLT_mu6_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_L1BTAG-MU5VFjJ40_2jJ30p0ETA25'                         , l1SeedThresholds=['MU5VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn277_j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_dRAF04_L1BTAG-MU5VFjJ40_2jJ30p0ETA25', l1SeedThresholds=['MU5VF']+['FSNOSEED']*6,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L1BTAG-MU5VFjJ40_2jJ30p0ETA25'                          , l1SeedThresholds=['MU5VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_dRAF04_L1BTAG-MU5VFjJ40_2jJ30p0ETA25'                   , l1SeedThresholds=['MU5VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),

        # ATR-21596 MET with DarkJet
        # PFO MET
        ChainProp(name='HLT_j0_HT650XX0eta240_pf_ftf_preselcHT450_xe0_pfopufit_L1HT190-jJ40s5pETA21', l1SeedThresholds=['FSNOSEED', 'FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+SingleJetGroup+METGroup+Topo3Group),
        ChainProp(name='HLT_j0_HT500XX0eta240_pf_ftf_preselcHT450_xe0_pfopufit_L1HT190-jJ40s5pETA21', l1SeedThresholds=['FSNOSEED', 'FSNOSEED'],groups=SupportPhIGroup+MultiJetGroup+METGroup+Topo3Group+['RATE:CPS_HT190-jJ40s5pETA21']),
        # NN MET
        ChainProp(name='HLT_j0_HT650XX0eta240_pf_ftf_preselcHT450_xe0_nn_L1HT190-jJ40s5pETA21', l1SeedThresholds=['FSNOSEED', 'FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+SingleJetGroup+METGroup+Topo3Group),
        ChainProp(name='HLT_j0_HT500XX0eta240_pf_ftf_preselcHT450_xe0_nn_L1HT190-jJ40s5pETA21', l1SeedThresholds=['FSNOSEED', 'FSNOSEED'],groups=SupportPhIGroup+MultiJetGroup+METGroup+Topo3Group+['RATE:CPS_HT190-jJ40s5pETA21']),

        # Anomaly detection (ATR-30618, ATR-30826)
        ChainProp(name='HLT_0e25_nopid_0g25_loose_0mu24noL1_j20_xe0_nn_anomdetL_L1ADVAET', l1SeedThresholds=['eEM18L','eEM18L','FSNOSEED','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+Topo2Group),
        ChainProp(name='HLT_0e25_nopid_0g25_loose_0mu24noL1_j20_xe0_nn_anomdetM_L1ADVAET', l1SeedThresholds=['eEM18L','eEM18L','FSNOSEED','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+Topo2Group),
        ChainProp(name='HLT_0e25_nopid_0g25_loose_0mu24noL1_j20_xe0_nn_anomdetT_L1ADVAET', l1SeedThresholds=['eEM18L','eEM18L','FSNOSEED','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+Topo2Group),
        ChainProp(name='HLT_0e25_nopid_0g25_loose_0mu24noL1_j20_xe0_nn_anomdetL_L1ADVAEL', l1SeedThresholds=['eEM18L','eEM18L','FSNOSEED','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+Topo2Group),
        ChainProp(name='HLT_0e25_nopid_0g25_loose_0mu24noL1_j20_xe0_nn_anomdetM_L1ADVAEL', l1SeedThresholds=['eEM18L','eEM18L','FSNOSEED','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+Topo2Group),
        ChainProp(name='HLT_0e25_nopid_0g25_loose_0mu24noL1_j20_xe0_nn_anomdetT_L1ADVAEL', l1SeedThresholds=['eEM18L','eEM18L','FSNOSEED','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+Topo2Group),

        ChainProp(name='HLT_0e25_nopid_0g25_loose_0mu24_j20_xe0_nn_anomdetL_L1ADVAET', l1SeedThresholds=['eEM18L','eEM18L','MU3V','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+Topo2Group),
        ChainProp(name='HLT_0e25_nopid_0g25_loose_0mu24_j20_xe0_nn_anomdetM_L1ADVAET', l1SeedThresholds=['eEM18L','eEM18L','MU3V','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+Topo2Group),
        ChainProp(name='HLT_0e25_nopid_0g25_loose_0mu24_j20_xe0_nn_anomdetT_L1ADVAET', l1SeedThresholds=['eEM18L','eEM18L','MU3V','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+Topo2Group),
        ChainProp(name='HLT_0e25_nopid_0g25_loose_0mu24_j20_xe0_nn_anomdetL_L1ADVAEL', l1SeedThresholds=['eEM18L','eEM18L','MU3V','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+Topo2Group),
        ChainProp(name='HLT_0e25_nopid_0g25_loose_0mu24_j20_xe0_nn_anomdetM_L1ADVAEL', l1SeedThresholds=['eEM18L','eEM18L','MU3V','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+Topo2Group),
        ChainProp(name='HLT_0e25_nopid_0g25_loose_0mu24_j20_xe0_nn_anomdetT_L1ADVAEL', l1SeedThresholds=['eEM18L','eEM18L','MU3V','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+Topo2Group),

        ChainProp(name='HLT_0e25_nopid_0g25_loose_0mu24_j20_xe0_tcpufit_anomdetL_L1ADVAET', l1SeedThresholds=['eEM18L','eEM18L','MU3V','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+Topo2Group),
        ChainProp(name='HLT_0e25_nopid_0g25_loose_0mu24_j20_xe0_tcpufit_anomdetM_L1ADVAEL', l1SeedThresholds=['eEM18L','eEM18L','MU3V','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+Topo2Group),
        ChainProp(name='HLT_0e25_nopid_0g25_loose_0mu24_j20_xe0_tcpufit_anomdetT_L1ADVAET', l1SeedThresholds=['eEM18L','eEM18L','MU3V','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+Topo2Group),
        ChainProp(name='HLT_0e25_nopid_0g25_loose_0mu24_j20_xe0_tcpufit_anomdetT_L1ADVAEL', l1SeedThresholds=['eEM18L','eEM18L','MU3V','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+Topo2Group),
    ]

    chains['Beamspot'] = [
    ]

    chains['MinBias'] = [

    ]

    chains['Calib'] = [

    ]

    chains['Streaming'] = [

        # ATR-24037
        ChainProp(name='HLT_noalg_L1jXEPerf100',     l1SeedThresholds=['FSNOSEED'], groups=['PS:NoBulkMCProd']+METPhaseIStreamersGroup),

    ]
    
    chains['Monitor'] = [
        #ATR-27211, ATR-27203
        ChainProp(name='HLT_l1topoPh1debug_L1All', l1SeedThresholds=['FSNOSEED'], stream=['L1TopoMismatches'], groups=['PS:NoHLTRepro', 'RATE:Monitoring', 'BW:Other']),
        ChainProp(name='HLT_caloclustermon_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=['RATE:Monitoring']),
        ChainProp(name='HLT_caloclustermon_L1RD0_EMPTY', l1SeedThresholds=['FSNOSEED'], groups=['RATE:Monitoring']),
    ]

    chains['UnconventionalTracking'] = [
        ChainProp(name='HLT_fslrt0_L1jJ160', groups=DevGroup+['PS:NoHLTRepro'], l1SeedThresholds=['FSNOSEED']),
    ]

    return chains

def setupMenu():

    chains = mc_menu.setupMenu()

    from AthenaCommon.Logging import logging
    log = logging.getLogger( __name__ )
    log.info('[setupMenu] going to add the Dev menu chains now')

    for sig,chainsInSig in getDevSignatures().items():
        chains[sig] += chainsInSig

    return chains
