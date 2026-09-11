# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""
Entry point the embedded Gaudi kernel uses to configure itself.
"""

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthCUDAServices.CUDAConfigFlags import CUDAStream
from AthDeviceComps.DeviceConfigFlags import DeviceBackend
from TracccTritonBackend.TracccTritonBackendConfig import (
    DEFAULT_SEQUENCE_NAME,
    STOREGATE_KEYS,
)


def bootstrap(nSlots: int = 1,
              sequenceName: str = DEFAULT_SEQUENCE_NAME) -> dict:
    """Configure and initialize the embedded Gaudi kernel. Raises on failure.

    @param nSlots number of event-store slots to create, one per Triton model instance
    @param sequenceName name of the AthSequencer holding the device chain; the
           C++ TracccTritonRunner looks this up and executes it per request
    @return the StoreGate keys the C++ side needs, as configured above.
            TracccTritonInitializer copies these into its Config, so the names
            live only in TracccTritonBackendConfig and cannot drift.
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
    flags.CUDA.Stream = CUDAStream.Single

    # No input / no event loop needed; the Runner drives the algorithms directly
    flags.Input.Files = []
    flags.Exec.MaxEvents = 0

    flags.lock()

    acc = MainServicesCfg(flags)

    # One event-store slot per Triton model instance.
    acc.addService(
        CompFactory.SG.HiveMgrSvc("EventDataSvc", NSlots=nSlots), create=True
    )

    from TracccTritonBackend.TracccTritonBackendConfig import (
        TracccTritonDeviceRecoCfg,
    )
    acc.merge(TracccTritonDeviceRecoCfg(flags, sequenceName=sequenceName))

    # Equivalent to ComponentAccumulator.run() 
    app = acc.createApp()
    sc = app.initialize()
    if not sc.isSuccess():
        raise RuntimeError(
            "TracccTritonBootstrap.bootstrap: ApplicationMgr.initialize() failed"
        )

    return dict(STOREGATE_KEYS)
