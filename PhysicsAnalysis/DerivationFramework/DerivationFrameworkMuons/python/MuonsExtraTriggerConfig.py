# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from TriggerMenuMT.TriggerAPI.TriggerAPI import TriggerAPI
from TriggerMenuMT.TriggerAPI.TriggerEnums import TriggerPeriod, TriggerType
from AthenaConfiguration.AutoConfigFlags import GetFileMD
from DerivationFrameworkPhys.TriggerListsHelper import read_trig_list_file
from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
from DerivationFrameworkPhys.TriggerMatchingCommonConfig import TriggerMatchingCommonRun2Cfg


# Helper class to add extra trigger content to the Muon derivations. 
class MuonExtraTriggerHelper:
    def __init__(self, flags, existingTriggerHelper):
        self.flags = flags
        TriggerAPI.setConfigFlags(flags)
        self.extraChainList = []
        self.existingTriggerHelper = existingTriggerHelper
        self.FindExtraChainsRun2()

    # Find the chains missing in the existing trigger helper 
    def FindExtraChainsRun2(self):

        md = GetFileMD(self.flags.Input.Files)
        hlt_menu = md.get('TriggerMenu', {}).get('HLTChains', None)

        if self.flags.Trigger.EDMVersion <= 2:

            # Trigger API for Runs 1, 2
            allperiods = TriggerPeriod.y2015 | TriggerPeriod.y2016 | TriggerPeriod.y2017 | TriggerPeriod.y2018 | TriggerPeriod.future2e34
            trig_bmu  = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.mu_bphys,  livefraction=0.8)

            ## Allow for an additional include-list without polluting PHYS 
            extra_file = read_trig_list_file("DerivationFrameworkMuons/run2ExtraMatchingTriggers.txt")

            ## Merge and remove duplicates
            trigger_names_full = list(set(trig_bmu+extra_file))
            ## Now reduce the list based on the content of the first input AOD
            if hlt_menu:
                for chain_name in hlt_menu:
                    if chain_name in trigger_names_full: 
                        self.extraChainList.append(chain_name)
            else: # No means to filter based on in-file metadata
                self.extraChainList = trigger_names_full
            
            # remove overlap with the existing chains 
            self.extraChainList = [c for c in self.extraChainList if c not in self.existingTriggerHelper.Run2TriggerNamesNoTau and c not in self.existingTriggerHelper.Run2TriggerNamesTau ] 
    
    # schedule the trigger matching tool for our new chains
    def Run2MatchAugmentationCfg(self, flags, name="PhysCommonTrigMatchExtraMuons"):
        acc = ComponentAccumulator()
        acc.merge(TriggerMatchingCommonRun2Cfg(
        flags,
        name = name,
        OutputContainerPrefix = "TrigMatch_",
        ChainNames = self.extraChainList))
        return acc

    # add our new match branches to the slimming helper
    def AddRun2MatchingToSlimmingHelper(self,SlimmingHelper, 
                                        OutputContainerPrefix = "TrigMatch_"):
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = SlimmingHelper, 
                                        OutputContainerPrefix = OutputContainerPrefix,
                                        TriggerList = self.extraChainList)
        