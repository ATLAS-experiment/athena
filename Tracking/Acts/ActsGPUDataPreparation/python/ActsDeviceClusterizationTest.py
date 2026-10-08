#!/usr/bin/env athena

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Run script for the full GPU clusterization chain:
#   RDO -> traccc cells -> traccc measurements -> xAOD clusters

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Constants import DEBUG

from ActsGPUDataPreparation.ActsGPUDataPreparationConfig import DeviceClusterizationAlgCfg
from ActsGPUEventCnv.ActsGPUEventCnvConfig import TracccMeasurementConverterAlgCfg
from ActsGPUGeometry.ActsGPUGeometryConfig import DeviceDetectorDescriptionCondAlgCfg

def GPUClusterizationCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # Service runs first — loads all device detector description data into detStore
    acc.merge(DeviceDetectorDescriptionCondAlgCfg(flags))

    if flags.Acts.EDM.PhaseII :
        print("Running PhaseII RDO to TracccCell conversion")
        from ActsConfig.ActsPhaseIIRawDataEdmConfig import (
            PhaseIIPixelRawDataContainerCfg,
            PhaseIIStripRawDataContainerCfg,
        )
        acc.merge(PhaseIIPixelRawDataContainerCfg(flags))
        acc.merge(PhaseIIStripRawDataContainerCfg(flags))
        from ActsGPUEventCnv.ActsGPUEventCnvConfig import PhaseIIRDOtoTracccCellConverterAlgCfg
        acc.merge(PhaseIIRDOtoTracccCellConverterAlgCfg(flags,
            TracccCells = "TracccCellCollection",
            OutputLevel = DEBUG
        ))
    else:
        from ActsGPUEventCnv.ActsGPUEventCnvConfig import RDOtoTracccCellConverterAlgCfg
        acc.merge(RDOtoTracccCellConverterAlgCfg(flags,
            TracccCells = "TracccCellCollection",
            OutputLevel = DEBUG
        ))

    acc.merge(DeviceClusterizationAlgCfg(flags,
        InputTracccCells="TracccCellCollection",
        OutputTracccMeasurements="TracccMeasurementCollection",
        OutputTracccClusters="TracccClusterCollection",
        RetrieveClusterCells=flags.Tracking.doTruth,
        OutputLevel = DEBUG,
            
    ))

    acc.merge(TracccMeasurementConverterAlgCfg(flags,
        InputMeasurements="TracccMeasurementCollection",
        InputClusters="TracccClusterCollection",
        InputCells="TracccCellCollection",
        ConvertClustersWithCells = flags.Tracking.doTruth,
        OutputPixelClusters="ITkTracccPixelClusters",
        OutputStripClusters="ITkTracccStripClusters",
        OutputLevel = DEBUG
    ))

    from SiLorentzAngleTool.ITkPixelLorentzAngleConfig import ITkPixelLorentzAngleToolCfg
    from SiLorentzAngleTool.ITkStripLorentzAngleConfig import ITkStripLorentzAngleToolCfg
    from AthenaConfiguration.ComponentFactory import CompFactory
    acc.addEventAlgo(CompFactory.ActsTrk.ActsClusterComparisonAlg(
        "GPU_ActsClusterComparisonAlg",
        TracccCondKey="TracccHostCondConfig",
        checkSpacepoints=False,
        monitoredSpacepointsKey="ITkTracccPixelSpacepoints",
        referenceSpacepointsKey="ITkPixelSpacePoints",
        monitoredPixelClustersKey="ITkTracccPixelClusters",
        referencePixelClustersKey="ITkPixelClusters",
        monitoredStripClustersKey="ITkTracccStripClusters",
        referenceStripClustersKey="ITkStripClusters",
        PixelLorentzAngleTool = acc.popToolsAndMerge(ITkPixelLorentzAngleToolCfg(flags)),
        StripLorentzAngleTool = acc.popToolsAndMerge(ITkStripLorentzAngleToolCfg(flags)),
        OutputLevel = DEBUG

    ))

    return acc

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    flags = initConfigFlags()
    
    flags.Input.Files = defaultTestFiles.RDO_RUN4
    flags.Exec.MaxEvents = 1

    # Set the Main Pass
    flags = flags.cloneAndReplace(
        "Tracking.ActiveConfig",
        "Tracking.ITkActsPass")
    
    flags.Acts.TrackingGeometry.UseBlueprint = True
    flags.Acts.TrackingGeometry.BuildDetrayGeometry = True

    # Keep calo/muon out of the tracking geometry: the calo volumes cannot be
    # converted to a consistent Detray geometry
    from InDetConfig.ConfigurationHelpers import OnlyTrackingPreInclude
    OnlyTrackingPreInclude(flags)

    flags.fillFromArgs()

    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    # Needed for PixelID and SCT_ID
    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc.merge(ITkPixelReadoutGeometryCfg(flags))
    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    acc.merge(ITkStripReadoutGeometryCfg(flags))

    # RoI creator
    from ActsConfig.ActsRegionsOfInterestConfig import ActsMainRegionsOfInterestCreatorAlgCfg
    acc.merge(ActsMainRegionsOfInterestCreatorAlgCfg(flags))

    # Data Preparation - Clustering
    from ActsConfig.ActsClusterizationConfig import ActsPixelClusterizationAlgCfg
    acc.merge(ActsPixelClusterizationAlgCfg(flags,OutputLevel = DEBUG))

    from ActsConfig.ActsClusterizationConfig import ActsStripClusterizationAlgCfg
    acc.merge(ActsStripClusterizationAlgCfg(flags,OutputLevel = DEBUG))


    acc.merge(GPUClusterizationCfg(flags))
    acc.printConfig(withDetails=True, summariseProps=True)

    statusCode = acc.run(flags.Exec.MaxEvents)
    assert statusCode.isSuccess(), "Application execution did not succeed"
