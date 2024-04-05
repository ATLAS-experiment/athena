# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# TriggerListsHelper: helper class which retrieves the full set of triggers needed for
# trigger matching in the DAODs and can then return them when needed. 

from TriggerMenuMT.TriggerAPI.TriggerAPI import TriggerAPI
from TriggerMenuMT.TriggerAPI.TriggerEnums import TriggerPeriod, TriggerType
from PathResolver import PathResolver
from AthenaConfiguration.AutoConfigFlags import GetFileMD
import re

def read_trig_list_file(fname):
   """Read a text file containing a list of triggers
   Returns the list of triggers held in the file
   """
   triggers = []
   with open(PathResolver.FindCalibFile(fname)) as fp:
      for line in fp:
         line = line.strip()
         if line == "" or line.startswith("#"):
            continue
         triggers.append(line)
   return triggers

def read_trig_list_flags(flags):
    trigger_names_notau = []
    trigger_names_tau = []
    for chain_name in flags.Trigger.derivationsExtraChains:
        if "tau" in chain_name:
            trigger_names_tau.append(chain_name)
        else:
            trigger_names_notau.append(chain_name)
    return (trigger_names_notau, trigger_names_tau)

class TriggerListsHelper:
    def __init__(self, flags):
        self.flags = flags
        TriggerAPI.setConfigFlags(flags)
        self.Run2TriggerNamesTau = []
        self.Run2TriggerNamesNoTau = []
        self.Run3TriggerNames = []
        self.GetTriggerLists()

    def GetTriggerLists(self):

        md = GetFileMD(self.flags.Input.Files)
        hlt_menu = md.get('TriggerMenu', {}).get('HLTChains', None)

        if self.flags.Trigger.EDMVersion <= 2:

            # Trigger API for Runs 1, 2
            #====================================================================
            # TRIGGER CONTENT
            #====================================================================
            ## See https://twiki.cern.ch/twiki/bin/view/Atlas/TriggerAPI
            ## Get single and multi mu, e, photon triggers
            ## Jet, tau, multi-object triggers not available in the matching code
            allperiods = TriggerPeriod.y2015 | TriggerPeriod.y2016 | TriggerPeriod.y2017 | TriggerPeriod.y2018 | TriggerPeriod.future2e34
            trig_el  = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.el,  livefraction=0.8)
            trig_mu  = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.mu,  livefraction=0.8)
            trig_g   = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.g,   livefraction=0.8)
            trig_tau = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.tau, livefraction=0.8)
            ## Add cross-triggers for some sets
            trig_em = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.el, additionalTriggerType=TriggerType.mu,  livefraction=0.8)
            trig_et = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.el, additionalTriggerType=TriggerType.tau, livefraction=0.8)
            trig_mt = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.mu, additionalTriggerType=TriggerType.tau, livefraction=0.8)
            # Note that this seems to pick up both isolated and non-isolated triggers already, so no need for extra grabs
            trig_txe = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.tau, additionalTriggerType=TriggerType.xe, livefraction=0.8)

            ## Add extra chains which were specified via repository include-lists
            extra_file_notau = read_trig_list_file("DerivationFrameworkPhys/run2ExtraMatchingTriggers.txt")
            extra_file_tau = read_trig_list_file("DerivationFrameworkPhys/run2ExtraMatchingTauTriggers.txt")

            ## Add extra chains from flags
            extra_flag_notau, extra_flag_tau = read_trig_list_flags(self.flags)

            ## Merge and remove duplicates
            trigger_names_full_notau = list(set(trig_el+trig_mu+trig_g+trig_em+trig_et+trig_mt+extra_file_notau+extra_flag_notau))
            trigger_names_full_tau = list(set(trig_tau+trig_txe+extra_file_tau+extra_flag_tau))

            ## Now reduce the list based on the content of the first input AOD
            trigger_names_notau = []
            trigger_names_tau = []

            if hlt_menu:
                for chain_name in hlt_menu:
                    if chain_name in trigger_names_full_notau: trigger_names_notau.append(chain_name)
                    if chain_name in trigger_names_full_tau:   trigger_names_tau.append(chain_name)
            else: # No means to filter based on in-file metadata
                trigger_names_notau = trigger_names_full_notau
                trigger_names_tau   = trigger_names_full_tau

            self.Run2TriggerNamesNoTau = trigger_names_notau
            self.Run2TriggerNamesTau = trigger_names_tau

        else: # Run 3 and Run 4

            ## TODO, Trigger API missing for Run 3 - currently wildcard

            ## NOTE: The splitting of tau vs. no-tau only needs to be maintained so long as we want to be able to
            ##       run the run-2 style matching during derivation over the run-3 trigger lists.

            r_tau = re.compile("HLT_.*tau.*")
            r_notau = re.compile("HLT_[1-9]*(e|mu|g|j).*")

            trigger_names_notau = []
            trigger_names_tau = []
            if hlt_menu:
                for chain_name in hlt_menu:
                    result_tau = r_tau.match(chain_name)
                    result_notau = r_notau.match(chain_name)
                    if result_tau is not None: trigger_names_tau.append(chain_name)
                    if result_notau is not None: trigger_names_notau.append(chain_name)

            ## Add extra chains from flags
            extra_flag_notau, extra_flag_tau = read_trig_list_flags(self.flags)

            ## Add extra chains from file (dropping the tau vs. non-tau distinction at this point)
            extra_file = read_trig_list_file("DerivationFrameworkPhys/run3ExtraMatchingTriggers.txt")

            ## Merge and remove duplicates
            trigger_names_full_notau = list(set(trigger_names_notau+extra_file+extra_flag_notau))
            trigger_names_full_tau = list(set(trigger_names_tau+extra_flag_tau))
            trigger_names_full_all = list(set.union(set(trigger_names_full_notau), set(trigger_names_full_tau)))

            self.Run3TriggerNames = trigger_names_full_all
            self.Run3TriggerNamesNoTau = trigger_names_full_notau
            self.Run3TriggerNamesTau = trigger_names_full_tau

        return
