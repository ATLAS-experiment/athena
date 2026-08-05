#!/usr/bin/env athenaEF.py
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthExMonitored.MonitoredConfig import MonitoredCfg


def TrigExISHistCfg(flags):
   """Test of histogram and IS publishing"""

   # Algorithm with histograms
   cfg = MonitoredCfg(flags)

   # Algorithm with IS publication
   cfg.addEventAlgo( CompFactory.TrigExISPublishing() )

   # athenaEF adds its own THistSvc
   cfg.dropService('THistSvc')

   return cfg
