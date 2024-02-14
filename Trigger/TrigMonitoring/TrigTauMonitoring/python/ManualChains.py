# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# This file contains a backup configuration for the chains that will be included in the Tau Monitoring, 
# if no HLTMonitoring configuration can be retrieved from the DB, external files, or from the AOD file metadata

mon_singletau = [
  # tau0
  'HLT_tau0_ptonly_L1TAU8',
  'HLT_tau0_ptonly_L1TAU60',

  'HLT_tau0_ptonly_L1eTAU12',
  'HLT_tau0_ptonly_L1eTAU80',

  # tau20
  'HLT_tau20_idperf_tracktwoMVA_L1TAU8',
  'HLT_tau20_perf_tracktwoMVA_L1TAU8',
  'HLT_tau20_mediumRNN_tracktwoMVA_L1TAU8',

  'HLT_tau20_idperf_tracktwoMVA_L1eTAU12',
  'HLT_tau20_perf_tracktwoMVA_L1eTAU12',
  'HLT_tau20_mediumRNN_tracktwoMVA_L1eTAU12',

  # tau25
  'HLT_tau25_idperf_tracktwoMVA_L1TAU12IM',
  'HLT_tau25_perf_tracktwoMVA_L1TAU12IM',
  'HLT_tau25_mediumRNN_tracktwoMVA_L1TAU12IM',
  'HLT_tau25_mediumRNN_tracktwoLLP_L1TAU12IM',

  'HLT_tau25_idperf_tracktwoMVA_L1eTAU20',
  'HLT_tau25_perf_tracktwoMVA_L1eTAU20',
  'HLT_tau25_mediumRNN_tracktwoMVA_L1eTAU20',

  'HLT_tau25_idperf_tracktwoMVA_L1eTAU20M',
  'HLT_tau25_perf_tracktwoMVA_L1eTAU20M',
  'HLT_tau25_mediumRNN_tracktwoMVA_L1eTAU20M',

  'HLT_tau25_idperf_tracktwoMVA_L1jTAU20',
  'HLT_tau25_perf_tracktwoMVA_L1jTAU20',
  'HLT_tau25_mediumRNN_tracktwoMVA_L1jTAU20',

  'HLT_tau25_idperf_tracktwoMVA_L1cTAU20M',
  'HLT_tau25_perf_tracktwoMVA_L1cTAU20M',
  'HLT_tau25_mediumRNN_tracktwoMVA_L1cTAU20M',
  'HLT_tau25_mediumRNN_tracktwoLLP_L1cTAU20M',


  # tau35
  'HLT_tau35_idperf_tracktwoMVA_L1TAU20IM',
  'HLT_tau35_perf_tracktwoMVA_L1TAU20IM',
  'HLT_tau35_mediumRNN_tracktwoMVA_L1TAU20IM',
  
  'HLT_tau35_idperf_tracktwoMVA_L1eTAU30',
  'HLT_tau35_perf_tracktwoMVA_L1eTAU30',
  'HLT_tau35_mediumRNN_tracktwoMVA_L1eTAU30',

  'HLT_tau35_idperf_tracktwoMVA_L1jTAU30',
  'HLT_tau35_perf_tracktwoMVA_L1jTAU30',
  'HLT_tau35_mediumRNN_tracktwoMVA_L1jTAU30',

  'HLT_tau35_idperf_tracktwoMVA_L1jTAU30M',
  'HLT_tau35_perf_tracktwoMVA_L1jTAU30M',
  'HLT_tau35_mediumRNN_tracktwoMVA_L1jTAU30M',

  'HLT_tau35_idperf_tracktwoMVA_L1cTAU30M',
  'HLT_tau35_perf_tracktwoMVA_L1cTAU30M',
  'HLT_tau35_mediumRNN_tracktwoMVA_L1cTAU30M',

  # tau60
  'HLT_tau60_mediumRNN_tracktwoMVA_L1TAU40',

  'HLT_tau60_mediumRNN_tracktwoMVA_L1eTAU60',

  # tau80
  'HLT_tau80_mediumRNN_tracktwoMVA_L1TAU60', 

  'HLT_tau80_mediumRNN_tracktwoMVA_L1eTAU80',

  'HLT_tau80_idperf_trackLRT_L1TAU60',
  'HLT_tau80_mediumRNN_trackLRT_L1TAU60',

  # tau160
  'HLT_tau160_idperf_tracktwoMVA_L1TAU100',
  'HLT_tau160_perf_tracktwoMVA_L1TAU100',
  'HLT_tau160_mediumRNN_tracktwoMVA_L1TAU100',

  'HLT_tau160_idperf_tracktwoMVA_L1eTAU140',
  'HLT_tau160_perf_tracktwoMVA_L1eTAU140',
  'HLT_tau160_mediumRNN_tracktwoMVA_L1eTAU140',
  'HLT_tau160_mediumRNN_tracktwoLLP_L1eTAU140',

  'HLT_tau160_idperf_trackLRT_L1TAU100',
  'HLT_tau160_mediumRNN_trackLRT_L1TAU100',

  # tau180
  'HLT_tau180_mediumRNN_tracktwoLLP_L1TAU100',

  'HLT_tau180_mediumRNN_tracktwoLLP_L1eTAU140',

  # tau200
  'HLT_tau200_mediumRNN_tracktwoLLP_L1TAU100',
  'HLT_tau200_tightRNN_tracktwoLLP_L1TAU100',

  'HLT_tau200_mediumRNN_tracktwoLLP_L1eTAU140',
  'HLT_tau200_tightRNN_tracktwoLLP_L1eTAU140',
]

mon_ditau = [
  # tau35 + tau25
  'HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB30_L1DR-TAU20ITAU12I-J25',
  'HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55',
  'HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB30_L1cTAU30M_2cTAU20M_DR-eTAU30MeTAU20M-jJ55',
  'HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB_L1TAU20IM_2TAU12IM_4J12p0ETA25',
  'HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB_L1cTAU30M_2cTAU20M_4jJ30p0ETA25',
  'HLT_tau40_mediumRNN_tracktwoMVA_tau35_mediumRNN_tracktwoMVA_03dRAB_L1TAU25IM_2TAU20IM_2J25_3J20',
  'HLT_tau40_mediumRNN_tracktwoMVA_tau35_mediumRNN_tracktwoMVA_03dRAB_L1cTAU35M_2cTAU30M_2jJ55_3jJ50',

  # tau80 + tau35
  'HLT_tau80_mediumRNN_tracktwoMVA_tau35_mediumRNN_tracktwoMVA_03dRAB30_L1TAU60_DR-TAU20ITAU12I',
  'HLT_tau80_mediumRNN_tracktwoMVA_tau35_mediumRNN_tracktwoMVA_03dRAB30_L1eTAU80_2cTAU30M_DR-eTAU30eTAU20',

  # tau80 + tau60
  'HLT_tau80_mediumRNN_tracktwoMVA_tau60_mediumRNN_tracktwoMVA_03dRAB_L1TAU60_2TAU40',
  'HLT_tau80_mediumRNN_tracktwoMVA_tau60_mediumRNN_tracktwoMVA_03dRAB_L1eTAU80_2eTAU60',
  'HLT_tau80_mediumRNN_tracktwoLLP_tau60_mediumRNN_tracktwoLLP_03dRAB_L1TAU60_2TAU40',
  'HLT_tau80_mediumRNN_tracktwoLLP_tau60_tightRNN_tracktwoLLP_03dRAB_L1TAU60_2TAU40',
  'HLT_tau80_tightRNN_tracktwoLLP_tau60_tightRNN_tracktwoLLP_03dRAB_L1TAU60_2TAU40',
  'HLT_tau80_mediumRNN_tracktwoLLP_tau60_mediumRNN_tracktwoLLP_03dRAB_L1eTAU80_2eTAU60',
  'HLT_tau80_mediumRNN_tracktwoLLP_tau60_tightRNN_tracktwoLLP_03dRAB_L1eTAU80_2eTAU60',
  'HLT_tau80_tightRNN_tracktwoLLP_tau60_tightRNN_tracktwoLLP_03dRAB_L1eTAU80_2eTAU60',

  # tau100 + tau80
  'HLT_tau100_mediumRNN_tracktwoLLP_tau80_mediumRNN_tracktwoLLP_03dRAB_L1TAU60_2TAU40',
  'HLT_tau100_mediumRNN_tracktwoLLP_tau80_mediumRNN_tracktwoLLP_03dRAB_L1eTAU80_2eTAU60',
]

mon_tag_and_probe = [
  # Legacy tau+X chains with muon L1
  'HLT_mu24_ivarmedium_tau20_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU14FCH',
  'HLT_mu24_ivarmedium_tau25_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU14FCH',
  'HLT_mu24_ivarmedium_tau80_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU14FCH',
  'HLT_mu24_ivarmedium_tau20_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU18VFCH',
  'HLT_mu24_ivarmedium_tau25_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU18VFCH',
  'HLT_mu24_ivarmedium_tau80_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU18VFCH',

  'HLT_mu26_ivarmedium_tau20_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU14FCH',
  'HLT_mu26_ivarmedium_tau20_mediumRNN_tracktwoMVA_probe_L1TAU12IM_03dRAB_L1MU14FCH',
  'HLT_mu26_ivarmedium_tau25_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU14FCH',
  'HLT_mu26_ivarmedium_tau30_mediumRNN_tracktwoMVA_probe_L1TAU20IM_03dRAB_L1MU14FCH',
  'HLT_mu26_ivarmedium_tau80_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU14FCH',
  'HLT_mu26_ivarmedium_tau20_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU18VFCH',
  'HLT_mu26_ivarmedium_tau25_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU18VFCH',
  'HLT_mu26_ivarmedium_tau80_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU18VFCH',

  # Phase-I tau+X chains with muon L1
  'HLT_mu24_ivarmedium_tau20_mediumRNN_tracktwoMVA_probe_L1eTAU12_03dRAB_L1MU14FCH',
  'HLT_mu24_ivarmedium_tau25_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU14FCH',
  'HLT_mu24_ivarmedium_tau80_mediumRNN_tracktwoMVA_probe_L1eTAU80_03dRAB_L1MU14FCH',
  'HLT_mu24_ivarmedium_tau20_mediumRNN_tracktwoMVA_probe_L1eTAU12_03dRAB_L1MU18VFCH',
  'HLT_mu24_ivarmedium_tau25_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU18VFCH',
  'HLT_mu24_ivarmedium_tau80_mediumRNN_tracktwoMVA_probe_L1eTAU80_03dRAB_L1MU18VFCH',

  'HLT_mu26_ivarmedium_tau20_mediumRNN_tracktwoMVA_probe_L1eTAU12_03dRAB_L1MU14FCH',
  'HLT_mu26_ivarmedium_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU14FCH',
  'HLT_mu26_ivarmedium_tau25_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU14FCH',
  'HLT_mu26_ivarmedium_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU14FCH',
  'HLT_mu26_ivarmedium_tau80_mediumRNN_tracktwoMVA_probe_L1eTAU80_03dRAB_L1MU14FCH',
  'HLT_mu26_ivarmedium_tau20_mediumRNN_tracktwoMVA_probe_L1eTAU12_03dRAB_L1MU18VFCH',
  'HLT_mu26_ivarmedium_tau20_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU18VFCH',
  'HLT_mu26_ivarmedium_tau25_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU18VFCH',
  'HLT_mu26_ivarmedium_tau30_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1MU18VFCH',
  'HLT_mu26_ivarmedium_tau80_mediumRNN_tracktwoMVA_probe_L1eTAU80_03dRAB_L1MU18VFCH',

  # Phase-I tau+X chains with elec L1
  'HLT_e26_lhtight_ivarloose_tau25_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1eEM26M',
  'HLT_e26_lhtight_ivarloose_tau80_mediumRNN_tracktwoMVA_probe_L1eTAU80_03dRAB_L1eEM26M',

  'HLT_e28_lhtight_ivarloose_tau25_mediumRNN_tracktwoMVA_probe_L1cTAU20M_03dRAB_L1eEM28M',
  'HLT_e28_lhtight_ivarloose_tau80_mediumRNN_tracktwoMVA_probe_L1eTAU80_03dRAB_L1eEM28M',

  # tau+X intermediate Phase-I tag, Legacy probe 
  'HLT_e26_lhtight_ivarloose_tau25_mediumRNN_tracktwoMVA_probe_L1TAU12IM_03dRAB_L1eEM26M',
  'HLT_e26_lhtight_ivarloose_tau80_mediumRNN_tracktwoMVA_probe_L1TAU60_03dRAB_L1eEM26M',
  'HLT_e28_lhtight_ivarloose_tau25_mediumRNN_tracktwoMVA_probe_L1TAU12IM_03dRAB_L1eEM28M',
  'HLT_e28_lhtight_ivarloose_tau80_mediumRNN_tracktwoMVA_probe_L1TAU60_03dRAB_L1eEM28M',
]

monitored_chains = mon_singletau + mon_ditau + mon_tag_and_probe