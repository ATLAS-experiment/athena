
"""
Entry point the embedded Gaudi kernel uses to configure itself.
"""

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthDeviceComps.DeviceConfigFlags import DeviceBackend


def bootstrap() -> None:
    """Configure and initialize the embedded Gaudi kernel. Raises on failure."""

    flags = initConfigFlags()

    # GPU device chain.
    flags.Device.Backend = DeviceBackend.CUDA
    flags.Acts.Device.doClusterization = True
    flags.Acts.Device.doSpacePointFormation = True
    flags.Acts.Device.doSeeding = True
    flags.Acts.Device.doTrackReconstruction = True

    # No input / no event loop needed; the Runner drives the algorithms
    # directly, so instance_group { count: 1 } pins the process to one GPU
    # (see AthCUDAServices' DeviceID handling / CUDA_VISIBLE_DEVICES).
    flags.Input.Files = []
    flags.Exec.MaxEvents = 0

    flags.lock()

    acc = MainServicesCfg(flags)

    from TracccTritonBackend.TracccTritonBackendConfig import (
        TracccTritonDeviceRecoCfg,
    )
    acc.merge(TracccTritonDeviceRecoCfg(flags))

    # Equivalent to ComponentAccumulator.run() up through app.initialize():
    # createApp() resolves every property (including the ToolHandle trees
    # under the provider tools) and calls ApplicationMgr.configure(); we then
    # call initialize() ourselves and stop, instead of start()/run().
    app = acc.createApp()
    sc = app.initialize()
    if not sc.isSuccess():
        raise RuntimeError(
            "TracccTritonBootstrap.bootstrap: ApplicationMgr.initialize() failed"
        )
