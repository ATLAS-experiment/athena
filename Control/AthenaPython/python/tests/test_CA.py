# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import AthenaCommon.SystemOfUnits as Units
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaPython.tests.PyTestsLib import MyAlg, MySvc, MyTool

import unittest

class Test( unittest.TestCase ):
   def test_basic(self):
      flags = initConfigFlags()
      flags.lock()

      cfg = MainServicesCfg(flags)
      cfg.addSequence(CompFactory.AthSequencer("MySeq"))

      cfg.addEventAlgo( MyAlg("MyAlg1", eta = 2.5, pt = 42.),
                        sequenceName = "MySeq" )

      cfg.addEventAlgo( MyAlg("MyAlg2", eta = 5.1, pt = 20.*Units.GeV, filterPassed = False),
                        sequenceName = "MySeq" )

      # alg3 should never be executed due to filterPassed of alg2
      cfg.addEventAlgo( MyAlg("MyAlg3", eta = 4.9, pt = 15.*Units.GeV,
                              mytool = MyTool("MyTool", counter = 50, parent="MyAlg3")),
                        sequenceName = "MySeq" )

      cfg.addService( MySvc(), create=True )

      with open('test_CA.pkl','wb') as f:
         cfg.store(f)
      sc = cfg.run(2)
      self.assertFalse(sc.isFailure())

   def test_merge(self):
      cfg1 = ComponentAccumulator()
      cfg2 = ComponentAccumulator()
      cfg1.addEventAlgo( MyAlg(name='Alg1', px=1) )
      cfg2.addEventAlgo( MyAlg(name='Alg2', px=2) )
      cfg1.merge(cfg2)
      cfg1.wasMerged()

   def test_merge_fail(self):
      cfg1 = ComponentAccumulator()
      cfg2 = ComponentAccumulator()
      cfg1.addEventAlgo( MyAlg(px=1) )
      cfg2.addEventAlgo( MyAlg(px=2) )
      with self.assertRaises(ValueError):
         cfg1.merge(cfg2)


if __name__ == "__main__":
   unittest.main()
