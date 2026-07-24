# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

""" Instantiate ActsEgammaSelectedTrackCopy with default configuration
"""

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def ActsEgammaSelectedTrackCopyCfg(flags, name="ActsEgammaSelectedTrackCopy", **kwargs):
    acc = ComponentAccumulator()

    assert flags.Acts.TrackingGeometry.UseBlueprint, "Must use ACTS Gen3 geometry"
    assert flags.Detector.GeometryCalo, "Calorimeter must be enabled"

    # I guess it makes sense to enforce ITk here, though technical one can run this with
    # Calo geometry only...
    assert flags.Detector.GeometryITk

    if "egammaCaloClusterSelector" not in kwargs:
        from egammaCaloTools.egammaCaloToolsConfig import (
            egammaCaloClusterSelectorGSFCfg,
        )

        kwargs["egammaCaloClusterSelector"] = acc.popToolsAndMerge(
            egammaCaloClusterSelectorGSFCfg(flags)
        )

    if 'ExtrapolationTool' not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg
        kwargs.setdefault(
            "ExtrapolationTool",
            acc.popToolsAndMerge(ActsExtrapolationToolCfg(flags, MaxSteps=10000)),
        )  # PrivateToolHandle

    kwargs.setdefault("ClusterContainerName",
                      flags.Egamma.Keys.Internal.EgammaTopoClusters)
    kwargs.setdefault("TrackParticleContainerName",
                      flags.Egamma.Keys.Input.TrackParticles)
    kwargs.setdefault("OutputTrkPartContainerName",
                      flags.Egamma.Keys.Output.TrkPartContainerName)

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

    actsEgseltrkcpAlg = CompFactory.ActsEgammaSelectedTrackCopy(name, **kwargs)

    acc.addEventAlgo(actsEgseltrkcpAlg)
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
    acc.merge(ActsEgammaSelectedTrackCopyCfg(flags))
    mlog = logging.getLogger("ActsEgammaSelectedTrackCopyConfigTest")
    mlog.info("Configuring  ActsEgammaSelectedTrackCopy: ")
    printProperties(
        mlog,
        acc.getEventAlgo("ActsEgammaSelectedTrackCopy"),
        nestLevel=1,
        printDefaults=True,
    )
    with open("ActsEgammaSelectedTrackCopy.pkl", "wb") as f:
        acc.store(f)
