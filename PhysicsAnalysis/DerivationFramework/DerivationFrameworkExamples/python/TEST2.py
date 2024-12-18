# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# TEST2.py - derivation framework example demonstrating skimming via means of string 

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory
from AthenaCommon.CFElements import seqAND

def TEST2SkimmingToolCfg(flags):
    """Configure the example skimming tool"""
    from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
    acc = ComponentAccumulator()
    tdt = acc.getPrimaryAndMerge(TrigDecisionToolCfg(flags))
    acc.addPublicTool(CompFactory.DerivationFramework.xAODStringSkimmingTool(name       = "TEST2StringSkimmingTool",
                                                                             expression = "count(Muons.pt > (1 * GeV)) >= 1",
                                                                             TrigDecisionTool=tdt), 
                      primary = True)
    return(acc)                          

def TEST2KernelCfg(flags, name='TEST2Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel)"""
    acc = ComponentAccumulator()
    # The next three lines are necessary in case the string skimming tool accesses containers which haven't
    # previously been accessed via ReadHandles (as here). One must create a new sequence, list all of the 
    # accessed container types and keys as ExtraDataForDynamicConsumers (just Muons here) and then set the property
    # ProcessDynamicDataDependencies to True for that sequence. The relevant skimming tools must then be attached
    # to this sequence. The use of seqAND here isn't relevant since there is only one sequence in use.
    # This step isn't needed in case the common augmentations are run first (e.g. with PHYS/PHYSLITE etc). In 
    # such cases one can omit the next three lines and the sequenceName argument in addEventAlgo.
    acc.addSequence( seqAND("TEST2Sequence") )
    acc.getSequence("TEST2Sequence").ExtraDataForDynamicConsumers = ['xAOD::MuonContainer/Muons']
    acc.getSequence("TEST2Sequence").ProcessDynamicDataDependencies = True
    skimmingTool = acc.getPrimaryAndMerge(TEST2SkimmingToolCfg(flags))
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, SkimmingTools = [skimmingTool]), sequenceName="TEST2Sequence")
    return acc


def TEST2Cfg(flags):

    acc = ComponentAccumulator()
    acc.merge(TEST2KernelCfg(flags, name="TEST2Kernel"))

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    TEST2SlimmingHelper = SlimmingHelper("TEST2SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    TEST2SlimmingHelper.SmartCollections = ["EventInfo",
                                            "Electrons",
                                            "Photons",
                                            "Muons",
                                            "PrimaryVertices",
                                            "InDetTrackParticles",
                                            "AntiKt4EMTopoJets",
                                            "AntiKt4EMPFlowJets",
                                            "BTagging_AntiKt4EMPFlow",
                                            "BTagging_AntiKtVR30Rmax4Rmin02Track", 
                                            "MET_Baseline_AntiKt4EMTopo",
                                            "MET_Baseline_AntiKt4EMPFlow",
                                            "TauJets",
                                            "DiTauJets",
                                            "DiTauJetsLowPt",
                                            "AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets",
                                            "AntiKtVR30Rmax4Rmin02PV0TrackJets"]
    TEST2ItemList = TEST2SlimmingHelper.GetItemList()

    acc.merge(OutputStreamCfg(flags, "DAOD_TEST2", ItemList=TEST2ItemList, AcceptAlgs=["TEST2Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_TEST2", AcceptAlgs=["TEST2Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc
