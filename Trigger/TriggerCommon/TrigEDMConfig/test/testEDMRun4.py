#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from TrigEDMConfig.TriggerEDM import testEDMList, getRawTriggerEDMList

def main():

  return testEDMList(getRawTriggerEDMList(flags=None,runVersion=4))

if __name__ == "__main__":
  import sys
  sys.exit(main())
