# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# TEST1.py - derivation framework example demonstrating skimming 

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

def TEST1SkimmingToolCfg(flags):
    """Configure the example skimming tool"""
    acc = ComponentAccumulator()
    acc.addPublicTool(CompFactory.DerivationFramework.SkimmingToolExample(name                    = "TEST1SkimmingTool", 
                                                                          MuonContainerKey        = "Muons",
                                                                          NumberOfMuons           = 1,
                                                                          MuonPtCut               = 1000.0),
                      primary = True)
    return(acc)                          

def TEST1KernelCfg(flags, name='TEST1Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel)"""
    acc = ComponentAccumulator()
    skimmingTool = acc.getPrimaryAndMerge(TEST1SkimmingToolCfg(flags))
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, SkimmingTools = [skimmingTool]))       
    return acc


def TEST1Cfg(flags):

    acc = ComponentAccumulator()
    acc.merge(TEST1KernelCfg(flags, name="TEST1Kernel"))

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    TEST1SlimmingHelper = SlimmingHelper("TEST1SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    TEST1SlimmingHelper.SmartCollections = ["EventInfo",
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
                                            "AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets",
                                            "AntiKtVR30Rmax4Rmin02PV0TrackJets"]
    TEST1ItemList = TEST1SlimmingHelper.GetItemList()

    acc.merge(OutputStreamCfg(flags, "DAOD_TEST1", ItemList=TEST1ItemList, AcceptAlgs=["TEST1Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_TEST1", AcceptAlgs=["TEST1Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc
