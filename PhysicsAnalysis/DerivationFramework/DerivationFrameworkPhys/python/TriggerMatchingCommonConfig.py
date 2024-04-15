# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

#====================================================================
# TriggerMatchingCommonConfig.py
# This defines the common trigger matching shared by PHYS and PHYSLITE
# Using it avoids name clashes when running in trains
# In principle it can also be used by other formats who want to take
# advantage of PHYS/PHYSLITE containers
# Component accumulator version
#====================================================================
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from PathResolver import PathResolver

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


def AddRun2TriggerMatchingToSlimmingHelper(**kwargs):
    """Adds the trigger matching info to the slimming helper"""

    slimmingHelper = kwargs['SlimmingHelper']
    triggerList = kwargs['TriggerList']
    containerPrefix = kwargs['OutputContainerPrefix']

    outputContainers = [
        "{0}{1}".format(containerPrefix, chain).replace(".", "_")
        for chain in triggerList
    ]

    slimmingHelper.AllVariables += outputContainers
    for cont in outputContainers:
        slimmingHelper.AppendToDictionary.update(
            {
                cont: "xAOD::TrigCompositeContainer",
                cont + "Aux": "AOD::AuxContainerBase!",
            }
        )


def TriggerMatchingCommonRun2Cfg(ConfigFlags, name, **kwargs):
    """Configure the common trigger matching for run 2 DAODs using the run 2 analysis formalism (matching happens during derivation)"""

    acc = ComponentAccumulator()
 
    # Create trigger matching decorations
    from DerivationFrameworkTrigger.TriggerMatchingToolConfig import TriggerMatchingToolCfg
    PhysCommonTriggerMatchingTool = acc.getPrimaryAndMerge(TriggerMatchingToolCfg(ConfigFlags, name=name, **kwargs))
    CommonAugmentation = CompFactory.DerivationFramework.CommonAugmentation
    outputContainerPrefix = kwargs['OutputContainerPrefix']
    acc.addEventAlgo(CommonAugmentation(f"{outputContainerPrefix}TriggerMatchingKernel",
                                        AugmentationTools=[PhysCommonTriggerMatchingTool]))

    return(acc)

def TriggerMatchingCommonRun2ToRun3Cfg(ConfigFlags, **kwargs):
    """Covert run 2 trigger navigation data these data into the run 3 formalism (matching happens from DAOD)"""

    acc = ComponentAccumulator()

    if not ConfigFlags.Trigger.doEDMVersionConversion:
        return(acc)

    from AthenaCommon.Logging import logging
    msg = logging.getLogger('TriggerMatchingCommonRun2ToRun3Cfg')
    msg.info('doEDMVersionConversion is True, now scheduling conversion of Run 2 trigger navigation to Run 3')

    from TrigNavTools.NavConverterConfig import NavConverterCfg

    # And then run the run 3 slimming on the output of NavConverter
    triggerList = kwargs['TriggerList']
    acc.merge(NavConverterCfg(ConfigFlags, chainsList = triggerList))
    from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import TrigNavSlimmingMTDerivationCfg
    acc.merge(TrigNavSlimmingMTDerivationCfg(ConfigFlags,chainsFilter=triggerList))

    return(acc)


def TriggerMatchingCommonRun3Cfg(ConfigFlags, **kwargs):
    """Configure the common trigger matching for run 3 DAODs using the run 3 formalism (matching happens from DAOD)"""
    
    if ConfigFlags.Trigger.EDMVersion != 3:
        raise ValueError('This configuration can only be used for Run 3 trigger navigation')

    triggerList = kwargs['TriggerList']

    acc = ComponentAccumulator()

    # Run 3 trigger navigation slimming proposal for in-DAOD trigger matching.
    from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import TrigNavSlimmingMTDerivationCfg
    acc.merge(TrigNavSlimmingMTDerivationCfg(ConfigFlags,chainsFilter=triggerList))

    return(acc)


