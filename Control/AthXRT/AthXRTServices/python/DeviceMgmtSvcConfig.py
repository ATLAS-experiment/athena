#!/usr/bin/env athena.py
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def DeviceMgmtSvcCfg(flags, xclbin_list):

    result = ComponentAccumulator()

    XRTSvc = CompFactory.AthXRT.DeviceMgmtSvc(XclbinPathsList = xclbin_list)
    result.addService(XRTSvc)
    
    return result
