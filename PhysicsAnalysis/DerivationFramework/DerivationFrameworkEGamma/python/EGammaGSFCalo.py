# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

__doc__ = """
          Implement GSF calo improvement tools
          """

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from egammaTrackTools.egammaTrackToolsConfig import CaloCluster_OnTrackBuilderCfg


def EGammaGSFCaloToolsCfg(flags, name = "GSFCaloImprovement", **kwargs):
    prefix = name + "_GSFCaloImprovement"
    acc = ComponentAccumulator()

    CCOTBuilder = acc.popToolsAndMerge(CaloCluster_OnTrackBuilderCfg(flags, 
                                                                     name=prefix+"CCOTBuilder"))

    if "TrackRefitTool" not in kwargs:
        from egammaTrackTools.egammaTrackToolsConfig import egammaTrkRefitterToolCfg
        TrackRefitTool = acc.popToolsAndMerge(egammaTrkRefitterToolCfg(flags, 
                                                                       name = prefix+"_trackRefit",
                                                                       CCOTBuilder=CCOTBuilder,
                                                                       useClusterPosition=True))
        kwargs.setdefault("TrackRefitTool", TrackRefitTool)

    if "TrackParticleCreatorTool" not in kwargs:
        from TrkConfig.TrkParticleCreatorConfig import (
            GSFBuildInDetParticleCreatorToolCfg)
        kwargs["TrackParticleCreatorTool"] = acc.popToolsAndMerge(
            GSFBuildInDetParticleCreatorToolCfg(flags,TRT_ElectronPidTool=None,PixelToTPIDTool=None))

    if "TrackSummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import GSFTrackSummaryToolCfg
        TrackSummaryTool = acc.popToolsAndMerge(
            GSFTrackSummaryToolCfg(flags))
        acc.addPublicTool(TrackSummaryTool)
        kwargs.setdefault("TrackSummaryTool", TrackSummaryTool)
    
    kwargs.setdefault("useTruth"  , flags.Input.isMC)
    kwargs.setdefault("useTRT"    , flags.Detector.EnableTRT)

    acc.setPrivateTools(CompFactory.DerivationFramework.EGammaGSFCalo(name, **kwargs))
    return acc

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaConfiguration.ComponentAccumulator import printProperties

    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RDO_RUN2
    flags.lock()
    acc = MainServicesCfg(flags)
    tool = acc.popToolsAndMerge(EGammaGSFCaloToolsCfg(flags, "GSFCaloImprovement"))
    acc.addPublicTool(tool)

    mlog = logging.getLogger("EGammaGSFCaloConfigTest")
    mlog.info("Configuring GSF Calo Tool: ")
    printProperties(
         mlog,
         acc.getPublicTool(name="GSFCaloImprovement"),
         nestLevel=1,
         printDefaults=True,
    )