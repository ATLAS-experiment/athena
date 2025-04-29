# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from typing import Optional, Callable, Any
from dataclasses import dataclass, field

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)


@dataclass
class AutoPrescaleSet:
    l1_prescales: dict[str, float] = field(default_factory=dict)
    hlt_prescales: dict[str, float] = field(default_factory=dict)


@dataclass
class AutoPrescaleSetGen:
    '''
    This is an extremely basic prescale set generator. No "smart" optimizations (e.g. prescaling 
    all HLT chains for an L1 trigger -> prescale L1 trigger by lowest prescale) are applied.

    The prescales are set based on the following priority: disable_* >> *_prescales >> enable_*.
    Also, HLT chain >> group (i.e. a group can be disabled while a single chain in that group is enables).
    If the enable_chains/groups/l1_triggers list is not set, all triggers in that category will be enabled by default.

    This is NOT meant to be used in data-taking, only for development tests and standard reprocessings/MC productions.

    For any more complex prescale sets, refer to the `TrigMenuRulebook` (used to create PS sets
    for data-taking), located at: https://gitlab.cern.ch/atlas-trigger-menu/TrigMenuRulebook/
    '''

    enable_chains: Optional[list[str]] = None
    disable_chains: list[str] = field(default_factory=list)
    chain_prescales: dict[str, float] = field(default_factory=dict)

    enable_groups: Optional[list[str]] = None
    disable_groups: list[str] = field(default_factory=list)
    group_prescales: dict[str, float] = field(default_factory=dict)

    enable_l1_triggers: Optional[list[str]] = None
    disable_l1_triggers: list[str] = field(default_factory=list)
    l1_trigger_prescales: dict[str, float] = field(default_factory=dict)
    disable_unused_l1_triggers: bool = False

    custom_rules: Optional[Callable[[AutoPrescaleSet, dict[str, dict[str, Any]]], AutoPrescaleSet]] = None

    def __post_init__(self):
        if self.enable_chains is not None: self.enable_chains = set(self.enable_chains)
        self.disable_chains = set(self.disable_chains)

        if self.enable_groups is not None: self.enable_groups = set(self.enable_groups)
        self.disable_groups = set(self.disable_groups)

        if self.enable_l1_triggers is not None: self.enable_l1_triggers = set(self.enable_l1_triggers)
        self.disable_l1_triggers = set(self.disable_l1_triggers)


    @staticmethod
    def disable_unused_l1(ps_set: AutoPrescaleSet, chain_dicts: dict[str, dict[str, Any]]) -> AutoPrescaleSet:
        '''Disable unused L1 triggers'''
        enabled_l1_triggers = set()
        
        for chain in ps_set.hlt_prescales:
            l1 = chain_dicts[chain]['L1item']
            if ps_set.hlt_prescales[chain] > 0 and l1: # Skip L1_All
                enabled_l1_triggers |= {l1} if isinstance(l1, str) else set(l1)

        for l1_trigger in ps_set.l1_prescales:
            if l1_trigger not in enabled_l1_triggers:
                ps_set.l1_prescales[l1_trigger] = -1

        return ps_set


    @staticmethod
    def store_json_files(flags, ps_set: AutoPrescaleSet):
        # Store the L1/HLTPrescale json files
        from TrigConfigSvc.TrigConfigSvcCfg import createL1PrescalesFileFromMenu
        createL1PrescalesFileFromMenu(flags, ps_set.l1_prescales)

        from TriggerMenuMT.HLT.Config.Utility.HLTMenuConfig import HLTMenuConfig
        from TriggerMenuMT.HLT.Config.JSON.HLTPrescaleJSON import generatePrescaleJSON
        generatePrescaleJSON(flags, HLTMenuConfig.dictsList())


    def generate(self, flags, store: bool = False) -> AutoPrescaleSet:
        # Get list of HLT chain dicts
        from TriggerMenuMT.HLT.Config.Utility.HLTMenuConfig import HLTMenuConfig

        # Get list of L1 triggers
        from TrigConfigSvc.TriggerConfigAccess import getL1MenuAccess
        l1_triggers = getL1MenuAccess(flags).itemNames()

        group_prescales_set = set(self.group_prescales.keys())

        def match(s1: set, s2: set) -> bool: return not s1.isdisjoint(s2)

        # Return PS set
        ps_set = AutoPrescaleSet()

        # Generate HLT prescales first
        for chain, chain_dict in HLTMenuConfig.dicts().items():
            ps = 1

            if chain in self.disable_chains: ps = -1
            elif self.enable_chains is not None and chain not in self.enable_chains: ps = -1
            elif chain in self.chain_prescales: ps = self.chain_prescales[chain]
            elif match(self.disable_groups, chain_dict['groups']): ps = -1
            elif self.enable_groups is not None and not match(self.enable_groups, chain_dict['groups']): ps = -1
            elif match(group_prescales_set, chain_dict['groups']):
                # Apply the PS of the last matched group
                for group in reversed(chain_dict['groups']):
                    if group in self.group_prescales:
                        ps = self.group_prescales[group]
                        break
            elif chain_dict['L1item']: # Skips the L1_All trigger (set to '')
                l1s = {chain_dict['L1item']} if isinstance(chain_dict['L1item'], str) else set(chain_dict['L1item'])
            
                if l1s.issubset(self.disable_l1_triggers): ps = -1
                elif self.enable_l1_triggers is not None and not l1s & self.enable_l1_triggers: ps = -1
            
            if ps <= 0: ps = -1

            ps_set.hlt_prescales[chain] = ps

        # Now generate L1 prescales
        for l1_trigger in l1_triggers:
            ps = 1
            if l1_trigger in self.disable_l1_triggers: ps = -1
            elif self.enable_l1_triggers is not None and l1_trigger not in self.enable_l1_triggers: ps = -1
            elif l1_trigger in self.l1_trigger_prescales: ps = self.l1_trigger_prescales[l1_trigger]

            if ps <= 0: ps = -1

            ps_set.l1_prescales[l1_trigger] = ps

        if self.disable_unused_l1_triggers:
            ps_set = self.disable_unused_l1(ps_set, HLTMenuConfig.dicts())

        # Apply any custom rules, if required
        if self.custom_rules:
            ps_set = self.custom_rules(ps_set, HLTMenuConfig.dicts())


        # Apply HLT prescales to the HLTMenuConfig
        for chain, ps in ps_set.hlt_prescales.items():
            if ps != 1: log.info(f'Applying HLTPS to the chain {chain}: {ps}')
            HLTMenuConfig.dicts()[chain]['prescale'] = ps

        # Store the prescale JSON files
        if store: self.store_json_files(flags, ps_set)

        return ps_set

