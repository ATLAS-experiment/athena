#!/usr/bin/env athena

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Run script for GPU geometry creation:
# JSON geometry file -> detray geometry (required)
# JSON digitization file -> traccc digitization config (required)
# JSON conditions file -> traccc conditions config (required)
# CSV map file -> athena<->detray ID map (required)
# JSON material file -> detray material (optional)
# JSON surface grid file -> detray surface grid (optional)
# CVF magnetic field -> covfie magnetic field (required)

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from AthCUDAServices.AthCUDAServicesConfig import HostMemoryResourceToolCfg, DeviceMemoryResourceToolCfg, CopyToolCfg, StreamToolCfg

from ActsGPUGeometry.ActsGPUGeometryConfig import JSONDeviceDetectorDescriptionProviderSvcCfg, DeviceDetectorDescriptionCondAlgCfg, DeviceDetectorDescriptionValidationAlgCfg
from ActsGPUDataPreparation.ActsGPUDataPreparationConfig import CUDAClusterizerToolCfg,  DeviceClusterizationAlgCfg
from ActsGPUEventCnv.ActsGPUEventCnvConfig import RDOtoTracccCellConverterAlgCfg, TracccMeasurementConverterAlgCfg

from AthenaCommon.Constants import DEBUG

def GPUGeometryCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # Service runs first — loads all device detector description data into detStore
    acc.merge(JSONDeviceDetectorDescriptionProviderSvcCfg(flags,
        HostConditionsObjectName="JSONTracccHostCondConfig",
        HostDigitizationObjectName="JSONTracccHostDigitizationConfig",
        DeviceConditionsObjectName="JSONTracccDeviceCondConfig",
        DeviceDigitizationObjectName="JSONTracccDeviceDigitizationConfig",
        DeviceDetectorName="JSONTracccDeviceDetectorGeometry",
        HostDetectorName="JSONTracccHostDetectorGeometry",
        OutputLevel = DEBUG
    ))

    acc.merge(DeviceDetectorDescriptionCondAlgCfg(flags,
        HostDetectorName = "JSONTracccHostDetectorGeometry",
        HostConditionsObjectName="TracccHostCondConfig",
        HostDigitizationObjectName="TracccHostDigitizationConfig",
        DeviceConditionsObjectName="TracccDeviceCondConfig",
        DeviceDigitizationObjectName="TracccDeviceDigitizationConfig",
        OutputLevel = DEBUG,
    ))

    acc.merge(DeviceDetectorDescriptionValidationAlgCfg(flags,
        MonDesignObjectName = "TracccHostDigitizationConfig",
        MonCondKey = "TracccHostCondConfig",
        RefHostDesignObjectName = "JSONTracccHostDigitizationConfig",
        RefHostCondKey = "JSONTracccHostCondConfig",
        OutputLevel = DEBUG
    ))

    return acc

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    flags = initConfigFlags()

    from AthenaConfiguration.Enums import ProductionStep
    flags.Common.ProductionStep = ProductionStep.Simulation
    from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultConditionsTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN4
    flags.IOVDb.GlobalTag = defaultConditionsTags.RUN4_MC
    flags.GeoModel.Align.Dynamic = False

    flags.Detector.GeometryITkPixel = True
    flags.Detector.GeometryITkStrip = True
    flags.Detector.GeometryBpipe = True
    flags.Detector.GeometryCalo = False
    flags.Detector.GeometryMuon = False

    # ---- Input ----
    flags.Input.Files = defaultTestFiles.RDO_RUN4

    flags.Exec.MaxEvents = 1

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

    from InDetConfig.SiSpacePointFormationConfig import ITkSiElementPropertiesTableCondAlgCfg
    acc.merge(ITkSiElementPropertiesTableCondAlgCfg(flags))


    msg_svc = acc.getService('MessageSvc')
    msg_svc.Format = "%t % F%{:d}W%C%7W%R%T %0W%M".format(flags.Common.MsgSourceLength)

    acc.merge(GPUGeometryCfg(flags))
    acc.printConfig(withDetails=True, summariseProps=True)

    statusCode = acc.run(flags.Exec.MaxEvents)
    assert statusCode.isSuccess(), "Application execution did not succeed"
