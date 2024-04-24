# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#==============================================================================
# Provides configs for the tools used for SUSY Derivations
#==============================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# SUSY20 trigger skimming
def SUSY20DTTriggerSkimmingToolCfg(flags, name, **kwargs):

	"""Configure the DT trigger skimming tool for SUSY20 derivations"""
	
	from TriggerMenuMT.TriggerAPI.TriggerAPI import TriggerAPI
	from TriggerMenuMT.TriggerAPI.TriggerEnums import TriggerPeriod, TriggerType

	allperiods = TriggerPeriod.y2015 | TriggerPeriod.y2016 | TriggerPeriod.y2017 | TriggerPeriod.y2018 | TriggerPeriod.future2e34
	
	TriggerAPI.setConfigFlags(flags)
	el_trig = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType = TriggerType.el_single, livefraction = 0.8)
	mu_trig = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType = TriggerType.mu_single, livefraction = 0.8)
	g_trig  = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType = TriggerType.g_single,  livefraction = 0.8)
	xe_trig = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType = TriggerType.xe,        livefraction = 0.8) 

	# Additional unprescaled met trigger for 2015
	xe_trig += ['HLT_xe70', 'HLT_xe70_tc_lcw', 'HLT_noalg_L1J400', 'HLT_noalg_L1J420', 'HLT_noalg_L1J450']

	triggers = el_trig + mu_trig + g_trig + xe_trig
	triggers = sorted(list(set(triggers)))

	acc = ComponentAccumulator()
	TriggerSkimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool
	acc.addPublicTool(TriggerSkimmingTool
		(
			name,
			TriggerListAND = [],
			TriggerListOR  = triggers,
			**kwargs
		),
		primary = True
	)			
	
	return acc