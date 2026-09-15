# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthDeviceComps.DeviceConfigFlags import DeviceBackend

# ============================================================
# Service configurations
# ============================================================

def CUDAMagFieldProviderToolCfg(flags,
                                name="CUDAMagFieldProviderTool",
                                **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault("MagFieldStorage","global_memory")
    acc.setPrivateTools(
        CompFactory.ActsTrk.CUDAMagFieldProviderTool(name, **kwargs))
    return acc

def HIPMagFieldProviderToolCfg(flags,
                                name="HIPMagFieldProviderTool",
                                **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault("MagFieldStorage","global_memory")
    acc.setPrivateTools(
        CompFactory.ActsTrk.HIPMagFieldProviderTool(name, **kwargs))
    return acc

def DeviceMagFieldProviderToolCfg(flags,
                                **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    if flags.Device.Backend is DeviceBackend.CUDA:
        acc.setPrivateTools(acc.popToolsAndMerge(CUDAMagFieldProviderToolCfg(flags)))
        
    elif ((flags.Device.Backend is DeviceBackend.HIPAMD) or (flags.Device.Backend is DeviceBackend.HIPNVIDIA)):    
        acc.setPrivateTools(acc.popToolsAndMerge(HIPMagFieldProviderToolCfg(flags)))
    else:
        raise ValueError(f"Unsupported device backend: {flags.Acts.DeviceBackend}")   

    return  acc

def JSONDeviceMagFieldProviderSvcCfg(flags, **kwargs) -> ComponentAccumulator:

    acc = ComponentAccumulator()

    kwargs.setdefault("MagFieldFile",     "dev/ACTS/detray-itk/ITk_bfield.cvf")
    kwargs.setdefault("DeviceMagFieldObjectName", "TracccDeviceMagField")
    kwargs.setdefault("HostMagFieldObjectName", "TracccHostMagField")

    kwargs.setdefault("DeviceMagFieldProviderTool", acc.popToolsAndMerge(DeviceMagFieldProviderToolCfg(flags)))
    
    svc = CompFactory.ActsTrk.JSONDeviceMagFieldProviderSvc(**kwargs)
    acc.addService(svc, primary=True, create=True)
    return acc



