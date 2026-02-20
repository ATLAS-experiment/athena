#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import unittest

from GaudiConfig2 import Configurables
from GaudiKernel.DataHandle import DataHandle

# always need to import this to initialize our semantics:
from AthenaConfiguration import AtlasSemantics


class TestVarHandleKey(unittest.TestCase):
   """Test VarHandleKey properties"""

   def test_handle(self):
      alg = Configurables.HiveAlgC()
      alg.Key_R1 = "R1"
      alg.Key_W1 = "W1"
      self.assertIsInstance(alg.Key_R1, DataHandle)
      self.assertIsInstance(alg.Key_W1, DataHandle)
      self.assertEqual(alg.Key_R1, "R1")
      self.assertEqual(alg.Key_R1.type(), "HiveDataObj")
      self.assertFalse(alg.Key_R1.isCondition())
      self.assertEqual(alg.Key_R1.mode(), "R")
      self.assertEqual(alg.Key_W1.mode(), "W")

   def test_handleAssign(self):
      alg = Configurables.HiveAlgC()
      alg.Key_W1 = "W1"
      self.assertIsInstance(alg.Key_W1, DataHandle)
      alg.Key_R1 = alg.Key_W1
      self.assertEqual(alg.Key_R1, "W1")          # key changes
      self.assertEqual(alg.Key_R1.mode(), "R")    # but still a ReadHandle
      self.assertIsInstance(alg.Key_R1.Path, str) # value type remains string

   def test_condHandle(self):
      alg = Configurables.CondAlgZ()
      alg.Key_RCH1 = "R1"
      alg.Key_WCH = "W1"
      self.assertIsInstance(alg.Key_RCH1, DataHandle)
      self.assertIsInstance(alg.Key_WCH, DataHandle)
      self.assertTrue(alg.Key_RCH1.isCondition())
      self.assertEqual(alg.Key_RCH1.mode(), "R")
      self.assertEqual(alg.Key_WCH.mode(), "W")


class TestVarHandleKeyArray(unittest.TestCase):
   """Test VarHandleKeyArray properties"""

   def test_semantics(self):
      alg = Configurables.HiveAlgV()
      s = alg._descriptors['Key_RV'].semantics
      self.assertIsInstance(s, AtlasSemantics.VarHandleArraySematics)
      self.assertEqual(s._mode, 'R')
      self.assertEqual(s._isCond, False)
      self.assertEqual(s._type, 'HiveDataObj')

      s = alg._descriptors['Key_WV'].semantics
      self.assertIsInstance(s, AtlasSemantics.VarHandleArraySematics)
      self.assertEqual(s._mode, 'W')
      self.assertEqual(s._isCond, False)
      self.assertEqual(s._type, 'HiveDataObj')

   def test_assign(self):
      alg1 = Configurables.HiveAlgV()
      alg2 = Configurables.HiveAlgV()
      alg1.Key_WV = ['foo']
      # The .data is needed due to https://gitlab.cern.ch/gaudi/Gaudi/-/issues/264
      self.assertEqual(alg1.Key_WV.data, ['foo'])
      self.assertEqual(alg2.Key_WV.data, [])  # check ATEAM-902

   def test_append(self):
      alg1 = Configurables.HiveAlgV()
      alg2 = Configurables.HiveAlgV()
      alg1.Key_WV += ['foo']
      self.assertEqual(alg1.Key_WV.data, ['foo'])
      self.assertEqual(alg2.Key_WV.data, [])
      alg1.Key_WV += ['bar']
      self.assertEqual(alg1.Key_WV.data, ['foo','bar'])

   def test_assignDataHandle(self):
      alg = Configurables.HiveAlgV()
      alg.Key_WV = [ DataHandle('foo', 'W', 'HiveDataObj'),
                     DataHandle('bar', 'W', 'HiveDataObj') ]
      self.assertEqual(alg.Key_WV.data, ['foo','bar'])

   def test_appendDataHandle(self):
      alg = Configurables.HiveAlgV()
      alg.Key_WV += [ DataHandle('foo', 'W', 'HiveDataObj') ]
      alg.Key_WV += [ DataHandle('bar', 'W', 'HiveDataObj') ]
      self.assertEqual(alg.Key_WV.data, ['foo','bar'])


if __name__ == "__main__":
    unittest.main()
