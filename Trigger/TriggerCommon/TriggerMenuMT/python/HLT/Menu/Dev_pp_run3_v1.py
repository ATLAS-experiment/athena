# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# Dev_pp_run3_v1.py menu for the long shutdown development
#------------------------------------------------------------------------#

# All chains are represented as ChainProp objects in a ChainStore
from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp
from .SignatureDicts import ChainStore

from . import MC_pp_run3_v1 as mc_menu

from .Physics_pp_run3_v1 import (PhysicsStream,
                                 SingleElectronGroup,
                                 SingleMuonGroup,
                                 MultiMuonGroup,
                                 METGroup,
                                 SingleJetGroup,
                                 MultiJetGroup,
                                 SingleBjetGroup,
                                 MultiBjetGroup,
                                 SingleTauGroup,
                                 MultiTauGroup,
                                 MultiPhotonGroup,
                                 TauPhotonGroup,
                                 TauJetGroup,
                                 TauBJetGroup,
                                 TauMETGroup,
                                 EgammaTauGroup,
                                 BphysicsGroup,
                                 EgammaMETGroup,
                                 EgammaJetGroup,
                                 MuonJetGroup,
                                 MuonTauGroup,
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
                                 EOFTauPhIGroup,
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

        #ATR-21566, di-muon TLA
        ChainProp(name='HLT_mu10_PhysicsTLA_L1MU8F',   stream=['TLA'], groups=SingleMuonGroup+DevGroup),
        ChainProp(name='HLT_mu10_mu6_probe_PhysicsTLA_L1MU8F', stream=['TLA'],l1SeedThresholds=['MU8F','PROBEMU3V'], groups=MultiMuonGroup+DevGroup),
        ChainProp(name='HLT_2mu4_PhysicsTLA_L12MU3V',  stream=['TLA'], groups=MultiMuonGroup+SupportGroup),
        ChainProp(name='HLT_2mu6_PhysicsTLA_L12MU5VF', stream=['TLA'], groups=MultiMuonGroup+SupportGroup),
        ChainProp(name='HLT_2mu10_PhysicsTLA_L12MU8F', stream=['TLA'], groups=MultiMuonGroup+SupportGroup),
        # di-muon TLA with L1TOPO
        ChainProp(name='HLT_mu6_mu4_PhysicsTLA_L1BPH-7M22-MU5VFMU3VF', l1SeedThresholds=['MU5VF','MU3VF'],stream=['TLA'], groups=MultiMuonGroup+EOFL1MuGroup+Topo3Group),
        ChainProp(name='HLT_2mu4_PhysicsTLA_L1BPH-7M22-0DR20-2MU3V', l1SeedThresholds=['MU3V'],stream=['TLA'], groups=MultiMuonGroup+EOFL1MuGroup+Topo3Group),

        # ATR-22782, ATR-28868, 4mu analysis
        ChainProp(name='HLT_mu4_ivarloose_mu4_L1BPH-7M14-0DR25-MU5VFMU3VF', l1SeedThresholds=['MU3VF','MU3VF'], stream=['BphysDelayed'], groups=MultiMuonGroup+EOFBPhysL1MuGroup+Topo3Group),
    ]

    chains['Egamma'] = [
        # ElectronChains----------

        # electron forward triggers (keep this only for dev now)
        #ChainProp(name='HLT_e30_etcut_fwd_L1EM22VHI', groups=SingleElectronGroup),

        # ATR-27156 Phase-1
        # dnn chains
        # ChainProp(name='HLT_e26_dnnloose_L1EM22VHI', groups=SupportLegGroup+SingleElectronGroup+['RATE:CPS_EM22VHI'], monGroups=['egammaMon:t0_tp']),
        # ChainProp(name='HLT_e26_dnnmedium_L1EM22VHI', groups=SupportLegGroup+SingleElectronGroup+['RATE:CPS_EM22VHI'], monGroups=['egammaMon:t0_tp']),
        # ChainProp(name='HLT_e26_dnntight_L1EM22VHI', groups=PrimaryLegGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),
        # ChainProp(name='HLT_e26_dnntight_ivarloose_L1EM22VHI', groups=PrimaryLegGroup+SingleElectronGroup, monGroups=['egammaMon:t0']),
        # ChainProp(name='HLT_e60_dnnmedium_L1EM22VHI', groups=PrimaryLegGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),
        # ChainProp(name='HLT_e140_dnnloose_L1EM22VHI', groups=PrimaryLegGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),

        # ChainProp(name='HLT_g20_loose_noiso_L1EM15VH', groups=SupportLegGroup+SinglePhotonGroup+['RATE:CPS_EM15VH']),

        # Photon chains for TLA
#         ChainProp(name='HLT_g35_loose_PhysicsTLA_L1EM22VHI',stream=['TLA'], groups=SinglePhotonGroup+DevGroup),

        # Alternative formulation of T&P chains with generic mass cut combohypotool
        # With & without 'probe' expression to check count consistency
        # ATR-24117

        # Jpsiee

        # low-mass boosted diphoton TLA
        # M, DR
        ChainProp(name='HLT_2g10_loose_EgammaPEBTLA_L12DR15-M70-2eEM9L',l1SeedThresholds=['eEM9'],stream=['EgammaPEBTLA'], groups=SupportPhIGroup+Topo2Group+DevGroup),
        ChainProp(name='HLT_2g10_medium_EgammaPEBTLA_L12DR15-M70-2eEM9L',l1SeedThresholds=['eEM9'],stream=['EgammaPEBTLA'], groups=SupportPhIGroup+Topo2Group+DevGroup),
        ChainProp(name='HLT_2g13_loose_EgammaPEBTLA_L12DR15-M70-2eEM12L',l1SeedThresholds=['eEM12L'],stream=['EgammaPEBTLA'], groups=SupportPhIGroup+Topo2Group+DevGroup),
        ChainProp(name='HLT_2g13_medium_EgammaPEBTLA_L12DR15-M70-2eEM12L',l1SeedThresholds=['eEM12L'],stream=['EgammaPEBTLA'], groups=SupportPhIGroup+Topo2Group+DevGroup),
        
        # Ranges + Asymmetric
        ChainProp(name='HLT_g13_loose_g10_loose_EgammaPEBTLA_L12DR15-0M30-eEM12LeEM9L',l1SeedThresholds=['eEM12L', 'eEM9'],stream=['EgammaPEBTLA'], groups=SupportPhIGroup+Topo2Group+DevGroup),
        ChainProp(name='HLT_g13_loose_g10_loose_EgammaPEBTLA_L113DR25-25M70-eEM12LeEM9L',l1SeedThresholds=['eEM12L','eEM9'],stream=['EgammaPEBTLA'], groups=SupportPhIGroup+Topo2Group+DevGroup),

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
        ChainProp(name='HLT_6j20c_020jvt_pf_ftf_preselZ197XX6c20_PhysicsTLA_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['tlaMon:shifter']),        
        ChainProp(name='HLT_6j20_pf_ftf_preselZ197XX6c20_PhysicsTLA_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_j0_pf_ftf_preselZ197XX6c20_PhysicsTLA_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_6j20c_020jvt_pf_ftf_preselZ182XX6c20_PhysicsTLA_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['tlaMon:shifter']),    
        ChainProp(name='HLT_6j20_pf_ftf_preselZ182XX6c20_PhysicsTLA_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_j0_pf_ftf_preselZ182XX6c20_PhysicsTLA_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_6j20c_020jvt_pf_ftf_preselZ142XX5c20_PhysicsTLA_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=PrimaryPhIGroup+MultiJetGroup, monGroups=['tlaMon:shifter']), 
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
        ChainProp(name='HLT_j20_j20_pf_ftf_preselj140XX2j45_PhysicsTLA_L1jJ85p0ETA21_3jJ40p0ETA25'       , l1SeedThresholds=['FSNOSEED', 'FSNOSEED'], stream=['TLA'], groups=EOFTLALegGroup+MultiJetGroup),
        ChainProp(name='HLT_j20_j20_pf_ftf_preselj80XX2j45_PhysicsTLA_L1jJ85p0ETA21_3jJ40p0ETA25'        , l1SeedThresholds=['FSNOSEED', 'FSNOSEED'], stream=['TLA'], groups=EOFTLALegGroup+MultiJetGroup),
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

        ### PT scan 
        ChainProp(name='HLT_2j45_2j55_3timeSig_L14jJ40', l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_2j45_2j75_3timeSig_L14jJ40', l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_2j55_2j45_3timeSig_L14jJ40', l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_2j75_2j45_3timeSig_L14jJ40', l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_2j55_2j55_3timeSig_L14jJ40', l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_2j75_2j75_3timeSig_L14jJ40', l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_2j90_2j90_3timeSig_L14jJ40', l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup+DevGroup),

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

        # ATR-24838 Large R L1J100 jet chains with jLJ L1 items (L1J100->L1jLJ140)
        ChainProp(name='HLT_j175_0eta180_emergingPTF0p08dR1p2_a10sd_cssk_pf_jes_ftf_L1jLJ140', groups=SingleJetGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_2j110_0eta200_emergingPTF0p1dR1p2_a10sd_cssk_pf_jes_ftf_L1jLJ140', groups=SingleJetGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_2j110_0eta180_emergingPTF0p09dR1p2_a10sd_cssk_pf_jes_ftf_L1jLJ140', groups=SingleJetGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_tracklessdR1p2_a10r_subjesIS_ftf_0eta200_L1jLJ140',    groups=SingleJetGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_tracklessdR1p2_a10r_subjesIS_ftf_0eta200_L1jLJ140',    groups=SingleJetGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED']),

        # Duplicated with old naming conventions only for validation
        ChainProp(name='HLT_2j330_a10sd_cssk_pf_jes_ftf_35smcINF_presel2j225_L1jLJ140', groups=DevGroup+MultiJetGroup, l1SeedThresholds=['FSNOSEED']),
        

#        ChainProp(name='HLT_2j330_a10sd_cssk_pf_jes_ftf_35smcINF_presel2j225_L1gLJ140', groups=DevGroup+MultiJetGroup, l1SeedThresholds=['FSNOSEED']),

        # ATR-28352: Test chains for multijet DIPZ 
        ]


    chains['Bjet'] = [
        
        # Test chain for X to bb tagging
        ChainProp(name='HLT_j110_a10sd_cssk_79bgntwox_pf_jes_ftf_preselj80_L1jJ60', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_79bgntwox_pf_jes_ftf_preselj140_L1jJ90', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_79bgntwox_pf_jes_ftf_preselj140_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_79bgntwox_pf_jes_ftf_preselj140_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_79bgntwox_pf_jes_ftf_preselj140_L1SC111-CjJ40', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_79bgntwox_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj140_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_79bgntwox_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj140_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_79bgntwox_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj140_L1SC111-CjJ40', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_79bgntwox_pf_jes_ftf_preselj200_L1jJ125', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_79bgntwox_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_79bgntwox_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_79bgntwox_pf_jes_ftf_preselj200_L1SC111-CjJ40', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_79bgntwox_j260_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_79bgntwox_j260_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_79bgntwox_j260_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC111-CjJ40', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j360_a10sd_cssk_79bgntwox_pf_jes_ftf_preselj225_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j420_a10sd_cssk_79bgntwox_pf_jes_ftf_preselj225_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j460_a10sd_cssk_79bgntwox_pf_jes_ftf_preselj225_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),

        ChainProp(name='HLT_j110_a10sd_cssk_86bgntwox_pf_jes_ftf_preselj80_L1jJ60', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_86bgntwox_pf_jes_ftf_preselj140_L1jJ90', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_86bgntwox_pf_jes_ftf_preselj140_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_a10sd_cssk_86bgntwox_pf_jes_ftf_preselj140_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_86bgntwox_pf_jes_ftf_preselj140_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_a10sd_cssk_86bgntwox_pf_jes_ftf_preselj140_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_86bgntwox_pf_jes_ftf_preselj140_L1SC111-CjJ40', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_a10sd_cssk_86bgntwox_pf_jes_ftf_preselj140_L1SC111-CjJ40', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_86bgntwox_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj140_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_86bgntwox_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj140_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_86bgntwox_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj140_L1SC111-CjJ40', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
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
        ChainProp(name='HLT_j260_35smcINF_86bgntwox_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED'], monGroups=['jetMon:online', 'bJetMon:t0']),
        ChainProp(name='HLT_j175_35smcINF_86bgntwox_j260_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED'], monGroups=['jetMon:online', 'bJetMon:t0']),
        ChainProp(name='HLT_j260_a10sd_cssk_86bgntwox_pf_jes_ftf_preselj200_L1jJ125', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_86bgntwox_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_2j260_a10sd_cssk_86bgntwox_pf_jes_ftf_presel2j225_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_86bgntwox_j260_a10sd_cssk_pf_jes_ftf_presel2j225_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_86bgntwox_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_2j260_a10sd_cssk_86bgntwox_pf_jes_ftf_presel2j225_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_86bgntwox_j260_a10sd_cssk_pf_jes_ftf_presel2j225_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=['FSNOSEED', 'FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_86bgntwox_pf_jes_ftf_preselj200_L1SC111-CjJ40', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_2j260_a10sd_cssk_86bgntwox_pf_jes_ftf_presel2j225_L1SC111-CjJ40', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_86bgntwox_j260_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC111-CjJ40', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_86bgntwox_j260_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_86bgntwox_j260_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_86bgntwox_j260_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC111-CjJ40', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j360_a10sd_cssk_86bgntwox_pf_jes_ftf_preselj225_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j420_a10sd_cssk_86bgntwox_pf_jes_ftf_preselj225_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j460_a10sd_cssk_86bgntwox_pf_jes_ftf_preselj225_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),

        ChainProp(name='HLT_j110_a10sd_cssk_91bgntwox_pf_jes_ftf_preselj80_L1jJ60', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_91bgntwox_pf_jes_ftf_preselj140_L1jJ90', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_91bgntwox_pf_jes_ftf_preselj140_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_91bgntwox_pf_jes_ftf_preselj140_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_91bgntwox_pf_jes_ftf_preselj140_L1SC111-CjJ40', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_91bgntwox_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj140_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_91bgntwox_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj140_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_91bgntwox_j175_35smcINF_a10sd_cssk_pf_jes_ftf_preselj140_L1SC111-CjJ40', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_91bgntwox_pf_jes_ftf_preselj200_L1jJ125', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_91bgntwox_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_91bgntwox_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_91bgntwox_pf_jes_ftf_preselj200_L1SC111-CjJ40', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_91bgntwox_j260_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_91bgntwox_j260_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_91bgntwox_j260_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC111-CjJ40', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j360_a10sd_cssk_91bgntwox_pf_jes_ftf_preselj225_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j420_a10sd_cssk_91bgntwox_pf_jes_ftf_preselj225_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j460_a10sd_cssk_91bgntwox_pf_jes_ftf_preselj225_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),

        ChainProp(name='HLT_j110_a10sd_cssk_96bgntwox_pf_jes_ftf_preselj80_L1jJ60', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_96bgntwox_pf_jes_ftf_preselj140_L1jJ90', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_96bgntwox_pf_jes_ftf_preselj140_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_96bgntwox_pf_jes_ftf_preselj140_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_96bgntwox_pf_jes_ftf_preselj140_L1SC111-CjJ40', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_96bgntwox_pf_jes_ftf_preselj200_L1jJ125', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_96bgntwox_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_96bgntwox_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_96bgntwox_pf_jes_ftf_preselj200_L1SC111-CjJ40', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
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
        #ChainProp(name='HLT_j20_0eta290_boffperf_pf_ftf_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleBjetGroup+DevGroup+LegacyTopoGroup),
        #ChainProp(name='HLT_4j20_0eta290_boffperf_pf_ftf_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=MultiBjetGroup+DevGroup+LegacyTopoGroup),
        #ChainProp(name='HLT_4j20_020jvt_boffperf_pf_ftf_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=MultiBjetGroup+DevGroup+LegacyTopoGroup),
        #ChainProp(name='HLT_3j20_020jvt_j20_0eta290_boffperf_pf_ftf_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED']*2, groups=MultiBjetGroup+DevGroup+LegacyTopoGroup),
        #ChainProp(name='HLT_4j20_020jvt_boffperf_pf_ftf_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=MultiBjetGroup+DevGroup),
        
        # ATR-29016: EOF TLA chains aiming for H->cc+ISRJet signature
        ChainProp(name='HLT_3j20c_pf_ftf_presel3c30_PhysicsTLA_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=EOFTLALegGroup+MultiJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_3j20c_pf_ftf_presel3c40_PhysicsTLA_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=EOFTLALegGroup+MultiJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_3j20c_pf_ftf_presel3c45_PhysicsTLA_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=EOFTLALegGroup+MultiJetGroup, monGroups=['tlaMon:shifter']),
            ## Without pf_ftf part 
        ChainProp(name='HLT_j0_roiftf_presel3c30_L1jJ85p0ETA21_3jJ40p0ETA25', groups=MultiJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j0_roiftf_presel3c40_L1jJ85p0ETA21_3jJ40p0ETA25', groups=MultiJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j0_roiftf_presel3c45_L1jJ85p0ETA21_3jJ40p0ETA25', groups=MultiJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),

        # TLA btag ATR-23002
        # multijet btag TLA - MultiJet L1
        ChainProp(name='HLT_j140_j20_0eta290_boffperf_pf_ftf_preselj140XXj45_PhysicsTLA_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*2, stream=['TLA'], groups=MultiBjetGroup+DevGroup),
        ## with calo fast-tag presel
        ChainProp(name='HLT_2j20_2j20_0eta290_boffperf_pf_ftf_presel2j25XX2j25b85_PhysicsTLA_L14jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*2, stream=['TLA'], groups=MultiBjetGroup+DevGroup),
        ChainProp(name='HLT_2j20_2j20_0eta290_boffperf_pf_ftf_presel2c20XX2c20b85_PhysicsTLA_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*2, stream=['TLA'], groups=MultiBjetGroup+DevGroup),

    ]

    chains['Tau'] = [
        #----------------------------------------------------------------------------------------------------------------------
        # GNTau Physics chains (ATR-30086) - to be moved to the Physics Menu for 2025 after validation

        # Single tau primaries
        ChainProp(name='HLT_tau160_mediumGNTau_L1eTAU140', stream=[PhysicsStream, 'express'], groups=PrimaryPhIGroup+SingleTauGroup, monGroups=['tauMon:online', 'tauMon:t0']),
        ChainProp(name='HLT_tau200_mediumGNTau_L1eTAU140', groups=PrimaryPhIGroup+SingleTauGroup),


        # Di-tau primaries and support chains
        ChainProp(name='HLT_tau35_mediumGNTau_tau25_mediumGNTau_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55', l1SeedThresholds=['cTAU30M','cTAU20M'], stream=[PhysicsStream, 'express'], groups=PrimaryPhIGroup+MultiTauGroup+Topo2Group, monGroups=['tauMon:online', 'tauMon:shifter']),
        ChainProp(name='HLT_tau35_mediumGNTau_tau25_mediumGNTau_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30MeTAU20M-jJ55', l1SeedThresholds=['cTAU30M','cTAU20M'], groups=PrimaryPhIGroup+MultiTauGroup+Topo2Group, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau35_mediumGNTau_tau25_mediumGNTau_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20', l1SeedThresholds=['cTAU30M','cTAU20M'], groups=EOFTauPhIGroup+MultiTauGroup+Topo2Group),
        ChainProp(name='HLT_tau35_mediumGNTau_tau25_mediumGNTau_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30MeTAU20M', l1SeedThresholds=['cTAU30M','cTAU20M'], groups=SupportPhIGroup+MultiTauGroup+Topo2Group),
        ChainProp(name='HLT_tau35_mediumGNTau_tau25_mediumGNTau_03dRAB_L1cTAU30M_2cTAU20M_4jJ30p0ETA25', l1SeedThresholds=['cTAU30M','cTAU20M'], groups=PrimaryPhIGroup+MultiTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau35_mediumGNTau_tau25_mediumGNTau_03dRAB_L1cTAU30M_2cTAU20M', l1SeedThresholds=['cTAU30M','cTAU20M'], groups=SupportPhIGroup+MultiTauGroup),

        ChainProp(name='HLT_tau40_mediumGNTau_tau35_mediumGNTau_03dRAB_L1cTAU35M_2cTAU30M_2jJ55_3jJ50', l1SeedThresholds=['cTAU35M','cTAU30M'], groups=PrimaryPhIGroup+MultiTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau40_mediumGNTau_tau35_mediumGNTau_03dRAB_L1eTAU35M_2eTAU30M', l1SeedThresholds=['eTAU35M','eTAU30M'], groups=SupportPhIGroup+MultiTauGroup),
        ChainProp(name='HLT_tau40_mediumGNTau_tau35_mediumGNTau_03dRAB_L1cTAU35M_2cTAU30M' , l1SeedThresholds=['cTAU35M','cTAU30M'], groups=SupportPhIGroup+MultiTauGroup),

        ChainProp(name='HLT_tau80_mediumGNTau_tau35_mediumGNTau_03dRAB30_L1eTAU80_2cTAU30M_DR-eTAU30eTAU20', stream=[PhysicsStream, 'express'], l1SeedThresholds=['eTAU80', 'cTAU30M'], groups=PrimaryPhIGroup+MultiTauGroup+Topo2Group, monGroups=['tauMon:online', 'tauMon:shifter']),

        ChainProp(name='HLT_tau80_mediumGNTau_tau60_mediumGNTau_03dRAB_L1eTAU80_2eTAU60', l1SeedThresholds=['eTAU80', 'eTAU60'], groups=PrimaryPhIGroup+MultiTauGroup, monGroups=['tauMon:t0']),


        # Delayed stream di-tau primaries and support chains
        ChainProp(name='HLT_tau30_mediumGNTau_tau20_mediumGNTau_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55', l1SeedThresholds=['cTAU30M', 'cTAU20M'], stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiTauGroup+Topo2Group),
        ChainProp(name='HLT_tau30_mediumGNTau_tau20_mediumGNTau_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20', l1SeedThresholds=['cTAU30M', 'cTAU20M'], stream=['VBFDelayed'], groups=EOFTauPhIGroup+MultiTauGroup+Topo2Group),
        ChainProp(name='HLT_tau30_mediumGNTau_tau20_mediumGNTau_03dRAB_L1cTAU30M_2cTAU20M_4jJ30p0ETA25', l1SeedThresholds=['cTAU30M', 'cTAU20M'], stream=['VBFDelayed'], groups=PrimaryPhIGroup+MultiTauGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_tau20_mediumGNTau_03dRAB_L1cTAU30M_2cTAU20M', l1SeedThresholds=['cTAU30M', 'cTAU20M'], stream=['VBFDelayed'], groups=SupportPhIGroup+MultiTauGroup),


        # Boosted di-tau chains
        ChainProp(name='HLT_tau20_mediumGNTau_tau20_mediumGNTau_02dRAB10_L1cTAU20M_DR-eTAU20eTAU12-jJ40', l1SeedThresholds=['cTAU20M', 'eTAU12'], groups=MultiTauGroup+SupportPhIGroup+Topo2Group),
        ChainProp(name='HLT_tau20_mediumGNTau_tau20_mediumGNTau_03dRAB10_L1cTAU20M_DR-eTAU20eTAU12-jJ40', l1SeedThresholds=['cTAU20M', 'eTAU12'], groups=MultiTauGroup+SupportPhIGroup+Topo2Group),
        ChainProp(name='HLT_tau25_mediumGNTau_tau20_mediumGNTau_02dRAB10_L1cTAU20M_DR-eTAU20eTAU12-jJ40', l1SeedThresholds=['cTAU20M', 'eTAU12'], groups=MultiTauGroup+SupportPhIGroup+Topo2Group),


        # HDBS di-tau support chains
        ChainProp(name='HLT_tau30_mediumGNTau_tau20_mediumGNTau_03dRAB_L14jJ30p0ETA24_0DETA24-eTAU30eTAU12', l1SeedThresholds=['eTAU20', 'eTAU12'], groups=SupportPhIGroup+MultiTauGroup+Topo3Group),
        ChainProp(name='HLT_tau30_mediumGNTau_tau20_mediumGNTau_03dRAB_L1cTAU20M_cTAU12M_4jJ30p0ETA24_0DETA24-eTAU30eTAU12', l1SeedThresholds=['cTAU20M', 'cTAU12M'], groups=SupportPhIGroup+MultiTauGroup+Topo3Group),


        # Single tau support chains
        ChainProp(name='HLT_tau20_mediumGNTau_L1eTAU12', groups=SingleTauGroup+DevGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau20_mediumGNTau_L1cTAU20M', groups=SingleTauGroup+DevGroup, monGroups=['tauMon:online', 'tauMon:t0']),
        ChainProp(name='HLT_tau25_mediumGNTau_L1eTAU20', groups=SingleTauGroup+DevGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau25_mediumGNTau_L1eTAU20M', groups=SingleTauGroup+DevGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau25_mediumGNTau_L1cTAU20M', groups=SingleTauGroup+DevGroup, monGroups=['tauMon:online', 'tauMon:t0']),
        ChainProp(name='HLT_tau30_mediumGNTau_L1cTAU30M', groups=SingleTauGroup+DevGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau35_mediumGNTau_L1cTAU30M', groups=SingleTauGroup+DevGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau40_mediumGNTau_L1cTAU35M', groups=SingleTauGroup+DevGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau50_mediumGNTau_L1cTAU50M', groups=SingleTauGroup+DevGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau60_mediumGNTau_L1eTAU60', groups=SingleTauGroup+DevGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau80_mediumGNTau_L1eTAU80', groups=SingleTauGroup+DevGroup, monGroups=['tauMon:t0']),
        #----------------------------------------------------------------------------------------------------------------------



        #----------------------------------------------------------------------------------------------------------------------
        # GNTau development chains (ATR-30086)

        # Single tau Loose and Tight variations
        ChainProp(name='HLT_tau25_looseGNTau_L1cTAU20M', groups=SingleTauGroup+DevGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau25_tightGNTau_L1cTAU20M', groups=SingleTauGroup+DevGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau35_looseGNTau_L1cTAU30M', groups=SingleTauGroup+DevGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau35_tightGNTau_L1cTAU30M', groups=SingleTauGroup+DevGroup, monGroups=['tauMon:t0']),


        # TES calibration triggers
        ChainProp(name='HLT_tau0_mediumGNTau_tau0_mediumGNTau_03dRAB_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55', l1SeedThresholds=['cTAU30M', 'cTAU20M'], groups=MultiTauGroup+DevGroup+Topo2Group), 
        ChainProp(name='HLT_tau0_mediumGNTau_tau0_mediumGNTau_03dRAB_L1cTAU30M_2cTAU20M_4jJ30p0ETA25', l1SeedThresholds=['cTAU30M', 'cTAU20M'], groups=MultiTauGroup+DevGroup),


        # Single tau jTAU- and eTAU-seeded chains to investigate the cTAU performance
        ChainProp(name='HLT_tau25_mediumGNTau_L1jTAU20', groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau35_mediumGNTau_L1eTAU30', groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),
        #----------------------------------------------------------------------------------------------------------------------


        # g-2 tau triggers (ATR-30638)
        ChainProp(name='HLT_2tau50_mediumRNN_29dphiAA_L12cTAU50M_DPHI-2eTAU50', l1SeedThresholds=['cTAU50M'], groups=MultiTauGroup+DevGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_2tau50_mediumGNTau_29dphiAA_L12cTAU50M_DPHI-2eTAU50', l1SeedThresholds=['cTAU50M'], groups=MultiTauGroup+DevGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau70_mediumRNN_tau50_mediumRNN_29dphiAB_L1eTAU70_2cTAU50M_DPHI-2eTAU50', l1SeedThresholds=['eTAU70', 'cTAU50M'], groups=MultiTauGroup+DevGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau70_mediumGNTau_tau50_mediumGNTau_29dphiAB_L1eTAU70_2cTAU50M_DPHI-2eTAU50', l1SeedThresholds=['eTAU70', 'cTAU50M'], groups=MultiTauGroup+DevGroup, monGroups=['tauMon:t0']),




        # Loose and Tight variations
        ChainProp(name='HLT_tau25_looseRNN_tracktwoMVA_L1cTAU20M', groups=SingleTauGroup+DevGroup),
        ChainProp(name='HLT_tau25_looseRNN_tracktwoLLP_L1cTAU20M', groups=SingleTauGroup+DevGroup),
        ChainProp(name='HLT_tau25_tightRNN_tracktwoMVA_L1cTAU20M', groups=SingleTauGroup+DevGroup),
        ChainProp(name='HLT_tau25_tightRNN_tracktwoLLP_L1cTAU20M', groups=SingleTauGroup+DevGroup),
        ChainProp(name='HLT_tau35_looseRNN_tracktwoMVA_L1cTAU30M', groups=SingleTauGroup+DevGroup),
        ChainProp(name='HLT_tau35_tightRNN_tracktwoMVA_L1cTAU30M', groups=SingleTauGroup+DevGroup),

        # TES calibration triggers
        ChainProp(name='HLT_tau160_ptonly_L1eTAU140', groups=SingleTauGroup+SupportPhIGroup),
        ChainProp(name='HLT_tau0_mediumRNN_tracktwoMVA_tau0_mediumRNN_tracktwoMVA_03dRAB_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55', l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup+Topo2Group), 
        ChainProp(name='HLT_tau0_mediumRNN_tracktwoMVA_tau0_mediumRNN_tracktwoMVA_03dRAB_L1cTAU30M_2cTAU20M_4jJ30p0ETA25',l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup),

        # Asymmetric Tau triggers for HH->bbtautau ATR-22230
        ChainProp(name="HLT_tau25_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup+Topo2Group), 
        ChainProp(name="HLT_tau35_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup+Topo2Group), 
        ChainProp(name="HLT_tau40_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup+Topo2Group), 
        ChainProp(name="HLT_tau25_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup+Topo2Group), 
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup+Topo2Group), 
        # ATR-26852: New Asymmetric ditau triggers for HH->bbtautau
        ChainProp(name="HLT_tau30_idperf_tracktwoMVA_tau20_idperf_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup+Topo2Group),
        ChainProp(name="HLT_tau30_idperf_tracktwoMVA_tau20_idperf_tracktwoMVA_03dRAB_L1cTAU30M_2cTAU20M_4jJ30p0ETA25", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup+Topo2Group),
        #ATR-27121
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L1jJ85p0ETA21_3jJ40p0ETA25", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup+Topo2Group),
        #ATR-27132
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L14jJ30p0ETA24_0DETA24_4DPHI99-eTAU30eTAU20" , l1SeedThresholds=['eTAU20','eTAU12'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L14jJ30p0ETA24_0DETA24_4DPHI99-eTAU30eTAU12" , l1SeedThresholds=['eTAU20','eTAU12'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L14jJ30p0ETA24_0DETA24_10DPHI99-eTAU30eTAU12", l1SeedThresholds=['eTAU20','eTAU12'], groups=MultiTauGroup+DevGroup), 
        #ATR-27132
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L1cTAU20M_cTAU12M_4jJ30p0ETA24_0DETA24_4DPHI99-eTAU30eTAU20" , l1SeedThresholds=['cTAU20M','cTAU12M'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L1cTAU20M_cTAU12M_4jJ30p0ETA24_0DETA24_4DPHI99-eTAU30eTAU12" , l1SeedThresholds=['cTAU20M','cTAU12M'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L1cTAU20M_cTAU12M_4jJ30p0ETA24_0DETA24_10DPHI99-eTAU30eTAU12", l1SeedThresholds=['cTAU20M','cTAU12M'], groups=MultiTauGroup+DevGroup), 
        #
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_L1jJ85p0ETA21_3jJ40p0ETA25", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup),
        #ATR-27132
        ChainProp(name="HLT_tau0_mediumRNN_tracktwoMVA_tau0_mediumRNN_tracktwoMVA_03dRAB_L14jJ30p0ETA24_0DETA24-eTAU30eTAU12"         , l1SeedThresholds=['eTAU20','eTAU12'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau0_mediumRNN_tracktwoMVA_tau0_mediumRNN_tracktwoMVA_03dRAB_L14jJ30p0ETA24_0DETA24_4DPHI99-eTAU30eTAU20" , l1SeedThresholds=['eTAU20','eTAU20'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau0_mediumRNN_tracktwoMVA_tau0_mediumRNN_tracktwoMVA_03dRAB_L14jJ30p0ETA24_0DETA24_4DPHI99-eTAU30eTAU12" , l1SeedThresholds=['eTAU20','eTAU12'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau0_mediumRNN_tracktwoMVA_tau0_mediumRNN_tracktwoMVA_03dRAB_L14jJ30p0ETA24_0DETA24_10DPHI99-eTAU30eTAU12", l1SeedThresholds=['eTAU20','eTAU12'], groups=MultiTauGroup+DevGroup), 

        # Eta L1
        ChainProp(name="HLT_tau25_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L1cTAU30M_2cTAU20M_4jJ30p0ETA25", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup),
        ChainProp(name="HLT_tau35_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L1cTAU30M_2cTAU20M_4jJ30p0ETA25", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup),
        ChainProp(name="HLT_tau40_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L1cTAU30M_2cTAU20M_4jJ30p0ETA25", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup),
        ChainProp(name="HLT_tau25_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB_L1cTAU30M_2cTAU20M_4jJ30p0ETA25", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup),
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB_L1cTAU30M_2cTAU20M_4jJ30p0ETA25", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup),


        # ---- jTAU and eTAU seeded chains to investigate cTAU performance
        ChainProp(name="HLT_tau25_idperf_tracktwoMVA_L1jTAU20",   groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau25_perf_tracktwoMVA_L1jTAU20",     groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau25_mediumRNN_tracktwoMVA_L1jTAU20",   groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),

        ChainProp(name="HLT_tau35_idperf_tracktwoMVA_L1eTAU30",   groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau35_perf_tracktwoMVA_L1eTAU30",   groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau35_mediumRNN_tracktwoMVA_L1eTAU30",   groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),

        # LRT tau dev ATR-23787
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
    ]

    chains['Bphysics'] = [
        #ATR-21003; default dimuon and Bmumux chains from Run2; l2io validation; should not be moved to Physics
        ChainProp(name='HLT_2mu4_noL2Comb_bJpsimumu_L12MU3V', stream=["BphysDelayed"], groups=BphysicsGroup+DevGroup),
        ChainProp(name='HLT_mu6_noL2Comb_mu4_noL2Comb_bJpsimumu_L1MU5VF_2MU3V', l1SeedThresholds=['MU5VF','MU3V'], stream=["BphysDelayed"], groups=BphysicsGroup+DevGroup),
        ChainProp(name='HLT_2mu4_noL2Comb_bBmumux_BpmumuKp_L12MU3V', stream=["BphysDelayed"], groups=BphysicsGroup+DevGroup),
        ChainProp(name='HLT_2mu4_noL2Comb_bBmumux_BsmumuPhi_L12MU3V', stream=["BphysDelayed"], groups=BphysicsGroup+DevGroup),
        ChainProp(name='HLT_2mu4_noL2Comb_bBmumux_LbPqKm_L12MU3V', stream=["BphysDelayed"], groups=BphysicsGroup+DevGroup),

    ]

    chains['Combined'] = [
        # Test chains for muon msonly + VBF for ATR-28412

        # mu-tag & tau-probe triggers for LLP (ATR-23150)
        ChainProp(name='HLT_mu26_ivarmedium_tau100_mediumRNN_tracktwoLLP_probe_L1eTAU80_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU80'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleMuonGroup),
        # ChainProp(name='HLT_e26_lhtight_ivarloose_tau100_mediumRNN_tracktwoLLP_probe_L1eTAU80_03dRAB_L1EM22VHI', l1SeedThresholds=['EM22VHI','PROBEeTAU80'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleElectronGroup),

        # tau + jet and tau + photon tag and probe (ATR-24031)
        # *** Temporarily commented because counts are fluctuating in CI and causing confusion ***
        #ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_L1eTAU12_j15_pf_ftf_03dRAB_L1RD0_FILLED', l1SeedThresholds=['PROBEeTAU12','FSNOSEED'], groups=TagAndProbeGroup+TauJetGroup),
        #ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_j15_pf_ftf_03dRAB_L1RD0_FILLED', l1SeedThresholds=['PROBEeTAU12','FSNOSEED'], groups=TagAndProbeGroup+TauJetGroup),
        # *** Temporarily commented because counts are fluctuating in CI and causing confusion ***

        #ATR-30799
        ChainProp(name="HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_SHARED_j20c_020jvt_bgn277_j20c_020jvt_bgn277_pf_ftf_presel1c20XX2c20bgtwo85_L1BTAG-MU5VFjJ20_2jJ30p0ETA25", l1SeedThresholds=['FSNOSEED']*5, groups=MultiBjetGroup + DevGroup),
        ChainProp(name="HLT_mu6_j75c_020jvt_j50c_020jvt_j25c_020jvt_SHARED_j20c_020jvt_bgn277_j20c_020jvt_bgn277_pf_ftf_presel1c20XX2c20bgtwo85_L1BTAG-MU5VFjJ20_2jJ30p0ETA25", l1SeedThresholds=['MU5VF']+['FSNOSEED']*5, groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_mu6_j75c_020jvt_j50c_020jvt_j25c_020jvt_SHARED_j20c_020jvt_bgn277_j20c_020jvt_bgn277_pf_ftf_presel1c20XX2c20bgtwo85_dRAF04_L1BTAG-MU5VFjJ20_2jJ30p0ETA25", l1SeedThresholds=['MU5VF']+['FSNOSEED']*5, groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_j55c_020jvt_j40c_020jvt_j25c_020jvt_SHARED_j20c_020jvt_bgn277_j20c_020jvt_bgn277_pf_ftf_presel1c20XX2c20bgtwo85_L1BTAG-MU5VFjJ20_2jJ30p0ETA25", l1SeedThresholds=['FSNOSEED']*5, groups=MultiBjetGroup + DevGroup),
        ChainProp(name="HLT_mu6_j55c_020jvt_j40c_020jvt_j25c_020jvt_SHARED_j20c_020jvt_bgn277_j20c_020jvt_bgn277_pf_ftf_presel1c20XX2c20bgtwo85_L1BTAG-MU5VFjJ20_2jJ30p0ETA25", l1SeedThresholds=['MU5VF']+['FSNOSEED']*5, groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name="HLT_mu6_j55c_020jvt_j40c_020jvt_j25c_020jvt_SHARED_j20c_020jvt_bgn277_j20c_020jvt_bgn277_pf_ftf_presel1c20XX2c20bgtwo85_dRAF04_L1BTAG-MU5VFjJ20_2jJ30p0ETA25", l1SeedThresholds=['MU5VF']+['FSNOSEED']*5, groups=SupportPhIGroup+MultiBjetGroup),


        #Photon+MET new NN without isolation ATR-26410
        # ChainProp(name='HLT_g25_tight_icaloloose_xe40_cell_xe50_tcpufit_80mTAC_L1EM22VHI',l1SeedThresholds=['EM22VHI','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportLegGroup+EgammaMETGroup),
        # ChainProp(name='HLT_g25_tight_icaloloose_xe40_cell_xe50_tcpufit_xe60_nn_80mTAC_L1EM22VHI',l1SeedThresholds=['EM22VHI','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportLegGroup+EgammaMETGroup),
        # ChainProp(name='HLT_g25_tight_icaloloose_xe40_cell_xe50_tcpufit_xe70_nn_80mTAC_L1EM22VHI',l1SeedThresholds=['EM22VHI','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportLegGroup+EgammaMETGroup),
        # ChainProp(name='HLT_g25_tight_icaloloose_xe40_cell_xe50_tcpufit_xe80_nn_80mTAC_L1EM22VHI',l1SeedThresholds=['EM22VHI','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportLegGroup+EgammaMETGroup),
        # ChainProp(name='HLT_g50_tight_xe40_cell_xe50_pfopufit_xe60_nn_80mTAC_L1EM22VHI',l1SeedThresholds=['EM22VHI','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportLegGroup+EgammaMETGroup),
        # ChainProp(name='HLT_g50_tight_xe40_cell_xe50_pfopufit_xe70_nn_80mTAC_L1EM22VHI',l1SeedThresholds=['EM22VHI','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportLegGroup+EgammaMETGroup),
        # ChainProp(name='HLT_g50_tight_xe40_cell_xe50_pfopufit_xe80_nn_80mTAC_L1EM22VHI',l1SeedThresholds=['EM22VHI','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportLegGroup+EgammaMETGroup),
        # #Photon+MET NN ATR-25574
        # ChainProp(name='HLT_g25_tight_icalotight_xe40_cell_xe50_tcpufit_xe70_nn_80mTAD_L1EM22VHI',l1SeedThresholds=['EM22VHI','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportLegGroup+EgammaMETGroup),
        # #Photon+MET ATR-25384
        # ChainProp(name='HLT_g25_tight_icalotight_xe40_cell_xe50_tcpufit_L1EM22VHI',l1SeedThresholds=['EM22VHI','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportLegGroup+EgammaMETGroup),
        # ChainProp(name='HLT_g25_loose_xe40_cell_xe50_tcpufit_18dphiAB_18dphiAC_80mTAC_L1EM22VHI',l1SeedThresholds=['EM22VHI','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportLegGroup+EgammaMETGroup),
        ChainProp(name='HLT_g25_loose_xe40_cell_xe50_tcpufit_18dphiAB_18dphiAC_80mTAC_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaMETGroup),
        ChainProp(name='HLT_g25_tight_icalotight_xe40_cell_xe50_tcpufit_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaMETGroup),

        #added for debugging: ATR-24946
        # ChainProp(name='HLT_g25_tight_icalotight_xe40_cell_xe50_tcpufit_18dphiAB_L1EM22VHI',l1SeedThresholds=['EM22VHI']+2*['FSNOSEED'],stream=[PhysicsStream], groups=DevGroup+EgammaMETGroup),
        # ChainProp(name='HLT_g25_tight_icalotight_xe40_cell_xe50_tcpufit_18dphiAC_L1EM22VHI',l1SeedThresholds=['EM22VHI']+2*['FSNOSEED'],stream=[PhysicsStream], groups=DevGroup+EgammaMETGroup),
        # ChainProp(name='HLT_g25_tight_icalotight_xe40_cell_xe50_tcpufit_18dphiAB_18dphiAC_L1EM22VHI',l1SeedThresholds=['EM22VHI']+2*['FSNOSEED'],stream=[PhysicsStream], groups=DevGroup+EgammaMETGroup),
        # ChainProp(name='HLT_g25_tight_icalotight_xe40_cell_xe50_tcpufit_18dphiAB_18dphiAC_80mTAC_L1EM22VHI',l1SeedThresholds=['EM22VHI']+2*['FSNOSEED'], groups=DevGroup+EgammaMETGroup),
        # ChainProp(name='HLT_g25_tight_icalotight_xe40_cell_xe40_tcpufit_xe40_pfopufit_18dphiAB_18dphiAC_80mTAC_L1EM22VHI',l1SeedThresholds=['EM22VHI']+3*['FSNOSEED'], groups=DevGroup+EgammaMETGroup),


        # Tests of potential TLA chains for cost/rate
        # ATR-19317 - dijet+ISR
        # ChainProp(name='HLT_g35_loose_3j25_pf_ftf_L1EM22VHI',          l1SeedThresholds=['EM22VHI','FSNOSEED'], groups=EgammaJetGroup),
        # ChainProp(name='HLT_g35_medium_3j25_pf_ftf_L1EM22VHI',         l1SeedThresholds=['EM22VHI','FSNOSEED'], groups=EgammaJetGroup),
        # ChainProp(name='HLT_g35_tight_3j25_0eta290_boffperf_pf_ftf_L1EM22VHI', l1SeedThresholds=['EM22VHI','FSNOSEED'], groups=EgammaJetGroup),

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
        #ChainProp(name='HLT_e26_lhtight_ivarloose_mu22noL1_j20_0eta290_020jvt_pf_ftf_boffperf_L1EM22VHI', l1SeedThresholds=['EM22VHI','FSNOSEED','FSNOSEED'], stream=[PhysicsStream,'express'], groups=DevGroup+EgammaBjetGroup, monGroups=['bJetMon:shifter']),

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

        #ATR-30179
        ChainProp(name='HLT_mu14_ivarloose_tau35_mediumRNN_tracktwoMVA_03dRAB_L1cTAU30M_3DR35-MU8F-eTAU30', l1SeedThresholds=['MU8F','cTAU30M'], stream=[PhysicsStream], groups=SupportPhIGroup+MuonTauGroup),

        ChainProp(name='HLT_mu14_ivarloose_tau35_mediumGNTau_03dRAB_L1cTAU30M_3DR35-MU8F-eTAU30', l1SeedThresholds=['MU8F', 'cTAU30M'], stream=[PhysicsStream], groups=SupportPhIGroup+MuonTauGroup),

        #ATR-30378
        ChainProp(name='HLT_mu10_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_dRAB04_L1BTAG-MU8FjJ40_2jJ40p0ETA25'            , l1SeedThresholds=['MU8F']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu10_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_dRAB04_L1BTAG-MU8FjJ30_2jJ30p0ETA25_jJ50p0ETA25', l1SeedThresholds=['MU8F']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_dRAB04_L1BTAG-MU5VFjJ40_2jJ40p0ETA25'            , l1SeedThresholds=['MU5VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_dRAB04_L1BTAG-MU5VFjJ30_2jJ30p0ETA25_jJ50p0ETA25', l1SeedThresholds=['MU5VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        #
        ChainProp(name='HLT_mu10_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn280_pf_ftf_presel2c20XX2c20bgtwo85_dRAB04_L1BTAG-MU8FjJ40_2jJ40p0ETA25'            , l1SeedThresholds=['MU8F']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu10_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn280_pf_ftf_presel2c20XX2c20bgtwo85_dRAB04_L1BTAG-MU8FjJ30_2jJ30p0ETA25_jJ50p0ETA25', l1SeedThresholds=['MU8F']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn280_pf_ftf_presel2c20XX2c20bgtwo85_dRAB04_L1BTAG-MU5VFjJ40_2jJ40p0ETA25'            , l1SeedThresholds=['MU5VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn280_pf_ftf_presel2c20XX2c20bgtwo85_dRAB04_L1BTAG-MU5VFjJ30_2jJ30p0ETA25_jJ50p0ETA25', l1SeedThresholds=['MU5VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),

        #same versions as above with presel3c20XX1c20bgtwo85 instead of presel2c20XX2c20bgtwo85
        ChainProp(name='HLT_mu10_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_dRAB04_L1BTAG-MU8FjJ40_2jJ40p0ETA25'            , l1SeedThresholds=['MU8F']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu10_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_dRAB04_L1BTAG-MU8FjJ30_2jJ30p0ETA25_jJ50p0ETA25', l1SeedThresholds=['MU8F']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_dRAB04_L1BTAG-MU5VFjJ40_2jJ40p0ETA25'            , l1SeedThresholds=['MU5VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_dRAB04_L1BTAG-MU5VFjJ30_2jJ30p0ETA25_jJ50p0ETA25', l1SeedThresholds=['MU5VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        #
        ChainProp(name='HLT_mu10_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn280_pf_ftf_presel3c20XX1c20bgtwo85_dRAB04_L1BTAG-MU8FjJ40_2jJ40p0ETA25'            , l1SeedThresholds=['MU8F']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu10_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn280_pf_ftf_presel3c20XX1c20bgtwo85_dRAB04_L1BTAG-MU8FjJ30_2jJ30p0ETA25_jJ50p0ETA25', l1SeedThresholds=['MU8F']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn280_pf_ftf_presel3c20XX1c20bgtwo85_dRAB04_L1BTAG-MU5VFjJ40_2jJ40p0ETA25'            , l1SeedThresholds=['MU5VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn280_pf_ftf_presel3c20XX1c20bgtwo85_dRAB04_L1BTAG-MU5VFjJ30_2jJ30p0ETA25_jJ50p0ETA25', l1SeedThresholds=['MU5VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),

        #addind a chain to enable optimization studies
        ChainProp(name='HLT_mu10_j20c_020jvt_3j20c_020jvt_SHARED_j20c_020jvt_bgn280_pf_ftf_presel3c20XX1c20bgtwo85_dRAB04_L1BTAG-MU8FjJ40_2jJ40p0ETA25'            , l1SeedThresholds=['MU8F']+['FSNOSEED']*3,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu10_j20c_020jvt_3j20c_020jvt_SHARED_j20c_020jvt_bgn280_pf_ftf_presel3c20XX1c20bgtwo85_dRAB04_L1BTAG-MU8FjJ30_2jJ30p0ETA25_jJ50p0ETA25', l1SeedThresholds=['MU8F']+['FSNOSEED']*3,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j20c_020jvt_3j20c_020jvt_SHARED_j20c_020jvt_bgn280_pf_ftf_presel3c20XX1c20bgtwo85_dRAB04_L1BTAG-MU5VFjJ40_2jJ40p0ETA25'            , l1SeedThresholds=['MU5VF']+['FSNOSEED']*3,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j20c_020jvt_3j20c_020jvt_SHARED_j20c_020jvt_bgn280_pf_ftf_presel3c20XX1c20bgtwo85_L1BTAG-MU5VFjJ30_2jJ30p0ETA25_jJ50p0ETA25'      , l1SeedThresholds=['MU5VF']+['FSNOSEED']*3,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),

        ChainProp(name='HLT_mu10_j20c_020jvt_3j20c_020jvt_SHARED_j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_dRAB04_L1BTAG-MU8FjJ40_2jJ40p0ETA25'            , l1SeedThresholds=['MU8F']+['FSNOSEED']*3,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu10_j20c_020jvt_3j20c_020jvt_SHARED_j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_dRAB04_L1BTAG-MU8FjJ30_2jJ30p0ETA25_jJ50p0ETA25', l1SeedThresholds=['MU8F']+['FSNOSEED']*3,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j20c_020jvt_3j20c_020jvt_SHARED_j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_dRAB04_L1BTAG-MU5VFjJ40_2jJ40p0ETA25'            , l1SeedThresholds=['MU5VF']+['FSNOSEED']*3,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j20c_020jvt_3j20c_020jvt_SHARED_j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_dRAB04_L1BTAG-MU5VFjJ30_2jJ30p0ETA25_jJ50p0ETA25', l1SeedThresholds=['MU5VF']+['FSNOSEED']*3,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        
        ChainProp(name='HLT_mu6_j20c_020jvt_3j20c_020jvt_SHARED_j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_dRAB04_L1BTAG-MU3VFjJ30_2jJ30p0ETA25', l1SeedThresholds=['MU3VF']+['FSNOSEED']*3,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu10_j20c_020jvt_3j20c_020jvt_SHARED_j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_dRAB04_L1BTAG-MU3VFjJ30_2jJ30p0ETA25', l1SeedThresholds=['MU3VF']+['FSNOSEED']*3,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j20c_020jvt_3j20c_020jvt_SHARED_j20c_020jvt_bgn280_pf_ftf_presel3c20XX1c20bgtwo85_dRAB04_L1BTAG-MU3VFjJ30_2jJ30p0ETA25', l1SeedThresholds=['MU3VF']+['FSNOSEED']*3,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu10_j20c_020jvt_3j20c_020jvt_SHARED_j20c_020jvt_bgn280_pf_ftf_presel3c20XX1c20bgtwo85_dRAB04_L1BTAG-MU3VFjJ30_2jJ30p0ETA25', l1SeedThresholds=['MU3VF']+['FSNOSEED']*3,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        
        ChainProp(name='HLT_mu6_j20c_020jvt_3j20c_020jvt_SHARED_j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_dRAB04_L1BTAG-MU3VFjJ30_2jJ40p0ETA25', l1SeedThresholds=['MU3VF']+['FSNOSEED']*3,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu10_j20c_020jvt_3j20c_020jvt_SHARED_j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_dRAB04_L1BTAG-MU3VFjJ30_2jJ40p0ETA25', l1SeedThresholds=['MU3VF']+['FSNOSEED']*3,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j20c_020jvt_3j20c_020jvt_SHARED_j20c_020jvt_bgn280_pf_ftf_presel3c20XX1c20bgtwo85_dRAB04_L1BTAG-MU3VFjJ30_2jJ40p0ETA25', l1SeedThresholds=['MU3VF']+['FSNOSEED']*3,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu10_j20c_020jvt_3j20c_020jvt_SHARED_j20c_020jvt_bgn280_pf_ftf_presel3c20XX1c20bgtwo85_dRAB04_L1BTAG-MU3VFjJ30_2jJ40p0ETA25', l1SeedThresholds=['MU3VF']+['FSNOSEED']*3,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        
        ChainProp(name='HLT_mu6_j20c_020jvt_3j20c_020jvt_SHARED_j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_dRAB04_L1BTAG-MU3VFjJ30_2jJ30p0ETA25_jJ40p0ETA25', l1SeedThresholds=['MU3VF']+['FSNOSEED']*3,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu10_j20c_020jvt_3j20c_020jvt_SHARED_j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_dRAB04_L1BTAG-MU3VFjJ30_2jJ30p0ETA25_jJ40p0ETA25', l1SeedThresholds=['MU3VF']+['FSNOSEED']*3,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j20c_020jvt_3j20c_020jvt_SHARED_j20c_020jvt_bgn280_pf_ftf_presel3c20XX1c20bgtwo85_dRAB04_L1BTAG-MU3VFjJ30_2jJ30p0ETA25_jJ40p0ETA25', l1SeedThresholds=['MU3VF']+['FSNOSEED']*3,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu10_j20c_020jvt_3j20c_020jvt_SHARED_j20c_020jvt_bgn280_pf_ftf_presel3c20XX1c20bgtwo85_dRAB04_L1BTAG-MU3VFjJ30_2jJ30p0ETA25_jJ40p0ETA25', l1SeedThresholds=['MU3VF']+['FSNOSEED']*3,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
    
        # ATR-21596 
        # Muon+HT Test chains for PEB
        ChainProp(name='HLT_mu6_probe_j20_pf_ftf_DarkJetPEBTLA_L1HT190-J15s5pETA21', l1SeedThresholds=['PROBEMU5VF','FSNOSEED'], stream=['DarkJetPEBTLA'], groups=DevGroup+MuonJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_mu10_probe_j20_pf_ftf_DarkJetPEBTLA_L1HT190-J15s5pETA21', l1SeedThresholds=['PROBEMU8F','FSNOSEED'], stream=['DarkJetPEBTLA'], groups=DevGroup+MuonJetGroup+LegacyTopoGroup),

        #ATR-30824
        ChainProp(name='HLT_mu10_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn277_j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_dRAF04_L1BTAG-MU3VFjJ20_2jJ30p0ETA25'            , l1SeedThresholds=['MU3VF']+['FSNOSEED']*6,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu10_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn277_j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_dRAD04_L1BTAG-MU3VFjJ20_2jJ30p0ETA25_jJ50p0ETA25', l1SeedThresholds=['MU3VF']+['FSNOSEED']*6,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu10_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn277_j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_dRAD04_L1BTAG-MU3VFjJ20_2jJ40p0ETA25'            , l1SeedThresholds=['MU3VF']+['FSNOSEED']*6,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),

        ChainProp(name='HLT_mu10_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn277_j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_dRAD04_L1BTAG-MU5VFjJ20_2jJ30p0ETA25'            , l1SeedThresholds=['MU5VF']+['FSNOSEED']*6,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu10_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn277_j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_dRAD04_L1BTAG-MU5VFjJ20_2jJ30p0ETA25_jJ50p0ETA25', l1SeedThresholds=['MU5VF']+['FSNOSEED']*6,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu10_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn277_j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_dRAD04_L1BTAG-MU5VFjJ20_2jJ40p0ETA25'            , l1SeedThresholds=['MU5VF']+['FSNOSEED']*6,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),

        ChainProp(name='HLT_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_L1BTAG-MU3VFjJ20_2jJ40p0ETA25'            , l1SeedThresholds=['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_L1BTAG-MU5VFjJ20_2jJ40p0ETA25'            , l1SeedThresholds=['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),

        ChainProp(name='HLT_mu6_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_L1BTAG-MU3VFjJ20_2jJ30p0ETA25'                         , l1SeedThresholds=['MU3VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn277_j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_dRAF04_L1BTAG-MU3VFjJ20_2jJ30p0ETA25', l1SeedThresholds=['MU3VF']+['FSNOSEED']*6,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L1BTAG-MU3VFjJ20_2jJ30p0ETA25'                          , l1SeedThresholds=['MU3VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_dRAF04_L1BTAG-MU3VFjJ20_2jJ30p0ETA25'                   , l1SeedThresholds=['MU3VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),

        ChainProp(name='HLT_mu6_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_L1BTAG-MU5VFjJ20_2jJ30p0ETA25'                         , l1SeedThresholds=['MU5VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn277_j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_dRAF04_L1BTAG-MU5VFjJ20_2jJ30p0ETA25', l1SeedThresholds=['MU5VF']+['FSNOSEED']*6,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_L1BTAG-MU5VFjJ20_2jJ30p0ETA25'                          , l1SeedThresholds=['MU5VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),
        ChainProp(name='HLT_mu6_j55c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn277_pf_ftf_presel3c20XX1c20bgtwo85_dRAF04_L1BTAG-MU5VFjJ20_2jJ30p0ETA25'                   , l1SeedThresholds=['MU5VF']+['FSNOSEED']*5,  stream=[PhysicsStream], groups=SupportPhIGroup+MultiBjetGroup),

        # ATR-21596 MET with DarkJet
        # PFO MET
        ChainProp(name='HLT_j0_HT650XX0eta240_pf_ftf_preselcHT450_xe0_pfopufit_L1HT190-jJ40s5pETA21', l1SeedThresholds=['FSNOSEED', 'FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+SingleJetGroup+METGroup+Topo3Group),
        # ChainProp(name='HLT_xe0_pfopufit_j0_HT500XX0eta240_pf_ftf_preselcHT450_DarkJetPEBTLA_L1HT190-jJ40s5pETA21', l1SeedThresholds=['FSNOSEED', 'FSNOSEED'], stream=['DarkJetPEBTLA'],groups=SupportPhIGroup+MultiJetGroup+METGroup+Topo3Group+['RATE:CPS_HT190-jJ40s5pETA21']),
        ChainProp(name='HLT_j0_HT500XX0eta240_pf_ftf_preselcHT450_xe0_pfopufit_L1HT190-jJ40s5pETA21', l1SeedThresholds=['FSNOSEED', 'FSNOSEED'],groups=SupportPhIGroup+MultiJetGroup+METGroup+Topo3Group+['RATE:CPS_HT190-jJ40s5pETA21']),
        # NN MET
        ChainProp(name='HLT_j0_HT650XX0eta240_pf_ftf_preselcHT450_xe0_nn_L1HT190-jJ40s5pETA21', l1SeedThresholds=['FSNOSEED', 'FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+SingleJetGroup+METGroup+Topo3Group),
        # ChainProp(name='HLT_xe0_nn_j0_HT500XX0eta240_pf_ftf_preselcHT450_DarkJetPEBTLA_L1HT190-jJ40s5pETA21', l1SeedThresholds=['FSNOSEED', 'FSNOSEED'], stream=['DarkJetPEBTLA'],groups=SupportPhIGroup+MultiJetGroup+METGroup+Topo3Group+['RATE:CPS_HT190-jJ40s5pETA21']),
        ChainProp(name='HLT_j0_HT500XX0eta240_pf_ftf_preselcHT450_xe0_nn_L1HT190-jJ40s5pETA21', l1SeedThresholds=['FSNOSEED', 'FSNOSEED'],groups=SupportPhIGroup+MultiJetGroup+METGroup+Topo3Group+['RATE:CPS_HT190-jJ40s5pETA21']),



        #----------------------------------------------------------------------------------------------------------------------
        # GNTau Physics chains (ATR-30086) - to be moved to the Physics Menu for 2025 after validation
        
        # Muon + Tau primaries
        ChainProp(name='HLT_mu14_ivarloose_tau25_mediumGNTau_03dRAB_L1MU8F_cTAU20M_3jJ30', l1SeedThresholds=['MU8F', 'cTAU20M'], groups=PrimaryPhIGroup+MuonTauGroup),
        ChainProp(name='HLT_mu14_ivarloose_tau35_mediumGNTau_03dRAB_L1MU8F_cTAU30M', l1SeedThresholds=['MU8F', 'cTAU30M'], groups=PrimaryPhIGroup+MuonTauGroup),
        ChainProp(name='HLT_mu20_ivarloose_tau20_mediumGNTau_L1eTAU12_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH', 'eTAU12'], groups=PrimaryPhIGroup+MuonTauGroup),
        ChainProp(name='HLT_mu23_ivarloose_tau20_mediumGNTau_L1eTAU12_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH', 'eTAU12'], groups=PrimaryPhIGroup+MuonTauGroup),



        # Electron + Tau primaries
        ChainProp(name='HLT_e17_lhmedium_ivarloose_tau25_mediumGNTau_03dRAB_L1eEM18M_2cTAU20M_4jJ30', l1SeedThresholds=['eEM18M', 'cTAU20M'], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_e24_lhmedium_ivarloose_tau20_mediumGNTau_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M', 'eTAU12'], groups=PrimaryPhIGroup+EgammaTauGroup),
        ChainProp(name='HLT_e24_lhmedium_ivarloose_tau20_mediumGNTau_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M', 'eTAU12'], groups=PrimaryPhIGroup+EgammaTauGroup),



        # MET + Tau primaries
        ChainProp(name='HLT_tau50_mediumGNTau_xe80_tcpufit_xe50_cell_L1jXE100', l1SeedThresholds=['cTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_xe80_pfopufit_xe50_cell_L1jXE100', l1SeedThresholds=['cTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau50_mediumGNTau_xe80_tcpufit_xe50_cell_L1gXEJWOJ100', l1SeedThresholds=['cTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_xe80_pfopufit_xe50_cell_L1gXEJWOJ100', l1SeedThresholds=['cTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau50_mediumGNTau_xe80_tcpufit_xe50_cell_L1jXE110', l1SeedThresholds=['cTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_xe80_pfopufit_xe50_cell_L1jXE110', l1SeedThresholds=['cTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau50_mediumGNTau_xe80_tcpufit_xe50_cell_L1gXEJWOJ110', l1SeedThresholds=['cTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_xe80_pfopufit_xe50_cell_L1gXEJWOJ110', l1SeedThresholds=['cTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),



        # Muon/Electron/Tau + Tau + MET primaries
        ChainProp(name='HLT_tau60_mediumGNTau_tau25_mediumGNTau_xe50_cell_03dRAB_L1eTAU60_2cTAU20M_jXE80', l1SeedThresholds=['eTAU60', 'cTAU20M', 'FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),
        ChainProp(name='HLT_e17_lhmedium_tau25_mediumGNTau_xe50_cell_03dRAB_L1eEM18M_2cTAU20M_jXE70', l1SeedThresholds=['eEM18M', 'cTAU20M', 'FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),
        ChainProp(name='HLT_mu14_tau25_mediumGNTau_xe50_cell_03dRAB_L1MU8F_cTAU20M_jXE70', l1SeedThresholds=['MU8F', 'cTAU20M', 'FSNOSEED'], groups=PrimaryPhIGroup+TauMETGroup),



        # HH->bbtautau: Bjet + Tau primaries, including backups for additional CPU savings (ATR-28198)
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU12_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel3c20XX1c20bgtwo85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU12_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo85XX1c20gntau90_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU12_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo85XX1c20gntau85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU12_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo82XX1c20gntau85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU12_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo82XX1c20gntau80_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], groups=PrimaryPhIGroup+TauBJetGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU12_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo80XX1c20gntau80_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['PROBEeTAU12']+5*['FSNOSEED'], groups=PrimaryPhIGroup+TauBJetGroup),



        # VBF triggers (ATR-22594)
        # Main stream
        ChainProp(name='HLT_tau25_mediumGNTau_tau20_mediumGNTau_03dRAB_j70_j50a_j0_DJMASS900j50_L1jMJJ-500-NFF', l1SeedThresholds=['eTAU12', 'eTAU12', 'FSNOSEED', 'FSNOSEED', 'FSNOSEED'], groups=PrimaryPhIGroup+TauJetGroup+Topo3Group),


        # Mu + Tau T&P chains
        ChainProp(name='HLT_mu26_ivarmedium_tau20_mediumGNTau_probe_L1eTAU12_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH', 'PROBEeTAU12'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau20_mediumGNTau_probe_L1cTAU20M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH', 'PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau25_mediumGNTau_probe_L1eTAU20_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH', 'PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau25_mediumGNTau_probe_L1eTAU20M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH', 'PROBEeTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau25_mediumGNTau_probe_L1cTAU20M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH', 'PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau30_mediumGNTau_probe_L1eTAU20_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH', 'PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau30_mediumGNTau_probe_L1cTAU20M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH', 'PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau30_mediumGNTau_probe_L1cTAU30M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH', 'PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau35_mediumGNTau_probe_L1cTAU30M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH', 'PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau40_mediumGNTau_probe_L1cTAU35M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH', 'PROBEcTAU35M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau50_mediumGNTau_probe_L1cTAU50M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH', 'PROBEcTAU50M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau60_mediumGNTau_probe_L1eTAU60_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH', 'PROBEeTAU60'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau70_mediumGNTau_probe_L1eTAU70_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH', 'PROBEeTAU70'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau80_mediumGNTau_probe_L1eTAU80_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH', 'PROBEeTAU80'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau160_mediumGNTau_probe_L1eTAU140_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH', 'PROBEeTAU140'], groups=TagAndProbePhIGroup+SingleMuonGroup),

        # ATR-25512
        ChainProp(name='HLT_mu26_ivarmedium_tau20_mediumGNTau_probe_L1eTAU12_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH', 'PROBEeTAU12'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau20_mediumGNTau_probe_L1cTAU20M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH', 'PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau25_mediumGNTau_probe_L1eTAU20_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH', 'PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau25_mediumGNTau_probe_L1eTAU20M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH', 'PROBEeTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau25_mediumGNTau_probe_L1cTAU20M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH', 'PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau30_mediumGNTau_probe_L1eTAU20_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH', 'PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau30_mediumGNTau_probe_L1cTAU20M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH', 'PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu26_ivarmedium_tau30_mediumGNTau_probe_L1cTAU30M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH', 'PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau35_mediumGNTau_probe_L1cTAU30M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH', 'PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau40_mediumGNTau_probe_L1cTAU35M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH', 'PROBEcTAU35M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau50_mediumGNTau_probe_L1cTAU50M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH', 'PROBEcTAU50M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau60_mediumGNTau_probe_L1eTAU60_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH', 'PROBEeTAU60'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau70_mediumGNTau_probe_L1eTAU70_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH', 'PROBEeTAU70'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau80_mediumGNTau_probe_L1eTAU80_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH', 'PROBEeTAU80'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu26_ivarmedium_tau160_mediumGNTau_probe_L1eTAU140_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH', 'PROBEeTAU140'], groups=TagAndProbePhIGroup+SingleMuonGroup),

        # ATR-27343
        ChainProp(name='HLT_mu24_ivarmedium_tau20_mediumGNTau_probe_L1eTAU12_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH', 'PROBEeTAU12'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau25_mediumGNTau_probe_L1eTAU20_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH', 'PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau25_mediumGNTau_probe_L1eTAU20M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH', 'PROBEeTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau25_mediumGNTau_probe_L1cTAU20M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH', 'PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau30_mediumGNTau_probe_L1eTAU20_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH', 'PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau30_mediumGNTau_probe_L1cTAU30M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH', 'PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau35_mediumGNTau_probe_L1cTAU30M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH', 'PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau40_mediumGNTau_probe_L1cTAU35M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH', 'PROBEcTAU35M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau50_mediumGNTau_probe_L1cTAU50M_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH', 'PROBEcTAU50M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau60_mediumGNTau_probe_L1eTAU60_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH', 'PROBEeTAU60'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau70_mediumGNTau_probe_L1eTAU70_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH', 'PROBEeTAU70'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau80_mediumGNTau_probe_L1eTAU80_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH', 'PROBEeTAU80'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau160_mediumGNTau_probe_L1eTAU140_03dRAB_L1MU18VFCH', l1SeedThresholds=['MU18VFCH', 'PROBEeTAU140'], groups=TagAndProbePhIGroup+SingleMuonGroup),

        # ATR-25512
        ChainProp(name='HLT_mu24_ivarmedium_tau20_mediumGNTau_probe_L1eTAU12_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH', 'PROBEeTAU12'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau20_mediumGNTau_probe_L1cTAU20M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH', 'PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau25_mediumGNTau_probe_L1eTAU20_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH', 'PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau25_mediumGNTau_probe_L1eTAU20M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH', 'PROBEeTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau25_mediumGNTau_probe_L1cTAU20M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH', 'PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleMuonGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_mu24_ivarmedium_tau30_mediumGNTau_probe_L1eTAU20_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH', 'PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau30_mediumGNTau_probe_L1cTAU30M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH', 'PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau35_mediumGNTau_probe_L1cTAU30M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH', 'PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau40_mediumGNTau_probe_L1cTAU35M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH', 'PROBEcTAU35M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau50_mediumGNTau_probe_L1cTAU50M_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH', 'PROBEcTAU50M'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau60_mediumGNTau_probe_L1eTAU60_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH', 'PROBEeTAU60'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau70_mediumGNTau_probe_L1eTAU70_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH', 'PROBEeTAU70'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau80_mediumGNTau_probe_L1eTAU80_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH', 'PROBEeTAU80'], groups=TagAndProbePhIGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu24_ivarmedium_tau160_mediumGNTau_probe_L1eTAU140_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH', 'PROBEeTAU140'], groups=TagAndProbePhIGroup+SingleMuonGroup),



        # Electron + Tau T&P chains
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau20_mediumGNTau_probe_L1eTAU12_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M', 'PROBEeTAU12'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau20_mediumGNTau_probe_L1cTAU20M_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M', 'PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau25_mediumGNTau_probe_L1eTAU20_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M', 'PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau25_mediumGNTau_probe_L1eTAU20M_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M', 'PROBEeTAU20M'], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau25_mediumGNTau_probe_L1cTAU20M_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M', 'PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau30_mediumGNTau_probe_L1eTAU20_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M', 'PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau30_mediumGNTau_probe_L1cTAU20M_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M', 'PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau30_mediumGNTau_probe_L1cTAU30M_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M', 'PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau35_mediumGNTau_probe_L1cTAU30M_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M', 'PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau40_mediumGNTau_probe_L1cTAU35M_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M', 'PROBEcTAU35M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau50_mediumGNTau_probe_L1cTAU50M_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M', 'PROBEcTAU50M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau60_mediumGNTau_probe_L1eTAU60_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M', 'PROBEeTAU60'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau70_mediumGNTau_probe_L1eTAU70_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M', 'PROBEeTAU70'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau80_mediumGNTau_probe_L1eTAU80_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M', 'PROBEeTAU80'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e26_lhtight_ivarloose_tau160_mediumGNTau_probe_L1eTAU140_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M', 'PROBEeTAU140'], groups=TagAndProbePhIGroup+SingleElectronGroup),

        ChainProp(name='HLT_e28_lhtight_ivarloose_tau20_mediumGNTau_probe_L1eTAU12_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M', 'PROBEeTAU12'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau20_mediumGNTau_probe_L1cTAU20M_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M', 'PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau25_mediumGNTau_probe_L1eTAU20_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M', 'PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau25_mediumGNTau_probe_L1eTAU20M_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M', 'PROBEeTAU20M'], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau25_mediumGNTau_probe_L1cTAU20M_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M', 'PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleElectronGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau30_mediumGNTau_probe_L1eTAU20_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M', 'PROBEeTAU20'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau30_mediumGNTau_probe_L1cTAU20M_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M', 'PROBEcTAU20M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau30_mediumGNTau_probe_L1cTAU30M_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M', 'PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau35_mediumGNTau_probe_L1cTAU30M_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M', 'PROBEcTAU30M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau40_mediumGNTau_probe_L1cTAU35M_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M', 'PROBEcTAU35M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau50_mediumGNTau_probe_L1cTAU50M_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M', 'PROBEcTAU50M'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau60_mediumGNTau_probe_L1eTAU60_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M', 'PROBEeTAU60'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau70_mediumGNTau_probe_L1eTAU70_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M', 'PROBEeTAU70'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau80_mediumGNTau_probe_L1eTAU80_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M', 'PROBEeTAU80'], groups=TagAndProbePhIGroup+SingleElectronGroup),
        ChainProp(name='HLT_e28_lhtight_ivarloose_tau160_mediumGNTau_probe_L1eTAU140_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M', 'PROBEeTAU140'], groups=TagAndProbePhIGroup+SingleElectronGroup),



        # Photon + Tau T&P chains (ATR-28791)
        ChainProp(name='HLT_g140_loose_tau20_mediumGNTau_probe_L1eTAU12_03dRAB_L1eEM26M', l1SeedThresholds=['eEM26M', 'PROBEeTAU12'], groups=TagAndProbePhIGroup+TauPhotonGroup),
        ChainProp(name='HLT_g140_loose_tau20_mediumGNTau_probe_L1eTAU12_03dRAB_L1eEM28M', l1SeedThresholds=['eEM28M', 'PROBEeTAU12'], groups=TagAndProbePhIGroup+TauPhotonGroup),



        # MET + Tau T&P chains (ATR-23507)
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU20_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU20M_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1cTAU20M_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumGNTau_probe_L1cTAU30M_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumGNTau_probe_L1cTAU35M_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_probe_L1cTAU50M_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU50M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumGNTau_probe_L1eTAU60_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU60', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau70_mediumGNTau_probe_L1eTAU70_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU70', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumGNTau_probe_L1eTAU80_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU80', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumGNTau_probe_L1eTAU140_xe65_cell_xe90_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU140', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU20_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU20M_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1cTAU20M_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumGNTau_probe_L1cTAU30M_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumGNTau_probe_L1cTAU35M_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_probe_L1cTAU50M_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU50M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumGNTau_probe_L1eTAU60_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU60', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau70_mediumGNTau_probe_L1eTAU70_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU70', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumGNTau_probe_L1eTAU80_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU80', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumGNTau_probe_L1eTAU140_xe65_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU140', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU20_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU20', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1eTAU20M_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1cTAU20M_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumGNTau_probe_L1cTAU30M_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumGNTau_probe_L1cTAU35M_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_probe_L1cTAU50M_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEcTAU50M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumGNTau_probe_L1eTAU60_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU60', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau70_mediumGNTau_probe_L1eTAU70_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU70', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumGNTau_probe_L1eTAU80_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU80', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumGNTau_probe_L1eTAU140_xe75_cell_xe100_pfopufit_L1jXE100', l1SeedThresholds=['PROBEeTAU140', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1cTAU20M_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumGNTau_probe_L1cTAU30M_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumGNTau_probe_L1cTAU35M_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_probe_L1cTAU50M_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU50M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumGNTau_probe_L1eTAU60_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU60', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau70_mediumGNTau_probe_L1eTAU70_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU70', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumGNTau_probe_L1eTAU80_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU80', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumGNTau_probe_L1eTAU140_xe65_cell_xe90_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU140', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1cTAU20M_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumGNTau_probe_L1cTAU30M_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumGNTau_probe_L1cTAU35M_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_probe_L1cTAU50M_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU50M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumGNTau_probe_L1eTAU60_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU60', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau70_mediumGNTau_probe_L1eTAU70_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU70', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumGNTau_probe_L1eTAU80_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU80', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumGNTau_probe_L1eTAU140_xe65_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU140', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1cTAU20M_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumGNTau_probe_L1cTAU30M_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumGNTau_probe_L1cTAU35M_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_probe_L1cTAU50M_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEcTAU50M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumGNTau_probe_L1eTAU60_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU60', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau70_mediumGNTau_probe_L1eTAU70_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU70', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumGNTau_probe_L1eTAU80_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU80', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumGNTau_probe_L1eTAU140_xe75_cell_xe100_pfopufit_L1gXEJWOJ100', l1SeedThresholds=['PROBEeTAU140', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1cTAU20M_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumGNTau_probe_L1cTAU30M_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumGNTau_probe_L1cTAU35M_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_probe_L1cTAU50M_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU50M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumGNTau_probe_L1eTAU60_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU60', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau70_mediumGNTau_probe_L1eTAU70_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU70', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumGNTau_probe_L1eTAU80_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU80', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumGNTau_probe_L1eTAU140_xe65_cell_xe90_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU140', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1cTAU20M_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumGNTau_probe_L1cTAU30M_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumGNTau_probe_L1cTAU35M_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_probe_L1cTAU50M_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU50M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumGNTau_probe_L1eTAU60_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU60', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau70_mediumGNTau_probe_L1eTAU70_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU70', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumGNTau_probe_L1eTAU80_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU80', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumGNTau_probe_L1eTAU140_xe65_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU140', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1cTAU20M_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumGNTau_probe_L1cTAU30M_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumGNTau_probe_L1cTAU35M_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumGNTau_probe_L1eTAU60_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU60', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumGNTau_probe_L1eTAU80_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU80', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumGNTau_probe_L1eTAU140_xe75_cell_xe100_pfopufit_L1jXE110', l1SeedThresholds=['PROBEeTAU140', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),

        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1cTAU20M_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumGNTau_probe_L1cTAU30M_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumGNTau_probe_L1cTAU35M_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_probe_L1cTAU50M_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU50M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumGNTau_probe_L1eTAU60_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU60', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau70_mediumGNTau_probe_L1eTAU70_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU70', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumGNTau_probe_L1eTAU80_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU80', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumGNTau_probe_L1eTAU140_xe65_cell_xe90_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU140', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1cTAU20M_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumGNTau_probe_L1cTAU30M_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumGNTau_probe_L1cTAU35M_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_probe_L1cTAU50M_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU50M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumGNTau_probe_L1eTAU60_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU60', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau70_mediumGNTau_probe_L1eTAU70_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU70', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumGNTau_probe_L1eTAU80_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU80', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumGNTau_probe_L1eTAU140_xe65_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU140', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        
        ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau25_mediumGNTau_probe_L1cTAU20M_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU20M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau35_mediumGNTau_probe_L1cTAU30M_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU30M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau40_mediumGNTau_probe_L1cTAU35M_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU35M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau50_mediumGNTau_probe_L1cTAU50M_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEcTAU50M', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau60_mediumGNTau_probe_L1eTAU60_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU60', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau70_mediumGNTau_probe_L1eTAU70_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU70', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau80_mediumGNTau_probe_L1eTAU80_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU80', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),
        ChainProp(name='HLT_tau160_mediumGNTau_probe_L1eTAU140_xe75_cell_xe100_pfopufit_L1gXEJWOJ110', l1SeedThresholds=['PROBEeTAU140', 'FSNOSEED', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauMETGroup),



        # Jet + Tau T&P (ATR-24031)
        ChainProp(name='HLT_tau20_mediumGNTau_L1RD0_FILLED', l1SeedThresholds=['eTAU12'], groups=SupportPhIGroup+TauJetGroup+['RATE:CPS_RD0_FILLED']),
        # Commented for now because the corresponding jet trigger is out due to fluctuating counts
        #ChainProp(name='HLT_tau20_mediumGNTau_probe_L1eTAU12_j15_pf_ftf_03dRAB_L1RD0_FILLED', l1SeedThresholds=['PROBEeTAU12', 'FSNOSEED'], groups=TagAndProbePhIGroup+TauJetGroup+['RATE:CPS_RD0_FILLED']),
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
        #----------------------------------------------------------------------------------------------------------------------

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
        ChainProp(name='HLT_0e25_nopid_0g25_loose_0mu24_j20_xe0_tcpufit_anomdetM_L1ADVAET', l1SeedThresholds=['eEM18L','eEM18L','MU3V','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+Topo2Group),
        ChainProp(name='HLT_0e25_nopid_0g25_loose_0mu24_j20_xe0_tcpufit_anomdetT_L1ADVAET', l1SeedThresholds=['eEM18L','eEM18L','MU3V','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+Topo2Group),
        ChainProp(name='HLT_0e25_nopid_0g25_loose_0mu24_j20_xe0_tcpufit_anomdetL_L1ADVAEL', l1SeedThresholds=['eEM18L','eEM18L','MU3V','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+Topo2Group),
        ChainProp(name='HLT_0e25_nopid_0g25_loose_0mu24_j20_xe0_tcpufit_anomdetM_L1ADVAEL', l1SeedThresholds=['eEM18L','eEM18L','MU3V','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+Topo2Group),
        ChainProp(name='HLT_0e25_nopid_0g25_loose_0mu24_j20_xe0_tcpufit_anomdetT_L1ADVAEL', l1SeedThresholds=['eEM18L','eEM18L','MU3V','FSNOSEED','FSNOSEED'], groups=PrimaryPhIGroup+Topo2Group),

    ]

    chains['Beamspot'] = [
    ]

    chains['MinBias'] = [

    ]

    chains['Calib'] = [

        # Calib Chains
        # ChainProp(name='HLT_larpsallem_L1EM3', groups=SingleElectronGroup+SupportLegGroup),
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
        #ChainProp(name='HLT_caloclustermon_L1RD0_FIRSTEMPTY', l1SeedThresholds=['FSNOSEED'], groups=['RATE:Monitoring']), #We don't have RD0_FIRSTEMPTY on the Dev L1.
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
