# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# SKIM.py - special format allowing skimming of PHYS/PHYSLITE via a string
# passed via the command line (--skimmingExpression) 
# All containers used for the skimming must be listed via --skimmingContainers
# NOT TO BE USED FOR CENTRAL PRODUCTION

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory
from AthenaCommon.CFElements import seqAND

def SKIMSkimmingToolCfg(flags):
    """Configure the skimming tool"""
    from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
    acc = ComponentAccumulator()
    tdt = acc.getPrimaryAndMerge(TrigDecisionToolCfg(flags))
    
    acc.addPublicTool(CompFactory.DerivationFramework.xAODStringSkimmingTool(name       = "SKIMStringSkimmingTool",
                                                                             expression = flags.Derivation.skimmingExpression,
                                                                             TrigDecisionTool=tdt), 
                      primary = True)
    return(acc)                       


def SKIMKernelCfg(flags, name='SKIMKernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel)"""
    acc = ComponentAccumulator()
    acc.addSequence( seqAND("SKIMSequence") )
    acc.getSequence("SKIMSequence").ExtraDataForDynamicConsumers = flags.Derivation.dynamicConsumers
    acc.getSequence("SKIMSequence").ProcessDynamicDataDependencies = True
    skimmingTool = acc.getPrimaryAndMerge(SKIMSkimmingToolCfg(flags))
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, SkimmingTools = [skimmingTool]), sequenceName="SKIMSequence")
    return acc


def SKIMCfg(flags):

    acc = ComponentAccumulator()
    acc.merge(SKIMKernelCfg(flags, name="SKIMKernel"))

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg

    acc.merge(OutputStreamCfg(flags, "D2AOD_SKIM", ItemList=flags.Input.TypedCollections, AcceptAlgs=["SKIMKernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "D2AOD_SKIM", AcceptAlgs=["SKIMKernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc
