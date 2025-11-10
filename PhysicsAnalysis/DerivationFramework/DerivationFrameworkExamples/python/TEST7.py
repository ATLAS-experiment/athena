# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# TEST7.py - derivation framework example demonstrating skimming via means of string 

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.CFElements import seqAND

def TEST7SkimmingToolCfg(flags):
    """Configure the example skimming tool"""
    from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
    acc = ComponentAccumulator()
    tdt = acc.getPrimaryAndMerge(TrigDecisionToolCfg(flags))
    acc.addPublicTool(CompFactory.DerivationFramework.xAODStringSkimmingTool(name       = "TEST7StringSkimmingTool",
                                                                             expression = "( count(Muons.pt > (6 * GeV)) + count(Electrons.pt > (6 * GeV)) ) >= 3",
                                                                             TrigDecisionTool=tdt),
                      primary = True)
    return(acc)                          

def TEST7KernelCfg(flags, name='TEST7Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel)"""
    acc = ComponentAccumulator()
    # The next three lines are necessary in case the string skimming tool accesses containers which haven't
    # previously been accessed via ReadHandles (as here). One must create a new sequence, list all of the 
    # accessed container types and keys as ExtraDataForDynamicConsumers (just Muons here) and then set the property
    # ProcessDynamicDataDependencies to True for that sequence. The relevant skimming tools must then be attached
    # to this sequence. The use of seqAND here isn't relevant since there is only one sequence in use.
    # This step isn't needed in case the common augmentations are run first (e.g. with PHYS/PHYSLITE etc). In 
    # such cases one can omit the next three lines and the sequenceName argument in addEventAlgo.
    acc.addSequence( seqAND("TEST7Sequence") )
    acc.getSequence("TEST7Sequence").ExtraDataForDynamicConsumers = ['xAOD::MuonContainer/Muons','xAOD::ElectronContainer/Electrons']
    acc.getSequence("TEST7Sequence").ProcessDynamicDataDependencies = True
    skimmingTool = acc.getPrimaryAndMerge(TEST7SkimmingToolCfg(flags))
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, SkimmingTools = [skimmingTool]), sequenceName="TEST7Sequence")       
    return acc


def TEST7Cfg(flags):

    acc = ComponentAccumulator()
    acc.merge(TEST7KernelCfg(flags, name="TEST7Kernel"))

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    TEST7SlimmingHelper = SlimmingHelper("TEST7SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    TEST7SlimmingHelper.SmartCollections = ["EventInfo",
                                            "Electrons",
                                            "Photons",
                                            "Muons",
                                            "PrimaryVertices",
                                            "InDetTrackParticles",
                                            "AntiKt4EMTopoJets",
                                            "AntiKt4EMPFlowJets",

                                            "MET_Baseline_AntiKt4EMTopo",
                                            "MET_Baseline_AntiKt4EMPFlow",
                                            "TauJets",
                                            "DiTauJets",
                                            "DiTauJetsLowPt",
                                            "AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets"]
    TEST7ItemList = TEST7SlimmingHelper.GetItemList()

    acc.merge(OutputStreamCfg(flags, "D2AOD_TEST7", ItemList=TEST7ItemList, AcceptAlgs=["TEST7Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "D2AOD_TEST7", AcceptAlgs=["TEST7Kernel"], propagateMetadataFromInput=True))

    return acc
