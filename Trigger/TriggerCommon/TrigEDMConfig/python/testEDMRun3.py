#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from TrigEDMConfig.TriggerEDMRun3 import TriggerHLTListRun3
from TrigEDMConfig.TriggerEDM import AllowedOutputFormats, testEDMList
from AthenaCommon.Logging import logging
log = logging.getLogger('testEDMRun3')

def dumpListToJson(fileName):
  from TrigEDMConfig.TriggerEDM import getTriggerEDMList
  import json
  edmDict = dict([(fmt, getTriggerEDMList(flags=None, key=fmt, runVersion=3)) for fmt in AllowedOutputFormats])
  with open(fileName,'w') as f:
    json.dump(edmDict, f)

def main():

  return testEDMList(TriggerHLTListRun3)

if __name__ == "__main__":
  import sys
  sys.exit(main())
