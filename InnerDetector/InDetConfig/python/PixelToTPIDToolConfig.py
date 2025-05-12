# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Configuration of PixelToTPIDTool package
from AthenaConfiguration.ComponentFactory import CompFactory

def PixelToTPIDToolCfg(flags, name = "PixelToTPIDTool", **kwargs):
    from PixelConditionsAlgorithms.PixelConditionsConfig import PixeldEdxAlgCfg, PixelChargeCalibCondCfg   
    acc = PixeldEdxAlgCfg(flags) # To produce PixeldEdxData CondHandle
    acc.merge(PixelChargeCalibCondCfg(flags))
    acc.setPrivateTools(CompFactory.InDet.PixelToTPIDTool(name, **kwargs))
    return acc
