# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import logging


def EMBremCollectionBuilderCfg(flags,
                               name="EMBremCollectionBuilder",
                               **kwargs):
    """ Algorithm to create the GSF collection
        We need to refit with GSF
        Create a Track Summary
        Create the relevant TrackParticles """

    acc = ComponentAccumulator()

    if "TrackRefitTool" not in kwargs:
        from egammaTrackTools.egammaTrackToolsConfig import (
            egammaTrkRefitterToolCfg)
        kwargs["TrackRefitTool"] = acc.popToolsAndMerge(
            egammaTrkRefitterToolCfg(flags))

    if "TrackParticleCreatorTool" not in kwargs:
        from TrkConfig.TrkParticleCreatorConfig import (
            GSFBuildInDetParticleCreatorToolCfg)
        kwargs["TrackParticleCreatorTool"] = acc.popToolsAndMerge(
            GSFBuildInDetParticleCreatorToolCfg(flags))

    if "TrackSlimmingTool" not in kwargs:
        from TrkConfig.TrkTrackSlimmingToolConfig import GSFTrackSlimmingToolCfg
        kwargs["TrackSlimmingTool"] = acc.popToolsAndMerge(GSFTrackSlimmingToolCfg(flags))

    kwargs.setdefault(
        "usePixel",
        flags.Detector.EnablePixel or flags.Detector.EnableITkPixel)
    kwargs.setdefault(
        "useSCT",
        flags.Detector.EnableSCT or flags.Detector.EnableITkStrip)
    kwargs.setdefault("useTRT", flags.Detector.EnableTRT)
    kwargs.setdefault("DoTruth", flags.Input.isMC)
    kwargs.setdefault("slimTrkTracks", flags.Egamma.slimGSFTrkTracks)
    kwargs.setdefault("useHGTD", flags.Reco.EnableHGTDExtension)

    # P->T conversion extra dependencies
    if flags.Detector.GeometryITk:
        kwargs.setdefault("ExtraInputs", [
            ("InDetDD::SiDetectorElementCollection",
             "ConditionStore+ITkPixelDetectorElementCollection"),
            ("InDetDD::SiDetectorElementCollection",
             "ConditionStore+ITkStripDetectorElementCollection"),
        ])
    else:
        kwargs.setdefault("ExtraInputs", [
            ("InDetDD::SiDetectorElementCollection",
             "ConditionStore+PixelDetectorElementCollection"),
            ("InDetDD::SiDetectorElementCollection",
             "ConditionStore+SCT_DetectorElementCollection"),
        ])

    alg = CompFactory.EMBremCollectionBuilder(name, **kwargs)
    acc.addEventAlgo(alg)
    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaConfiguration.ComponentAccumulator import printProperties
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RDO_RUN2
    flags.lock()
    acc = MainServicesCfg(flags)
    acc.merge(EMBremCollectionBuilderCfg(flags))
    mlog = logging.getLogger("EMBremCollectionBuilderConfigTest")
    mlog.info("Configuring  EMBremCollectionBuilder: ")
    printProperties(mlog,
                    acc.getEventAlgo("EMBremCollectionBuilder"),
                    nestLevel=1,
                    printDefaults=True)
    with open("embremcollectionbuilder.pkl", "wb") as f:
        acc.store(f)
