# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthDeviceComps.AthDeviceCompsConfig import MemoryResourcesToolCfg, CopyToolCfg

# ============================================================
# Service configurations
# ============================================================

def DeviceDetectorDescriptionSvcCfg(flags, name="ActsDeviceDetectorDescriptionSvc", **kwargs) -> ComponentAccumulator:

    acc = ComponentAccumulator()

    if not flags.Acts.TrackingGeometry.UseBlueprint:
        raise ValueError("In-memory conversion to Detray detector is not possible without ACTS Gen3 Geometry (set by 'Acts.TrackingGeometry.UseBlueprint' flag).")
    if not flags.Acts.TrackingGeometry.BuildDetrayGeometry:
        raise ValueError("Detray detector description can not be built without a Detray detector (set by 'Acts.TrackingGeometry.BuildDetrayGeometry' flag).")
    if flags.Detector.GeometryCalo or flags.Detector.GeometryMuon:
        raise ValueError("Detray detector description can not be built with calo/muon volumes in the tracking geometry yet.")

    if 'TrackingGeometrySvc' not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsTrackingGeometrySvcCfg
        kwargs.setdefault(
            "TrackingGeometrySvc",
            acc.getPrimaryAndMerge(ActsTrackingGeometrySvcCfg(flags)),
        )

    kwargs.setdefault("MemoryResourcesTool", acc.popToolsAndMerge(MemoryResourcesToolCfg(flags)))
    kwargs.setdefault("CopyProviderTool", acc.popToolsAndMerge(CopyToolCfg(flags)))
    kwargs.setdefault("DeviceDigitizationObjectName", "TracccDeviceDigitizationConfig")
    kwargs.setdefault("HostDigitizationObjectName", "TracccHostDigitizationConfig")
    kwargs.setdefault("GeoIdMappingObjectName", "TracccGeometryIdMapping")
    kwargs.setdefault("HostDetectorName", "TracccHostDetectorGeometry")
    kwargs.setdefault("DeviceDetectorName", "TracccDeviceDetectorGeometry")

    # The objects recorded by the service are retrieved from the detector store
    # by the device algorithms during their initialization, so the service has
    # to be created before any of the algorithms is initialized.
    svc = CompFactory.ActsTrk.DeviceDetectorDescriptionSvc(name, **kwargs)
    acc.addService(svc, primary=True, create=True)
    return acc

# ============================================================
# Conditions algorithm configurations
# ============================================================

# Properties which belong to the service building the static detector description
_detDescSvcProperties = (
    "TrackingGeometrySvc",
    "DeviceDigitizationObjectName",
    "HostDigitizationObjectName",
    "GeoIdMappingObjectName",
    "HostDetectorName",
    "DeviceDetectorName",
)

def DeviceDetectorDescriptionCondAlgCfg(flags, name="ActsDeviceDetectorDescriptionCondAlg", **kwargs) -> ComponentAccumulator:

    acc = ComponentAccumulator()

    if 'DeviceDetectorDescriptionSvc' not in kwargs:
        svcKwargs = {k: kwargs.pop(k) for k in _detDescSvcProperties if k in kwargs}
        if 'OutputLevel' in kwargs:
            svcKwargs['OutputLevel'] = kwargs['OutputLevel']
        kwargs.setdefault(
            "DeviceDetectorDescriptionSvc",
            acc.getPrimaryAndMerge(DeviceDetectorDescriptionSvcCfg(flags, **svcKwargs)),
        )

    from SiLorentzAngleTool.ITkPixelLorentzAngleConfig import ITkPixelLorentzAngleToolCfg
    from SiLorentzAngleTool.ITkStripLorentzAngleConfig import ITkStripLorentzAngleToolCfg

    kwargs.setdefault("PixelLorentzAngleTool", acc.popToolsAndMerge(ITkPixelLorentzAngleToolCfg(flags)))
    kwargs.setdefault("StripLorentzAngleTool", acc.popToolsAndMerge(ITkStripLorentzAngleToolCfg(flags)))

    kwargs.setdefault("MemoryResourcesTool", acc.popToolsAndMerge(MemoryResourcesToolCfg(flags)))
    kwargs.setdefault("CopyProviderTool", acc.popToolsAndMerge(CopyToolCfg(flags)))
    kwargs.setdefault("DeviceConditionsObjectName", "TracccDeviceCondConfig")
    kwargs.setdefault("HostConditionsObjectName", "TracccHostCondConfig")

    the_alg = CompFactory.ActsTrk.DeviceDetectorDescriptionCondAlg(name, **kwargs)
    acc.addCondAlgo(the_alg, primary = True)

    return acc
