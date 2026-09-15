# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthDeviceComps.AthDeviceCompsConfig import MemoryResourcesToolCfg, CopyToolCfg

# ============================================================
# Service configurations
# ============================================================

def JSONDeviceDetectorDescriptionProviderSvcCfg(flags, **kwargs) -> ComponentAccumulator:

    acc = ComponentAccumulator()

    geoTag = flags.GeoModel.AtlasVersion
    if geoTag not in ["ATLAS-P2-RUN4-03-00-01","ATLAS-P2-RUN4-03-00-00"]:
        from AthenaCommon.Logging import logging
        log = logging.getLogger("JSONDeviceDetectorDescriptionCfg")
        log.warning(
            "detray<->Athena id mapping (athenaIdentifierToDetrayMap.txt) is only "
            "validated against geo tag %s — this job is using '%s'. The detray/"
            "Athena identifier map will likely be incomplete or wrong for this "
            "geometry, and RDOtoTracccCellConverterAlg may fail with "
            "'No detray id found for Athena identifier ...' partway through the job.",
            "ATLAS-P2-RUN4-03-00-*", geoTag,
        )

    kwargs.setdefault("MemoryResourcesTool", acc.popToolsAndMerge(MemoryResourcesToolCfg(flags)))
    kwargs.setdefault("CopyProviderTool", acc.popToolsAndMerge(CopyToolCfg(flags)))
    kwargs.setdefault("GeometryFile",     "dev/ACTS/detray-itk/detray_detector_geometry-for-fun.json")
    kwargs.setdefault("DigitizationFile", "dev/ACTS/detray-itk/ITk_digitization_config.json")
    kwargs.setdefault("ConditionsFile",   "dev/ACTS/detray-itk/ITk_conditions_config.json")
    kwargs.setdefault("MapFile",          "dev/ACTS/detray-itk/athenaIdentifierToDetrayMap.txt")
    kwargs.setdefault("SurfaceGridFile",  "dev/ACTS/detray-itk/detray_detector_surface_grids.json")
    kwargs.setdefault("MaterialFile",     "dev/ACTS/detray-itk/detray_detector_material_maps.json")
    kwargs.setdefault("GeoIdMappingObjectName",     "TracccGeometryIdMapping")
    kwargs.setdefault("DeviceDigitizationObjectName", "TracccDeviceDigitizationConfig")
    kwargs.setdefault("HostDigitizationObjectName", "TracccHostDigitizationConfig")
    kwargs.setdefault("DeviceConditionsObjectName", "TracccDeviceCondConfig")
    kwargs.setdefault("HostConditionsObjectName", "TracccHostCondConfig")
    kwargs.setdefault("DeviceDetectorName", "TracccDeviceDetectorGeometry")
    kwargs.setdefault("HostDetectorName", "TracccHostDetectorGeometry")
    svc = CompFactory.ActsTrk.JSONDeviceDetectorDescriptionProviderSvc(**kwargs)
    acc.addService(svc, primary=True, create=True)
    return acc
