# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# TriggerListsHelper: helper class which retrieves the full set of triggers needed for
# trigger matching in the DAODs and can then return them when needed. 

from TriggerMenuMT.TriggerAPI import TriggerAPISession, TriggerType, TriggerAPI, TriggerPeriod
from PathResolver import PathResolver
from AthenaConfiguration.AutoConfigFlags import GetFileMD

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

def getTapisSession(flags):
    from Campaigns.Utils import Campaign
    from AthenaConfiguration.Enums import Format
    yearStr = str(flags.Input.DataYear) if not flags.Input.isMC else ""
    if flags.Input.isMC:
        if flags.Input.MCCampaign == Campaign.MC16a or flags.Input.MCCampaign == Campaign.MC20a:
            yearStr = "2015_2016" 
        elif flags.Input.MCCampaign == Campaign.MC16d or flags.Input.MCCampaign == Campaign.MC20d:
            yearStr = "2017"
        elif flags.Input.MCCampaign == Campaign.MC16e or flags.Input.MCCampaign == Campaign.MC20e:
            yearStr = "2018"
        elif flags.Input.MCCampaign == Campaign.MC23a or flags.Input.MCCampaign == Campaign.MC21a:
            yearStr = "2022"
        elif flags.Input.MCCampaign == Campaign.MC23c or flags.Input.MCCampaign == Campaign.MC23d:
            yearStr = "2023"
        elif flags.Input.MCCampaign == Campaign.MC23e:
            yearStr = "2024"

    session_files = {
        "2015": "TriggerAPISessions/tapis_data15_13TeV_20190708_PHYS_StandardGRL_All_Good_25ns.json",
        "2016": "TriggerAPISessions/tapis_data16_13TeV_20190708_PHYS_StandardGRL_All_Good_25ns_WITH_IGNORES.json",
        "2015_2016": "TriggerAPISessions/tapis_data15_13TeV_20190708_PHYS_StandardGRL_All_Good_25ns_data16_13TeV_20190708_PHYS_StandardGRL_All_Good_25ns_WITH_IGNORES.json",
        "2017": "TriggerAPISessions/tapis_data17_13TeV_20190708_PHYS_StandardGRL_All_Good_25ns_Triggerno17e33prim.json",
        "2018": "TriggerAPISessions/tapis_data18_13TeV_20190708_PHYS_StandardGRL_All_Good_25ns_Triggerno17e33prim.json",
        "2022": "TriggerAPISessions/tapis_data22_13p6TeV_20230207_PHYS_StandardGRL_All_Good_25ns.json",
        "2023": "TriggerAPISessions/tapis_data23_13p6TeV_20230828_PHYS_StandardGRL_All_Good_25ns.json",
        "2024": "TriggerAPISessions/tapis_data24_13p6TeV_20241118_PHYS_StandardGRL_All_Good_25ns.json",
    }

    if yearStr in session_files:
        return TriggerAPISession(json=session_files[yearStr])

    # Otherwise: data24, MC23e, phase-II. Base this on the menu from the AOD.
    if flags.Input.Format == Format.POOL:
        return TriggerAPISession(file=flags.Input.Files[0])

    from AthenaCommon.Logging import logging
    logging.getLogger('TriggerListHelper::GetTriggerLists::getTapisSession').error('Failed to obtain a session.')
    return None


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

            # TriggerAPI Session based trigger lists
            session = getTapisSession(self.flags)
            lf = 0.8 # Prescale weighted life fraction of the GRL's LBs 
            api_trigger_names = set()
            api_trigger_names = session.getLowestUnprescaled(triggerType=TriggerType.el, livefraction=lf).union(api_trigger_names)
            api_trigger_names = session.getLowestUnprescaled(triggerType=TriggerType.mu, livefraction=lf).union(api_trigger_names)
            api_trigger_names = session.getLowestUnprescaled(triggerType=TriggerType.g, livefraction=lf).union(api_trigger_names)
            api_trigger_names = session.getLowestUnprescaled(triggerType=TriggerType.tau, livefraction=lf).union(api_trigger_names)
            ## Add Run 2 cross-triggers for some sets
            api_trigger_names = session.getLowestUnprescaled(triggerType=[TriggerType.el,  TriggerType.mu], livefraction=lf).union(api_trigger_names)
            api_trigger_names = session.getLowestUnprescaled(triggerType=[TriggerType.el,  TriggerType.tau], livefraction=lf).union(api_trigger_names)
            api_trigger_names = session.getLowestUnprescaled(triggerType=[TriggerType.mu,  TriggerType.tau], livefraction=lf).union(api_trigger_names)
            api_trigger_names = session.getLowestUnprescaled(triggerType=[TriggerType.tau, TriggerType.xe], livefraction=lf).union(api_trigger_names)
            ## Add additional cross-trigger categories identified from investigating API output
            api_trigger_names = session.getLowestUnprescaled(triggerType=[TriggerType.el,  TriggerType.g], livefraction=lf).union(api_trigger_names)
            api_trigger_names = session.getLowestUnprescaled(triggerType=[TriggerType.el,  TriggerType.xe], livefraction=lf).union(api_trigger_names)
            api_trigger_names = session.getLowestUnprescaled(triggerType=[TriggerType.mu,  TriggerType.g], livefraction=lf).union(api_trigger_names)
            api_trigger_names = session.getLowestUnprescaled(triggerType=[TriggerType.g,   TriggerType.xe], livefraction=lf).union(api_trigger_names)
            api_trigger_names = session.getLowestUnprescaled(triggerType=[TriggerType.tau, TriggerType.g], livefraction=lf).union(api_trigger_names)

            # Add hadronic categories, new for Run 3
            api_trigger_names = session.getLowestUnprescaled(triggerType=TriggerType.j, livefraction=lf).union(api_trigger_names)
            api_trigger_names = session.getLowestUnprescaled(triggerType=TriggerType.bj, livefraction=lf).union(api_trigger_names)
            # Add cross-triggers as well
            api_trigger_names = session.getLowestUnprescaled(triggerType=[TriggerType.el, TriggerType.j], livefraction=lf).union(api_trigger_names)
            api_trigger_names = session.getLowestUnprescaled(triggerType=[TriggerType.mu, TriggerType.j], livefraction=lf).union(api_trigger_names)
            api_trigger_names = session.getLowestUnprescaled(triggerType=[TriggerType.j,  TriggerType.bj], livefraction=lf).union(api_trigger_names)
            api_trigger_names = session.getLowestUnprescaled(triggerType=[TriggerType.j,  TriggerType.g], livefraction=lf).union(api_trigger_names)
            api_trigger_names = session.getLowestUnprescaled(triggerType=[TriggerType.j,  TriggerType.tau], livefraction=lf).union(api_trigger_names)
            api_trigger_names = session.getLowestUnprescaled(triggerType=[TriggerType.j,  TriggerType.xe], livefraction=lf).union(api_trigger_names)
            api_trigger_names = session.getLowestUnprescaled(triggerType=[TriggerType.j,  TriggerType.ht], livefraction=lf).union(api_trigger_names)
            api_trigger_names = session.getLowestUnprescaled(triggerType=[TriggerType.bj, TriggerType.xe], livefraction=lf).union(api_trigger_names)

            ## Add extra chains from flags
            extra_flag_notau, extra_flag_tau = read_trig_list_flags(self.flags)
            extra_flag = extra_flag_notau + extra_flag_tau

            ## Add extra chains from file
            extra_file = read_trig_list_file("DerivationFrameworkPhys/run3ExtraMatchingTriggers.txt")

            ## Merge and remove duplicates
            self.Run3TriggerNames = list(set(extra_file + extra_flag + list(api_trigger_names)))

        return
