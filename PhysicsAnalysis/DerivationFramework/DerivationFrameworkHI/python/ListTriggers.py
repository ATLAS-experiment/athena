# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# ListTriggers.py - List of triggers for skimming from athena 21.2 [HION4,HION12] 

#################################################################################
#HION2

def HION2PbPb_2015_5TeV():
    mb_triggers  = []
    mb_triggers += ["HLT_noalg_mb_L1TE50"]
    mb_triggers += ["HLT_mb_sptrk_ion_L1ZDC_A_C_VTE50"]
    
    vn_triggers  = []
    vn_triggers += ["HLT_hi_v23_th14_L1TE50"]
    vn_triggers += ["HLT_hi_v23_th15_L1TE50"]     
    vn_triggers += ["HLT_hi_v2_th10_L1TE50"]
    vn_triggers += ["HLT_hi_v2_th13_L1TE50"]
    vn_triggers += ["HLT_hi_v2_th13_veto3_L1TE50"] 
    vn_triggers += ["HLT_hi_v2_th14_L1TE50"]
    vn_triggers += ["HLT_hi_v2_th15_L1TE50"]
    vn_triggers += ["HLT_hi_v2_th16_L1TE50"]
    vn_triggers += ["HLT_hi_v2_th5_L1TE50"]
    vn_triggers += ["HLT_hi_v3_th10_L1TE50"]
    vn_triggers += ["HLT_hi_v3_th13_L1TE50"]
    vn_triggers += ["HLT_hi_v3_th13_veto2_L1TE50"]
    vn_triggers += ["HLT_hi_v3_th14_L1TE50"]
    vn_triggers += ["HLT_hi_v3_th15_L1TE50"]
    vn_triggers += ["HLT_hi_v3_th16_L1TE50"]
    vn_triggers += ["HLT_hi_v3_th5_L1TE50"]
    
    uc_triggers  = []
    uc_triggers += ["HLT_hi_th1_ucc_L1TE10000"]
    uc_triggers += ["HLT_hi_th2_ucc_L1TE10000"]
    uc_triggers += ["HLT_hi_th3_ucc_L1TE10000"]
    uc_triggers += ["HLT_hi_th1_ucc_L1TE12000"]
    uc_triggers += ["HLT_hi_th2_ucc_L1TE12000"]
    uc_triggers += ["HLT_hi_th3_ucc_L1TE12000"]
    uc_triggers += ["HLT_hi_th1_ucc_L1TE14000"]
    uc_triggers += ["HLT_hi_th2_ucc_L1TE14000"]
    uc_triggers += ["HLT_hi_th3_ucc_L1TE14000"]
    
    triggers=mb_triggers+vn_triggers+uc_triggers
    
    return triggers

def HION2pPb_2016_8TeV():
    vn_triggers  = []
    vn_triggers += ["HLT_hi_v2A_th05p_mb_sp3100_trk160_hmt_L1TE120"]
    vn_triggers += ["HLT_hi_v2A_th05p_mb_sp4100_trk200_hmt_L1TE120"]
    vn_triggers += ["HLT_hi_v2A_th05p_mb_sp4100_trk200_hmt_L1TE160"]
    vn_triggers += ["HLT_hi_v2A_th05p_mb_sp4800_trk240_hmt_L1TE120"]
    vn_triggers += ["HLT_hi_v2A_th05p_mb_sp5000_trk260_hmt_L1TE160"]
    vn_triggers += ["HLT_hi_v2A_th05p_mb_sp4100_pusup900_trk200_hmt_L1TE200"]
    vn_triggers += ["HLT_hi_v2A_th05p_mb_sp4500_pusup1000_trk220_hmt_L1TE200"]
    vn_triggers += ["HLT_hi_v2A_th05p_mb_sp4800_pusup1100_trk240_hmt_L1TE200"]
    vn_triggers += ["HLT_hi_v2A_th05p_mb_sp3100_pusup700_trk160_hmt_L1TE120"]
    vn_triggers += ["HLT_hi_v2A_th05p_mb_sp4100_pusup900_trk200_hmt_L1TE120"]
    vn_triggers += ["HLT_hi_v2A_th05p_mb_sp4100_pusup900_trk200_hmt_L1TE160"]
    vn_triggers += ["HLT_hi_v2A_th05p_mb_sp4800_pusup1100_trk240_hmt_L1TE120"]
    vn_triggers += ["HLT_hi_v2A_th05p_mb_sp5000_pusup1200_trk260_hmt_L1TE160"]
    vn_triggers += ["HLT_hi_v2C_th05p_mb_sp3100_trk160_hmt_L1TE120"]
    vn_triggers += ["HLT_hi_v2C_th05p_mb_sp4100_trk200_hmt_L1TE120"]
    vn_triggers += ["HLT_hi_v2C_th05p_mb_sp4100_trk200_hmt_L1TE160"]
    vn_triggers += ["HLT_hi_v2C_th05p_mb_sp4800_trk240_hmt_L1TE120"]
    vn_triggers += ["HLT_hi_v2C_th05p_mb_sp5000_trk260_hmt_L1TE160"]
    vn_triggers += ["HLT_hi_v2C_th05p_mb_sp4100_pusup900_trk200_hmt_L1TE200"]
    vn_triggers += ["HLT_hi_v2C_th05p_mb_sp4500_pusup1000_trk220_hmt_L1TE200"]
    vn_triggers += ["HLT_hi_v2C_th05p_mb_sp4800_pusup1100_trk240_hmt_L1TE200"]
    vn_triggers += ["HLT_hi_v2C_th05p_mb_sp3100_pusup700_trk160_hmt_L1TE120"]
    vn_triggers += ["HLT_hi_v2C_th05p_mb_sp4100_pusup900_trk200_hmt_L1TE120"]
    vn_triggers += ["HLT_hi_v2C_th05p_mb_sp4100_pusup900_trk200_hmt_L1TE160"]
    vn_triggers += ["HLT_hi_v2C_th05p_mb_sp4800_pusup1100_trk240_hmt_L1TE120"]
    vn_triggers += ["HLT_hi_v2C_th05p_mb_sp5000_pusup1200_trk260_hmt_L1TE160"]
    
    hmt_triggers  = []
    hmt_triggers += ["HLT_mb_sptrk_L1MBTS_1"]
    hmt_triggers += ["HLT_mb_sp2400_pusup500_trk120_hmt_L1TE20"]
    hmt_triggers += ["HLT_mb_sp2800_pusup600_trk140_hmt_L1TE20"]
    hmt_triggers += ["HLT_mb_sp2800_pusup600_trk140_hmt_L1TE50"]
    hmt_triggers += ["HLT_mb_sp3100_pusup700_trk160_hmt_L1TE50"]
    hmt_triggers += ["HLT_mb_sp3100_pusup700_trk160_hmt_L1TE70"]
    hmt_triggers += ["HLT_mb_sp3500_pusup800_trk180_hmt_L1TE70"]
    hmt_triggers += ["HLT_mb_sp3500_pusup800_trk180_hmt_L1TE90"]
    hmt_triggers += ["HLT_mb_sp4100_pusup900_trk200_hmt_L1TE90"]
    hmt_triggers += ["HLT_mb_sp4100_pusup900_trk200_hmt_L1TE120"]
    hmt_triggers += ["HLT_mb_sp4500_pusup1000_trk220_hmt_L1TE120"]
    hmt_triggers += ["HLT_mb_sp4800_pusup1100_trk240_hmt_L1TE160"]
    hmt_triggers += ["HLT_mb_sp5000_pusup1100_trk240_hmt_L1TE200"]
    hmt_triggers += ["HLT_mb_sp5000_pusup1200_trk260_hmt_L1TE160"]
    hmt_triggers += ["HLT_mb_sp5600_pusup1300_trk260_hmt_L1TE200"]
    hmt_triggers += ["HLT_mb_sp5200_pusup1300_trk280_hmt_L1TE160"]
    hmt_triggers += ["HLT_mb_sp6000_pusup1400_trk280_hmt_L1TE200"]
                     
    muon_triggers  = []
    muon_triggers += ["HLT_mu4_mb_sp1200_pusup200_trk60_hmt_L1MU4"]
    muon_triggers += ["HLT_mu4_mb_sp1200_pusup200_trk60_hmt_L1MU4_TE10.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp1200_pusup200_trk60_hmt_L1MU4_TE15.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp1200_pusup200_trk60_hmt_L1MU4_TE20.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp1200_trk60_hmt_L1MU4"]
    muon_triggers += ["HLT_mu4_mb_sp1200_trk60_hmt_L1MU4_TE10.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp1200_trk60_hmt_L1MU4_TE15.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp1200_trk60_hmt_L1MU4_TE20.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp1600_pusup300_trk80_hmt_L1MU4"]
    muon_triggers += ["HLT_mu4_mb_sp1600_pusup300_trk80_hmt_L1MU4_TE15.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp1600_pusup300_trk80_hmt_L1MU4_TE20.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp1600_pusup300_trk80_hmt_L1MU4_TE25.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp1600_trk80_hmt_L1MU4"]
    muon_triggers += ["HLT_mu4_mb_sp1600_trk80_hmt_L1MU4_TE15.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp1600_trk80_hmt_L1MU4_TE20.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp1600_trk80_hmt_L1MU4_TE25.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp2100_pusup400_trk100_hmt_L1MU4"]
    muon_triggers += ["HLT_mu4_mb_sp2100_pusup400_trk100_hmt_L1MU4_TE15.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp2100_pusup400_trk100_hmt_L1MU4_TE20.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp2100_pusup400_trk100_hmt_L1MU4_TE25.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp2100_pusup400_trk100_hmt_L1MU4_TE50"]
    muon_triggers += ["HLT_mu4_mb_sp2100_trk100_hmt_L1MU4"]
    muon_triggers += ["HLT_mu4_mb_sp2100_trk100_hmt_L1MU4_TE15.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp2100_trk100_hmt_L1MU4_TE20.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp2100_trk100_hmt_L1MU4_TE25.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp2100_trk100_hmt_L1MU4_TE50"]
    muon_triggers += ["HLT_mu4_mb_sp2400_pusup500_trk120_hmt_L1MU4"]
    muon_triggers += ["HLT_mu4_mb_sp2400_pusup500_trk120_hmt_L1MU4_TE15.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp2400_pusup500_trk120_hmt_L1MU4_TE20.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp2400_pusup500_trk120_hmt_L1MU4_TE25.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp2400_pusup500_trk120_hmt_L1MU4_TE50"]
    muon_triggers += ["HLT_mu4_mb_sp2400_trk120_hmt_L1MU4"]
    muon_triggers += ["HLT_mu4_mb_sp2400_trk120_hmt_L1MU4_TE15.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp2400_trk120_hmt_L1MU4_TE20.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp2400_trk120_hmt_L1MU4_TE25.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp2400_trk120_hmt_L1MU4_TE50"]
    muon_triggers += ["HLT_mu4_mb_sp2800_pusup600_trk140_hmt_L1MU4_TE50"]
    muon_triggers += ["HLT_mu4_mb_sp2800_pusup600_trk140_hmt_L1MU4_TE70"]
    muon_triggers += ["HLT_mu4_mb_sp2800_pusup600_trk140_hmt_L1MU4_TE90"]
    muon_triggers += ["HLT_mu4_mb_sp2800_trk140_hmt_L1MU4_TE50"]
    muon_triggers += ["HLT_mu4_mb_sp2800_trk140_hmt_L1MU4_TE70"]
    muon_triggers += ["HLT_mu4_mb_sp2800_trk140_hmt_L1MU4_TE90"]
    muon_triggers += ["HLT_mu4_mb_sp3100_pusup700_trk160_hmt_L1MU4_TE20.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp3100_pusup700_trk160_hmt_L1MU4_TE25.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp3100_pusup700_trk160_hmt_L1MU4_TE30.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp3100_pusup700_trk160_hmt_L1MU4_TE50"]
    muon_triggers += ["HLT_mu4_mb_sp3100_pusup700_trk160_hmt_L1MU4_TE70"]
    muon_triggers += ["HLT_mu4_mb_sp3100_trk160_hmt_L1MU4_TE20.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp3100_trk160_hmt_L1MU4_TE25.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp3100_trk160_hmt_L1MU4_TE30.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp3100_trk160_hmt_L1MU4_TE50"]
    muon_triggers += ["HLT_mu4_mb_sp3100_trk160_hmt_L1MU4_TE70"]
    muon_triggers += ["HLT_mu4_mb_sp3500_pusup800_trk180_hmt_L1MU4_TE120"]
    muon_triggers += ["HLT_mu4_mb_sp3500_pusup800_trk180_hmt_L1MU4_TE70"]
    muon_triggers += ["HLT_mu4_mb_sp3500_pusup800_trk180_hmt_L1MU4_TE90"]
    muon_triggers += ["HLT_mu4_mb_sp3500_trk180_hmt_L1MU4_TE120"]
    muon_triggers += ["HLT_mu4_mb_sp3500_trk180_hmt_L1MU4_TE70"]
    muon_triggers += ["HLT_mu4_mb_sp3500_trk180_hmt_L1MU4_TE90"]
    muon_triggers += ["HLT_mu4_mb_sp4100_pusup900_trk200_hmt_L1MU4_TE120"]
    muon_triggers += ["HLT_mu4_mb_sp4100_pusup900_trk200_hmt_L1MU4_TE160"]
    muon_triggers += ["HLT_mu4_mb_sp4100_pusup900_trk200_hmt_L1MU4_TE90"]
    muon_triggers += ["HLT_mu4_mb_sp4100_trk200_hmt_L1MU4_TE120"]
    muon_triggers += ["HLT_mu4_mb_sp4100_trk200_hmt_L1MU4_TE160"]
    muon_triggers += ["HLT_mu4_mb_sp4100_trk200_hmt_L1MU4_TE90"]
    muon_triggers += ["HLT_mu4_mb_sp4500_pusup1000_trk220_hmt_L1MU4_TE30.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp4500_pusup1000_trk220_hmt_L1MU4_TE35.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp4500_pusup1000_trk220_hmt_L1MU4_TE40.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp4500_trk220_hmt_L1MU4_TE30.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp4500_trk220_hmt_L1MU4_TE35.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp4500_trk220_hmt_L1MU4_TE40.0ETA24"]
    muon_triggers += ["HLT_mu4_mb_sp4800_pusup1100_trk240_hmt_L1MU4_TE120"]
    muon_triggers += ["HLT_mu4_mb_sp4800_pusup1100_trk240_hmt_L1MU4_TE160"]
    muon_triggers += ["HLT_mu4_mb_sp4800_trk240_hmt_L1MU4_TE120"]
    muon_triggers += ["HLT_mu4_mb_sp4800_trk240_hmt_L1MU4_TE160"]
    muon_triggers += ["HLT_mu4"]
    
    triggers=vn_triggers+hmt_triggers+muon_triggers
    
    return triggers

def HION2pPb_2016_5TeV():
    triggers  = []
    triggers += ["HLT_noalg_mb_L1MBTS_1"]
    triggers += ["HLT_mb_sp100_trk60_hmt_L1MBTS_1_1"]
    triggers += ["HLT_mb_sp100_trk80_hmt_L1MBTS_1_1"]
    triggers += ["HLT_mb_sp100_trk100_hmt_L1MBTS_1_1"]
    triggers += ["HLT_mb_sp100_trk110_hmt_L1MBTS_1_1"]
    
    return triggers

def HION2MinBias2023():
    triggers  = []
    triggers += ["HLT_noalg_L1TE600p0ETA49"]
    triggers += ["HLT_noalg_L1TE50_VTE600p0ETA49"]
    triggers += ["HLT_mb_sptrk_pc_L1ZDC_A_C_VTE50"]
    
    return triggers

def HION2MinBias2024():
    triggers  = []
    triggers += ["HLT_noalg_L1jTE600"]
    triggers += ["HLT_noalg_L1jTE50_VjTE600"]
    triggers += ["HLT_mb_sptrk_pc_L1ZDC_A_C_VjTE50"]
    
    return triggers
    
#################################################################################
#HION4

def HION4SkimmingTriggers2015():
    triggers  = ["HLT_mb_sptrk_vetombts2in_L1MU0_VTE50"]
    triggers += ["HLT_noalg_L1MU0_VTE50"]
    triggers += ["HLT_hi_loose_upc_L1ZDC_A_C_VTE50"]
    triggers += ["HLT_mb_sptrk_vetombts2in_L1ZDC_A_C_VTE50"]
    triggers += ["HLT_mb_sptrk_ion_L1ZDC_A_C_VTE50"]
    triggers += ["HLT_noalg_L1ZDC_A_C_VTE50"]
    triggers += ["HLT_hi_gg_upc_L1TE5_VTE200"]

    return triggers

def HION4SkimmingTriggers2016():
    triggers  = ["HLT_e5_etcut_L1EM3_VZDC_A"]
    triggers += ["HLT_e5_etcut_L1EM3_VZDC_C"]
    triggers += ["HLT_e5_lhloose_nod0"]
    triggers += ["HLT_e5_lhloose_L1EM3_VZDC_A"]
    triggers += ["HLT_e5_lhloose_L1EM3_VZDC_C"]
    triggers += ["HLT_e5_lhloose_nod0_L1EM3_VZDC_A"]
    triggers += ["HLT_e5_lhloose_nod0_L1EM3_VZDC_C"]
    triggers += ["HLT_2e5_etcut_L12EM3_VZDC_A"]
    triggers += ["HLT_2e5_etcut_L12EM3_VZDC_C"]
    triggers += ["HLT_2e5_lhloose_L12EM3_VZDC_A"]
    triggers += ["HLT_2e5_lhloose_L12EM3_VZDC_C"]
    triggers += ["HLT_2e5_lhloose_nod0_L12EM3_VZDC_A"]
    triggers += ["HLT_2e5_lhloose_nod0_L12EM3_VZDC_C"]
    triggers += ["HLT_g5_loose_L1VZDC_A"]
    triggers += ["HLT_g5_loose_L1VZDC_C"]
    triggers += ["HLT_g10_loose_L1VZDC_A"]
    triggers += ["HLT_g10_loose_L1VZDC_C"]
    triggers += ["HLT_g5_etcut_L1VZDC_A"]
    triggers += ["HLT_g5_etcut_L1VZDC_C"]
    triggers += ["HLT_g10_etcut_L1VZDC_A"]
    triggers += ["HLT_g10_etcut_L1VZDC_C"]
    triggers += ["HLT_mu4_L1MU4_VTE10"]
    triggers += ["HLT_mu4_L1MU4_VZDC_A"]
    triggers += ["HLT_mu4_L1MU4_VZDC_C"]
    triggers += ["HLT_2mu4_L12MU4_VTE10"]
    triggers += ["HLT_2mu4_L12MU4_VZDC_A"]
    triggers += ["HLT_2mu4_L12MU4_VZDC_C"]
    triggers += ["HLT_mu4_L1MU4_VTE10_VZDC_A"]
    triggers += ["HLT_mu4_L1MU4_VTE10_VZDC_C"]
    triggers += ["HLT_2mu4_L12MU4_VTE10_VZDC_A"]
    triggers += ["HLT_2mu4_L12MU4_VTE10_VZDC_C"]
    triggers += ["HLT_mu6_L1MU6_VTE10_VZDC_A"]
    triggers += ["HLT_mu6_L1MU6_VTE10_VZDC_C"]
    triggers += ["HLT_mb_sptrk_exclusiveloose_L1VTE10_VZDC_A"]
    triggers += ["HLT_mb_sptrk_exclusiveloose_L1VTE10_VZDC_C"]
    triggers += ["HLT_mb_sptrk_exclusiveloose_L1RD0_FILLED"]
    triggers += ["HLT_mb_sptrk_exclusiveloose_L1ZDC"]
    triggers += ["HLT_mb_sptrk_exclusiveloose_L1MU4_VTE10"]
    triggers += ["HLT_mb_sptrk_exclusivetight_L1VTE10_VZDC_A"]
    triggers += ["HLT_mb_sptrk_exclusivetight_L1VTE10_VZDC_C"]
    triggers += ["HLT_mb_sptrk_exclusivetight_L1RD0_FILLED"]
    triggers += ["HLT_mb_sptrk_exclusivetight_L1ZDC"]
    triggers += ["HLT_hi_upc_FgapA_L1RD0"]
    triggers += ["HLT_hi_upc_FgapC_L1RD0"]
    triggers += ["HLT_hi_upc_FgapAC_L1RD0"]
    triggers += ["HLT_hi_upc_FgapA_L1MBTS_1"]
    triggers += ["HLT_hi_upc_FgapC_L1MBTS_1"]
    triggers += ["HLT_hi_upc_FgapAC_L1MBTS_1"]
    triggers += ["HLT_hi_upc_L2FgapA_L1MBTS_1"]
    triggers += ["HLT_hi_upc_L2FgapC_L1MBTS_1"]
    triggers += ["HLT_hi_upc_L2FgapAC_L1MBTS_1"]
    triggers += ["HLT_hi_upc_EFFgapA_L1MBTS_1"]
    triggers += ["HLT_hi_upc_EFFgapC_L1MBTS_1"]
    triggers += ["HLT_hi_upc_EFFgapAC_L1MBTS_1"]
    triggers += ["HLT_hi_upc_FgapA_L1ZDC"]
    triggers += ["HLT_hi_upc_FgapC_L1ZDC"]
    triggers += ["HLT_hi_upc_FgapAC_L1ZDC"]
    triggers += ["HLT_noalg_L1MBTS_2_C_VZDC_A"]
    triggers += ["HLT_noalg_L1MBTS_2_A_VZDC_C"]
    triggers += ["HLT_noalg_L1ZDC_C_VZDC_A"]
    triggers += ["HLT_noalg_L1ZDC_A_VZDC_C"]
    triggers += ["HLT_noalg_L1EM3_VZDC_A"]
    triggers += ["HLT_noalg_L1EM3_VZDC_C"]
    triggers += ["HLT_noalg_L12EM3_VZDC_A"]
    triggers += ["HLT_noalg_L12EM3_VZDC_C"]
    triggers += ["HLT_noalg_L1EM5_VZDC_A"]
    triggers += ["HLT_noalg_L1EM5_VZDC_C"]
    triggers += ["HLT_noalg_L1J5_VZDC_A"]
    triggers += ["HLT_noalg_L1J5_VZDC_C"]
    triggers += ["HLT_noalg_L1J10_VZDC_A"]
    triggers += ["HLT_noalg_L1J10_VZDC_C"]
    triggers += ["HLT_noalg_L1MU4_VZDC_A"]
    triggers += ["HLT_noalg_L1MU4_VZDC_C"]
    triggers += ["HLT_noalg_L1MU6_VZDC_A"]
    triggers += ["HLT_noalg_L1MU6_VZDC_C"]
    triggers += ["HLT_noalg_L1TE5_VZDC_A"]
    triggers += ["HLT_noalg_L1TE5_VZDC_C"]
    triggers += ["HLT_hi_upc_FgapA_mb_sptrk_L1RD0_FILLED"]
    triggers += ["HLT_hi_upc_FgapC_mb_sptrk_L1RD0_FILLED"]
    triggers += ["HLT_hi_upc_FgapAC_mb_sptrk_L1RD0_FILLED"]
    triggers += ["HLT_hi_upc_FgapA_mb_sptrk_L1MBTS_2_C"]
    triggers += ["HLT_hi_upc_FgapC_mb_sptrk_L1MBTS_2_A"]
    triggers += ["HLT_hi_upc_FgapAC_mb_sptrk_L1MBTS_2"]
    triggers += ["HLT_hi_upc_FgapA_mb_sptrk_L1ZDC"]
    triggers += ["HLT_hi_upc_FgapC_mb_sptrk_L1ZDC"]
    triggers += ["HLT_hi_upc_FgapAC_mb_sptrk_L1ZDC"]
    triggers += ["HLT_hi_upc_FgapAC_mb_sptrk_exclusiveloose_L1ZDC"]
    triggers += ["HLT_hi_upc_FgapAC_mb_sptrk_exclusiveloose"]
    triggers += ["HLT_hi_upc_FgapAC_mb_sptrk_exclusiveloose_L1VTE10_VZDC_A"]
    triggers += ["HLT_hi_upc_FgapAC_mb_sptrk_exclusiveloose_L1VTE10_VZDC_C"]
    triggers += ["HLT_hi_upc_FgapAC_mb_sptrk_exclusivetight_L1ZDC"]
    triggers += ["HLT_hi_upc_FgapAC_mb_sptrk_exclusivetight"]
    triggers += ["HLT_hi_upc_FgapAC_mb_sptrk_exclusivetight_L1VTE10_VZDC_A"]
    triggers += ["HLT_hi_upc_FgapAC_mb_sptrk_exclusivetight_L1VTE10_VZDC_C"]
    triggers += ["HLT_hi_upc_FgapAC_mu4"]
    triggers += ["HLT_hi_upc_FgapAC_mu6"]
    triggers += ["HLT_hi_upc_FgapAC_2mu4"]
    triggers += ["HLT_hi_upc_FgapAC_mu4_mu4noL1"]
    triggers += ["HLT_hi_upc_FgapAC_mu4_L1MU4_VTE10"]
    triggers += ["HLT_hi_upc_FgapAC_e5_etcut"]
    triggers += ["HLT_hi_upc_FgapAC_e5_lhloose_nod0"]
    triggers += ["HLT_hi_upc_FgapAC_2e5_etcut"]
    triggers += ["HLT_hi_upc_FgapAC_2e5_lhloose_nod0"]
    triggers += ["HLT_hi_upc_FgapA_g5_loose"]
    triggers += ["HLT_hi_upc_FgapC_g5_loose"]
    triggers += ["HLT_hi_upc_FgapA_g10_loose"]
    triggers += ["HLT_hi_upc_FgapC_g10_loose"]
    triggers += ["HLT_hi_upc_FgapA_g5_etcut"]
    triggers += ["HLT_hi_upc_FgapC_g5_etcut"]
    triggers += ["HLT_hi_upc_FgapA_g10_etcut"]
    triggers += ["HLT_hi_upc_FgapC_g10_etcut"]

    triggers += ["HLT_mb_sptrk_exclusiveloose1_L1VTE10_VZDC_A"]
    triggers += ["HLT_mb_sptrk_exclusiveloose1_L1VTE10_VZDC_C"]
    triggers += ["HLT_mb_sptrk_exclusiveloose1_L1RD0_FILLED"]
    triggers += ["HLT_mb_sptrk_exclusiveloose1_L1ZDC"]
    triggers += ["HLT_mb_sptrk_exclusiveloose1_L1MU4_VTE10"] 

    triggers += ["HLT_mb_sptrk_exclusiveloose2_L1VTE10_VZDC_A"]
    triggers += ["HLT_mb_sptrk_exclusiveloose2_L1VTE10_VZDC_C"]
    triggers += ["HLT_mb_sptrk_exclusiveloose2_L1RD0_FILLED"]
    triggers += ["HLT_mb_sptrk_exclusiveloose2_L1ZDC"]
    triggers += ["HLT_mb_sptrk_exclusiveloose2_L1MU4_VTE10"] 

    return triggers

def HION4SkimmingTriggers2018Primary():
    #primary and backup triggers
    triggers  = ["HLT_hi_upc_FgapAC3_hi_gg_upc_noiseSup_L1TE4_VTE200"]
    triggers += ["HLT_hi_upc_FgapAC3_hi_gg_upc_noiseSup_L1TE4_VTE50"]
    triggers += ["HLT_hi_upc_FgapAC3_hi_gg_upc_noiseSup_L1TE5_VTE200"]
    triggers += ["HLT_hi_upc_FgapAC3_hi_gg_upc_noiseSup_L1TE5_VTE50"]
    triggers += ["HLT_hi_upc_FgapAC3_hi_gg_upc_noiseSup_L1TAU1_TE4_VTE200"]
    triggers += ["HLT_hi_upc_FgapAC3_hi_gg_upc_noiseSup_L1TAU1_TE5_VTE200"]
    triggers += ["HLT_hi_upc_FgapAC3_hi_gg_upc_noiseSup_L12TAU1_VTE50"]
    triggers += ["HLT_hi_upc_FgapAC3_hi_gg_upc_L12TAU2_VTE50"]
    triggers += ["HLT_hi_upc_FgapAC3_hi_gg_upc_L12TAU2_VTE100"]
    triggers += ["HLT_hi_upc_FgapAC3_hi_gg_upc_L12TAU2_VTE200"]
    triggers += ["HLT_hi_upc_FgapAC3_hi_gg_upc_L12TAU3_VTE50"]
    triggers += ["HLT_hi_upc_FgapAC3_hi_gg_upc_L12TAU3_VTE100"]
    triggers += ["HLT_hi_upc_FgapAC3_hi_gg_upc_L12TAU3_VTE200"]

    return triggers

def HION4SkimmingTriggers2018Support():
    # supporting triggers
    triggers  = ["HLT_hi_upc_FgapAC3_mb_sptrk_exclusiveloose2_L12TAU1_VTE50"]
    triggers += ["HLT_hi_upc_FgapAC3_mb_sptrk_exclusiveloose2_L12TAU2_VTE200"]
    triggers += ["HLT_hi_upc_FgapAC3_mb_sptrk_exclusiveloose1_L1MU4_VTE50"]
    triggers += ["HLT_hi_upc_FgapAC3_mb_sptrk_exclusiveloose2_L1MU4_VTE50"]
    triggers += ["HLT_mb_sptrk_exclusiveloose2_L12TAU2_VTE50"]
    triggers += ["HLT_mb_sptrk_exclusiveloose2_L12TAU2_VTE100"]
    triggers += ["HLT_mb_sptrk_exclusiveloose2_L12TAU2_VTE200"]
    triggers += ["HLT_mb_sptrk_vetombts2in_exclusiveloose2_L12TAU1_VTE50"]
    triggers += ["HLT_hi_upc_FgapAC3_mb_sptrk_exclusiveloose1_L1ZDC_XOR_VTE50"]
    triggers += ["HLT_hi_upc_FgapAC3_mb_sptrk_exclusiveloose2_L1ZDC_XOR_VTE50"]
    triggers += ["HLT_hi_upc_FgapAC3_mb_sptrk_exclusiveloose2_L1ZDC_A_C_VTE50"]
    triggers += ["HLT_hi_upc_FgapAC3_mb_sptrk_exclusiveloose2_L1VZDC_A_C_VTE50"]
    triggers += ["HLT_hi_upc_FgapAC3_hi_gg_upc_noiseSup_L1TE4_VTE200_EMPTY"]
    triggers += ["HLT_hi_upc_FgapAC3_hi_gg_upc_noiseSup_L1TE5_VTE200_EMPTY"]
    triggers += ["HLT_hi_upc_FgapAC3_hi_gg_upc_L1TE50_VTE200"]
    triggers += ["HLT_hi_upc_FgapAC3_hi_gg_upc_L1TE4_VTE50"]
    triggers += ["HLT_hi_upc_FgapAC3_hi_gg_upc_L1TE4_VTE100"]
    triggers += ["HLT_hi_upc_FgapAC3_hi_gg_upc_L1TE4_VTE200"]
    triggers += ["HLT_hi_upc_FgapAC3_hi_gg_upc_L1TE5_VTE50"]
    triggers += ["HLT_hi_upc_FgapAC3_hi_gg_upc_L1TE5_VTE100"]
    triggers += ["HLT_hi_upc_FgapAC3_hi_gg_upc_L1TE5_VTE200"]
    triggers += ["HLT_hi_gg_upc_L1TE4_VTE50"]
    triggers += ["HLT_hi_gg_upc_L1TE4_VTE100"]
    triggers += ["HLT_hi_gg_upc_L1TE4_VTE200"]
    triggers += ["HLT_hi_gg_upc_L1TE5_VTE50"]
    triggers += ["HLT_hi_gg_upc_L1TE5_VTE100"]
    triggers += ["HLT_hi_gg_upc_L1TE5_VTE200"]
    triggers += ["HLT_hi_upc_FgapAC3_hi_gg_upc_L1TAU1_TE4_VTE200"]
    triggers += ["HLT_hi_upc_FgapAC3_hi_gg_upc_L1TAU1_TE5_VTE200"]
    triggers += ["HLT_hi_upc_FgapAC3_hi_gg_upc_L12TAU1_VTE50"]
    triggers += ["HLT_mb_sptrk_vetombts2in_exclusiveloose2_L12TAU2_VTE50"]
    triggers += ["HLT_mb_sptrk_vetombts2in_exclusiveloose2_L12TAU2_VTE100"]
    triggers += ["HLT_mb_sptrk_vetombts2in_exclusiveloose2_L12TAU2_VTE200"]
    triggers += ["HLT_mb_sp_L1VTE50"]
    triggers += ["HLT_mb_sptrk_exclusiveloose2_L12TAU1_VTE50"]
    triggers += ["HLT_mu4_hi_upc_FgapAC3_L1MU4_VTE50"]

    return triggers

def HION4SkimmingTriggers2023():
    #primary and backup triggers
    triggers  = []
    triggers += ["HLT_mu3_L1MU3V_VTE50"]
    triggers += ["HLT_mu3_hi_FgapAC5_L1MU3V_VTE50"]
    triggers += ["HLT_mu4_L1MU3V_VTE50"]
    triggers += ["HLT_mu4_hi_FgapAC5_L1MU3V_VTE50"]
    triggers += ["HLT_mb_sp_vpix30_hi_FgapAC5_2g0_etcut_25dphiCC_L12TAU1_VTE200"]    
    triggers += ["HLT_mb_sp_vpix30_hi_FgapAC5_L1TAU1_TE4_VTE200"]
    triggers += ["HLT_mb_sp_vpix30_hi_FgapAC5_L12TAU1_VTE200"]
    triggers += ["HLT_mb_sp_vpix30_hi_FgapAC5_2g0_etcut_L12TAU1_VTE200"]
    triggers += ["HLT_mb_sptrk_hi_FgapA5_L1VZDC_A_ZDC_C_VTE200"]
    triggers += ["HLT_mb_sptrk_hi_FgapC5_L1ZDC_A_VZDC_C_VTE200"]
    triggers += ["HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L11ZDC_A_1ZDC_C_VTE200"]
    triggers += ["HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1ZDC_XOR4_VTE200"]
    triggers += ["HLT_mb_excl_1trk5_pt1_L1TRT_VZDC_A_VZDC_C_VTE50"]
    triggers += ["HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1TRT_VTE20"]
    triggers += ["HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1TAU1_TE4_VTE200"]
    triggers += ["HLT_mb_excl_1trk5_pt1_L1TAU1_TE4_VTE200"]
    triggers += ["HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L12TAU1_VTE200"]
    triggers += ["HLT_mb_sptrk_hi_FgapAC5_L12TAU1_VTE200"]
    triggers += ["HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1TAU2_VZDC_A_VZDC_C_VTE100"]
    triggers += ["HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1TAU2_ZDC_XOR4_VTE100"]
    triggers += ["HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1TAU1_TRT_VTE50"]
    triggers += ["HLT_mb_sp_vpix15_hi_FgapAC5_L1TAU1_TE4_VTE200_EMPTY"]
    triggers += ["HLT_mb_sp_vpix30_hi_FgapAC5_L1TAU1_TE4_VTE200_EMPTY"]
    triggers += ["HLT_mb_sp_vpix30_hi_FgapAC5_L1TAU8_VTE200_EMPTY"]

    return triggers

def HION4SkimmingTriggers2024():
    #primary and backup triggers
    triggers  = []
    triggers += ["HLT_mu3_hi_FgapAC5_L1MU3V_VjTE50"]
    triggers += ["HLT_mu3_L1MU3V_VjTE50"]
    triggers += ["HLT_mu4_hi_FgapAC5_L1MU3V_VjTE50"]
    triggers += ["HLT_mu4_L1MU3V_VjTE50"]
    triggers += ["HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1eTAU1_TRT_VZDC_A_VZDC_C_VjTE100"]
    triggers += ["HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1TRT_VjTE20"]
    triggers += ["HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1VZDC_A_VZDC_C_VjTE200"]
    triggers += ["HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L11ZDC_A_1ZDC_C_VjTE200"]
    triggers += ["HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1ZDC_XOR_VjTE200"]
    triggers += ["HLT_mb_excl_1trk5_pt1_L1TRT_VjTE20"]
    triggers += ["HLT_mb_excl_1trk5_pt0p5_L1TRT_VjTE20"]
    triggers += ["HLT_mb_sptrk_L1TRT_ZDC_XOR_VjTE200"]
    triggers += ["HLT_mb_excl_1trk5_pt0p5_hi_FgapAC5_L1TRT_VjTE20"]
    triggers += ["HLT_mb_sptrk_L1ZDC_XOR_VjTE200"]
    triggers += ["HLT_mb_sptrk_L11ZDC_A_1ZDC_C_VjTE200"]
    triggers += ["HLT_mb_sptrk_L1ZDC_1XOR5_VjTE200"]
    triggers += ["HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L12eTAU1_VjTE200"]
    triggers += ["HLT_mb_excl_1trk5_pt1_L12eTAU1_VjTE200"]
    triggers += ["HLT_mb_sp_vpix30_hi_FgapAC5_L1DPHI_2eTAU1_VjTE200"]
    triggers += ["HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1DPHI_2eTAU1_VjTE200"]
    triggers += ["HLT_mb_sp_vpix60_hi_FgapAC5_L1DPHI_2eTAU1_VjTE200"]
    triggers += ["HLT_mb_excl_1trk5_pt2_L1eEM1_VTE200"]
    triggers += ["HLT_mb_sp_vpix60_hi_FgapAC5_2g0_etcut_L12eTAU1_VjTE200"]
    triggers += ["HLT_mb_sp_vpix60_hi_FgapAC5_L12eTAU1_VjTE200"]
    triggers += ["HLT_mb_sp_vpix60_hi_FgapAC5_2g0_etcut_L1DPHI_2eEM1_VjTE200"]
    triggers += ["HLT_mb_sp_vpix60_hi_FgapAC5_L1DPHI_2eEM1_VjTE200"]
    triggers += ["HLT_mb_sp_vpix60_hi_FgapAC5_L1eEM5_VjTE200"]
    triggers += ["HLT_mb_sp_vpix60_hi_FgapAC5_L1eEM9_VjTE200_EMPTY"]
    triggers += ["HLT_mb_sp_vpix60_hi_FgapAC5_L1DPHI_2eEM1_VjTE200_EMPTY"]
    triggers += ["HLT_mb_sp_vpix60_hi_FgapAC5_L1DPHI_2eTAU1_VjTE200_EMPTY"]
    triggers += ["HLT_mb_sp_vpix60_hi_FgapAC5_L12eTAU1_VjTE200_EMPTY"]
    triggers += ["HLT_mb_sp_vpix60_hi_FgapAC5_L1eTAU1_TRT_VjTE200_EMPTY"]

    return triggers

def HION4SkimmingTriggersALL():
    triggers  = HION4SkimmingTriggers2015()
    triggers += HION4SkimmingTriggers2016()
    triggers += HION4SkimmingTriggers2018Primary()
    triggers += HION4SkimmingTriggers2018Support()
    triggers += HION4SkimmingTriggers2023()
    triggers += HION4SkimmingTriggers2024()

    return triggers

def HION4SkimmingTriggersVM():
    
    VMtrigger = ["HLT_mb_sptrk_exclusiveloose_vetosp1500_L1VTE20"]
    
    return VMtrigger

#################################################################################
#HION5

def HION5SkimmingTriggers():
    triggers  = []
    triggers += ["HLT_e15_lhloose_nod0"]
    triggers += ["HLT_mu15"]
    triggers += ["HLT_g10_loose"]
    triggers += ["HLT_g15_loose"]
    triggers += ["HLT_g20_loose"]
    triggers += ["HLT_g25_loose"]
    triggers += ["HLT_g30_loose"]
    triggers += ["HLT_g35_loose"]


    #2023 HI
    triggers += ["HLT_e15_lhloose_nogsf_ion_L1EM12"]
    triggers += ["HLT_e15_loose_nogsf_ion_L1EM12"]
    triggers += ["HLT_e15_lhmedium_nogsf_ion_L1EM12"]
    triggers += ["HLT_e15_medium_nogsf_ion_L1EM12"]
    triggers += ["HLT_e18_lhloose_nogsf_ion_L1EM15"]
    triggers += ["HLT_e18_loose_nogsf_ion_L1EM15"]
    triggers += ["HLT_e18_lhmedium_nogsf_ion_L1EM15"]
    triggers += ["HLT_e18_medium_nogsf_ion_L1EM15"]
    triggers += ["HLT_e20_lhloose_nogsf_ion_L1EM15"]
    triggers += ["HLT_e20_lhmedium_nogsf_ion_L1EM15"]
    triggers += ["HLT_e20_loose_nogsf_ion_L1EM15"]
    triggers += ["HLT_e20_medium_nogsf_ion_L1EM15"]
    triggers += ["HLT_mu10_L1MU8F"] #also 2024
    triggers += ["HLT_mu10_L1MU5VF"] #also 2024
    triggers += ["HLT_g15_loose_ion_L1eEM12"]
    triggers += ["HLT_g15_loose_ion_L1eEM15"]
    triggers += ["HLT_g20_loose_ion_L1eEM15"]
    triggers += ["HLT_g20_loose_ion_L1eEM18"]
    triggers += ["HLT_g30_loose_ion_L1eEM18"]
    triggers += ["HLT_g50_loose_ion_L1eEM26"]

    #2024 HI
    triggers += ["HLT_e15_lhloose_nogsf_ion_L1eEM15"]
    triggers += ["HLT_e15_loose_nogsf_ion_L1eEM15"]
    triggers += ["HLT_e15_lhmedium_nogsf_ion_L1eEM15"]
    triggers += ["HLT_e15_medium_nogsf_ion_L1eEM15"]
    triggers += ["HLT_e20_lhloose_nogsf_ion_L1eEM18"]
    triggers += ["HLT_e20_lhmedium_nogsf_ion_L1eEM18"]
    triggers += ["HLT_e20_loose_nogsf_ion_L1eEM18"]
    triggers += ["HLT_e20_loose_nogsf_ion_L1eEM18L"]
    triggers += ["HLT_e20_medium_nogsf_ion_L1eEM18"]

    #2024 pp ref
    triggers += ["HLT_e14_lhvloose_L1eEM12L"]
    triggers += ["HLT_e15_lhloose_L1eEM15"]
    triggers += ["HLT_e15_loose_L1eEM15"]
    triggers += ["HLT_e15_lhmedium_L1eEM15"]
    triggers += ["HLT_e15_medium_L1eEM15"]
    triggers += ["HLT_e20_lhloose_L1eEM18"]
    triggers += ["HLT_e20_lhmedium_L1eEM18"]
    triggers += ["HLT_e20_loose_L1eEM18"]
    triggers += ["HLT_e20_loose_L1eEM18L"]
    triggers += ["HLT_e20_medium_L1eEM18"]
    triggers += ["HLT_e30_lhloose_L1eEM18"]
    triggers += ["HLT_e30_lhmedium_L1eEM18"]
    triggers += ["HLT_e30_loose_L1eEM18"]
    triggers += ["HLT_e30_medium_L1eEM18"]
    triggers += ["HLT_mu12_L1MU8F"]
    triggers += ["HLT_mu15_L1MU8F"]
    triggers += ["HLT_mu15_L1MU14FCH"]
    triggers += ["HLT_mu4noL1_L1MBTS_1"]
    triggers += ["HLT_g15_loose_L1eEM15"]
    triggers += ["HLT_g20_loose_L1eEM15"]
    triggers += ["HLT_g30_loose_L1eEM18"]
    triggers += ["HLT_g40_loose_L1eEM18"]
    triggers += ["HLT_g40_loose_L1eEM26"]
    triggers += ["HLT_g50_loose_L1eEM26"]

    # 2025 OO

    triggers += ["HLT_mu8_L1MU5VF"]
    triggers += ["HLT_e10_loose_L1eEM9"]

    return triggers

#################################################################################
#HION7
# for all jet triggers HLT jet threshold as cutoff

def HION7JetTriggers2023():
    triggers  = {}
    triggers.update({'HLT_j60_ion_L1J15' : 60})
    triggers.update({'HLT_j75_ion_L1J20' : 75})
    triggers.update({'HLT_j85_ion_L1J15' : 85})

    return triggers

def HION7BJetTriggers2023():
    triggers  = {}
    triggers.update({'HLT_mu4_j50_ion_dRAB05_L1MU3V' : 50})
    triggers.update({'HLT_mu4_j50_ion_dRAB05_L1MU3V_J12' : 50})
    triggers.update({'HLT_mu4_j60_ion_dRAB05_L1MU3V' : 60})
    triggers.update({'HLT_mu6_j40_ion_dRAB05_L1MU5VF' : 40})
    triggers.update({'HLT_mu6_j50_ion_dRAB05_L1MU5VF' : 50})
    triggers.update({'HLT_mu4_j40_ion_dRAB05_L1MU3V' : 40})
    triggers.update({'HLT_mu6_j30_ion_dRAB05_L1MU5VF' : 30})

    return triggers

def HION7FwdJetTriggers2023(): # currently not included in HION7
    triggers  = {}
    triggers.update({'HLT_j30f_ion_L1TE20' : 30})
    triggers.update({'HLT_j50f_ion_L1TE50' : 50})
    triggers.update({'HLT_j50f_ion_L1J15p31ETA49' : 50})

    return triggers

def HION7PCCCTriggers2023():
    triggers  = {}
    triggers.update({'HLT_mb_sptrk_pc_L1ZDC_A_C_VTE50' : 20})
    triggers.update({'HLT_noalg_L1TE50_VTE600p0ETA49' : 20})
    triggers.update({'HLT_noalg_L1TE600p0ETA49' : 20})

    return triggers

def HION7JetTriggers2024():
    triggers  = {}
    triggers.update({'HLT_j60_ion_L1jJ40' : 60})
    triggers.update({'HLT_j75_ion_L1jJ50' : 75})

    return triggers

def HION7MuonTriggers():
    triggers  = {}
    triggers.update({'HLT_mu4_L1MU3V' : 25}) # accept any event with jet above 25 GeV

    return triggers

def HION7FwdJetTriggers2024(): # currently not included in HION7
    triggers  = {}
    triggers.update({'HLT_j30f_ion_L1jTE20' : 30})
    triggers.update({'HLT_j50f_ion_L1jTE50' : 50})
    triggers.update({'HLT_j50f_ion_L1jJ40p30ETA49' : 50})

    return triggers

def HION7PCCCTriggers2024():
    triggers  = {}
    triggers.update({'HLT_mb_sptrk_pc_L1ZDC_A_C_VjTE50' : 20})
    triggers.update({'HLT_noalg_L1jTE50_VjTE600' : 20})
    triggers.update({'HLT_noalg_L1jTE600' : 20})

    return triggers

def HION7JetTriggersPP():
    triggers  = {}
    triggers.update({'HLT_j20_L1RD0_FILLED' : 20})
    triggers.update({'HLT_j30a_L1jTE20' : 30})
    triggers.update({'HLT_j40_L1jJ40' : 40})
    triggers.update({'HLT_j50_L1jJ40' : 50})
    triggers.update({'HLT_j60_L1jJ50' : 60})
    triggers.update({'HLT_j75_L1jJ50' : 75})
    triggers.update({'HLT_j85_L1jJ50' : 85})
    triggers.update({'HLT_j100_L1jJ50': 100})
    triggers.update({'HLT_j100_L1jJ60': 100})
    triggers.update({'HLT_j120_L1jJ60': 120})

    return triggers

def HION7BJetTriggersPP():
    triggers  = {}
    triggers.update({'HLT_mu4_j40_L1MU3V_jJ50' : 40})
    triggers.update({'HLT_j30_0eta290_020jvt_bgn260_pf_ftf_L1jTE50' : 30})
    triggers.update({'HLT_j45_0eta290_020jvt_bgn260_pf_ftf_L1jJ40' : 45})
    triggers.update({'HLT_j60_0eta290_020jvt_bgn260_pf_ftf_L1jJ50' : 60})
    triggers.update({'HLT_j80_0eta290_020jvt_bgn260_pf_ftf_L1jJ60' : 80})
    triggers.update({'HLT_j30_0eta290_020jvt_boffperf_pf_ftf_L1jTE50' : 30})
    triggers.update({'HLT_j45_0eta290_020jvt_boffperf_pf_ftf_L1jJ40' : 45})
    triggers.update({'HLT_j60_0eta290_020jvt_boffperf_pf_ftf_L1jJ50' : 60})
    triggers.update({'HLT_j80_0eta290_020jvt_boffperf_pf_ftf_L1jJ60' : 80})
    triggers.update({'HLT_j100_0eta290_020jvt_boffperf_pf_ftf_L1jJ60' : 100})

    return triggers

def HION7FwdJetTriggersPP(): # currently not included in HION7
    triggers  = {}
    triggers.update({'HLT_j15f_L1jTE5': 15})
    triggers.update({'HLT_j25f_L1jTE10' : 25})
    triggers.update({'HLT_j35f_L1jTE20' : 35})
    triggers.update({'HLT_j45f_L1jJ40p30ETA49' : 45})
    triggers.update({'HLT_j60f_L1jJ50p30ETA49' : 60})

    return triggers

def HION7MinBiasTriggersPP():
    triggers  = {}
    triggers.update({'HLT_mb_sptrk_L1RD0_FILLED' : 15})

    return triggers

def HION7JetTriggerspO():
    triggers  = {}
    triggers.update({'HLT_j20_L1jJ20' : 20})
    triggers.update({'HLT_j30_L1jJ20' : 30})
    triggers.update({'HLT_j40_L1jJ20' : 40})
    triggers.update({'HLT_j20_pf_ftf_L1jJ20' : 20})
    triggers.update({'HLT_j30_pf_ftf_L1jJ20' : 30})
    triggers.update({'HLT_j40_pf_ftf_L1jJ20' : 40})
    triggers.update({'HLT_j20_ionp_L1jJ10' : 20})
    triggers.update({'HLT_j30_ionp_L1jJ10' : 30})
    triggers.update({'HLT_j40_ionp_L1jJ20' : 40})

    return triggers

def HION7MinBiasTriggerspO():
    triggers  = {}
    triggers.update({'HLT_mb_sptrk_L1RD0_FILLED' : 15})
    triggers.update({'HLT_mb_sptrk_L1TRT_FILLED' : 15})
    triggers.update({'HLT_mb_sptrk_L1ZDC_OR' : 15})
    triggers.update({'HLT_mb_sptrk_L1jTE20' : 15})
    triggers.update({'HLT_mb_sptrk_L1jTE50' : 15})

    return triggers

def HION7JetTriggersOO():
    triggers  = {}
    triggers.update({'HLT_j20_ionp_L1jJ10' : 20})
    triggers.update({'HLT_j40_ionp_L1jJ20' : 40})
    triggers.update({'HLT_j50_ionp_L1jJ30' : 50})

    return triggers

def HION7SkimmingTriggers2023():
    triggers  = HION7JetTriggers2023()
    triggers.update(HION7BJetTriggers2023())
    triggers.update(HION7MuonTriggers())
    triggers.update(HION7PCCCTriggers2023())

    return triggers

def HION7SkimmingTriggers2024():
    triggers  = HION7JetTriggers2024()
    triggers.update(HION7MuonTriggers())
    triggers.update(HION7PCCCTriggers2024())

    return triggers

def HION7SkimmingTriggersPP():
    triggers  = HION7JetTriggersPP()
    triggers.update(HION7BJetTriggersPP())
    triggers.update(HION7MuonTriggers())
    triggers.update(HION7MinBiasTriggersPP())

    return triggers

def HION7SkimmingTriggerspO():
    triggers  = HION7JetTriggerspO()
    triggers.update(HION7MinBiasTriggerspO())

    return triggers

def HION7SkimmingTriggersOO():
    triggers  = HION7JetTriggerspO()
    triggers.update(HION7JetTriggersOO())
    triggers.update(HION7MinBiasTriggerspO())

    return triggers

def HION7SkimmingTriggers2025():
    triggers  = HION7SkimmingTriggers2024()

    return triggers

#################################################################################
def GetTriggers(project_tag, isSmallSystem):
    switcher_HION7 = {
        'data23_hi': HION7SkimmingTriggers2023(),
        'data24_hi': HION7SkimmingTriggers2024(),
        'data25_hi': HION7SkimmingTriggers2025(),
        }

    switcher_SmallSystems = {
        'data24_5p36TeV': HION7SkimmingTriggersPP(),
        'data25_hip': HION7SkimmingTriggerspO(),
        'data25_hi': HION7SkimmingTriggersOO(),
        }

    if isSmallSystem:
        return  switcher_SmallSystems.get(project_tag, "Invalid project tag")
    else:
        return  switcher_HION7.get(project_tag, "Invalid project tag")


#################################################################################
#HION12

def HION12MBtriggers2018():
    triggers  = []
    
    # Photo-nuclear min-bias triggers

    triggers += ["HLT_mb_sptrk_L1ZDC_XOR_TE5_VTE200"]  # Main min-bias stream for photo-nuclear dijets
    triggers += ["HLT_mb_sptrk_L1ZDC_XOR_VTE200"]  # Min-bias stream to check the impact of the TE5 cut. Very pre-scaled.
    triggers += ["HLT_noalg_L1ZDC_XOR_TE5_VTE200"]  # Stream to deal with events where no tracks were re-constructed due to the rapidity gap
    triggers += ["HLT_noalg_L1ZDC_XOR"]  # Stream to deal with overall effects of TE5 and VTE200 cuts. Extremely pre-scaled.
    triggers += ["HLT_noalg_L1TE5_VTE200"] # Stream to examine the impact of the ZDC cuts. Prescale is 276.6.
 
    #2015 photo-nuclear triggers which do not have an identical version in 2018
    
    triggers += ["HLT_noalg_mb_L1TE50"] # 2015 MB trigger with high pre-scale on events with low total energy
    triggers += ["HLT_mb_sptrk_ion_L1ZDC_A_C_VTE50"] # 2015 MB trigger with full ZDC activation for inefficiency

    # Photoproduction min-bias triggers
    # Need to apply a ZDC veto in order to assess actual efficiencies for comparison.

    triggers += ["HLT_mb_sp_L1VTE50"]  # Min-bias stream for dijet photoproduction up to 50 GeV. No minimum cut so high pre-scale.
    triggers += ["HLT_noalg_pc_L1TE50_VTE600.0ETA49"]  # Part of the main heavy ion min-bias stream from 50 to 600 GeV. In the PC stream so requires special attention.
    triggers += ["HLT_mb_sptrk_L1ZDC_A_C_VTE50"]  # Part of the main heavy ion min-bias stream from 0 to 50 GeV. In the PC stream so requires special attention.
    
    return triggers

def HION12triggers2018():
    triggers  = []
    # Photo-nuclear dijet physics triggers (j40 not included since j30 is un-prescaled for the entire run)

    triggers += ["HLT_j10_L1ZDC_XOR_TE5_VTE200"]  # j10 trigger for first half of 2018 run
    triggers += ["HLT_j10_0eta490_L1ZDC_XOR_TE5_VTE200"]  # j10 trigger for second half of 2018 run
    triggers += ["HLT_j15_L1ZDC_XOR_TE5_VTE200"]  # j15 trigger for first half of 2018 run
    triggers += ["HLT_j15_0eta490_L1ZDC_XOR_TE5_VTE200"]  # j15 trigger for second half of 2018 run
    triggers += ["HLT_j20_L1ZDC_XOR_TE5_VTE200"]  # j20 trigger for first half of 2018 run
    triggers += ["HLT_j20_0eta490_L1ZDC_XOR_TE5_VTE200"]  # j20 trigger for second half of 2018 run
    triggers += ["HLT_j30_L1ZDC_XOR_TE20_VTE200"]  # j30 trigger for first half of 2018 run
    triggers += ["HLT_j30_0eta490_L1ZDC_XOR_TE20_VTE200"]  # j30 trigger for second half of 2018 run
    triggers += ["HLT_j10_rcu4_0eta490_L1ZDC_XOR_TE5_VTE200"]  # j10 rcu4 trigger for second half of 2018 run (Lower un-calibrated min-pT cut)

    # Photoproduction dijet R = 0.4 physics triggers (j40 not included because j30 is un-prescaled for the entire run)
    # Actually, all of these triggers (except the rcu4) are un-prescaled. Yay!

    triggers += ["HLT_j10_L1VZDC_A_C_TE5_VTE200"]  # j10 trigger for the first half of the 2018 run
    triggers += ["HLT_j15_L1VZDC_A_C_TE5_VTE200"]  # j15 trigger for the first half of the 2018 run
    triggers += ["HLT_j20_L1VZDC_A_C_TE5_VTE200"]  # j20 trigger for the first half of the 2018 run
    triggers += ["HLT_j30_L1VZDC_A_C_TE20_VTE200"]  # j30 trigger for the first half of the 2018 run
    triggers += ["HLT_j10_0eta490_L1VZDC_A_C_TE5_VTE200"]  # j10 trigger for the second half of the 2018 run
    triggers += ["HLT_j15_0eta490_L1VZDC_A_C_TE5_VTE200"]  # j15 trigger for the second half of the 2018 run
    triggers += ["HLT_j20_0eta490_L1VZDC_A_C_TE5_VTE200"]  # j20 trigger for the second half of the 2018 run
    triggers += ["HLT_j30_0eta490_L1VZDC_A_C_TE20_VTE200"]  # j30 trigger for the second half of the 2018 run
    triggers += ["HLT_j10_rcu4_0eta490_L1VZDC_A_C_TE5_VTE200"]  # j10 rcu4 trigger for second half of 2018 run (Lower un-calibrated min-pT cut)

    # Photoproduction dijet R = 1.0 physics triggers

    triggers += ["HLT_j10_a10_lcw_subjes_L1VZDC_A_C_TE5_VTE200"] # j10 R=1.0 trigger for second half of the 2018 run. Same pre-scale as j15.
    triggers += ["HLT_j15_a10_lcw_subjes_L1VZDC_A_C_TE5_VTE200"] # j15 R=1.0 trigger for second half of the 2018 run. Same pre-scale as j10.
    triggers += ["HLT_j20_a10_lcw_subjes_L1VZDC_A_C_TE5_VTE200"] # j20 R=1.0 trigger for second half of the 2018 run. Same pre-scale as j30.
    triggers += ["HLT_j30_a10_lcw_subjes_L1VZDC_A_C_TE5_VTE200"] # j30 R=1.0 trigger for second half of the 2018 run. Same pre-scale as j20.
    
    #2015 photo-nuclear triggers which do not have an identical version in 2018.

    triggers += ["HLT_j10_320eta490_ion_L1TE5_VTE200"] # Forward trigger on HI jets in 2015 without ZDC requirement
    triggers += ["HLT_j10_ion_L1TE5_VTE200"]  # Mid-rapidity j10 trigger on HI jets in 2015 without ZDC requirement
    triggers += ["HLT_j15_ion_L1TE5_VTE200"]  # Mid-rapidity j15 trigger on HI jets in 2015 without ZDC requirement
    triggers += ["HLT_j20_ion_L1TE5_VTE200"]  # Mid-rapidity j20 trigger on HI jets in 2015 without ZDC requirement
    triggers += ["HLT_j20_ion_L1ZDC_XOR_TE5_VTE200"]  # Mid-rapidity j20 trigger on HI jets in 2015 with ZDC requirement
    triggers += ["HLT_j10_ion_L1ZDC_XOR_TE5_VTE200"]  # Mid-rapidity j10 trigger on HI jets in 2015 with ZDC requirement
    triggers += ["HLT_j15_ion_L1ZDC_XOR_TE5_VTE200"]  # Mid-rapidity j15 trigger on HI jets in 2015 with ZDC requirement
    triggers += ["HLT_j10_ion_mb_mbts_vetombts1side2in_L1ZDC_XOR_TE5_VTE200"]  # j10 trigger on HI jets in 2015 with an asymmetric MBTS and ZDC XOR requirement

    return triggers

def HION12nJetCuts2018():
    nJetCuts  = [] 
    nJetCuts += ["(count(AntiKt4EMTopoJets.pt > 7.0*GeV) > 0)"]
    nJetCuts += ["(count(AntiKt4LCTopoJets.pt > 7.0*GeV) > 0)"]
    nJetCuts += ["(count(AntiKt4EMPFlowJets.pt > 7.0*GeV) > 0)"]
    nJetCuts += ["(count(AntiKt4HIJets.pt > 7.0*GeV) > 0)"]
    nJetCuts += ["(count(AntiKt4HITrackJets.pt > 7.0*GeV) > 0)"]
    nJetCuts += ["(count(AntiKt10LCTopoJets.pt > 7.0*GeV) > 0)"]
    
    return nJetCuts

def HION14SkimmingTriggersOpenData():
    triggers = []
    
    # Eliminate UCC peak
    triggers += ['HLT_mb_sptrk_ion_L1ZDC_A_C_VTE50']
    triggers += ['HLT_noalg_mb_L1TE50']

    return triggers
