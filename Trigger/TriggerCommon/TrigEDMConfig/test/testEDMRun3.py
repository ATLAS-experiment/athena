#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from TrigEDMConfig.TriggerEDM import testEDMList, getRawTriggerEDMList

def main():

  runVersion = 3
  return testEDMList(getRawTriggerEDMList(flags=None,runVersion=runVersion),check_edmdetails_for_version=runVersion)

if __name__ == "__main__":
  import sys
  sys.exit(main())
