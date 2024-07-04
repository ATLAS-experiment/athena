# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from TriggerMenuMT.TriggerAPI.TriggerAPI import TriggerAPI
from TriggerMenuMT.TriggerAPI.TriggerEnums import TriggerPeriod, TriggerType
from AthenaConfiguration.AutoConfigFlags import GetFileMD
from AthenaConfiguration.Enums import LHCPeriod
import re
from AthenaCommon.Logging import logging
log = logging.getLogger('DAODJetEtMissTriggerList')

# jet triggers (single Jets, prescaled and unprescaled)
def jetTrig(flags):
	period2015tofuture = TriggerPeriod.y2015 | TriggerPeriod.y2016 | TriggerPeriod.y2017 | TriggerPeriod.y2018 | TriggerPeriod.future
	period2017tofuture = TriggerPeriod.y2017 | TriggerPeriod.y2018 | TriggerPeriod.future
	TriggerAPI.setConfigFlags(flags)
	API_jetTrig = TriggerAPI.getActive(period2015tofuture, TriggerType.j_single)
	# Large-radius multijet triggers
	API_jetTrig += TriggerAPI.getActive(period2017tofuture,TriggerType.j_multi,matchPattern=".*_a10t_.*")
	# Eta-intercalibration triggers (one central, one forward jet)
	API_jetTrig += TriggerAPI.getActive(period2015tofuture,TriggerType.j_multi,matchPattern="HLT_j.*_320eta490")
	return API_jetTrig

# electron triggers (unprescaled)
def single_el_Trig(flags):
	TriggerAPI.setConfigFlags(flags)
	API_singleElTriggers = TriggerAPI.getLowestUnprescaled(TriggerPeriod.y2015, TriggerType.el_single) + TriggerAPI.getLowestUnprescaled(TriggerPeriod.y2016, TriggerType.el_single) + TriggerAPI.getLowestUnprescaled(TriggerPeriod.y2017, TriggerType.el_single) + TriggerAPI.getLowestUnprescaled(TriggerPeriod.y2018, TriggerType.el_single) + TriggerAPI.getLowestUnprescaled(TriggerPeriod.future, TriggerType.el_single)
	return API_singleElTriggers

def multi_el_Trig(flags):
	TriggerAPI.setConfigFlags(flags)
	API_multiElTriggers = TriggerAPI.getLowestUnprescaled(TriggerPeriod.y2015, TriggerType.el_multi) + TriggerAPI.getLowestUnprescaled(TriggerPeriod.y2016, TriggerType.el_multi) + TriggerAPI.getLowestUnprescaled(TriggerPeriod.y2017, TriggerType.el_multi) + TriggerAPI.getLowestUnprescaled(TriggerPeriod.y2018, TriggerType.el_multi) + TriggerAPI.getLowestUnprescaled(TriggerPeriod.future, TriggerType.el_multi)
	return API_multiElTriggers

# single muon triggers (unprescaled)
def single_mu_Trig(flags):
	TriggerAPI.setConfigFlags(flags)
	API_singleMuTriggers = TriggerAPI.getLowestUnprescaled(TriggerPeriod.y2015, TriggerType.mu_single) + TriggerAPI.getLowestUnprescaled(TriggerPeriod.y2016, TriggerType.mu_single) + TriggerAPI.getLowestUnprescaled(TriggerPeriod.y2017, TriggerType.mu_single) + TriggerAPI.getLowestUnprescaled(TriggerPeriod.y2018, TriggerType.mu_single) + TriggerAPI.getLowestUnprescaled(TriggerPeriod.future, TriggerType.mu_single)
	return API_singleMuTriggers

def multi_mu_Trig(flags):
	TriggerAPI.setConfigFlags(flags)
	API_multiMuTriggers = TriggerAPI.getLowestUnprescaled(TriggerPeriod.y2015, TriggerType.mu_multi) + TriggerAPI.getLowestUnprescaled(TriggerPeriod.y2016, TriggerType.mu_multi) + TriggerAPI.getLowestUnprescaled(TriggerPeriod.y2017, TriggerType.mu_multi) + TriggerAPI.getLowestUnprescaled(TriggerPeriod.y2018, TriggerType.mu_multi) + TriggerAPI.getLowestUnprescaled(TriggerPeriod.future, TriggerType.mu_multi)
	return API_multiMuTriggers

# xe triggers (unprescaled)
def MET_Trig(flags):
	TriggerAPI.setConfigFlags(flags)
	API_MET_Triggers = TriggerAPI.getLowestUnprescaled(TriggerPeriod.y2015, TriggerType.xe) + TriggerAPI.getLowestUnprescaled(TriggerPeriod.y2016, TriggerType.xe) + TriggerAPI.getLowestUnprescaled(TriggerPeriod.y2017, TriggerType.xe) + TriggerAPI.getLowestUnprescaled(TriggerPeriod.y2018, TriggerType.xe) + TriggerAPI.getLowestUnprescaled(TriggerPeriod.future, TriggerType.xe)
	return API_MET_Triggers

# photon triggers (prescaled and unprescaled)
def single_photon_Trig(flags):
	TriggerAPI.setConfigFlags(flags)
	API_singlePhotonTriggers = TriggerAPI.getActive(TriggerPeriod.y2015 | TriggerPeriod.y2016 | TriggerPeriod.y2017 | TriggerPeriod.y2018 | TriggerPeriod.future, TriggerType.g_single)
	return API_singlePhotonTriggers

# get jet triggers
# Run 2: returns list returned by trigAPI
# Run 3: TrigAPI doesn't return complete list. Adds additional jet triggers from file metadata
# and filters out unnecessary chains - ATLJETMET-1717.
# Use this function until trigAPI works for Run 3.
def get_jetTrig(flags):

	trigger_list = jetTrig(flags)

	if flags.GeoModel.Run < LHCPeriod.Run3:
		return trigger_list

	pattern = re.compile(r"HLT_\d*j\d+")
	vetoes = ['PhysicsTLA', 'calratio', 'emergingPTF', 'dispjet', 'trackless', 'hitdvjet', 'bgn1', 'bgn2', 'bdl1', 'boffperf', 'xe', 'afprec', 'LArPEBHLT', 'TAU', 'XE', 'L1jLJ']

	# retrieve trigger list from input file metadata
	# and add to trigger_list from trigAPI
	metadata = GetFileMD(flags.Input.Files)
	menu = metadata['TriggerMenu']
	if not menu:
		log.error("TriggerMenu could not be retrieved from file metadata.")
	else:
		HLTChainskey = 'HLTChains'
		if HLTChainskey not in menu:
			log.error("HLTChains could not be retrieved from file metadata.")
		else:
			md_hltchains = menu[HLTChainskey]
			log.info(f"{len(md_hltchains)} HLT chains retrieved from file metadata")
			trigger_list += md_hltchains
	
	filtered_j_chains = []
	for chain in set(trigger_list):
		if not pattern.match(chain): continue
		if any(v in chain for v in vetoes): continue
		filtered_j_chains.append(chain)

	return filtered_j_chains
