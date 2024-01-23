# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
import itertools

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.CFElements import seqAND
from TriggerMenuMT.HLT.Config.Utility.HLTMenuConfig import HLTMenuConfig
from TriggerMenuMT.HLT.Config.Utility.MenuAlignmentTools import MenuAlignment
from TriggerMenuMT.HLT.Menu.MenuPrescaleConfig import applyHLTPrescale
from TriggerMenuMT.HLT.Menu.MenuPrescaleConfig import MenuPrescaleConfig

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)


_isCAMenu = False

def isCAMenu():
  return _isCAMenu


def doMenuAlignment(chains):
    """
    Invoke menu alignment procedures and register aligned chains in the HLTMenuConfig

    Input is a list of pairs, (chain dict, chain config)
    """
                        
    groups = [c[0]['alignmentGroups'] for c in chains]
    log.info('Alignment Combinations %s', groups)
    
    alignmentCombinations = set([tuple(set(g)) for g in groups if len(set(g)) > 1])
    log.info('Alignment reduced Combinations %s', alignmentCombinations)
    alignmentGroups=set(list(itertools.chain(*alignmentCombinations)))
    log.info('Alignment Groups %s', alignmentGroups)

    alignmentLengths = dict.fromkeys(list(itertools.chain(*groups)), 0)
    
    for chainDict, chainConfig in chains:
        for group, clen in chainDict['alignmentLengths'].items():
            alignmentLengths[group] = max(alignmentLengths[group], clen)
        del chainDict['alignmentLengths']
    log.info('Alignment Lengths %s', alignmentLengths)

    menuAlignment = MenuAlignment(alignmentCombinations,
                                  alignmentGroups,
                                  alignmentLengths)
    menuAlignment.analyse_combinations()

    reverseAlignmentLengths = [ el[::-1] for el in alignmentLengths.items()]
    for chainDict, chainConfig in chains:
        # needs to match up with the maximum number of steps in a signature in the menu (length_of_configs)
        alignmentGroups = chainDict['alignmentGroups']
        #parallel-merged single-signature chains or single signature chains. Anything that needs no splitting!
        if len(set(alignmentGroups)) == 1:
            alignedChainConfig = menuAlignment.single_align(chainDict, chainConfig)
        elif len(alignmentGroups) == 2:
            alignedChainConfig = menuAlignment.multi_align(chainDict, chainConfig, reverseAlignmentLengths)
        else:
            assert False, "Do not handle more than one calignment group"
        HLTMenuConfig.registerChain(chainDict, alignedChainConfig)


class FilterChainsToGenerate(object):
    """
    class to use filters for chains
    """
    def __init__(self,flags):
        self.enabledSignatures  = flags.Trigger.enabledSignatures  if flags.hasFlag("Trigger.enabledSignatures") else []
        self.disabledSignatures = flags.Trigger.disabledSignatures if flags.hasFlag("Trigger.disabledSignatures") else []
        self.selectChains       = flags.Trigger.selectChains       if flags.hasFlag("Trigger.selectChains") else []
        self.disableChains      = flags.Trigger.disableChains      if flags.hasFlag("Trigger.disableChains") else []          
    def __call__(self, signame, chain):            
        return ((signame in self.enabledSignatures and signame not in self.disabledSignatures) and \
            (not self.selectChains or chain in self.selectChains) and chain not in self.disableChains)
     

def generateMenuMT(flags): 
    """
    Interface between CA and MenuMT using ChainConfigurationBase
    """         
    global _isCAMenu
    _isCAMenu = True

    # generate L1 menu
    # This probably will go to TriggerConfig.triggerRunCfg
    from TrigConfigSvc.TrigConfigSvcCfg import generateL1Menu, createL1PrescalesFileFromMenu
    generateL1Menu(flags)
    createL1PrescalesFileFromMenu(flags)

    # Generate the menu, stolen from HLT_standalone
    from TriggerMenuMT.HLT.Config.GenerateMenuMT import GenerateMenuMT
    menu = GenerateMenuMT() 

    chainsToGenerate = FilterChainsToGenerate(flags)
    menu.setChainFilter(chainsToGenerate)
    log.debug("Filtering chains = %d", (menu.chainFilter is not None))
    finalListOfChainConfigs = menu.generateAllChainConfigs(flags)
    log.info("Length of FinalListOfChainConfigs %s", len(finalListOfChainConfigs))

    # Add prescales for disabling items (e.g. MC production)
    log.info("Applying HLT prescales")
    (menu.L1Prescales, menu.HLTPrescales, menu.chainsInMenu) = MenuPrescaleConfig(HLTMenuConfig, flags)
    applyHLTPrescale(HLTMenuConfig, menu.HLTPrescales, menu.signaturesOverwritten)
 
    # make sure that we didn't generate any steps that are fully empty in all chains
    # if there are empty steps, remove them
    finalListOfChainConfigs = menu.resolveEmptySteps(finalListOfChainConfigs)

    log.debug("finalListOfChainConfig %s", finalListOfChainConfigs)
    log.info("Making the HLT configuration tree")
    menuAcc=makeHLTTree(flags)

    # Configure ChainFilters for ROBPrefetching
    from TriggerJobOpts.TriggerConfigFlags import ROBPrefetching
    if ROBPrefetching.InitialRoI in flags.Trigger.ROBPrefetchingOptions:
        from TrigGenericAlgs.TrigGenericAlgsConfig import prefetchingInitialRoIConfig
        menuAcc.merge( prefetchingInitialRoIConfig(flags, HLTMenuConfig.configsList()), 'HLTBeginSeq')

    return menuAcc
    

def makeHLTTree(flags):
    """
    Generate appropriate Control Flow Graph wiht all HLT algorithms
    """
    from TriggerMenuMT.HLT.Config.ControlFlow.HLTCFConfig import decisionTreeFromChains, sequenceScanner
    from TriggerJobOpts.TriggerConfig import collectViewMakers
    from AthenaCommon.CFElements import compName   

    log.info("newJO version of makeHLTTree")
    acc = ComponentAccumulator()
    
    steps = seqAND('HLTAllSteps')
    finalDecisions, menuAcc = decisionTreeFromChains(flags, steps, HLTMenuConfig.configsList(), HLTMenuConfig.dictsList(), newJO=False)
    if log.getEffectiveLevel() <= logging.DEBUG:
        menuAcc.printConfig()

    acc.merge(menuAcc)
    successful_scan = sequenceScanner( steps )
    if not successful_scan:
        raise Exception("[makeHLTTree] At least one sequence is expected in more than one step. Check error messages and fix!")    

    flatDecisions=[]
    for step in finalDecisions:
        flatDecisions.extend (step)
 
    viewMakers = collectViewMakers(steps)
    viewMakerMap = {compName(vm):vm for vm in viewMakers}
    for vmname, vm in viewMakerMap.items():
        log.debug(f"[makeHLTTree] {vmname} InputMakerOutputDecisions: {vm.InputMakerOutputDecisions}")
        if vmname.endswith("_probe"):
            try:
                log.debug(f"Setting InputCachedViews on {vmname} to read decisions from tag leg {vmname[:-6]}: {vm.InputMakerOutputDecisions}")
                vm.InputCachedViews = viewMakerMap[vmname[:-6]].InputMakerOutputDecisions
            except KeyError: # We may be using a probe leg that has different reco from the tag
                log.debug(f"Tag leg does not match probe: '{vmname[:-6]}', will not use cached views")

    
    # generate JSON representation of the config
    from TriggerMenuMT.HLT.Config.JSON.HLTMenuJSON import generateJSON_newJO
    generateJSON_newJO(flags, HLTMenuConfig.dictsList(), HLTMenuConfig.configsList(), menuAcc.getSequence("HLTAllSteps"))

    from TriggerMenuMT.HLT.Config.JSON.HLTPrescaleJSON import generateJSON_newJO as generatePrescaleJSON_newJO
    generatePrescaleJSON_newJO(flags, HLTMenuConfig.dictsList(), HLTMenuConfig.configsList())

    from TriggerMenuMT.HLT.Config.JSON.HLTMonitoringJSON import generateDefaultMonitoringJSON_newJO
    generateDefaultMonitoringJSON_newJO(flags, HLTMenuConfig.dictsList())

    from AthenaCommon.CFElements import checkSequenceConsistency 
    checkSequenceConsistency(steps)
    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.Trigger.triggerMenuSetup = "Dev_pp_run3_v1"
    flags.lock()

    ca = makeHLTTree(flags)
    ca.printConfig()
    ca.wasMerged()
    log.info("All ok")

  
