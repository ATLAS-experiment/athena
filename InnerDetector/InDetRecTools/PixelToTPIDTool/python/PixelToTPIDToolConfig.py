# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
  
#********************************************************************
# PixelToTPIDToolConfig.py 
# Decorates tracks with equalized dE/dx measurements. 
# for use in physics analysis
#********************************************************************

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def PixelToTPIDToolCfg(flags, name="PixelToTPIDTool", **kwargs):
   """Configure the pixel ToT PID tool"""
   acc = ComponentAccumulator()

   from PixelConditionsAlgorithms.PixelConditionsConfig import PixelChargeCalibCondCfg
   acc.merge(PixelChargeCalibCondCfg(flags))

   acc.setPrivateTools(CompFactory.CP.PixelToTPIDTool(name, **kwargs))

   return acc   
