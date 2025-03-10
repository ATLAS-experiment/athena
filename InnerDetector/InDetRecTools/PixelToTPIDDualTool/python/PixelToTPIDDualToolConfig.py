# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
  
#********************************************************************
# PixelToTPIDDualToolConfig.py 
# Decorates tracks with equalized dE/dx measurements. 
# for use in physics analysis
#********************************************************************

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def PixelToTPIDDualToolCfg(flags, name="PixelToTPIDDualTool", **kwargs):
   """Configure the pixel ToT PID tool"""
   acc = ComponentAccumulator()
   # kwargs.setdefault("<property>", <value> )
   the_tool = CompFactory.CP.PixelToTPIDDualTool(name, **kwargs)   
   acc.setPrivateTools(the_tool)
   return acc   
