# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# ********************************************************************
# HIGG1TriggerContent.py
# 
# Configures content on HIGG1D2 derivations needed for trigger skimming
# ********************************************************************


# List of electron, muon and photon triggers
# See https://twiki.cern.ch/twiki/bin/view/Atlas/LowestUnprescaled
expressionTriggers = {}

expressionTriggers['Run3'] = [
    ###from 2023###
    "HLT_g25_medium_L1eEM18L_mu24_L1MU14FCH", #photon muon
    "HLT_g35_loose_mu15_mu2noL1_L1eEM28M", # photon dimuon
    "HLT_g35_loose_mu18_L1eEM28M", # photon muon
    "HLT_e26_lhtight_ivarloose_L1eEM26M", #single electron eEM
    "HLT_e60_lhmedium_L1eEM26M", #single electron eEM
    "HLT_e140_lhloose_L1eEM26M", #single electron eEM
    "HLT_2e17_lhvloose_L12eEM18M", #di-electron eEM
    "HLT_2e24_lhvloose_L12eEM24L", #di-electron eEM
    "HLT_e25_mergedtight_g35_medium_90invmAB_02dRAB_L12eEM24L", #merged electron eEM
    "HLT_e24_lhmedium_g25_medium_02dRAB_L12eEM24L", #photon+electron eEM
    "HLT_g35_medium_g25_medium_L12eEM24L", # di photon eEM
    "HLT_2g50_loose_L12eEM24L", # di photon eEM
    "HLT_g140_loose_L1eEM26M", #single photon
    "HLT_g300_etcut_L1eEM26M", #single photon
    ###from 2022###
    "HLT_g25_medium_mu24_L1MU14FCH", #photon muon
    "HLT_g35_loose_mu15_mu2noL1_L1EM24VHI", #photon dimuon
    "HLT_g35_loose_mu18_L1EM24VHI", #photon muon
    "HLT_2mu14_L12MU8F", #dimuon
    "HLT_mu22_mu8noL1_L1MU14FCH", #dimuon
    "HLT_mu24_ivarmedium_L1MU14FCH", #single muon
    "HLT_mu50_L1MU14FCH",
    "HLT_mu60_0eta105_msonly_L1MU14FCH",
    "HLT_e26_lhtight_ivarloose_L1EM22VHI", #single electron
    "HLT_e60_lhmedium_L1EM22VHI",
    "HLT_e140_lhloose_L1EM22VHI",
    "HLT_2e17_lhvloose_L12EM15VHI", #dielectron
    "HLT_2e24_lhvloose_L12EM20VH",
    "HLT_e25_mergedtight_g35_medium_90invmAB_02dRAB_L12EM20VH", #merged electron
    "HLT_e24_lhmedium_g25_medium_02dRAB_L12EM20VH", #photon+electron
    "HLT_g35_medium_g25_medium_L12EM20VH", #di photon
    "HLT_2g50_loose_L12EM20VH", #di photon
    "HLT_g140_loose_L1EM22VHI", #single photon
    "HLT_g300_etcut_L1EM22VHI" #single photon
]

expressionTriggers['Run2'] = [
    ###2017-18###
    "HLT_g25_medium_mu24", #photon muon
    "HLT_g15_loose_2mu10_msonly", #photon dimuon
    "HLT_g35_loose_L1EM24VHI_mu15_mu2noL1", # photon dimuon
    "HLT_g35_loose_L1EM24VHI_mu18",  #photon muon
    "HLT_g35_tight_icalotight_L1EM24VHI_mu15noL1_mu2noL1", # photon dimuon
    "HLT_g35_tight_icalotight_L1EM24VHI_mu18noL1", # photon muon
    "HLT_2mu14", #dimuon
    "HLT_mu22_mu8noL1", #dimuon
    "HLT_mu26_ivarmedium", #single muon
    "HLT_mu50",
    "HLT_mu60_0eta105_msonly",
    "HLT_e26_lhtight_nod0_ivarloose", #single electron
    "HLT_e60_lhmedium_nod0",
    "HLT_e140_lhloose_nod0",
    "HLT_2e17_lhvloose_nod0_L12EM15VHI", #dielectron
    "HLT_2e24_lhvloose_nod0",
    "HLT_e25_mergedtight_g35_medium_Heg", #merged electron
    "HLT_e30_mergedtight_g35_medium_Heg",
    "HLT_e24_lhmedium_nod0_L1EM20VH_g25_medium",
    "HLT_g35_medium_g25_medium_L12EM20VH",
    "HLT_2g50_loose_L12EM20VH", #di photon
    "HLT_g140_loose", #single photon
    "HLT_g200_loose",
    "HLT_g300_etcut",  #single photon
    ###2016###
    "HLT_g25_medium_mu24", #photon muon
    "HLT_g15_loose_2mu10_msonly", #photon dimuon
    "HLT_g35_loose_L1EM22VHI_mu15noL1_mu2noL1", # photon dimuon
    "HLT_g35_loose_L1EM22VHI_mu18noL1",  #photon muon
    "HLT_2mu10",
    "HLT_2mu14", #dimuon
    "HLT_mu22_mu8noL1", #dimuon
    "HLT_mu20_mu8noL1",
    "HLT_mu24_ivarmedium", #single muon
    "HLT_mu26_ivarmedium",
    "HLT_mu50",
    "HLT_e24_lhtight_nod0_ivarloose", #single electron
    "HLT_e26_lhtight_nod0_ivarloose",
    "HLT_e60_lhmedium_nod0",
    "HLT_e60_medium",
    "HLT_e140_lhloose_nod0",
    "HLT_2e15_lhvloose_nod0_L12EM13VHI", #dielectron
    "HLT_2e17_lhvloose_nod0",
    "HLT_g35_loose_g25_loose", #di photon
    "HLT_g35_medium_g25_medium",
    "HLT_e20_lhmedium_nod0_g35_loose", # electron  +  photon
    "HLT_g140_loose", #single photon
    "HLT_g300_etcut",  #single photon
    ###2015###
    "HLT_g25_medium_mu24", #photon muon
    "HLT_g15_loose_2mu10_msonly", #photon dimuon
    "HLT_2mu10", #dimuon
    "HLT_mu18_mu8noL1", #dimuon
    "HLT_mu20_iloose_L1MU15", #single muon
    "HLT_mu40",
    "HLT_mu60_0eta105_msonly",
    "HLT_e24_lhmedium_L1EM20VH", #single electron
    "HLT_e60_lhmedium",
    "HLT_e120_lhloose",
    "HLT_2e12_lhloose_L12EM10VH", #dielectron
    "HLT_e20_lhmedium_g35_loose", #electron + photon
    "HLT_g35_loose_g25_loose", #di photon
    "HLT_g120_loose", #single photon
    "HLT_g200_etcut"  #single photon
]


# List of electron and photon triggers for events with merged electrons
# See https://twiki.cern.ch/twiki/bin/view/Atlas/LowestUnprescaled
mergedTriggers = {}

mergedTriggers['Run3'] = [
    ###2023###
    "HLT_e25_mergedtight_g35_medium_90invmAB_02dRAB_L12eEM24L", #merged electron eEM
    "HLT_g35_medium_g25_medium_L12eEM24L", #di-photon eEM
    "HLT_2g50_loose_L12eEM24L", #di-photon eEM
    "HLT_e24_lhmedium_g25_medium_02dRAB_L12eEM24L", #photon+electron eEM
    ###2022###
    "HLT_e25_mergedtight_g35_medium_90invmAB_02dRAB_L12EM20VH", #merged electron
    "HLT_g35_medium_g25_medium_L12EM20VH", #di photon
    "HLT_2g50_loose_L12EM20VH", #di photon
    "HLT_e24_lhmedium_g25_medium_02dRAB_L12EM20VH" #photon+electron
]

mergedTriggers['Run2'] = [
    ###2017-18###
    "HLT_e25_mergedtight_g35_medium_Heg","HLT_e30_mergedtight_g35_medium_Heg","HLT_2g50_loose_L12EM20VH","HLT_e24_lhmedium_nod0_L1EM20VH_g25_medium","HLT_g35_medium_g25_medium_L12EM20VH",
    ###2016###
    "HLT_g35_loose_g25_loose","HLT_g35_medium_g25_medium","HLT_e20_lhmedium_nod0_g35_loose",
    ###2015###
    "HLT_g35_loose_g25_loose","HLT_e20_lhmedium_g35_loose"
]
