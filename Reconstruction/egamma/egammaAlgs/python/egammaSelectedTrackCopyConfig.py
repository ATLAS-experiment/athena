# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

""" Instantiate egammaSelectedTrackCopy with default configuration
"""

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def egammaSelectedTrackCopyCfg(flags, name="egammaSelectedTrackCopy", **kwargs):
    acc = ComponentAccumulator()

    if "egammaCaloClusterSelector" not in kwargs:
        from egammaCaloTools.egammaCaloToolsConfig import (
            egammaCaloClusterSelectorGSFCfg,
        )

        kwargs["egammaCaloClusterSelector"] = acc.popToolsAndMerge(
            egammaCaloClusterSelectorGSFCfg(flags)
        )

    if "ExtrapolationTool" not in kwargs:
        from egammaTrackTools.egammaTrackToolsConfig import (
            EMExtrapolationToolsCfg,
        )

        extraptool = EMExtrapolationToolsCfg(
            flags, name="EMExtrapolationTools")
        kwargs["ExtrapolationTool"] = acc.popToolsAndMerge(extraptool)

    kwargs.setdefault("ClusterContainerName",
                      flags.Egamma.Keys.Internal.EgammaTopoClusters)
    kwargs.setdefault("TrackParticleContainerName",
                      flags.Egamma.Keys.Input.TrackParticles)
    kwargs.setdefault("OutputTrkPartContainerName",
                      flags.Egamma.Keys.Output.TrkPartContainerName)

    if flags.Reco.EnableHGTDExtension:
        kwargs.setdefault("TrackParticleTimeDecoration",
                          flags.Egamma.Keys.Input.TrackParticles+".time")

    doFwd = flags.Detector.GeometryITk and flags.Egamma.doForward
    kwargs.setdefault("doFwdTracks", doFwd)
    if doFwd:
        kwargs.setdefault("FwdClusterContainerName",
                          flags.Egamma.Keys.Internal.ForwardTopoClusters)

    # P->T conversion extra dependencies
    if flags.Detector.GeometryITk:
        kwargs.setdefault(
            "ExtraInputs",
            [
                (
                    "InDetDD::SiDetectorElementCollection",
                    "ConditionStore+ITkPixelDetectorElementCollection",
                ),
                (
                    "InDetDD::SiDetectorElementCollection",
                    "ConditionStore+ITkStripDetectorElementCollection",
                ),
            ],
        )
    else:
        kwargs.setdefault(
            "ExtraInputs",
            [
                (
                    "InDetDD::SiDetectorElementCollection",
                    "ConditionStore+PixelDetectorElementCollection",
                ),
                (
                    "InDetDD::SiDetectorElementCollection",
                    "ConditionStore+SCT_DetectorElementCollection",
                ),
            ],
        )

    egseltrkcpAlg = CompFactory.egammaSelectedTrackCopy(name, **kwargs)

    acc.addEventAlgo(egseltrkcpAlg)
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
    acc.merge(egammaSelectedTrackCopyCfg(flags))
    mlog = logging.getLogger("egammaSelectedTrackCopyConfigTest")
    mlog.info("Configuring  egammaSelectedTrackCopy: ")
    printProperties(
        mlog,
        acc.getEventAlgo("egammaSelectedTrackCopy"),
        nestLevel=1,
        printDefaults=True,
    )
    with open("egammaselectedtrackCopy.pkl", "wb") as f:
        acc.store(f)
