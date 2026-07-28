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
                                 MultiTauGroup,
                                 MultiPhotonGroup,
                                 TauBJetGroup,
                                 BphysicsGroup,
                                 EgammaMETGroup,
                                 EgammaJetGroup,
                                 MinBiasGroup,
                                 SupportGroup,
                                 SupportLegGroup,
                                 SupportPhIGroup,
                                 METPhaseIStreamersGroup,
                                 EOFTLALegGroup,
                                 Topo2Group,
                                 Topo3Group,
                                 EOFBPhysL1MuGroup,
                                 SingleElectronGroup,
                                 MultiElectronGroup
                                 )

# Some of the group names are modified for MC and Dev, see the MC menu or ATR-30593 for more info.
from .MC_pp_run3_v1 import (PrimaryLegGroup,
                            PrimaryPhIGroup,
                            TagAndProbeLegGroup,
                            TagAndProbePhIGroup,
                            )

DevGroup = ['Development']

def getDevSignatures():
    chains = ChainStore()
    chains['Muon'] = [

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
    ]

    chains['Egamma'] = [

        # ATR-23625
        ChainProp(name='HLT_g50_medium_g20_medium_L12eEM18M', l1SeedThresholds=['eEM18M','eEM18M'], groups=SupportPhIGroup+MultiPhotonGroup),
        ChainProp(name='HLT_g50_medium_g20_medium_L12eEM18L', l1SeedThresholds=['eEM18L','eEM18L'], groups=SupportPhIGroup+MultiPhotonGroup),

        # ATR-31757: test for DNN chains
        ChainProp(name='HLT_2e17_dnnloose_L12eEM18M', groups=PrimaryPhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_2e24_dnnloose_L12eEM24L', groups=PrimaryPhIGroup+MultiElectronGroup),
        ChainProp(name='HLT_e5_dnntight_e9_etcut_1invmAB5_L1JPSI-1M5-eEM9', stream=[PhysicsStream], l1SeedThresholds=['eEM5','eEM9'], groups=SupportPhIGroup+MultiElectronGroup+Topo2Group+['RATE:CPS_JPSI-1M5-eEM9'], monGroups=['egammaMon:shifter_topo']),
        ChainProp(name='HLT_e24_dnnloose_2e12_dnnloose_L1eEM24L_3eEM12L',l1SeedThresholds=['eEM24L','eEM12L'], groups=PrimaryPhIGroup+MultiElectronGroup), 
        ChainProp(name='HLT_e60_dnnmedium_L1eEM26M', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),
        ChainProp(name='HLT_e140_dnnloose_L1eEM26M', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:t0_tp']),
        ChainProp(name='HLT_e26_dnntight_ivarloose_L1eEM26M', groups=PrimaryPhIGroup+SingleElectronGroup, monGroups=['egammaMon:online','egammaMon:t0_tp']),

        # ATR-32380: test for electron calibrated chains
        ChainProp(name='HLT_e26_lhtight_calibringer_ivarloose_L1eEM26M', stream=[PhysicsStream], groups=SupportPhIGroup+SingleElectronGroup, monGroups=['egammaMon:online','egammaMon:shifter_tp','caloMon:t0']),
        ChainProp(name='HLT_e26_lhtight_calibringer_L1eEM26M', stream=[PhysicsStream], groups=SupportPhIGroup+SingleElectronGroup, monGroups=['egammaMon:online','egammaMon:shifter_tp','caloMon:t0']),
        ChainProp(name='HLT_e60_lhmedium_calibringer_L1eEM26M', stream=[PhysicsStream], groups=SupportPhIGroup+SingleElectronGroup, monGroups=['egammaMon:online','egammaMon:shifter_tp']),
        ChainProp(name='HLT_e140_lhloose_calibringer_L1eEM26M', stream=[PhysicsStream], monGroups=['egammaMon:shifter_tp'], groups=SupportPhIGroup+SingleElectronGroup+['RATE:CPS_eEM26M']),
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

        # ATR-28103 Test chains for delayed jets, based on significance of delay

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

        # Central jet chains for rate plots
        ChainProp(name='HLT_j175C_35smcINF_60bgntwoxt_j260C_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260C_35smcINF_60bgntwoxt_j175C_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),

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

        # Central jet chains for rate plots
        ChainProp(name='HLT_j175C_35smcINF_65bgntwoxt_j260C_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260C_35smcINF_65bgntwoxt_j175C_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),

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

        # Central jet chains for rate plots
        ChainProp(name='HLT_j175C_35smcINF_70bgntwoxt_j260C_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260C_35smcINF_70bgntwoxt_j175C_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),

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

        # Central jet chains for rate plots
        ChainProp(name='HLT_j175C_35smcINF_75bgntwoxt_j260C_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260C_35smcINF_75bgntwoxt_j175C_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),

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

        # Central jet chains for rate plots
        ChainProp(name='HLT_j175C_35smcINF_80bgntwoxt_j260C_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260C_35smcINF_80bgntwoxt_j175C_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),

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

        # Central jet chains for rate plots
        ChainProp(name='HLT_j175C_35smcINF_85bgntwoxt_j260C_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260C_35smcINF_85bgntwoxt_j175C_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),

        # 90% WP
        ChainProp(name='HLT_j210_35smcINF_a10sd_cssk_90bgntwoxt_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j210_35smcINF_a10sd_cssk_90bgntwoxt_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j210_35smcINF_a10sd_cssk_90bgntwoxt_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_a10sd_cssk_90bgntwoxt_pf_jes_ftf_preselj200_L1jJ160', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_a10sd_cssk_90bgntwoxt_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_j260_35smcINF_a10sd_cssk_90bgntwoxt_pf_jes_ftf_preselj200_L1SC175-SCjJ10', groups=DevGroup+SingleBjetGroup, l1SeedThresholds=['FSNOSEED']),

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

        # Central jet chains for rate plots
        ChainProp(name='HLT_j175C_35smcINF_95bgntwoxt_j260C_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ChainProp(name='HLT_j260C_35smcINF_95bgntwoxt_j175C_35smcINF_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25', groups=DevGroup+MultiBjetGroup, l1SeedThresholds=2*['FSNOSEED']),
        ######################################################################################################################################################################################################################################################
        # Tests of potential TLA chains for cost/rate

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
        # HitZ test chains for 2026 (ATR-32384)
        ChainProp(name='HLT_tau20_mediumGNTau_HitZ_L1cTAU20M', groups=SupportPhIGroup+SingleTauGroup+DevGroup, monGroups=['tauMon:online', 'tauMon:t0']),
        ChainProp(name='HLT_tau160_mediumGNTau_HitZ_L1eTAU140', groups=SupportPhIGroup+SingleTauGroup+DevGroup, monGroups=['tauMon:online', 'tauMon:t0']),
        ChainProp(name='HLT_tau35_mediumGNTau_HitZ_tau25_mediumGNTau_HitZ_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55', l1SeedThresholds=['cTAU30M', 'cTAU20M'], groups=SupportPhIGroup+MultiTauGroup+Topo2Group, monGroups=['tauMon:online', 'tauMon:shifter']),
        ChainProp(name='HLT_tau35_mediumGNTau_HitZ_tau25_mediumGNTau_HitZ_03dRAB_L1cTAU30M_2cTAU20M_4jJ30p0ETA25', l1SeedThresholds=['cTAU30M', 'cTAU20M'], groups=SupportPhIGroup+MultiTauGroup+Topo2Group, monGroups=['tauMon:online', 'tauMon:shifter']),
        ChainProp(name='HLT_tau35_mediumGNTau_HitZ_tau25_mediumGNTau_HitZ_03dRAB_L1cTAU30M_2cTAU20M', l1SeedThresholds=['cTAU30M', 'cTAU20M'], groups=SupportPhIGroup+MultiTauGroup+Topo2Group, monGroups=['tauMon:online', 'tauMon:shifter']),

        # Single tau Loose and Tight variations
        ChainProp(name='HLT_tau20_mediumGNTau_L1cTAU20M', groups=SingleTauGroup+DevGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau25_looseRNN_tracktwoLLP_L1cTAU20M', groups=SingleTauGroup+DevGroup),
        ChainProp(name='HLT_tau25_tightRNN_tracktwoLLP_L1cTAU20M', groups=SingleTauGroup+DevGroup),
        ChainProp(name='HLT_tau30_mediumGNTau_L1cTAU30M', groups=SingleTauGroup+DevGroup, monGroups=['tauMon:t0']),

        # TES calibration triggers
        ChainProp(name='HLT_tau160_ptonly_L1eTAU140', groups=SingleTauGroup+SupportPhIGroup),
        ChainProp(name='HLT_tau0_mediumGNTau_tau0_mediumGNTau_03dRAB_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55', l1SeedThresholds=['cTAU30M', 'cTAU20M'], groups=MultiTauGroup+DevGroup+Topo2Group), 
        ChainProp(name='HLT_tau0_mediumGNTau_tau0_mediumGNTau_03dRAB_L1cTAU30M_2cTAU20M_4jJ30p0ETA25', l1SeedThresholds=['cTAU30M', 'cTAU20M'], groups=MultiTauGroup+DevGroup),

        # HH->2b2tau: asymmetric di-tau triggers
        # ATR-22230
        ChainProp(name="HLT_tau25_mediumGNTau_tau20_mediumGNTau_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup+Topo2Group), 
        ChainProp(name="HLT_tau35_mediumGNTau_tau20_mediumGNTau_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup+Topo2Group), 
        ChainProp(name="HLT_tau30_mediumGNTau_tau25_mediumGNTau_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup+Topo2Group), 

        ChainProp(name="HLT_tau25_mediumGNTau_tau20_mediumGNTau_03dRAB_L1cTAU30M_2cTAU20M_4jJ30p0ETA25", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup),
        ChainProp(name="HLT_tau35_mediumGNTau_tau20_mediumGNTau_03dRAB_L1cTAU30M_2cTAU20M_4jJ30p0ETA25", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup),
        ChainProp(name="HLT_tau30_mediumGNTau_tau25_mediumGNTau_03dRAB_L1cTAU30M_2cTAU20M_4jJ30p0ETA25", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup),

        # ATR-26852
        ChainProp(name="HLT_tau30_idperf_tracktwoMVA_tau20_idperf_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup+Topo2Group),
        ChainProp(name="HLT_tau30_idperf_tracktwoMVA_tau20_idperf_tracktwoMVA_03dRAB_L1cTAU30M_2cTAU20M_4jJ30p0ETA25", l1SeedThresholds=['cTAU30M','cTAU20M'], groups=MultiTauGroup+DevGroup+Topo2Group),

        # ATR-27121, ATR-27132
        ChainProp(name="HLT_tau30_mediumGNTau_tau20_mediumGNTau_03dRAB_L1cTAU20M_cTAU12M_4jJ30p0ETA24_0DETA24_4DPHI99-eTAU30eTAU20", l1SeedThresholds=['cTAU20M','cTAU12M'], groups=MultiTauGroup+DevGroup), 
        ChainProp(name="HLT_tau30_mediumGNTau_tau20_mediumGNTau_03dRAB_L1cTAU20M_cTAU12M_4jJ30p0ETA24_0DETA24_4DPHI99-eTAU30eTAU12", l1SeedThresholds=['cTAU20M','cTAU12M'], groups=MultiTauGroup+DevGroup), 

        # eTAU-seeded chains to investigate cTAU performance
        ChainProp(name="HLT_tau35_idperf_tracktwoMVA_L1eTAU30",   groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name="HLT_tau35_perf_tracktwoMVA_L1eTAU30",   groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),
        ChainProp(name='HLT_tau35_mediumGNTau_L1eTAU30', groups=SupportPhIGroup+SingleTauGroup, monGroups=['tauMon:t0']),

        # LRT tau chains (ATR-23787)
        ChainProp(name="HLT_tau25_idperf_tracktwoLLP_L1cTAU20M", groups=DevGroup),
        ChainProp(name="HLT_tau25_idperf_trackLRT_L1cTAU20M", groups=DevGroup, monGroups=['tauMon:t0']),
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

        # tau + jet and tau + photon tag and probe (ATR-24031)
        # *** Temporarily commented because counts are fluctuating in CI and causing confusion ***
        ChainProp(name='HLT_g25_loose_xe40_cell_xe50_tcpufit_18dphiAB_18dphiAC_80mTAC_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaMETGroup),
        ChainProp(name='HLT_g25_tight_icalotight_xe40_cell_xe50_tcpufit_L1eEM26M',l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED'],stream=[PhysicsStream], groups=SupportPhIGroup+EgammaMETGroup),

        # ATR-28443, test H to yjj trigger
        ChainProp(name='HLT_g24_tight_icaloloose_j50c_j30c_j24c_03dRAB35_03dRAC35_15dRBC45_50invmBC130_pf_ftf_L1eEM26M', groups=PrimaryLegGroup+EgammaJetGroup, l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream]),
        ChainProp(name='HLT_g24_tight_icaloloose_j40c_j30c_j24c_03dRAB35_03dRAC35_15dRBC45_50invmBC130_pf_ftf_L1eEM26M', groups=PrimaryLegGroup+EgammaJetGroup, l1SeedThresholds=['eEM26M','FSNOSEED','FSNOSEED','FSNOSEED'],stream=[PhysicsStream]),

        # high-mu AFP
        ChainProp(name='HLT_2j20_mb_afprec_afpdijet_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],groups=MinBiasGroup+SupportLegGroup),

        # Test PEB chains for AFP (single/di-lepton-seeded, can be prescaled)
        # ATR-23946
        ChainProp(name='HLT_noalg_AFPPEB_L1MU14FCH', l1SeedThresholds=['FSNOSEED'], stream=['AFPPEB'], groups=['PS:NoBulkMCProd']+MinBiasGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L12MU5VF', l1SeedThresholds=['FSNOSEED'], stream=['AFPPEB'], groups=['PS:NoBulkMCProd']+MinBiasGroup),

        # Maintain consistency with old naming conventions for validation

        ChainProp(name='HLT_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel3c20XX1c20bgtwo85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=5*['FSNOSEED'], stream=['VBFDelayed'], groups=TagAndProbePhIGroup+TauBJetGroup),
        ChainProp(name='HLT_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo85XX1c20gntau90_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=5*['FSNOSEED'], stream=['VBFDelayed'], groups=TagAndProbePhIGroup+TauBJetGroup),
        ChainProp(name='HLT_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo85XX1c20gntau85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=5*['FSNOSEED'], stream=['VBFDelayed'], groups=TagAndProbePhIGroup+TauBJetGroup),
        ChainProp(name='HLT_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo82XX1c20gntau85_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=5*['FSNOSEED'], stream=['VBFDelayed'], groups=TagAndProbePhIGroup+TauBJetGroup),
        ChainProp(name='HLT_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo82XX1c20gntau80_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=5*['FSNOSEED'], stream=['VBFDelayed'], groups=TagAndProbePhIGroup+TauBJetGroup),
        ChainProp(name='HLT_j65c_020jvt_j40c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_j20c_020jvt_bgn285_pf_ftf_presel2c20XX1c20bgtwo80XX1c20gntau80_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=5*['FSNOSEED'], stream=['VBFDelayed'], groups=TagAndProbePhIGroup+TauBJetGroup),

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

        # ATR-31983
        # bbyy triggers
        ChainProp(name='HLT_g50_nopid_g10_nopid_j50c_nnJvtv1_pf_ftf_115masswisoABC135_L1eEM40L_2eEM18L', l1SeedThresholds=['eEM40L', 'eEM18L', 'FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiPhotonGroup+SingleJetGroup),
        ChainProp(name='HLT_g50_nopid_g10_nopid_j50c_nnJvtv1_pf_ftf_115masswisoABC135_L12eEM24L', l1SeedThresholds=['eEM24L','eEM24L', 'FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiPhotonGroup+SingleJetGroup),
        ChainProp(name='HLT_g50_nopid_g10_nopid_j50c_020jvt_pf_ftf_115masswisoABC135_L1eEM40L_2eEM18L', l1SeedThresholds=['eEM40L', 'eEM18L', 'FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiPhotonGroup+SingleJetGroup),
        ChainProp(name='HLT_g50_nopid_g10_nopid_j50c_020jvt_pf_ftf_115masswisoABC135_L12eEM24L', l1SeedThresholds=['eEM24L','eEM24L', 'FSNOSEED'], stream=[PhysicsStream], groups=PrimaryPhIGroup+MultiPhotonGroup+SingleJetGroup),


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
