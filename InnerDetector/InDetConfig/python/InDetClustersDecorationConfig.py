#Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

"""
Intended use:
Reco_tf.py --postInclude 'all:InDetConfig.InDetClustersDecorationConfig.fromRunArgs'

This schedules:
  - PixelPrepDataToxAOD
  - SCT_PrepDataToxAOD
  - TrackStateOnSurfaceDecorator

This creates the Pixel & SCT MSOSs containers and the links from the track --> MSOSs --> clusters.
  The decoration runs for InDetTrackParticles and InDetDisappearingTrackParticles.
  Also, the Pixel & SCT cluster & MSOS containers are added to the AOD output stream.

This is an alternative to the standard approach of using --preExec "flags.Tracking.writeExtendedSi_PRDInfo=True"...
  which has bugs that were fixed by !72727, affecting releases 24.0.62+.

After the bug fix, the resulting AODs are identical between the two methods, except:
  The --postInclude method only decorates the aforementioned track containers.
  The --preExec method additionally decorates GSF, forward, and LargeD0 tracks.

To save clusters in older 24.0 releases, or in 22.0 and 23.0, this --postInclude method can be used instead.
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def AODTSOSKernelCfg(flags, name="AODTSOSKernel", listOfExtensions=[]):
    acc = ComponentAccumulator()

    from DerivationFrameworkInDet.InDetToolsConfig import TrackStateOnSurfaceDecoratorCfg

    listOfAugmTools = []
    for extension in listOfExtensions:
        PixelMsosName = f"{extension}PixelMSOSs"
        SctMsosName = f"{extension}SCT_MSOSs"

        TrackStateOnSurfaceDecorator = acc.getPrimaryAndMerge(
            TrackStateOnSurfaceDecoratorCfg(
                flags,
                name=f"{extension}TrackStateOnSurfaceDecorator",
                ContainerName=f"InDet{extension}TrackParticles",
                PixelMsosName=PixelMsosName,
                SctMsosName=SctMsosName,
                TrtMsosName=f"{extension}TRT_MSOSs",  # TRT not used, but harmless
                StorePixel=True,   # explicitly override the flag
                StoreSCT=True      # explicitly override the flag
            )
        )
        TrackStateOnSurfaceDecorator.DecorationPrefix = "Reco_"
        listOfAugmTools.append(TrackStateOnSurfaceDecorator)

    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation(
        name, AugmentationTools=listOfAugmTools))
    return acc


def MSOSAndClustersCfg(flags):
    acc = ComponentAccumulator()

    from PixelConditionsTools.PixelConditionsSummaryConfig import PixelConditionsSummaryCfg
    from SiLorentzAngleTool.PixelLorentzAngleConfig import PixelLorentzAngleToolCfg
    from InDetConfig.TrackRecoConfig import ClusterSplitProbabilityContainerName

    # Pixel clusters
    pixel_kwargs = {}
    pixel_kwargs.setdefault("ClusterSplitProbabilityName", ClusterSplitProbabilityContainerName(flags))
    pixel_kwargs.setdefault("PixelConditionsSummaryTool", acc.popToolsAndMerge(PixelConditionsSummaryCfg(flags)))
    pixel_kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge(PixelLorentzAngleToolCfg(flags)))
    pixel_kwargs.setdefault("UseTruthInfo", flags.Input.isMC)
    pixel_kwargs.setdefault("WriteRDOinformation", True)
    pixel_kwargs.setdefault("WriteNNinformation", True)
    pixel_kwargs.setdefault("WriteExtendedPRDinformation", True)

    acc.addEventAlgo(
        CompFactory.PixelPrepDataToxAOD(
            name="xAOD_PixelPrepDataToxAOD", **pixel_kwargs
        )
    )

    # SCT clusters
    sct_kwargs = {}
    sct_kwargs.setdefault("UseTruthInfo", flags.Input.isMC)
    sct_kwargs.setdefault("WriteRDOinformation", True)

    acc.addEventAlgo(
        CompFactory.SCT_PrepDataToxAOD(
            name="xAOD_SCT_PrepDataToxAOD", **sct_kwargs
        )
    )

    # Use this version to set container names properly
    acc.merge(AODTSOSKernelCfg(flags, listOfExtensions=["", "Disappearing"]))

    return acc

def fromRunArgs(flags, cfg):

    # Note there is bug in 24.0 versions before 24.0.62.
    #   - The bug prevented saving a separate MSOS container for disappearing tracks.
    # Fixed by MR !72727 (which also created a cluster thinning alg).
    #   - Also changed the prefix of the msosLink to "Reco_"
    #   - Also standardized the cluster and MSOS container names.
    # For this reason, not relying on the following to get this right:
    #   - writeExtendedSi_PRDInfo
    #   - TrackRecoConfig
    #   - InDetTrackOutputConfig
    
    acc = MSOSAndClustersCfg(flags)
    cfg.merge(acc)

    # Explicitly add MSOS containers to the existing AOD stream
    from OutputStreamAthenaPool.OutputStreamConfig import addToAOD
    cfg.merge(addToAOD(flags, [
        "xAOD::TrackMeasurementValidationContainer#PixelClusters",
        "xAOD::TrackMeasurementValidationAuxContainer#PixelClustersAux.",
        "xAOD::TrackMeasurementValidationContainer#SCT_Clusters",
        "xAOD::TrackMeasurementValidationAuxContainer#SCT_ClustersAux.",
        "xAOD::TrackStateValidationContainer#PixelMSOSs",
        "xAOD::TrackStateValidationAuxContainer#PixelMSOSsAux.",
        "xAOD::TrackStateValidationContainer#SCT_MSOSs",
        "xAOD::TrackStateValidationAuxContainer#SCT_MSOSsAux.",
        "xAOD::TrackStateValidationContainer#DisappearingPixelMSOSs",
        "xAOD::TrackStateValidationAuxContainer#DisappearingPixelMSOSsAux.",
        "xAOD::TrackStateValidationContainer#DisappearingSCT_MSOSs",
        "xAOD::TrackStateValidationAuxContainer#DisappearingSCT_MSOSsAux."
    ]))
    
    return cfg
