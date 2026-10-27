# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""
Entry point the embedded Athena application uses to configure itself.
"""

import os

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthCUDAServices.CUDAConfigFlags import CUDAStream
from AthDeviceComps.DeviceConfigFlags import DeviceBackend
from ActsConfig.ActsConfigFlags import DeviceTritonMode
from InDetConfig.ITkActsDeviceTrackRecoConfig import ITkActsDeviceTrackRecoCfg

# Name of the AthSequencer that holds the per-request device chain
DEFAULT_SEQUENCE_NAME = "TracccDeviceRecoSeq"

# The store keys the C++ side has to name for itself, as used by
# ITkActsDeviceTrackRecoCfg
STOREGATE_KEYS = {
    "cells": "TracccCellCollection",
    "measurements": "TracccMeasurementCollection",
    "clusters": "TracccClusterCollection",
    "tracks": "TracccTrackCollection",
}


def bootstrap(nSlots: int = 1,
              sequenceName: str = DEFAULT_SEQUENCE_NAME) -> dict:
    """Configure and initialize the embedded Athena application.

    Raises on failure.

    @param nSlots number of event-store slots to create, one per Triton model instance
    @param sequenceName name of the AthSequencer holding the device chain; the
           C++ TracccTritonRunner looks this up and executes it per request
    @return the StoreGate keys the C++ side needs, as configured above.
            TracccTritonInitializer copies these into its Config.
    """

    if nSlots < 1:
        raise ValueError(
            f"TracccTritonBootstrap.bootstrap: nSlots must be >= 1, got {nSlots}"
        )

    flags = initConfigFlags()

    # GPU device chain.
    flags.Device.Backend = DeviceBackend.CUDA
    flags.Acts.Device.doClusterization = True
    flags.Acts.Device.doSpacePointFormation = True
    flags.Acts.Device.doSeeding = True
    flags.Acts.Device.doTrackReconstruction = True
    flags.Acts.Device.tritonMode = DeviceTritonMode.Server
    flags.CUDA.Stream = CUDAStream.PerEvent

    # The device detector description is converted from the ACTS Gen3 tracking geometry
    flags.Acts.TrackingGeometry.UseBlueprint = True
    flags.Acts.TrackingGeometry.BuildDetrayGeometry = True

    # No input / no event loop needed; the Runner drives the algorithms directly
    flags.Input.Files = []
    flags.Exec.MaxEvents = 0

    flags.Input.isMC = True
    flags.GeoModel.AtlasVersion = "ATLAS-P2-RUN4-03-00-01"
    flags.IOVDb.GlobalTag = "COND-MC21-SDR-RUN4-06"

    # The same detector as the client's job is needed for conversions
    from InDetConfig.ConfigurationHelpers import OnlyTrackingPreInclude
    OnlyTrackingPreInclude(flags)
    flags.Detector.EnableHGTD = False

    # Flag overrides from the tritonserver environment, e.g.
    # TRACCC_TRITON_FLAGS="Acts.Device.seedingStrategy=SeedingStrategy.Gbts"
    for flag in os.environ.get("TRACCC_TRITON_FLAGS", "").split(";"):
        if flag.strip():
            flags.fillFromString(flag)

    flags.lock()

    acc = MainServicesCfg(flags)

    # One event-store slot per Triton model instance.
    acc.addService(
        CompFactory.SG.HiveMgrSvc("EventDataSvc", NSlots=nSlots), create=True
    )

    # The full sequency the runner uses
    acc.addSequence(CompFactory.AthSequencer(
        sequenceName,
        Sequential=True,
        IgnoreFilterPassed=True,
        StopOverride=True,
    ))
    # Same cuts as the primary pass of the client's job
    passFlags = flags.cloneAndReplace(
        "Tracking.ActiveConfig",
        f"Tracking.{flags.Tracking.ITkPrimaryPassConfig.value}Pass")
    acc.merge(ITkActsDeviceTrackRecoCfg(passFlags), sequenceName=sequenceName)

    # SG::getNSlots() caches the number of event slots the first time it is
    # called, and gives 1 if EventDataSvc cannot be retrieved yet. With
    # MetaDataSvc as a proxy provider, the first StoreGateSvc (initialized by
    # GeoModelSvc) creates MetaDataSvc -> OutputStreamSequencerSvc, which asks
    # before EventDataSvc exists; PerEventStreamSvc then has streams for one
    # slot only. There is no input file, so no input metadata to provide.
    pps = acc.getService("ProxyProviderSvc")
    pps.ProviderNames = [name for name in pps.ProviderNames
                         if "MetaDataSvc" not in name]
    # MetaDataSvc also created AthenaPoolCnvSvc at initialization. Created
    # lazily instead, it is created while IOVDbSvc holds the database lock and
    # registers with IncidentSvc, while another slot's clearStore() holds the
    # IncidentSvc lock waiting for the database lock: deadlock.
    acc.addService(acc.getService("AthenaPoolCnvSvc"), create=True)

    # Equivalent to ComponentAccumulator.run() without the the event loop
    app = acc.createApp()
    sc = app.initialize()
    if not sc.isSuccess():
        raise RuntimeError(
            "TracccTritonBootstrap.bootstrap: ApplicationMgr.initialize() failed"
        )

    sc = app.start()
    if not sc.isSuccess():
        raise RuntimeError(
            "TracccTritonBootstrap.bootstrap: ApplicationMgr.start() failed"
        )

    return dict(STOREGATE_KEYS)
