# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# Dev_pp_run3_v1.py menu for the long shutdown development
#------------------------------------------------------------------------#

# All chains are represented as ChainProp objects in a ChainStore
from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp
from .SignatureDicts import ChainStore

from . import MC_pp_run3_v1 as mc_menu

# this is not the best option, due to flake violation, this list has to be changed when some groups are removed

from .Physics_pp_run3_v1 import (PhysicsStream,
                                                                 SingleMuonGroup,
                                                                 MultiMuonGroup,
                                                                 MultiElectronGroup,
                                                                 METGroup,
                                                                 SingleJetGroup,
                                                                 MultiJetGroup,
                                                                 SingleBjetGroup,
                                                                 MultiBjetGroup,
                                                                 SingleTauGroup,
                                                                 MultiTauGroup,
                                                                 SinglePhotonGroup,
                                                                 MultiPhotonGroup,
                                                                 TauBJetGroup,
                                                                 TauMETGroup,
                                                                 BphysicsGroup,
                                                                 EgammaMETGroup,
                                                                 EgammaJetGroup,
                                                                 MuonJetGroup,
                                                                 MuonMETGroup,
                                                                 MuonTauGroup,
                                                                 MinBiasGroup,
                                                                 PrimaryLegGroup,
                                                                 PrimaryPhIGroup,
                                                                 PrimaryL1MuGroup,
                                                                 SupportGroup,
                                                                 SupportLegGroup,
                                                                 SupportPhIGroup,
                                                                 TagAndProbeLegGroup,
                                                                 TagAndProbePhIGroup,
                                                                 UnconvTrkGroup,
                                                                 METPhaseIStreamersGroup,
                                                                 EOFTLALegGroup,
                                                                 LegacyTopoGroup,
                                                                 Topo2Group,
                                                                 Topo3Group,
                                                                 EOFL1MuGroup,
                                                                 EOFBPhysL1MuGroup,
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
        ChainProp(name='HLT_e5_lhtight_e9_etcut_probe_1invmAB5_L1JPSI-1M5-EM7', l1SeedThresholds=['EM3','PROBEEM7'], groups=DevGroup+MultiElectronGroup+LegacyTopoGroup),
        ChainProp(name='HLT_e5_lhtight_e14_etcut_probe_1invmAB5_L1JPSI-1M5-EM12', l1SeedThresholds=['EM3','PROBEEM12'], groups=DevGroup+MultiElectronGroup+LegacyTopoGroup),
        ChainProp(name='HLT_e9_lhtight_e4_etcut_probe_1invmAB5_L1JPSI-1M5-EM7', l1SeedThresholds=['EM7','PROBEEM3'], groups=DevGroup+MultiElectronGroup+LegacyTopoGroup),
        ChainProp(name='HLT_e14_lhtight_e4_etcut_probe_1invmAB5_L1JPSI-1M5-EM12', l1SeedThresholds=['EM12','PROBEEM3'], groups=DevGroup+MultiElectronGroup+LegacyTopoGroup),

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

        # ATR-29062      
        ChainProp(name='HLT_g15_tight_ringer_L1eEM12L', groups=SinglePhotonGroup+DevGroup, monGroups=['egammaMon:shifter']),

    ]

    chains['MET'] = [

        ChainProp(name='HLT_xe30_cell_L1XE30',       l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_mht_L1XE30',        l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_tcpufit_L1XE30',    l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_trkmht_L1XE30',     l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_pfsum_L1XE30',      l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_pfsum_cssk_L1XE30', l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_pfsum_vssk_L1XE30', l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_pfopufit_L1XE30',   l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_cvfpufit_L1XE30',   l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_mhtpufit_em_L1XE30', l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_mhtpufit_pf_L1XE30', l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),

        ChainProp(name='HLT_xe110_tc_em_L1XE50',      l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe110_mht_L1XE50',        l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe110_tcpufit_L1XE50',    l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe110_pfsum_L1XE50',      l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe110_pfsum_cssk_L1XE50', l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe110_pfsum_vssk_L1XE50', l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),

        # Test chains to determine rate after calo-only preselection for tracking
        ChainProp(name='HLT_xe60_cell_L1XE50', l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),

        # ATR-25509 Triggers needed to test nSigma=3
        ChainProp(name='HLT_xe30_pfopufit_sig30_L1XE30', l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
        ChainProp(name='HLT_xe30_tcpufit_sig30_L1XE30', l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),

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
        # Calratio LLP-NoMATCH analogs
        ChainProp(name='HLT_j30_CLEANllp_momemfrac006_calratio_L1eTAU40HT', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
        ChainProp(name='HLT_j30_CLEANllp_momemfrac006_calratiormbib_L1eTAU40HT', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
        ChainProp(name='HLT_j30_CLEANllp_momemfrac006_calratio_L1eTAU60HM', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
        ChainProp(name='HLT_j30_CLEANllp_momemfrac006_calratiormbib_L1eTAU60HM', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
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
        ChainProp(name='HLT_j70_j50a_j0_DJMASS900j50dphi200x400deta_L1MJJ-500-NFF', l1SeedThresholds=['FSNOSEED']*3,stream=['VBFDelayed'],groups=PrimaryLegGroup+MultiJetGroup+LegacyTopoGroup), # previously HLT_j70_j50_0eta490_invm1000j70_dphi20_deta40_L1MJJ-500-NFF
        ChainProp(name='HLT_j70_j50a_j0_DJMASS1000j50dphi260x200deta_L1MJJ-500-NFF', l1SeedThresholds=['FSNOSEED']*3,stream=['VBFDelayed'],groups=PrimaryLegGroup+MultiJetGroup+LegacyTopoGroup), # previously HLT_j70_j50_0eta490_invm1000j70_dphi26_deta20_L1MJJ-500-NFF
        ChainProp(name='HLT_j70_j50a_j0_DJMASS900j50dphi260x200deta_L1MJJ-500-NFF', l1SeedThresholds=['FSNOSEED']*3,stream=['VBFDelayed'],groups=PrimaryLegGroup+MultiJetGroup+LegacyTopoGroup), # previously HLT_j70_j50_0eta490_invm1000j70_dphi26_deta20_L1MJJ-500-NFF
        ChainProp(name='HLT_j70_j50a_j0_DJMASS1000j50dphi260_L1MJJ-500-NFF', l1SeedThresholds=['FSNOSEED']*3,stream=['VBFDelayed'],groups=PrimaryLegGroup+MultiJetGroup+LegacyTopoGroup), # previously HLT_j70_j50_0eta490_invm1000j70_dphi26_L1MJJ-500-NFF
        ChainProp(name='HLT_j70_j50a_j0_DJMASS900j50dphi260_L1MJJ-500-NFF', l1SeedThresholds=['FSNOSEED']*3,stream=['VBFDelayed'],groups=PrimaryLegGroup+MultiJetGroup+LegacyTopoGroup), # previously HLT_j70_j50_0eta490_invm1000j70_dphi26_deta40_L1MJJ-500-NFF
        ChainProp(name='HLT_j70_j50a_j0_DJMASS1000j50x200deta_L1MJJ-500-NFF', l1SeedThresholds=['FSNOSEED']*3,stream=['VBFDelayed'],groups=PrimaryLegGroup+MultiJetGroup+LegacyTopoGroup), # previously HLT_j70_j50_0eta490_invm1000j70_deta20_L1MJJ-500-NFF
        ChainProp(name='HLT_j70_j50a_j0_DJMASS900j50x200deta_L1MJJ-500-NFF', l1SeedThresholds=['FSNOSEED']*3,stream=['VBFDelayed'],groups=PrimaryLegGroup+MultiJetGroup+LegacyTopoGroup), # previously HLT_j70_j50_0eta490_invm1000j70_deta20_L1MJJ-500-NFF
        ChainProp(name='HLT_j50_j30a_j0_DJMASS1000j30dphi260x200deta_L1MJJ-500-NFF', l1SeedThresholds=['FSNOSEED']*3,stream=['VBFDelayed'],groups=PrimaryLegGroup+MultiJetGroup+LegacyTopoGroup), # previously HLT_j50_j30_0eta490_invm1000j70_dphi26_deta20_L1MJJ-500-NFF
        ChainProp(name='HLT_j50_j30a_j0_DJMASS900j30dphi260x200deta_L1MJJ-500-NFF', l1SeedThresholds=['FSNOSEED']*3,stream=['VBFDelayed'],groups=PrimaryLegGroup+MultiJetGroup+LegacyTopoGroup), # previously HLT_j50_j30_0eta490_invm1000j70_dphi26_deta20_L1MJJ-500-NFF       
 
        # ATR-28412 test caloratio with VBF
        ChainProp(name='HLT_j20_calratiovar_j70_j50a_j0_DJMASS900j50dphi260x200deta_roiftf_preselj20emf48_L1MJJ-500-NFF', l1SeedThresholds=['FSNOSEED']*4,stream=['VBFDelayed'],groups=PrimaryLegGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j20_calratiovar82_j70_j50a_j0_DJMASS900j50dphi260x200deta_roiftf_preselj20emf60_L1MJJ-500-NFF', l1SeedThresholds=['FSNOSEED']*4,stream=['VBFDelayed'],groups=PrimaryLegGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j20_calratiovar103_j70_j50a_j0_DJMASS900j50dphi260x200deta_roiftf_preselj20emf48_L1MJJ-500-NFF', l1SeedThresholds=['FSNOSEED']*4,stream=['VBFDelayed'],groups=PrimaryLegGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j20_n041pileuprmn015_j70_j50a_j0_DJMASS900j50dphi260x200deta_L1MJJ-500-NFF', l1SeedThresholds=['FSNOSEED']*4,stream=['VBFDelayed'],groups=PrimaryLegGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j20_calratiovar_j70_j50a_j0_DJMASS900j50dphi260x200deta_roiftf_preselj20emf72_L1MJJ-500-NFF', l1SeedThresholds=['FSNOSEED']*4,stream=['VBFDelayed'],groups=PrimaryLegGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j20_calratiovar59_j70_j50a_j0_DJMASS900j50dphi260x200deta_roiftf_preselj20emf72_L1MJJ-500-NFF', l1SeedThresholds=['FSNOSEED']*4,stream=['VBFDelayed'],groups=PrimaryLegGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_DJMASS900j50dphi260x200deta_calratiovar59_roiftf_preselj20emf72_L1MJJ-500-NFF',l1SeedThresholds=['FSNOSEED'],stream=['Main'],groups=PrimaryLegGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_DJMASS900j50dphi260x200deta_calratiovar82_roiftf_preselj20emf60_L1MJJ-500-NFF',l1SeedThresholds=['FSNOSEED'],stream=['Main'],groups=PrimaryLegGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_DJMASS900j50dphi260x200deta_calratiovar59_roiftf_preselj30emf72_L1MJJ-500-NFF',l1SeedThresholds=['FSNOSEED'],stream=['Main'],groups=PrimaryLegGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j20_CLEANllp_momemfrac072_calratiovar59_roiftf_preselj20emf72_L1MJJ-500-NFF', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
        ChainProp(name='HLT_j20_CLEANllp_momemfrac072_calratiovar82_roiftf_preselj20emf60_L1MJJ-500-NFF', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),

        ChainProp(name='HLT_j20_CLEANllp_momemfrac048_calratiovar103_roiftf_preselj20emf48_L1MJJ-500-NFF', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
        ChainProp(name='HLT_j20_CLEANllp_momemfrac072_calratiovar59_roiftf_preselj20emf72_L1eTAU80', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
        ChainProp(name='HLT_j20_CLEANllp_momemfrac072_calratiovar59_roiftf_preselj20emf72_L1jJ160', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),

        # pflow jet chains without pile-up residual correction for calibration derivations and calibration cross-checks ATR-26827
        ChainProp(name='HLT_j0_perf_pf_subjesgscIS_ftf_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_j25_pf_subjesgscIS_ftf_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),

        # candidate jet TLA chains ATR-20395
        ChainProp(name='HLT_4j25_PhysicsTLA_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=DevGroup+MultiJetGroup), # adding for study of EMTopo TLA fast b-tagging.
        ChainProp(name='HLT_j20_pf_ftf_presel4j85_PhysicsTLA_L13J50', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_j20_pf_ftf_presel5j50_PhysicsTLA_L14J15', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_j20_pf_ftf_presel6j40_PhysicsTLA_L14J15', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=MultiJetGroup+DevGroup),
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
        ChainProp(name='HLT_6j25c_L14J15', l1SeedThresholds=['FSNOSEED'],            groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_6j25c_ftf_L14J15', l1SeedThresholds=['FSNOSEED'],        groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_6j25c_010jvt_ftf_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_6j25c_020jvt_ftf_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_6j25c_050jvt_ftf_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),
        #
        ChainProp(name='HLT_6j25c_pf_ftf_L14J15', l1SeedThresholds=['FSNOSEED'],        groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_6j25c_010jvt_pf_ftf_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_6j25c_020jvt_pf_ftf_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_6j25c_050jvt_pf_ftf_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),

        ### PURE TEST CHAINS

        ChainProp(name='HLT_j0_FBDJNOSHARED10etXX20etXX34massXX50fbet_L1J20', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
        ChainProp(name='HLT_j0_FBDJSHARED_L1J20', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
        ChainProp(name='HLT_j60_j0_FBDJSHARED_L1J20', l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup+DevGroup),

        # Emerging Jets test chains ATR-21593
        # alternate emerging jet single-jet chain
        ChainProp(name='HLT_j175_0eta160_emergingPTF0p08dR1p2_a10sd_cssk_pf_jes_ftf_preselj225_L1J100', groups=SingleJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),

        #PANTELIS Emerging Jets test chains low pT threshold with restricted eta range of 1.4
        ChainProp(name='HLT_j200_0eta140_emergingPTF0p08dR1p2_a10sd_cssk_pf_jes_ftf_preselj160_L1jJ160', groups=SingleJetGroup+PrimaryPhIGroup, l1SeedThresholds=['FSNOSEED']),

        # backup emerging jets chains to be used for rate refinement in enhanced bias reprocessing
        ChainProp(name='HLT_j175_0eta180_emergingPTF0p08dR1p2_a10sd_cssk_pf_jes_ftf_L1J100', groups=SingleJetGroup+PrimaryLegGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_0eta180_emergingPTF0p075dR1p2_a10sd_cssk_pf_jes_ftf_L1J100', groups=SingleJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_0eta160_emergingPTF0p075dR1p2_a10sd_cssk_pf_jes_ftf_L1J100', groups=SingleJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_0eta180_emergingPTF0p07dR1p2_a10sd_cssk_pf_jes_ftf_L1J100', groups=SingleJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_0eta160_emergingPTF0p07dR1p2_a10sd_cssk_pf_jes_ftf_L1J100', groups=SingleJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),

        ChainProp(name='HLT_j175_0eta180_emergingPTF0p075dR1p2_a10sd_cssk_pf_jes_ftf_preselj200_L1J100', groups=SingleJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_0eta160_emergingPTF0p075dR1p2_a10sd_cssk_pf_jes_ftf_preselj200_L1J100', groups=SingleJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_0eta180_emergingPTF0p07dR1p2_a10sd_cssk_pf_jes_ftf_preselj200_L1J100', groups=SingleJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_0eta160_emergingPTF0p07dR1p2_a10sd_cssk_pf_jes_ftf_preselj200_L1J100', groups=SingleJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),

        # end of emerging jets chains

        #####

        # Primary jet chains w/o preselection, for comparison
        ChainProp(name='HLT_2j250c_j120c_pf_ftf_L1J100',    l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup+DevGroup ),
        ChainProp(name='HLT_4j115_pf_ftf_L13J50', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_5j70c_pf_ftf_L14J15', l1SeedThresholds=['FSNOSEED'],  groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_5j85_pf_ftf_L14J15', l1SeedThresholds=['FSNOSEED'],  groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_6j55c_pf_ftf_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_6j70_pf_ftf_L14J15', l1SeedThresholds=['FSNOSEED'],  groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_7j45_pf_ftf_L14J15', l1SeedThresholds=['FSNOSEED'],  groups=MultiJetGroup+DevGroup),
        ChainProp(name='HLT_10j40_pf_ftf_L14J15', l1SeedThresholds=['FSNOSEED'], groups=MultiJetGroup+DevGroup),

        ChainProp(name='HLT_j0_HT1000_pf_ftf_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
        ChainProp(name='HLT_j0_HT1000_pf_ftf_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),

        # CSSKPFlow
        ChainProp(name='HLT_j420_35smcINF_a10sd_cssk_pf_jes_ftf_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
        ChainProp(name='HLT_2j330_35smcINF_a10sd_cssk_pf_jes_ftf_L1J100', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup),
        ChainProp(name='HLT_j420_35smcINF_a10sd_cssk_pf_jes_ftf_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_2j330_35smcINF_a10sd_cssk_pf_jes_ftf_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j360_60smcINF_j360_a10sd_cssk_pf_jes_ftf_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED']*2, groups=DevGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j370_35smcINF_j370_a10sd_cssk_pf_jes_ftf_L1SC111-CJ15', l1SeedThresholds=['FSNOSEED']*2, groups=DevGroup+MultiJetGroup+LegacyTopoGroup),

        ##### End no-preselection

        # ATR-24720 Testing additions to Run 3 baseline menu
        # HT preselection studies
        ChainProp(name='HLT_j0_HT1000_pf_ftf_presel3j45_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000_pf_ftf_presel4j40_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000_pf_ftf_presel4c40_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000XX0eta240XX020jvt_pf_ftf_presel4c40_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000_pf_ftf_presel4j45_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000XX020jvt_pf_ftf_presel4j45_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000_pf_ftf_presel4j50_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000_pf_ftf_presel5j25_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000XX020jvt_pf_ftf_presel5j25_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ###
        ChainProp(name='HLT_j0_HT50_pf_ftf_preseljHT400_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000_pf_ftf_preseljHT400_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000_pf_ftf_preselcHT400_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000XX020jvt_pf_ftf_preseljHT400_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000XX020jvt_pf_ftf_preselcHT400_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000XX0eta240_pf_ftf_preselcHT400_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT50_pf_ftf_preseljHT450_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000_pf_ftf_preseljHT450_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        #ChainProp(name='HLT_j0_HT1000_pf_ftf_preselj180_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000XX020jvt_pf_ftf_preseljHT450_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000XX020jvt_pf_ftf_preselcHT450_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000XX0eta240_pf_ftf_preselcHT450_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT50_pf_ftf_preseljHT500_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000_pf_ftf_preseljHT500_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000XX020jvt_pf_ftf_preseljHT500_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT1000XX020jvt_pf_ftf_preselcHT500_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleJetGroup+DevGroup+LegacyTopoGroup),


         #TLA+PEB test for jets ATR-21596, matching "multijet+PFlow" TLA chain in physics menu for cross-check of event size
        ChainProp(name='HLT_j20_pf_ftf_preselcHT450_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=DevGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j20_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'],  groups=DevGroup+MultiJetGroup+LegacyTopoGroup),
        # HT preseleection tests
        ChainProp(name='HLT_j20_pf_ftf_preselcHT650_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'],  groups=DevGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j20_pf_ftf_preselcHT850_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'],  groups=DevGroup+MultiJetGroup+LegacyTopoGroup),
        # jet preselection
        ChainProp(name='HLT_j20_pf_ftf_preselj180_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'],  groups=DevGroup+MultiJetGroup+LegacyTopoGroup),
        # with HT leg at the HLT
        # + ht preselection
        ChainProp(name='HLT_j0_HT500XX0eta240_pf_ftf_preselcHT450_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=DevGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT850XX0eta240_pf_ftf_preselcHT450_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=DevGroup+MultiJetGroup+LegacyTopoGroup),
        # + jet preselection
        ChainProp(name='HLT_j0_HT850XX0eta240_pf_ftf_preselj180_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=DevGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT650XX0eta240_pf_ftf_preselj180_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=DevGroup+MultiJetGroup+LegacyTopoGroup),
        # no preselection
        ChainProp(name='HLT_j0_HT500XX0eta240_pf_ftf_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'],  groups=DevGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT500XX0eta240_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'],  groups=DevGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT650XX0eta240_pf_ftf_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'],  groups=DevGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT650XX0eta240_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'],  groups=DevGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT850XX0eta240_pf_ftf_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'],  groups=DevGroup+MultiJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_HT850XX0eta240_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'],  groups=DevGroup+MultiJetGroup+LegacyTopoGroup),

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
        ChainProp(name='HLT_j45_320eta490_L1J15p31ETA49', groups=DevGroup+SingleJetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j220_320eta490_L1J75p31ETA49', groups=DevGroup+SingleJetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j420_a10t_lcw_jes_35smcINF_L1SC111-CJ15', groups=DevGroup+SingleJetGroup+LegacyTopoGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_2j330_a10t_lcw_jes_35smcINF_L1J100', groups=DevGroup+MultiJetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_2j330_a10sd_cssk_pf_jes_ftf_35smcINF_presel2j225_L1SC111-CJ15', groups=DevGroup+MultiJetGroup+LegacyTopoGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_2j330_a10sd_cssk_pf_jes_ftf_35smcINF_presel2j225_L1jLJ140', groups=DevGroup+MultiJetGroup, l1SeedThresholds=['FSNOSEED']),
        

#        ChainProp(name='HLT_2j330_a10sd_cssk_pf_jes_ftf_35smcINF_presel2j225_L1gLJ140', groups=DevGroup+MultiJetGroup, l1SeedThresholds=['FSNOSEED']),

        # ATR-28352: Test chains for multijet DIPZ 
        ChainProp(name='HLT_j0_Z120XX4c120_roiftf_presel4j85_L13J50', groups=MultiJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_4j120c_L13J50', groups=MultiJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j0_Z120XX5c70_roiftf_presel5j50_L14J15', groups=MultiJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j0_Z120XX6c55_roiftf_presel6j40_L14J15', groups=MultiJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j0_Z120XX10c40_roiftf_presel7j30_L14J15', groups=MultiJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_10j40c_L14J15', groups=MultiJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_4j110_pf_ftf_preselZ120XX4c85_L13J50', groups=MultiJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_4j110_pf_ftf_preselZ120MAXMULT20cXX4c85_L13J50', groups=MultiJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_4j20c_pf_ftf_preselZ120XX4c20_L1RD0_FILLED', groups=MultiJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_4j20c_pf_ftf_presel4c20_L1RD0_FILLED', groups=MultiJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name="HLT_4j20c_roiftf_presel4c20_L1RD0_FILLED", groups=MultiJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name="HLT_j0_Z120XX4c20_roiftf_presel4c20_L1RD0_FILLED", groups=MultiJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name="HLT_j0_Z120XX4c20_MAXMULT6c_roiftf_presel4c20_L1RD0_FILLED", groups=MultiJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name="HLT_j0_Z120XX4c20_MAXMULT20c_roiftf_presel4c20_L1RD0_FILLED", groups=MultiJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ]


    chains['Bjet'] = [
        
        # Test chain for X to bb tagging
        ChainProp(name='HLT_j110_a10sd_cssk_60bgntwox_pf_jes_ftf_preselj80_L1jJ60', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_60bgntwox_pf_jes_ftf_preselj140_L1jJ90', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_60bgntwox_pf_jes_ftf_preselj140_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_60bgntwox_pf_jes_ftf_preselj140_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_60bgntwox_pf_jes_ftf_preselj140_L1SC111-CjJ40', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_60bgntwox_pf_jes_ftf_preselj200_L1jJ125', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_60bgntwox_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_60bgntwox_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_60bgntwox_pf_jes_ftf_preselj200_L1SC111-CjJ40', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j360_a10sd_cssk_60bgntwox_pf_jes_ftf_preselj225_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j420_a10sd_cssk_60bgntwox_pf_jes_ftf_preselj225_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j460_a10sd_cssk_60bgntwox_pf_jes_ftf_preselj225_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),

        ChainProp(name='HLT_j110_a10sd_cssk_70bgntwox_pf_jes_ftf_preselj80_L1jJ60', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_70bgntwox_pf_jes_ftf_preselj140_L1jJ90', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_70bgntwox_pf_jes_ftf_preselj140_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_a10sd_cssk_70bgntwox_pf_jes_ftf_preselj140_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_70bgntwox_pf_jes_ftf_preselj140_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_a10sd_cssk_70bgntwox_pf_jes_ftf_preselj140_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_70bgntwox_pf_jes_ftf_preselj140_L1SC111-CjJ40', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_35smcINF_a10sd_cssk_70bgntwox_pf_jes_ftf_preselj140_L1SC111-CjJ40', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_70bgntwox_pf_jes_ftf_preselj200_L1jJ125', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_70bgntwox_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_2j260_a10sd_cssk_70bgntwox_pf_jes_ftf_presel2j225_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_70bgntwox_j260_a10sd_cssk_pf_jes_ftf_presel2j225_L1jJ160', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=['FSNOSEED', 'FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_70bgntwox_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_2j260_a10sd_cssk_70bgntwox_pf_jes_ftf_presel2j225_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_70bgntwox_j260_a10sd_cssk_pf_jes_ftf_presel2j225_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=['FSNOSEED', 'FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_70bgntwox_pf_jes_ftf_preselj200_L1SC111-CjJ40', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_2j260_a10sd_cssk_70bgntwox_pf_jes_ftf_presel2j225_L1SC111-CjJ40', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_70bgntwox_j260_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC111-CjJ40', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=['FSNOSEED', 'FSNOSEED']),
        ChainProp(name='HLT_j360_a10sd_cssk_70bgntwox_pf_jes_ftf_preselj225_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j420_a10sd_cssk_70bgntwox_pf_jes_ftf_preselj225_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j460_a10sd_cssk_70bgntwox_pf_jes_ftf_preselj225_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),

        ChainProp(name='HLT_j110_a10sd_cssk_80bgntwox_pf_jes_ftf_preselj80_L1jJ60', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_80bgntwox_pf_jes_ftf_preselj140_L1jJ90', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_80bgntwox_pf_jes_ftf_preselj140_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_80bgntwox_pf_jes_ftf_preselj140_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_80bgntwox_pf_jes_ftf_preselj140_L1SC111-CjJ40', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_80bgntwox_pf_jes_ftf_preselj200_L1jJ125', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_80bgntwox_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_80bgntwox_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_80bgntwox_pf_jes_ftf_preselj200_L1SC111-CjJ40', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j360_a10sd_cssk_80bgntwox_pf_jes_ftf_preselj225_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j420_a10sd_cssk_80bgntwox_pf_jes_ftf_preselj225_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j460_a10sd_cssk_80bgntwox_pf_jes_ftf_preselj225_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),

        ChainProp(name='HLT_j110_a10sd_cssk_90bgntwox_pf_jes_ftf_preselj80_L1jJ60', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_90bgntwox_pf_jes_ftf_preselj140_L1jJ90', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_90bgntwox_pf_jes_ftf_preselj140_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_90bgntwox_pf_jes_ftf_preselj140_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j175_a10sd_cssk_90bgntwox_pf_jes_ftf_preselj140_L1SC111-CjJ40', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_90bgntwox_pf_jes_ftf_preselj200_L1jJ125', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_90bgntwox_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_90bgntwox_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_a10sd_cssk_90bgntwox_pf_jes_ftf_preselj200_L1SC111-CjJ40', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j360_a10sd_cssk_90bgntwox_pf_jes_ftf_preselj225_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j420_a10sd_cssk_90bgntwox_pf_jes_ftf_preselj225_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j460_a10sd_cssk_90bgntwox_pf_jes_ftf_preselj225_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),

        ######################################################################################################################################################################################################################################################
        #HH->bbbb
        ChainProp(name="HLT_j55c_020jvt_j40c_020jvt_j20c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b85_L1MU8F_2J15_J20", l1SeedThresholds=['FSNOSEED']*5, groups=MultiBjetGroup + DevGroup),
        ChainProp(name="HLT_j40c_020jvt_j40c_020jvt_j20c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b85_L1MU8F_2J15_J20", l1SeedThresholds=['FSNOSEED']*5, groups=MultiBjetGroup + DevGroup),
        ChainProp(name="HLT_j55c_020jvt_j28c_020jvt_j20c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b85_L1MU8F_2J15_J20", l1SeedThresholds=['FSNOSEED']*5, groups=MultiBjetGroup + DevGroup),
        ChainProp(name="HLT_j40c_020jvt_j28c_020jvt_j20c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b85_L1MU8F_2J15_J20", l1SeedThresholds=['FSNOSEED']*5, groups=MultiBjetGroup + DevGroup),
        ChainProp(name="HLT_j75c_020jvt_j50c_020jvt_j20c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b85_L1MU8F_2J15_J20", l1SeedThresholds=['FSNOSEED']*5, groups=MultiBjetGroup + DevGroup),
        ChainProp(name="HLT_j75c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b85_L1MU8F_2J15_J20", l1SeedThresholds=['FSNOSEED']*5, groups=MultiBjetGroup + DevGroup),
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
        ChainProp(name='HLT_j20_0eta290_boffperf_pf_ftf_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=SingleBjetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_4j20_0eta290_boffperf_pf_ftf_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=MultiBjetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_4j20_020jvt_boffperf_pf_ftf_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], groups=MultiBjetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_3j20_020jvt_j20_0eta290_boffperf_pf_ftf_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED']*2, groups=MultiBjetGroup+DevGroup+LegacyTopoGroup),
        ChainProp(name='HLT_4j20_020jvt_boffperf_pf_ftf_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED'], groups=MultiBjetGroup+DevGroup),
        
        # ATR-29016: EOF TLA chains aiming for H->cc+ISRJet signature
        ChainProp(name='HLT_3j20c_pf_ftf_presel3c30_PhysicsTLA_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=EOFTLALegGroup+MultiJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_3j20c_pf_ftf_presel3c40_PhysicsTLA_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=EOFTLALegGroup+MultiJetGroup, monGroups=['tlaMon:shifter']),
        ChainProp(name='HLT_3j20c_pf_ftf_presel3c45_PhysicsTLA_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=EOFTLALegGroup+MultiJetGroup, monGroups=['tlaMon:shifter']),
            ## Without pf_ftf part 
        ChainProp(name='HLT_j0_roiftf_presel3c30_L1jJ85p0ETA21_3jJ40p0ETA25', groups=MultiJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j0_roiftf_presel3c40_L1jJ85p0ETA21_3jJ40p0ETA25', groups=MultiJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j0_roiftf_presel3c45_L1jJ85p0ETA21_3jJ40p0ETA25', groups=MultiJetGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),

        # TLA btag ATR-23002
        ## dijet btag TLA
        ChainProp(name='HLT_j20_0eta290_boffperf_pf_ftf_preselj140_PhysicsTLA_L1J50', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=EOFTLALegGroup+SingleBjetGroup),
        ChainProp(name='HLT_j20_0eta290_boffperf_pf_ftf_preselj140_PhysicsTLA_L1J50_DETA20-J50J', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=EOFTLALegGroup+SingleBjetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j20_0eta290_boffperf_pf_ftf_preselj180_PhysicsTLA_L1J100', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=SingleBjetGroup+DevGroup),
        ## multijet btag TLA - HT190
        ChainProp(name='HLT_j20_0eta290_boffperf_pf_ftf_preselcHT450_PhysicsTLA_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], stream=['TLA'], groups=MultiBjetGroup+DevGroup+LegacyTopoGroup),
        # multijet btag TLA - MultiJet L1
        ChainProp(name='HLT_j140_j20_0eta290_boffperf_pf_ftf_preselj140XXj45_PhysicsTLA_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*2, stream=['TLA'], groups=MultiBjetGroup+DevGroup),
        ## with calo fast-tag presel
        ChainProp(name='HLT_2j20_2j20_0eta290_boffperf_pf_ftf_presel2j25XX2j25b85_PhysicsTLA_L14jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*2, stream=['TLA'], groups=MultiBjetGroup+DevGroup),
        ChainProp(name='HLT_2j20_2j20_0eta290_boffperf_pf_ftf_presel2c20XX2c20b85_PhysicsTLA_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED']*2, stream=['TLA'], groups=MultiBjetGroup+DevGroup),

        #ATR-28870 WorkingPoint scan of fastDIPS 85 + GN1 
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn165_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn170_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn172_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn175_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn180_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn182_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn185_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, groups=DevGroup+MultiBjetGroup),


        #ATR-28870 WorkingPoint scan of fastGN2 85 + GN2
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn265_pf_ftf_presel2c20XX2c20bgtwo85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn270_pf_ftf_presel2c20XX2c20bgtwo85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn272_pf_ftf_presel2c20XX2c20bgtwo85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn275_pf_ftf_presel2c20XX2c20bgtwo85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn277_pf_ftf_presel2c20XX2c20bgtwo85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn280_pf_ftf_presel2c20XX2c20bgtwo85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn282_pf_ftf_presel2c20XX2c20bgtwo85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, groups=DevGroup+MultiBjetGroup),
        ChainProp(name='HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn285_pf_ftf_presel2c20XX2c20bgtwo85_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED']*5, groups=DevGroup+MultiBjetGroup),
       
    ]

    chains['Tau'] = [
        ChainProp(name="HLT_tau25_looseRNN_tracktwoMVA_L1TAU12IM", groups=SingleTauGroup),
        ChainProp(name="HLT_tau25_looseRNN_tracktwoLLP_L1TAU12IM", groups=SingleTauGroup),
        ChainProp(name="HLT_tau25_tightRNN_tracktwoMVA_L1TAU12IM", groups=SingleTauGroup),
        ChainProp(name="HLT_tau25_tightRNN_tracktwoLLP_L1TAU12IM", groups=SingleTauGroup),
        ChainProp(name="HLT_tau35_looseRNN_tracktwoMVA_L1TAU20IM", groups=SingleTauGroup),
        ChainProp(name="HLT_tau35_tightRNN_tracktwoMVA_L1TAU20IM", groups=SingleTauGroup),
        ChainProp(name="HLT_tau160_ptonly_L1eTAU140", groups=SingleTauGroup+SupportPhIGroup),
        ChainProp(name="HLT_tau0_mediumRNN_tracktwoMVA_tau0_mediumRNN_tracktwoMVA_03dRAB_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55" , l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup+Topo2Group), 
        ChainProp(name="HLT_tau0_mediumRNN_tracktwoMVA_tau0_mediumRNN_tracktwoMVA_03dRAB_L1TAU20IM_2TAU12IM_4J12p0ETA25",l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+DevGroup),
        #New DiTau Boosted chain Loose
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_02dRAB10_L1eTAU20_DR-eTAU20eTAU12-jJ40', l1SeedThresholds=['eTAU20','eTAU12'], groups=MultiTauGroup+SupportPhIGroup+Topo2Group),
        ChainProp(name='HLT_tau25_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_02dRAB10_L1eTAU20_DR-eTAU20eTAU12-jJ30', l1SeedThresholds=['eTAU20','eTAU12'], groups=MultiTauGroup+SupportPhIGroup+Topo2Group),


        # Asymmetric Tau triggers for HH->bbtautau ATR-22230
        ChainProp(name="HLT_tau25_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup+Topo2Group), 
        ChainProp(name="HLT_tau35_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup+Topo2Group), 
        ChainProp(name="HLT_tau40_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup+Topo2Group), 
        ChainProp(name="HLT_tau25_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup+Topo2Group), 
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup+Topo2Group), 
        # ATR-26852: New Asymmetric ditau triggers for HH->bbtautau
        ChainProp(name="HLT_tau30_idperf_tracktwoMVA_tau20_idperf_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup+Topo2Group),
        ChainProp(name="HLT_tau30_idperf_tracktwoMVA_tau20_idperf_tracktwoMVA_03dRAB_L1TAU20IM_2TAU12IM_4J12p0ETA25", l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+DevGroup+LegacyTopoGroup),
        #ATR-27121
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L1jJ85p0ETA21_3jJ40p0ETA25", l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+DevGroup+LegacyTopoGroup),#gianipez
        #ATR-27132
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L12cTAU20M_4DR28-eTAU30eTAU20-jJ55" , l1SeedThresholds=['cTAU20M','cTAU20M'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L12cTAU20M_4DR32-eTAU30eTAU20-jJ55" , l1SeedThresholds=['cTAU20M','cTAU20M'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L12cTAU20M_10DR32-eTAU30eTAU20-jJ55", l1SeedThresholds=['cTAU20M','cTAU20M'], groups=MultiTauGroup+DevGroup), 

        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L14jJ30p0ETA24_0DETA24_4DPHI99-eTAU30eTAU20" , l1SeedThresholds=['eTAU20','eTAU12'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L14jJ30p0ETA24_0DETA24_4DPHI99-eTAU30eTAU12" , l1SeedThresholds=['eTAU20','eTAU12'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L14jJ30p0ETA24_0DETA24_10DPHI99-eTAU30eTAU12", l1SeedThresholds=['eTAU20','eTAU12'], groups=MultiTauGroup+DevGroup), 
        #ATR-27132
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L1cTAU20M_cTAU12M_4jJ30p0ETA24_0DETA24_4DPHI99-eTAU30eTAU20" , l1SeedThresholds=['cTAU20M','cTAU12M'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L1cTAU20M_cTAU12M_4jJ30p0ETA24_0DETA24_4DPHI99-eTAU30eTAU12" , l1SeedThresholds=['cTAU20M','cTAU12M'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L1cTAU20M_cTAU12M_4jJ30p0ETA24_0DETA24_10DPHI99-eTAU30eTAU12", l1SeedThresholds=['cTAU20M','cTAU12M'], groups=MultiTauGroup+DevGroup), 
        #
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_L1jJ85p0ETA21_3jJ40p0ETA25", l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+DevGroup),#gianipez
        #ATR-27132
        ChainProp(name="HLT_tau0_mediumRNN_tracktwoMVA_tau0_mediumRNN_tracktwoMVA_03dRAB_L12cTAU20M_4DR28-eTAU30eTAU20-jJ55" , l1SeedThresholds=['cTAU20M','cTAU20M'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau0_mediumRNN_tracktwoMVA_tau0_mediumRNN_tracktwoMVA_03dRAB_L12cTAU20M_4DR32-eTAU30eTAU20-jJ55" , l1SeedThresholds=['cTAU20M','cTAU20M'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau0_mediumRNN_tracktwoMVA_tau0_mediumRNN_tracktwoMVA_03dRAB_L12cTAU20M_10DR32-eTAU30eTAU20-jJ55", l1SeedThresholds=['cTAU20M','cTAU20M'], groups=MultiTauGroup+DevGroup), 

        ChainProp(name="HLT_tau0_mediumRNN_tracktwoMVA_tau0_mediumRNN_tracktwoMVA_03dRAB_L14jJ30p0ETA24_0DETA24-eTAU30eTAU12"         , l1SeedThresholds=['eTAU20','eTAU12'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau0_mediumRNN_tracktwoMVA_tau0_mediumRNN_tracktwoMVA_03dRAB_L14jJ30p0ETA24_0DETA24_4DPHI99-eTAU30eTAU20" , l1SeedThresholds=['eTAU20','eTAU20'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau0_mediumRNN_tracktwoMVA_tau0_mediumRNN_tracktwoMVA_03dRAB_L14jJ30p0ETA24_0DETA24_4DPHI99-eTAU30eTAU12" , l1SeedThresholds=['eTAU20','eTAU12'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau0_mediumRNN_tracktwoMVA_tau0_mediumRNN_tracktwoMVA_03dRAB_L14jJ30p0ETA24_0DETA24_10DPHI99-eTAU30eTAU12", l1SeedThresholds=['eTAU20','eTAU12'], groups=MultiTauGroup+DevGroup), 
      

        # eta L1
        ChainProp(name="HLT_tau25_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L1TAU20IM_2TAU12IM_4J12p0ETA25", l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+DevGroup),
        ChainProp(name="HLT_tau35_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L1TAU20IM_2TAU12IM_4J12p0ETA25", l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+DevGroup),
        ChainProp(name="HLT_tau40_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L1TAU20IM_2TAU12IM_4J12p0ETA25", l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+DevGroup),
        ChainProp(name="HLT_tau25_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB_L1TAU20IM_2TAU12IM_4J12p0ETA25", l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+DevGroup),
        ChainProp(name="HLT_tau30_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB_L1TAU20IM_2TAU12IM_4J12p0ETA25", l1SeedThresholds=['TAU20IM','TAU12IM'], groups=MultiTauGroup+DevGroup),

        # ---- jTAU and eTAU seeded chains to investigate cTAU performance
        ChainProp(name="HLT_tau25_idperf_tracktwoMVA_L1jTAU20",   groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau25_perf_tracktwoMVA_L1jTAU20",     groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau25_mediumRNN_tracktwoMVA_L1jTAU20",   groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),

        ChainProp(name="HLT_tau35_idperf_tracktwoMVA_L1jTAU30",   groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau35_perf_tracktwoMVA_L1jTAU30",     groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau35_mediumRNN_tracktwoMVA_L1jTAU30",   groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),

        ChainProp(name="HLT_tau35_idperf_tracktwoMVA_L1jTAU30M",   groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau35_perf_tracktwoMVA_L1jTAU30M",     groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau35_mediumRNN_tracktwoMVA_L1jTAU30M",   groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),

        ChainProp(name="HLT_tau35_idperf_tracktwoMVA_L1eTAU30",   groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau35_perf_tracktwoMVA_L1eTAU30",   groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau35_mediumRNN_tracktwoMVA_L1eTAU30",   groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),

        # LRT tau dev ATR-23787
        # [ATR-26377] Remove NoHLTRepro groups in order to include chains in reprocessings to get cost/rate information
        ChainProp(name="HLT_tau25_looseRNN_trackLRT_L1TAU12IM", groups=DevGroup),
        ChainProp(name="HLT_tau25_mediumRNN_trackLRT_L1TAU12IM", groups=DevGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau25_tightRNN_trackLRT_L1TAU12IM", groups=DevGroup),
        ChainProp(name="HLT_tau25_mediumRNN_trackLRT_L1cTAU20M", groups=DevGroup),
        ChainProp(name="HLT_tau25_mediumRNN_trackLRT_L1eTAU20", groups=DevGroup),
        ChainProp(name="HLT_tau80_mediumRNN_trackLRT_L1eTAU80", groups=DevGroup),
        ChainProp(name="HLT_tau160_mediumRNN_trackLRT_L1eTAU140", groups=DevGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau25_idperf_trackLRT_L1TAU12IM", groups=DevGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau25_idperf_tracktwoLLP_L1TAU12IM", groups=DevGroup),
        ChainProp(name="HLT_tau80_idperf_trackLRT_L1eTAU80", groups=DevGroup),
        ChainProp(name="HLT_tau160_idperf_trackLRT_L1eTAU140", groups=DevGroup, monGroups=['tauMon:t0']), 

        
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
        ChainProp(name='HLT_mu6_msonly_j70_j50a_j0_DJMASS1000j50dphi260x200deta_L1MJJ-500-NFF', l1SeedThresholds=['MU5VF','FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream],groups=PrimaryLegGroup+MuonJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_mu6_msonly_j70_j50a_j0_DJMASS900j50dphi260x200deta_L1MJJ-500-NFF', l1SeedThresholds=['MU5VF','FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream],groups=PrimaryLegGroup+MuonJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_2mu6noL1_msonly_nscan_j70_j50a_j0_DJMASS900j50dphi260x200deta_L1MJJ-500-NFF', l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream],groups=PrimaryLegGroup+MuonJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_2mu6noL1_msonly_nscan_j70_j50a_j0_DJMASS1000j50dphi260x200deta_L1MJJ-500-NFF', l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream],groups=PrimaryLegGroup+MuonJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_mu6_msonly_j50_j30a_j0_DJMASS1000j30dphi260x200deta_L1MJJ-500-NFF', l1SeedThresholds=['MU5VF','FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream],groups=PrimaryLegGroup+MuonJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_mu6_msonly_j50_j30a_j0_DJMASS900j30dphi260x200deta_L1MJJ-500-NFF', l1SeedThresholds=['MU5VF','FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream],groups=PrimaryLegGroup+MuonJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_2mu6noL1_msonly_nscan_j50_j30a_j0_DJMASS900j30dphi260x200deta_L1MJJ-500-NFF', l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream],groups=PrimaryLegGroup+MuonJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_2mu6noL1_msonly_nscan_j50_j30a_j0_DJMASS1000j30dphi260x200deta_L1MJJ-500-NFF', l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream],groups=PrimaryLegGroup+MuonJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_mu20_msonly_j70_j50a_j0_DJMASS1000j50dphi260x200deta_L1MJJ-500-NFF', l1SeedThresholds=['MU5VF','FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream],groups=PrimaryLegGroup+MuonJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_mu20_msonly_j70_j50a_j0_DJMASS900j50dphi260x200deta_L1MJJ-500-NFF', l1SeedThresholds=['MU5VF','FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream],groups=PrimaryLegGroup+MuonJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_mu6noL1_msonly_j70_j50a_j0_DJMASS1000j50dphi260x200deta_L1MJJ-500-NFF', l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream],groups=PrimaryLegGroup+MuonJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_mu6noL1_msonly_j70_j50a_j0_DJMASS900j50dphi260x200deta_L1MJJ-500-NFF', l1SeedThresholds=['FSNOSEED','FSNOSEED','FSNOSEED','FSNOSEED'], stream=[PhysicsStream],groups=PrimaryLegGroup+MuonJetGroup+LegacyTopoGroup),


        # Test chains for muon + jet/MET merging/aligning
        ChainProp(name='HLT_mu6_xe30_mht_L1XE30', l1SeedThresholds=['MU5VF','FSNOSEED'], stream=[PhysicsStream], groups=MuonMETGroup),
        ChainProp(name='HLT_mu6_j45_nojcalib_L1J20', l1SeedThresholds=['MU5VF','FSNOSEED'], stream=[PhysicsStream], groups=MuonJetGroup),

        # mu-tag & tau-probe triggers for LLP (ATR-23150)
        ChainProp(name='HLT_mu26_ivarmedium_tau100_mediumRNN_tracktwoLLP_probe_L1eTAU80_03dRAB_L1MU14FCH', l1SeedThresholds=['MU14FCH','PROBEeTAU80'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleMuonGroup),
        # ChainProp(name='HLT_e26_lhtight_ivarloose_tau100_mediumRNN_tracktwoLLP_probe_L1eTAU80_03dRAB_L1EM22VHI', l1SeedThresholds=['EM22VHI','PROBEeTAU80'], stream=[PhysicsStream], groups=TagAndProbeLegGroup+SingleElectronGroup),

        # tau + jet and tau + photon tag and probe (ATR-24031)
        # *** Temporarily commented because counts are fluctuating in CI and causing confusion ***
        #ChainProp(name='HLT_tau20_mediumRNN_tracktwoMVA_probe_j15_pf_ftf_03dRAB_L1RD0_FILLED', l1SeedThresholds=['PROBETAU8','FSNOSEED'], groups=TagAndProbeLegGroup+TauJetGroup),
        # *** Temporarily commented because counts are fluctuating in CI and causing confusion ***


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
        ChainProp(name='HLT_noalg_AFPPEB_L1J100', l1SeedThresholds=['FSNOSEED'], stream=['AFPPEB'], groups=['PS:NoBulkMCProd']+MinBiasGroup),

        # ATR-21596 
        # Muon+HT Test chains for PEB
        ChainProp(name='HLT_mu6_probe_j20_pf_ftf_DarkJetPEBTLA_L1HT190-J15s5pETA21', l1SeedThresholds=['PROBEMU5VF','FSNOSEED'], stream=['DarkJetPEBTLA'], groups=DevGroup+MuonJetGroup+LegacyTopoGroup),
        ChainProp(name='HLT_mu10_probe_j20_pf_ftf_DarkJetPEBTLA_L1HT190-J15s5pETA21', l1SeedThresholds=['PROBEMU8F','FSNOSEED'], stream=['DarkJetPEBTLA'], groups=DevGroup+MuonJetGroup+LegacyTopoGroup),

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
        ChainProp(name='HLT_mu14_ivarloose_tau35_mediumRNN_tracktwoMVA_03dRAB_L1cTAU30M_3DR99-MU8F-eTAU30', l1SeedThresholds=['MU8F','cTAU30M'], stream=[PhysicsStream], groups=SupportPhIGroup+MuonTauGroup),
        ChainProp(name='HLT_mu14_ivarloose_tau35_mediumRNN_tracktwoMVA_03dRAB_L1cTAU30M_3DR35-MU8F-eTAU30', l1SeedThresholds=['MU8F','cTAU30M'], stream=[PhysicsStream], groups=SupportPhIGroup+MuonTauGroup),
        ChainProp(name='HLT_mu14_ivarloose_tau35_mediumRNN_tracktwoMVA_03dRAB_L1cTAU30M_3DR30-MU8F-eTAU30', l1SeedThresholds=['MU8F','cTAU30M'], stream=[PhysicsStream], groups=SupportPhIGroup+MuonTauGroup),
        ChainProp(name='HLT_mu14_ivarloose_tau35_mediumRNN_tracktwoMVA_03dRAB_L1cTAU30M_3DR28-MU8F-eTAU30', l1SeedThresholds=['MU8F','cTAU30M'], stream=[PhysicsStream], groups=SupportPhIGroup+MuonTauGroup),

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
    ]

    chains['UnconventionalTracking'] = [
        #Isolated High Pt Trigger Test chain for optimisation studies
        ChainProp(name='HLT_isotrk50_L1XE50', groups=UnconvTrkGroup+DevGroup, l1SeedThresholds=['FSNOSEED']),


        ChainProp(name='HLT_fslrt0_L1J100', groups=DevGroup+['PS:NoHLTRepro'], l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_fslrt0_L14J15', groups=DevGroup+['PS:NoHLTRepro'], l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_fslrt0_L1XE50', groups=DevGroup+['PS:NoHLTRepro'], l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_fslrt0_L1All',  groups=DevGroup+['PS:NoHLTRepro'], l1SeedThresholds=['FSNOSEED']),
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
