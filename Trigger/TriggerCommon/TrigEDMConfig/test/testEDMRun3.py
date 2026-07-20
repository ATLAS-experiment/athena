#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from TrigEDMConfig.TriggerEDMRun3 import TriggerHLTListRun3
from TrigEDMConfig.TriggerEDM import testEDMList
from AthenaCommon.Logging import logging
log = logging.getLogger('testEDMRun3')

def main():

  return testEDMList(TriggerHLTListRun3)

if __name__ == "__main__":
  import sys
  sys.exit(main())
