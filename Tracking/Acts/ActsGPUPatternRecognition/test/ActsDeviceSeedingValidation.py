#!/usr/bin/env athena

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Run script for the GPU seeding validation:
#   Traccc chain: RDO -> traccc cells -> traccc measurements -> spacepoints -> seeds
#   ACTS chain: RDO -> clusters -> spacepoints -> seeds
#   Traccc vs ACTS comparison: clusters, spacepoints, seeds

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Constants import DEBUG

from ActsGPUGeometry.ActsGPUGeometryConfig import JSONDeviceDetectorDescriptionProviderSvcCfg
from ActsGPUDataPreparation.ActsGPUDataPreparationConfig import (
    DeviceClusterizationAlgCfg,
    DeviceSPFormationAlgCfg,
    
)
from ActsGPUPatternRecognition.ActsGPUPatternRecognitionConfig import DeviceGBTSSeedingAlgCfg, DeviceTripletSeedingAlgCfg

from ActsGPUEventCnv.ActsGPUEventCnvConfig import (
    RDOtoTracccCellConverterAlgCfg,
    TracccMeasurementConverterAlgCfg,
    TracccSeedConverterAlgCfg,
)

from AthCUDAServices.AthCUDAServicesConfig import StreamToolCfg
from AthDeviceComps.AthDeviceCompsConfig import HostMemoryResourceToolCfg, DeviceMemoryResourceToolCfg, CopyToolCfg, DeviceMemoryResourceToolCfg

# ============================================================
# Top-level configuration
# ============================================================

def RunGPUSeedingCfg(flags) -> ComponentAccumulator:

    try:
        if flags.Tracking.ActiveConfig.extension != "Acts":
            raise RuntimeError(f"wrong tracking pass: {flags.Tracking.ActiveConfig.extension}")
    except AttributeError:
        flags = flags.cloneAndReplace("Tracking.ActiveConfig",
                                      "Tracking.ITkActsPass")

    acc = ComponentAccumulator()

    # Service runs first — loads all device detector description data into detStore
    acc.merge(JSONDeviceDetectorDescriptionProviderSvcCfg(flags))

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

    acc.merge(DeviceSPFormationAlgCfg(flags,
        InputTracccMeasurements="TracccMeasurementCollection",
        OutputTracccPixelSpacepoints="TracccPixelSpacepointCollection",
        OutputLevel = DEBUG))

    acc.merge(DeviceGBTSSeedingAlgCfg(flags,
        InputTracccPixelSpacepoints="TracccPixelSpacepointCollection",
        InputTracccMeasurements="TracccMeasurementCollection",
        OutputTracccPixelSeeds="TracccPixelGBTSSeedCollection",
        OutputLevel = DEBUG))

    acc.merge(TracccMeasurementConverterAlgCfg(flags,
        InputMeasurements="TracccMeasurementCollection",
        InputClusters="TracccClusterCollection",
        InputCells="TracccCellCollection",
        ConvertClustersWithCells = flags.Tracking.doTruth,
        OutputPixelClusters="ITkTracccPixelClusters",
        OutputPixelSpacePoints="ITkTracccPixelSpacepoints",
        OutputMeasToPixelSP="ITkTracccMeasToPixelSP",
        OutputStripClusters="ITkTracccStripClusters",
        OutputLevel = DEBUG
    ))

    acc.merge(TracccSeedConverterAlgCfg(flags,
        InputSpacepoints="ITkTracccPixelSpacepoints",
        InputSpacepointsDevice="TracccPixelSpacepointCollection",
        InputMeasToPixelSP="ITkTracccMeasToPixelSP",
        InputSeeds="TracccPixelGBTSSeedCollection",
        OutputSeeds="ITkTracccPixelSeeds",
        OutputLevel = DEBUG
    ))


    ################################
    # ACTS Clusterization + Seeding
    ################################

    # RoI creator
    from ActsConfig.ActsRegionsOfInterestConfig import ActsMainRegionsOfInterestCreatorAlgCfg
    acc.merge(ActsMainRegionsOfInterestCreatorAlgCfg(flags))

    # Data Preparation - Clustering
    from ActsConfig.ActsClusterizationConfig import ActsPixelClusterizationAlgCfg
    acc.merge(ActsPixelClusterizationAlgCfg(flags,OutputLevel = DEBUG))

    from ActsConfig.ActsClusterizationConfig import ActsStripClusterizationAlgCfg
    acc.merge(ActsStripClusterizationAlgCfg(flags,OutputLevel = DEBUG))

    from ActsConfig.ActsSpacePointFormationConfig import ActsPixelSpacePointFormationAlgCfg
    acc.merge(ActsPixelSpacePointFormationAlgCfg(flags,name="ACTSPixelSPFormation",useCache=False, PixelClusters = "ITkPixelClusters", PixelSpacePoints = "ITkPixelSpacepoints"))

    from ActsConfig.ActsSeedingConfig import ActsPixelSeedingAlgCfg
    acc.merge(ActsPixelSeedingAlgCfg(
        flags,
        name             = 'ActsSeedingAlg',
        InputSpacePoints = ["ITkPixelSpacepoints"],
        OutputSeeds      = 'ActsPixelSeeds',
        useFastTracking  = flags.Tracking.doITkFastTracking,
        UsePixel         = True,
    ))

    # validation
    from SiLorentzAngleTool.ITkPixelLorentzAngleConfig import ITkPixelLorentzAngleToolCfg
    from SiLorentzAngleTool.ITkStripLorentzAngleConfig import ITkStripLorentzAngleToolCfg
    from AthenaConfiguration.ComponentFactory import CompFactory
    acc.addEventAlgo(CompFactory.ActsTrk.ActsClusterComparisonAlg(
        "GPU_ActsClusterComparisonAlg",
        checkSpacepoints=False,
        checkSeeds=True,
        monitoredSpacepointsKey="ITkTracccPixelSpacepoints",
        referenceSpacepointsKey="ITkPixelSpacePoints",
        monitoredPixelClustersKey="ITkTracccPixelClusters",
        referencePixelClustersKey="ITkPixelClusters",
        monitoredStripClustersKey="ITkTracccStripClusters",
        referenceStripClustersKey="ITkStripClusters",
        monitoredSeedsKey="ITkTracccPixelSeeds",
        referenceSeedsKey="ActsPixelSeeds",
        PixelLorentzAngleTool = acc.popToolsAndMerge(ITkPixelLorentzAngleToolCfg(flags)),
        StripLorentzAngleTool = acc.popToolsAndMerge(ITkStripLorentzAngleToolCfg(flags)),
        OutputLevel = DEBUG

    ))

    return acc

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    flags = initConfigFlags()

    # ---- Input ----
    flags.Input.Files = defaultTestFiles.RDO_RUN4
    
    flags.Output.AODFileName = "AOD.test.root"
    flags.Exec.MaxEvents = 1

    from ActsConfig.ActsConfigFlags import SeedingStrategy
    flags.Acts.SeedingStrategy=SeedingStrategy.Gbts

    # Set the Main Pass
    flags = flags.cloneAndReplace(
        "Tracking.ActiveConfig",
        "Tracking.ITkActsPass")
    
    flags.Tracking.doPixelDigitalClustering = True
    flags.Acts.Device.doCellSorting = True

    flags.fillFromArgs()

    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    msg_svc = acc.getService('MessageSvc')
    msg_svc.Format = "%t % F%{:d}W%C%7W%R%T %0W%M".format(flags.Common.MsgSourceLength)

    # Needed for PixelID and SCT_ID
    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc.merge(ITkPixelReadoutGeometryCfg(flags))
    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    acc.merge(ITkStripReadoutGeometryCfg(flags))

    if flags.Input.isMC:

        from xAODTruthCnv.xAODTruthCnvConfig import GEN_AOD2xAODCfg
        acc.merge(GEN_AOD2xAODCfg(flags))

        from InDetConfig.InDetPrepRawDataToxAODConfig import TruthParticleIndexDecoratorAlgCfg
        acc.merge(TruthParticleIndexDecoratorAlgCfg(flags))

        from JetRecConfig.JetRecoSteering import addTruthPileupJetsToOutputCfg
        acc.merge(addTruthPileupJetsToOutputCfg(flags))

    acc.merge(RunGPUSeedingCfg(flags))
    acc.printConfig(withDetails=True, summariseProps=True)

    statusCode = acc.run(flags.Exec.MaxEvents)
    assert statusCode.isSuccess(), "Application execution did not succeed"
