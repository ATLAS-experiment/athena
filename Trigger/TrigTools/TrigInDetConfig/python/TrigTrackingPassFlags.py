# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
import AthenaCommon.SystemOfUnits as Units


def createTrigTrackingPassFlags(mode="InDet"):

  from TrigInDetConfig.BuildSignatureFlags import signatureTrigTrackingFlags
  return signatureTrigTrackingFlags(mode)



import unittest

class FlagsCopiedTest(unittest.TestCase):
    def setUp(self):
        from AthenaConfiguration.AllConfigFlags import initConfigFlags
        flags = initConfigFlags()
        flags.Trigger.doID
        flags.Trigger.InDetTracking.muon
        flags.Trigger.InDetTracking.electron.minPT = 2.0 * Units.GeV
        flags.Trigger.ITkTracking.muon
        self.newflags = flags.cloneAndReplace('Tracking.ActiveConfig', 'Trigger.InDetTracking.electron')
        self.newflags4 = flags.cloneAndReplace('Tracking.ActiveConfig', 'Trigger.ITkTracking.muon')
        
        self.newflags.dump(".*InDet")

    def runTest(self):
        self.assertEqual(self.newflags.Tracking.ActiveConfig.minPT, 2.0 * Units.GeV, msg="Flags are not copied")
        self.assertEqual(type(self.newflags4.Tracking.ActiveConfig.minPT), list, msg="Eta dependant cuts don't exist")


if __name__ == "__main__":
    unittest.main()
