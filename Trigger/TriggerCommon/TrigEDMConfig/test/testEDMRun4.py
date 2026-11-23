#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from TrigEDMConfig.TriggerEDM import testEDMList, getRawTriggerEDMList

def main():

  runVersion = 5 # TODO: Update to 4 once no longer relying on proto run 4 EDM.
  return testEDMList(getRawTriggerEDMList(flags=None,runVersion=runVersion),do_extra_checks_for_version=runVersion)

if __name__ == "__main__":
  import sys
  sys.exit(main())
