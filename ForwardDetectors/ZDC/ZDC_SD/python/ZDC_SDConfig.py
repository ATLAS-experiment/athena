# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ZDC_FiberSDCfg(flags, name="ZDC_FiberSD", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("LogicalVolumeNames", ["ZDC::Strip_Logical"
                                            ,"ZDC::RPD_Core_Active_Logical*"   
                                            ,"ZDC::RPD_Clad_Active_Logical*"   
                                            ,"ZDC::RPD_Buff_Active_Logical*" 
                                            ,"ZDC::RPD_Core_Readout_Logical"  
                                            ,"ZDC::RPD_Clad_Readout_Logical"  
                                            ,"ZDC::RPD_Buff_Readout_Logical"
                                            ,"ZDC::BRAN_Rod_Logical"])
    kwargs.setdefault("OutputCollectionNames", ["ZDC_SimFiberHit_Collection"])
    result.setPrivateTools(CompFactory.ZDC_FiberSDTool(name, **kwargs))
    return result

def ZDC_G4CalibSDCfg(flags, name="ZDC_G4CalibSD", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("LogicalVolumeNames", ["ZDC::*"])
    kwargs.setdefault("OutputCollectionNames", ["ZDC_CalibrationHits"])
    result.setPrivateTools(CompFactory.ZDC_G4CalibSDTool(name, **kwargs))
    return result
