# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# TEST3.py - derivation framework example demonstrating thinning 

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

def TEST3ThinningToolCfg(flags,streamName):
    """Configure the example thinning tool"""
    acc = ComponentAccumulator()
    acc.addPublicTool(CompFactory.DerivationFramework.ThinningToolExample(name       = "TEST3ThinningTool",
                                                                          StreamName = streamName,
                                                                          TrackPtCut = 20000.0),
                      primary = True)
    return(acc)                          


def TEST3KernelCfg(flags, name='TEST3Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel)"""
    acc = ComponentAccumulator()
    thinningTool = acc.getPrimaryAndMerge(TEST3ThinningToolCfg(flags,kwargs["StreamName"]))
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, ThinningTools = [thinningTool]))       
    return acc


def TEST3Cfg(flags):

    acc = ComponentAccumulator()
    acc.merge(TEST3KernelCfg(flags, name="TEST3Kernel",StreamName = "StreamDAOD_TEST3"))

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    TEST3SlimmingHelper = SlimmingHelper("TEST3SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    TEST3SlimmingHelper.SmartCollections = ["EventInfo","InDetTrackParticles"]
    TEST3ItemList = TEST3SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_TEST3", ItemList=TEST3ItemList, AcceptAlgs=["TEST3Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_TEST3", AcceptAlgs=["TEST3Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))
    return acc
