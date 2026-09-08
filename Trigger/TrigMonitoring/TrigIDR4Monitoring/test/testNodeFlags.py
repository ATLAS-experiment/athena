#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags, isGaudiEnv
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.Enums import Format
from TrigIDR4Monitoring.Node import decode_flags
# from AthenaConfiguration.NodeUtils import decodeFlags
# from AthenaConfiguration.NodeUtils import updateFlags

import argparse
import copy
import unittest
import sys

class FlagsSetup(unittest.TestCase):
    def setUp(self):
        self.iflags = AthConfigFlags()
        self.iflags.addFlag("Atest", True)
        self.iflags.addFlag("A.One", True)
        self.iflags.addFlag("A.B.C", False)
        self.iflags.addFlag("A.dependentFlag", lambda prevFlags: ["FALSE VALUE", "TRUE VALUE"][prevFlags.A.B.C] )
        self.flags = decode_flags(self.iflags)
        #        self.iflags = updateFlags(self.flags)

    def _assertTrue(self, thing ):
        print( thing )
        return self.assertTrue( thing )

    def _assertFalse(self, thing ):
        print( thing )
        return self.assertFalse( thing )

    def _assertRaises(self, thing ):
        print( thing )
        return self.assertRaises( thing )


class BasicTests(FlagsSetup):

    def test_Access(self):
        """Test access"""
        self.assertFalse( self.flags.A.B.C, "Can't read A.B.C flag")
        self.flags.A.B.C = True
        self.assertTrue( self.flags.A.B.C, "Flag value not changed")

    def test_wrongAccess(self):
        """Wrong type of access to a dode or flag should raise as AttributeError"""
        self.assertFalse( self.flags.A is True )
        self.assertFalse( self.flags.A.B == 6 )
        # self.flags.A is a node, not a flag
        with self._assertRaises(AttributeError): self.flags.A = 5
        # self.flags.Atest is a flag, not a node
        with self._assertRaises(AttributeError): self.flags.Atest.B = 5

        
    def test_noFlagOrCategory(self):
        """Trying to access something which isn't a node or flag should raise an error"""
        with self.assertRaises(AttributeError): self.flags.X
        with self.assertRaises(AttributeError): self.flags.A.B.X

    def test_exists(self):
        """Test `has` methods"""
        self.assertTrue( self.flags.hasFlag("Atest") )
        self.assertFalse( self.flags.hasFlag("A") )  # category, not flag
        self.assertTrue( self.flags.A.hasFlag("One") )
        self.assertTrue( self.flags.A.hasNode("B") )
        self.assertFalse( self.flags.hasNode("Atest") )  # flag, not category
        self.assertFalse( self.flags.hasNode("Z") )

        self.assertTrue( self.flags.has("Atest") )
        self.assertTrue( self.flags.has("A") )  # True for nodes, cetagories as well of course
        self.assertTrue( self.flags.A.has("One") )
        self.assertTrue( self.flags.A.has("B") )
        self.assertTrue( self.flags.has("Atest") )  # flag, not category
        self.assertFalse( self.flags.has("Z") )

        

        
    def test_hasattr(self):
        """Test hasattr"""
        self.assertTrue( hasattr(self.flags, "Atest") )
        self.assertTrue( hasattr(self.flags, "A") )  # also works for category
        self.assertTrue( hasattr(self.flags.A, "One") )
        self.assertTrue( hasattr(self.flags.A, "B") )
        self.assertFalse( hasattr(self.flags, "Z") )

    def test_contains(self):
        """Test in operator"""
        self.assertTrue( "Atest" in self.flags )
        self.assertTrue( "A" in self.flags )  # also works for category
        self.assertTrue( "One" in self.flags.A )
        self.assertTrue( "B" in self.flags.A )
        self.assertFalse( "Z" in self.flags )

    def test_dependentFlag(self):
        """The dependent flags will use another flag value to establish its own value"""
        
        flags = self.flags        
        flags.A.B.C = True
        self.assertEqual( flags.A.dependentFlag, "TRUE VALUE", " dependent flag setting does not work")
        self.assertEqual( flags._flags.A.dependentFlag, "TRUE VALUE", " dependent flag setting does not work")
        flags.A.B.C = False
        self.assertEqual( flags.A.dependentFlag, "FALSE VALUE", " dependent flag setting does not work")
        self.assertEqual( flags._flags.A.dependentFlag, "FALSE VALUE", " dependent flag setting does not work")
      
    def test_lock(self):
        """Test flag locking"""
        self.flags.lock()
        with self.assertRaises(RuntimeError): self.flags.Atest = False
        with self.assertRaises(RuntimeError): self.flags.addFlag("X", True)
        with self.assertRaises(RuntimeError): del self.flags.A
        with self.assertRaises(RuntimeError): del self.flags['A']

        self.flags.unlock()

        self.flags.Atest = False
        self.flags.addFlag("X.Z",  True)
        self.flags.addFlag("X.Y.Z2", False)

        del self.flags.Atest
        del self.flags.X.Y
        # no node, or flag, Q
        with self.assertRaises(AttributeError): del self.flags.Q
        
        self.flags.lock_forever()

        with self._assertRaises(RuntimeError): del self.flags.A
        with self._assertRaises(RuntimeError): del self.flags.Z

        #E can't unlock flags that have been forever locked ...
        
        with self._assertRaises(RuntimeError): self.flags.unlock()

        with self._assertRaises(RuntimeError): self.flags.A.One = False 
        with self._assertRaises(RuntimeError): del self.flags.A
        
        
    def test_copy(self):
        """Test that a node can be cloned independently"""
        
        #        self.flags.print()

        flags = self.flags
        aclone = flags.clone("new")
        
        self.assertIsNot(flags, aclone)
        self.assertEqual(flags._name, "root")
        self.assertEqual(aclone._name, "new")
        
        self.assertTrue(aclone.Atest)
        self.assertTrue(aclone.A.One)
        self.assertFalse(aclone.A.B.C)
        
        self.assertIsNot(flags.A, aclone.A)
        self.assertIsNot(flags.A.B, aclone.A.B)

        a = copy.copy(flags)

        self.assertTrue(flags.A.B.C is aclone.A.B.C )
        self.assertTrue(flags.A.B.C is      a.A.B.C )

#       deepcopy is irrelevant - we would first need to deepcopy the
#       underlying AthConfigFlags, then run decode_flags again, so 
#       there would be literally no point even bothering to check
#         copy.deepcopy(self.flags)

        
class TestFlagsSetupDynamic(FlagsSetup):
    def setUp(self):
        super().setUp()

        def theXFlags():
            nf = AthConfigFlags()
            nf.addFlag("a", 17)
            nf.addFlag("b", 55)
            nf.addFlag("c", "Hello")
            return nf

        def theZFlags():
            nf = AthConfigFlags()
            nf.addFlag("Z.A", 7)
            nf.addFlag("Z.B", True)
            nf.addFlag("Z.C.setting", 99)
            nf.addFlagsCategory( 'Z.Xclone1', theXFlags, prefix=True )
            nf.addFlagsCategory( 'Z.Xclone2', theXFlags, prefix=True )
            return nf

        def theTFlags():
            nf = AthConfigFlags()
            nf.addFlag("T.Abool", False)
            return nf

        self.iflags.addFlagsCategory( 'Z', theZFlags )
        self.iflags.addFlagsCategory( 'X', theXFlags, prefix=True )
        self.iflags.addFlagsCategory( 'T', theTFlags )

        self.flags = decode_flags(self.iflags)
        
        print("\nFlag values before test:")
        print("-"*80)
        self.flags.dump()
        print("-"*80)

    def tearDown(self):
        print("\nFlag values after test:")
        print("-"*80)
        self.flags.dump()
        print("-"*80)

    def test_dynamicFlagsRead(self):
        """Check if dynamic flags reading works"""
        self.assertEqual( self.flags.X.a, 17, "dynamically loaded flags have wrong value")
        print("")
        self.assertEqual( self.flags.Z.A, 7, "dynamically loaded flags have wrong value")
        self.assertEqual( self.flags.Z.Xclone1.b, 55, "dynamically loaded flags have wrong value")
        self.flags.Z.Xclone2.b = 56
        self.assertEqual( self.flags.Z.Xclone2.b, 56, "dynamically loaded flags have wrong value")

    def test_dynamicFlagsSet(self):
        """Check if dynamic flags setting works"""
        self.flags.Z.A = 15
        self.flags.Z.Xclone1.a = 20
        self.flags.X.a = 30
        self.assertEqual( self.flags.Z.Xclone1.a, 20, "dynamically loaded flags have wrong value")
        self.assertEqual( self.flags.X.a, 30, "dynamically loaded flags have wrong value")
        self.assertEqual( self.flags.Z.A, 15, "dynamically loaded flags have wrong value")

    def test_hasattr(self):
        """Test hasattr"""
        self.assertTrue( hasattr(self.flags,   "Z") )
        self.assertTrue( hasattr(self.flags.Z, "C") )  # sub-category is loaded on check
        self.assertTrue( hasattr(self.flags.Z, "A") )

    def test_has(self):
        """Test has"""
        # self._assertTrue( has(self.flags,   "Z") )
        # self._assertTrue( has(self.flags.Z, "C") )  # sub-category is loaded on check
        # self._assertTrue( has(self.flags.Z, "A") )

        self.assertTrue( self.flags.has("Z")   )
        self.assertTrue( self.flags.Z.has("C") )  # sub-category is loaded on check
        self.assertTrue( self.flags.Z.has("A") )
        self.assertTrue( self.flags.Z.C.has("setting") )

        
    def test_contains(self):
        """Test in operator (II)"""
        self.assertTrue( "Z" in self.flags )
        self.assertTrue( "C" in self.flags.Z )
        self.assertTrue( "A" in self.flags.Z )


class TestDynamicDependentFlags(unittest.TestCase):
    def test(self):
        """Check if dynamic dependent flags work"""
        iflags = AthConfigFlags()
        
        iflags.addFlag("A", True)
        iflags.addFlag("B", lambda prevFlags: 3 if prevFlags.A is True else 10 )
        iflags.addFlag("C", lambda prevFlags: 'A' if prevFlags.A is True else 'B' )

        flags = decode_flags(iflags)
        
        assert flags.B == 3
        flags.A = False
        assert flags.B == 10
        flags.A = True
        flags.lock()
        assert flags.C == 'A'
        
        print("")


class FlagsFromArgsTest(unittest.TestCase):
    def setUp(self):
        self.iflags = initConfigFlags()

        self.iflags.addFlag('detA.flagB',0)
        self.iflags.addFlag("detA.flagC","")
        self.iflags.addFlag("detA.flagD", [], type=list)
        self.iflags.addFlag("intE", 123, type=int)
        self.iflags.addFlag("floatF", 123.45, type=float)
        self.iflags.addFlag("boolB", False, type=bool)
        self.iflags.addFlag("bool_notype", False)
        self.iflags.addFlag("Format", Format.BS, type=Format)
        self.flags = decode_flags(self.iflags)

        
    def test(self):
        argline="-l VERBOSE --evtMax=10 --skipEvents=3 --filesInput=bla1.data,bla2.data detA.flagB=7 Format=Format.BS detA.flagC=a.2 detA.flagD+=['val'] intE=42 floatF=42.42 boolB=True bool_notype=True"
        if isGaudiEnv():
            argline += " --debug exec"
        print (f"Interpreting arguments: '{argline}'")
        self.iflags.fillFromArgs(argline.split())
        self.flags = decode_flags(self.iflags)

        
        self.assertEqual(self.flags.Exec.OutputLevel,1,"Failed to set output level from args")
        self.assertEqual(self.flags.Exec.MaxEvents,10,"Failed to set MaxEvents from args")
        self.assertEqual(self.flags.Exec.SkipEvents,3,"Failed to set SkipEvents from args")
        self.assertEqual(self.flags.Exec.DebugStage,"exec" if isGaudiEnv() else "","Failed to set DebugStage from args")
        self.assertEqual(self.flags.Input.Files,["bla1.data","bla2.data"],"Failed to set FileInput from args")
        self.assertEqual(self.flags.detA.flagB,7,"Failed to set arbitrary from args")
        self.assertEqual(self.flags.detA.flagC,"a.2","Failed to set arbitrary unquoted string from args")
        self.assertEqual(self.flags.detA.flagD,["val"],"Failed to append to list flag")
        self.assertEqual(self.flags.intE, 42, "Failed to set integer flag")
        self.assertEqual(self.flags.floatF, 42.42, "Failed to set floating point value flag")
        self.assertEqual(self.flags.boolB, True, "Failed to set boolean flag")
        self.assertEqual(self.flags.bool_notype, True, "Failed to set boolean flag")
        self.assertEqual(self.flags.Format, Format.BS,"Failed to set FlagEnum")


        

        
if __name__ == "__main__":
    unittest.main(verbosity=2)
