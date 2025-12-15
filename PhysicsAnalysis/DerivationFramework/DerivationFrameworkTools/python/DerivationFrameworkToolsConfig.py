# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#==============================================================================
# Provides configs for the common derivation framework tools
#==============================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# Wrapper to allow ASG tools to decorste via the derivation framework
def AsgSelectionToolWrapperCfg(flags, name, **kwargs):
    """Configure the ASG selection tool wrapper"""
    acc = ComponentAccumulator()
    AsgSelectionToolWrapper = CompFactory.DerivationFramework.AsgSelectionToolWrapper
    acc.addPublicTool(AsgSelectionToolWrapper(name, **kwargs),
                      primary = True)
    return acc   

# Generic thinning tool (via ExpressionEvaluation strings)
def GenericObjectThinningCfg(flags, name, **kwargs):
    """Configure the generic object thinning tool"""
    acc = ComponentAccumulator()
    GenericObjectThinning = CompFactory.DerivationFramework.GenericObjectThinning
    acc.addPublicTool(GenericObjectThinning(name, **kwargs),
                      primary = True)
    return acc

# Skimming via ExpressionEvaluation strings
def xAODStringSkimmingToolCfg(flags, name, **kwargs):
    """Configure the generic skimming tool"""
    acc = ComponentAccumulator()
    if flags.Trigger.EDMVersion < 0:
        kwargs.setdefault("TrigDecisionTool", None)
    acc.addPublicTool(
        CompFactory.DerivationFramework.xAODStringSkimmingTool(name, **kwargs),
        primary = True)
    return acc

### Trigger skimming list
def TriggerSkimmingToolCfg(flags, name, **kwargs):
    acc = ComponentAccumulator()
    the_tool = CompFactory.DerivationFramework.TriggerSkimmingTool(name,**kwargs)
    acc.addPublicTool(the_tool, primary = True)
    return acc
### Invariant mass tool config
def InvariantMassToolCfg(flags, name , **kwargs):
    acc= ComponentAccumulator()
    the_tool = CompFactory.DerivationFramework.InvariantMassTool(name,**kwargs)
    acc.addPublicTool(the_tool, primary = True)
    return acc
# Prescale tool
def PrescaleToolCfg(flags, name, **kwargs):
    """Configure the DAOD prescale tool"""
    acc = ComponentAccumulator()
    PrescaleTool = CompFactory.DerivationFramework.PrescaleTool
    acc.addPublicTool(PrescaleTool(name, **kwargs),
                      primary = True)
    return acc 

# Tool for combining several filter tools with AND logic
def FilterCombinationANDCfg(flags, name, **kwargs):
    """Configure the FilterCombinationAND tool"""
    acc = ComponentAccumulator()
    FilterCombinationAND = CompFactory.DerivationFramework.FilterCombinationAND
    acc.addPublicTool(FilterCombinationAND(name, **kwargs),
                      primary = True)
    return acc
   
# Tool for combining several filter tools with OR logic
def FilterCombinationORCfg(flags, name, **kwargs):
    """Configure the FilterCombinationOR tool"""
    acc = ComponentAccumulator()
    FilterCombinationOR = CompFactory.DerivationFramework.FilterCombinationOR
    acc.addPublicTool(FilterCombinationOR(name, **kwargs),
                      primary = True)
    return acc 
