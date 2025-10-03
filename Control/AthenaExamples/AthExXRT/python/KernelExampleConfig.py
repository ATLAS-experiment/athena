#!/usr/bin/env athena.py
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# "Standalone" test for exercising AthXRT with multiple kernels (krnl_VectorAdd and krnl_VectorMult).
#

import os
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import logging
from AthXRTServices.DeviceMgmtSvcConfig import DeviceMgmtSvcCfg
log = logging.getLogger("AthExXRT.XrtKernelExampleCfg")

def OCLKernelExampleCfg(flags):

    result = ComponentAccumulator()

    xclbin_list = [os.path.join(flags.FPGAMgmt.HLSDir, 'krnl_Combined.xclbin')]
    result.merge(DeviceMgmtSvcCfg(flags,
                                   xclbin_list))

    OCLVectorAddEx = CompFactory.AthExXRT.VectorAddOCLExampleAlg()
    result.addEventAlgo(OCLVectorAddEx)

    OCLVectorMulEx = CompFactory.AthExXRT.VectorMultOCLExampleAlg()
    result.addEventAlgo(OCLVectorMulEx)

    return result

def XrtKernelExampleCfg(flags):

    result = ComponentAccumulator()

    xclbin_list = [os.path.join(flags.FPGAMgmt.HLSDir, 'krnl_Combined.xclbin')]
    result.merge(DeviceMgmtSvcCfg(flags,
                                   xclbin_list))

    XRTVectorAddEx = CompFactory.AthExXRT.VectorAddXRTExampleAlg()
    result.addEventAlgo(XRTVectorAddEx)

    XRTVectorMulEx = CompFactory.AthExXRT.VectorMultXRTExampleAlg()
    result.addEventAlgo(XRTVectorMulEx)

    return result

def MultiKernelExampleCfg(flags):

    result = ComponentAccumulator()
    result.merge(XrtKernelExampleCfg(flags))
    result.merge(OCLKernelExampleCfg(flags))

    return result

if __name__ == "__main__":
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Exec.MaxEvents = 10

    from AthXRTServices.FPGAConfigFlags import createFPGAMgmtFlags
    createFPGAMgmtFlags()

    flags.addFlag("FPGAMgmt.doXRT", True)
    flags.addFlag("FPGAMgmt.doOCL", True)
    flags.fillFromArgs()
    flags.lock()

    cfg = MainServicesCfg(flags)
    if flags.FPGAMgmt.doXRT:
        cfg.merge(XrtKernelExampleCfg(flags))
    if flags.FPGAMgmt.doOCL:
        cfg.merge(OCLKernelExampleCfg(flags))

    import sys
    sys.exit(cfg.run().isFailure())
