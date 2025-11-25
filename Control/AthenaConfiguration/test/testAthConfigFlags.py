#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags, isGaudiEnv
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.Enums import Format

import argparse
import copy
import unittest

class FlagsSetup(unittest.TestCase):
    def setUp(self):
        self.flags = AthConfigFlags()
        self.flags.addFlag("Atest", True)
        self.flags.addFlag("A.One", True)
        self.flags.addFlag("A.B.C", False)
        self.flags.addFlag("A.dependentFlag", lambda prevFlags: ["FALSE VALUE", "TRUE VALUE"][prevFlags.A.B.C] )


class BasicTests(FlagsSetup):

    def test_Access(self):
        """Test access"""
        self.assertFalse( self.flags.A.B.C, "Can't read A.B.C flag")
        self.flags.A.B.C = True
        self.assertTrue( self.flags.A.B.C, "Flag value not changed")
        # test setitem
        self.flags['A'].B['C'] = False
        self.assertFalse( self.flags.A.B.C, "Flag value not chenged")

    def test_wrongAccess(self):
        """Access to the flag that are missnames should give an exception"""
        with self.assertRaises(RuntimeError):
            print(".... test printout {}".format( self.flags.A is True ))
            print(".... test printout {}".format( self.flags.A.B == 6 ))

    def test_noFlagOrCategory(self):
        """Trying to access something which isn't a flag/attribute should raise an error"""
        with self.assertRaises(AttributeError):
            self.flags.X

        with self.assertRaises(AttributeError):
            self.flags.A.B.X

    def test_exists(self):
        """Test `has` methods"""
        self.assertTrue( self.flags.hasFlag("Atest") )
        self.assertFalse( self.flags.hasFlag("A") )  # category, not flag
        self.assertTrue( self.flags.hasFlag("A.One") )
        self.assertTrue( self.flags.hasCategory("A.B") )
        self.assertFalse( self.flags.hasCategory("Atest") )  # flag, not category
        self.assertFalse( self.flags.hasCategory("Z") )

    def test_hasattr(self):
        """Test hasattr"""
        self.assertTrue( hasattr(self.flags, "Atest") )
        self.assertTrue( hasattr(self.flags, "A") )  # also works for category
        self.assertTrue( hasattr(self.flags, "A.One") )
        self.assertTrue( hasattr(self.flags, "A.B") )
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
        self.flags.A.B.C = True
        self.flags.lock()
        self.assertEqual(self.flags.A.dependentFlag, "TRUE VALUE", " dependent flag setting does not work")

    def test_lock(self):
        """Test flag locking"""
        self.flags.lock()
        with self.assertRaises(RuntimeError):
            self.flags.Atest = False
        with self.assertRaises(RuntimeError):
            self.flags.addFlag("X", True)
        with self.assertRaises(RuntimeError):
            del self.flags.A
        with self.assertRaises(RuntimeError):
            del self.flags['A']

    def test_hash(self):
        """Test flag hashing"""
        if not isGaudiEnv():
            return

        from AthenaConfiguration.AccumulatorCache import AccumulatorCache

        self.assertEqual(self.flags.locked() , False)
        with self.assertRaises(RuntimeError):
            self.flags.athHash()
        self.flags.lock()
        self.assertIsNotNone(self.flags.athHash())
        self.assertEqual(self.flags.locked() , True)

        @AccumulatorCache
        def testAthHash(flags , number , key = 123):
            return number + key

        a = testAthHash(self.flags , 123 , key = 321)
        b = testAthHash(self.flags , 321 , key = 123)
        c = testAthHash(self.flags , 123 , key = 321)
        d = testAthHash(self.flags , 321 , key = 123)

        self.assertEqual(a , c)
        self.assertEqual(b , d)

        cacheInfo = testAthHash.getInfo()

        self.assertEqual(cacheInfo["misses"] , 2)
        self.assertEqual(cacheInfo["hits"] , 2)
        self.assertEqual(cacheInfo["cache_size"] , 2)

    def test_hash_invariance(self):
        """Test that hash doesn't change on dynamic flag loading"""
        def generator():
            extraFlags = AthConfigFlags()
            extraFlags.addFlag('Extra.X', 'foo')
            extraFlags.addFlag('Extra.Y', lambda flags : flags.Extra.X+'_bar')
            return extraFlags

        self.flags.addFlagsCategory('Extra', generator)
        self.flags.lock()
        hash_value = self.flags.athHash()
        self.assertEqual(self.flags.Extra.X, 'foo')
        self.assertEqual(self.flags.athHash() , hash_value)
        self.assertEqual(self.flags.Extra.Y, 'foo_bar')
        self.assertEqual(self.flags.athHash() , hash_value)

    def test_enums(self):
        """Test that enums are properly validated"""
        self.flags.addFlag("Format", Format.BS, type=Format)
        self.flags.addFlag("FormatFun", lambda flags : Format.POOL if flags.Atest else Format.BS, type=Format)
        self.flags.addFlag("FormatPOOL", Format.BS, type=Format)
        self.flags.FormatPOOL = Format.POOL
        self.flags.lock()

    def test_enums_incorrect_assign(self):
        """Test that enums are properly validated (incorrect flags)"""
        self.flags.addFlag("FormatWrong", Format.BS, type=Format)
        with self.assertRaises(TypeError) as _:
            self.flags.FormatWrong = "BS"

        with self.assertRaises(TypeError) as _:
            self.flags.FormatWrong = "POOL"

    def test_enums_incorrect_lambda(self):
        """Test that enums are properly validated (incorrect flags)"""
        self.flags.addFlag("FormatWrong", lambda flags : "ABC", type=Format)
        with self.assertRaises(TypeError) as _:
            x = self.flags.FormatWrong  # noqa: F841

    def test_types(self):
        """Test that types are properly validated"""
        self.flags.addFlag("ListFlag", [123], type=list)
        self.flags.addFlag("ListFlag2", lambda flags : [1] if flags.Atest else [0], type=list)
        self.flags.addFlag("TupleFlag", (123456,), type=tuple)
        self.flags.addFlag("IntFlag", 123, type=int)
        self.flags.addFlag("FloatFlag", 123.45, type=float)
        self.flags.addFlag("FloatFlag2", 123, type=float)  # implicit int to float conversion OK
        self.flags.addFlag("BoolFlag", True, type=bool)
        self.flags.TupleFlag = (123456, 1234567)
        self.flags.lock()

    def test_types_incorrect_assign(self):
        """Test that types are properly validated (incorrect flags)"""
        self.flags.addFlag("IntWrong", 123, type=int)
        with self.assertRaises(TypeError):
            self.flags.IntWrong = 123.45

        self.flags.addFlag("ListWrong", [], type=list)
        with self.assertRaises(TypeError):
            self.flags.ListWrong = 123

        with self.assertRaises(TypeError):
            self.flags.ListWrong = "123"

        self.flags.addFlag("BoolWrong", False, type=bool)
        with self.assertRaises(TypeError):
            self.flags.BoolWrong = 1

    def test_types_incorrect_lambda(self):
        """Test that types are properly validated (incorrect flags)"""
        self.flags.addFlag("ListWrong", lambda flags : 123, type=list)
        with self.assertRaises(TypeError) as _:
            x = self.flags.ListWrong  # noqa: F841

    def test_copy(self):
        """Test that flags can be copied"""
        copy.copy(self.flags)
        copy.deepcopy(self.flags)

    def test_asdict(self):
        adict = self.flags.A.asdict()
        self.assertEqual(self.flags.A.B.C, adict['B']['C'])
        full_dict = self.flags.asdict()
        self.assertEqual(self.flags.A.B.C, full_dict['A']['B']['C'])
        bdict = self.flags.A.B.asdict()
        self.assertEqual(self.flags.A.B.C, bdict['C'])

    def test_iterator(self):
        self.assertEqual( list(iter(self.flags)), ['Atest', 'A'] )
        self.assertEqual( list(iter(self.flags.A)), ['One', 'B', 'dependentFlag'] )


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

        self.flags.addFlagsCategory( 'Z', theZFlags )
        self.flags.addFlagsCategory( 'X', theXFlags, prefix=True )
        self.flags.addFlagsCategory( 'T', theTFlags )
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

    def test_overwriteFlags(self):
        """Check if overwriting works"""
        self.flags.Z.Xclone1.a = 20
        self.flags.Z.Xclone2.a = 40

        self.flags.X.a = 30
        copyf = self.flags.cloneAndReplace( "X", "Z.Xclone1")
        self.assertEqual( copyf.X.a, 20, "dynamically loaded flags have wrong value")
        copyf.dump()

        self.flags.lock()
        copyf = self.flags.cloneAndReplace( "X", "Z.Xclone2").cloneAndReplace( "X2", "Z.Xclone1")
        self.assertEqual( copyf.X.a, 40, "cloned loaded flags have wrong value")
        self.assertEqual( copyf.T.Abool, False, "The flags clone does not have dynamic flags")
        self.assertEqual( copyf.T.Abool, False, "The flags clone does not have dynamic flags")
        self.assertEqual( copyf.X2.a, 20, "cloned loaded flags have wrong value")
        def _touchRemappedFlag():
            copyf.Z.Xclone1.a
        self.assertRaises(AttributeError, _touchRemappedFlag)

        copyf = self.flags.cloneAndReplace( "X", "Z.Xclone2").cloneAndReplace( "X2", "Z.Xclone1", keepOriginal=True)
        self.assertEqual( copyf.X2.a, 20, "cloned flags have wrong value")
        self.assertEqual( copyf.Z.Xclone1.a, 20, "original flag have wrong value")

        print("\nFlag after double remap:")
        print("-"*80)
        copyf.dump()
        print("\nFlag after double remap ..")
        print("-"*80)

    def test_overwriteFlagsProtectAgainstSubClones(self):
        self.flags.lock()
        newf = self.flags.cloneAndReplace("R", "X")
        with self.assertRaises(RuntimeError):
            newf = newf.cloneAndReplace("X.R", "A")
    
    def test_copyAsDict(self):
        """test for asdict with cloned flags"""
        zdict = self.flags.asdict()['Z']
        copyf = self.flags.cloneAndReplace('W', 'Z')
        wdict = copyf.asdict()['W']
        self.assertEqual(zdict, wdict)

        # try again with flag address
        cdict = self.flags.Z.asdict()['C']
        copyf = self.flags.cloneAndReplace('Z.W', 'Z.C')
        wdict = copyf.Z.asdict()['W']
        self.assertEqual(cdict, wdict)

    def test_exists(self):
        """Test `has` methods"""
        self.assertTrue( self.flags.hasCategory("Z") )
        self.assertTrue( self.flags.hasCategory("Z.C") )  # sub-category is loaded on check
        # now load category:
        self.flags.needFlagsCategory("Z")
        self.assertTrue( self.flags.hasFlag("Z.A") )
        self.assertTrue( self.flags.hasCategory("Z.C") )

    def test_hasattr(self):
        """Test hasattr"""
        self.assertTrue( hasattr(self.flags, "Z") )
        self.assertTrue( hasattr(self.flags, "Z.C") )  # sub-category is loaded on check
        self.assertTrue( hasattr(self.flags, "Z.A") )

    def test_contains(self):
        """Test in operator"""
        self.assertTrue( "Z" in self.flags )
        self.assertTrue( "C" in self.flags.Z )
        self.assertTrue( "A" in self.flags.Z )

    def test_cloneExists(self):
        """test if flags can be found after cloning"""
        clonef = self.flags.cloneAndReplace('W', 'Z')
        self.assertTrue(clonef.hasFlag('W.A'))
        clonef.W.A  # check that access works
        self.assertFalse(clonef.hasFlag('Z.A'))
        
    def test_nonReplacingCloneExists(self):
        clonef = self.flags.cloneAndReplace('W', 'Z', True)
        self.assertTrue(clonef.hasFlag('W.A'))
        self.assertTrue(clonef.hasFlag('Z.A'))
    
    def test_nonReplacingMultiCloneExists(self):
        clonef = self.flags.cloneAndReplace('W1', 'Z', True)
        clonef = clonef.cloneAndReplace('W2', 'Z', True)
        clonef = clonef.cloneAndReplace('W3', 'W1', True)
        self.assertTrue(clonef.hasFlag('W1.A'))
        self.assertTrue(clonef.hasFlag('W2.A'))
        self.assertTrue(clonef.hasFlag('W3.A'))
        self.assertTrue(clonef.hasFlag('Z.A'))
    
    def test_complexClone(self):
        clonef = self.flags.cloneAndReplace('W1', 'Z', True)
        clonef = clonef.cloneAndReplace('W2', 'Z', True)
        clonef = clonef.cloneAndReplace('W3', 'W1', True)
        clonef = clonef.cloneAndReplace('W4', 'W1', False)
        clonef = clonef.cloneAndReplace('W5', 'Z', False)
        self.assertTrue(clonef.hasFlag('W2.A'))
        self.assertTrue(clonef.hasFlag('W3.A'))
        self.assertTrue(clonef.hasFlag('W4.A'))
        self.assertTrue(clonef.hasFlag('W5.A'))
        self.assertFalse(clonef.hasFlag('W1.A'))
        self.assertFalse(clonef.hasFlag('Z.A'))
        
    def test_circularClone(self):
        clonef1 = self.flags.cloneAndReplace('W', 'Z')
        clonef2 = clonef1.cloneAndReplace('Z', 'W')
        self.assertTrue(clonef1.hasFlag('W.A'))
        self.assertTrue(clonef2.hasFlag('Z.A'))
        self.assertFalse(clonef1.hasFlag('Z.A'))
        self.assertFalse(clonef2.hasFlag('W.A'))
        
    def test_circularNonReplacingClone(self):
        clonef1 = self.flags.cloneAndReplace('W', 'Z', True)
        clonef2 = clonef1.cloneAndReplace('Z', 'W', True)
        self.assertTrue(clonef1.hasFlag('Z.A'))
        self.assertTrue(clonef1.hasFlag('W.A'))
        self.assertTrue(clonef2.hasFlag('Z.A'))
        self.assertTrue(clonef2.hasFlag('W.A'))
        
    def test_complexCircularClone(self):
        clonef0 = self.flags.cloneAndReplace('W', 'Z', True)
        clonef0 = clonef0.cloneAndReplace('W1', 'W', True)
        clonef0 = clonef0.cloneAndReplace('W2', 'W1', True)
        clonef0 = clonef0.cloneAndReplace('WW', 'W')
        clonef1 = clonef0.cloneAndReplace('ZZ', 'Z')
        clonef2 = clonef1.cloneAndReplace('Z', 'ZZ')
        clonef3 = clonef1.cloneAndReplace('Z', 'T')
        
        self.assertTrue(clonef0.hasFlag('Z.A'))
        self.assertTrue(clonef0.hasFlag('W1.A'))
        self.assertTrue(clonef0.hasFlag('W2.A'))
        self.assertTrue(clonef0.hasFlag('WW.A'))
        self.assertFalse(clonef0.hasFlag('W.A'))
        
        self.assertTrue(clonef1.hasFlag('ZZ.A'))
        self.assertTrue(clonef1.hasFlag('W1.A'))
        self.assertTrue(clonef1.hasFlag('W2.A'))
        self.assertTrue(clonef1.hasFlag('WW.A'))
        self.assertFalse(clonef1.hasFlag('Z.A'))
        self.assertFalse(clonef1.hasFlag('W.A'))
        
        self.assertTrue(clonef2.hasFlag('Z.A'))
        self.assertTrue(clonef2.hasFlag('W1.A'))
        self.assertTrue(clonef2.hasFlag('W2.A'))
        self.assertTrue(clonef2.hasFlag('WW.A'))
        self.assertFalse(clonef2.hasFlag('ZZ.A'))
        self.assertFalse(clonef2.hasFlag('W.A'))
        
        self.assertTrue(clonef3.hasFlag('ZZ.A'))
        self.assertTrue(clonef3.hasFlag('W1.A'))
        self.assertTrue(clonef3.hasFlag('W2.A'))
        self.assertTrue(clonef3.hasFlag('WW.A'))
        self.assertTrue(clonef3.hasFlag('Z.Abool'))
        self.assertFalse(clonef3.hasFlag('Z.A'))
        self.assertFalse(clonef3.hasFlag('W.A'))
        self.assertFalse(clonef3.hasFlag('ZZ.Abool'))
        self.assertFalse(clonef3.hasFlag('W.Abool'))
        self.assertFalse(clonef3.hasFlag('W1.Abool'))
        self.assertFalse(clonef3.hasFlag('W2.Abool'))
        self.assertFalse(clonef3.hasFlag('WWW.Abool'))
        
    def test_cloneIter(self):
        # top level check
        self.assertTrue('Z' in self.flags)
        self.assertTrue('A' in self.flags.Z)
        clonez2w = self.flags.cloneAndReplace('W', 'Z')
        self.assertEqual(list(iter(clonez2w)), ['Atest', 'A', 'W', 'X', 'T'])
        self.assertEqual(list(iter(clonez2w.W)), ['A', 'B', 'C', 'Xclone1', 'Xclone2'])
        self.assertFalse('Z' in clonez2w)
        self.assertTrue('W' in clonez2w)
        self.assertFalse('Z' in clonez2w.W)
        self.assertTrue('A' in clonez2w.W)

        # check one level down
        self.assertTrue('C' in self.flags.Z)
        clonec2x = self.flags.cloneAndReplace('Z.X', 'Z.C')
        self.assertEqual(list(iter(clonec2x.Z)), ['A', 'B', 'X', 'Xclone1', 'Xclone2'])
        self.assertTrue('X' in clonec2x.Z)
        self.assertFalse('C' in clonec2x.Z)

    def test_delete(self):
        # test item delete
        no_A = copy.deepcopy(self.flags)
        self.assertTrue( no_A.hasCategory("A") )
        del no_A['A']
        with self.assertRaises(AttributeError):
            no_A.A
        self.assertNotEqual(self.flags, no_A)
        self.assertFalse( no_A.hasCategory("A") )
        # test attribute delete
        cval = self.flags.A.B.C
        no_C = copy.deepcopy(self.flags)
        del no_C.A.B.C
        with self.assertRaises(AttributeError):
            no_C.A.B.C
        # test adding back a flag
        no_C.addFlag("A.B.C", cval)
        no_C.lock()
        self.flags.lock()
        self.assertEqual(no_C.A.B.C, self.flags.A.B.C)

    def test_hash_after_loadAllDynamic(self):
        copyflags = copy.deepcopy(self.flags)
        copyflags.lock()
        initialHash = copyflags.athHash()
        postHash = copyflags.athHash()
        self.assertEqual(initialHash, postHash, "After loading all dynamic flags the hash has changed")

    def test_hash_clonedFlagsHaveDifferenHash(self):
        cloneFlags = self.flags.clone()
        cloneFlags.lock()
        cloneHash = cloneFlags.athHash()

        cloneFlags2 = self.flags.clone()
        cloneFlags2.lock()
        cloneHash2 = cloneFlags2.athHash()
        self.assertNotEqual(cloneHash, cloneHash2, "flags after another clone should have different hash")

    def test_hash_cloneAndReplace(self):
        origFlags = self.flags.clone()
        origFlags.lock()
        origHash = origFlags.athHash()

        cloneWFlags = origFlags.cloneAndReplace('W', 'Z')
        cloneWHash = cloneWFlags.athHash()

        self.assertNotEqual(origHash, cloneWHash, "flags after clone and replace should have different hash")


        cloneZFlags = cloneWFlags.cloneAndReplace('Z', 'W')
        cloneZHash = cloneZFlags.athHash()
        self.assertEqual(origHash, cloneZHash, "cloneAndReplace reversing application should restore the hash")

class TestDynamicDependentFlags(unittest.TestCase):
    def test(self):
        """Check if dynamic dependent flags work"""
        flags = AthConfigFlags()
        flags.addFlag("A", True)
        flags.addFlag("B", lambda prevFlags: 3 if prevFlags.A is True else 10 )
        flags.addFlag("C", lambda prevFlags: 'A' if prevFlags.A is True else 'B' )
        assert flags.B == 3
        flags.A = False
        assert flags.B == 10
        flags.A = True
        flags.lock()
        assert flags.C == 'A'
        print("")


class FlagsFromArgsTest(unittest.TestCase):
    def setUp(self):
        self.flags = initConfigFlags()
        self.flags.addFlag('detA.flagB',0)
        self.flags.addFlag("detA.flagC","")
        self.flags.addFlag("detA.flagD", [], type=list)
        self.flags.addFlag("intE", 123, type=int)
        self.flags.addFlag("floatF", 123.45, type=float)
        self.flags.addFlag("boolB", False, type=bool)
        self.flags.addFlag("bool_notype", False)
        self.flags.addFlag("Format", Format.BS, type=Format)

    def test(self):
        argline="-l VERBOSE --evtMax=10 --skipEvents=3 --filesInput=bla1.data,bla2.data detA.flagB=7 Format=Format.BS detA.flagC=a.2 detA.flagD+=['val'] intE=42 floatF=42.42 boolB=True bool_notype=True"
        if isGaudiEnv():
            argline += " --debug exec"
        print (f"Interpreting arguments: '{argline}'")
        self.flags.fillFromArgs(argline.split())
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

    def test2(self):
        with self.assertRaises(TypeError):
            self.flags.fillFromArgs(['intE=42.3'])

        with self.assertRaises(TypeError):
            self.flags.fillFromArgs(['boolB=2'])

        self.flags.fillFromArgs(['floatF=42'])  # implicit conversion
        self.assertEqual(self.flags.floatF, 42)
    
        with self.assertRaises(KeyError):
            self.flags.fillFromArgs(['floatF=23','unknownFlag=42'],return_unknown=False)
        _, unknown_args = self.flags.fillFromArgs(['floatF=23','unknownFlag=42'],return_unknown=True)
        self.assertEqual(unknown_args,["unknownFlag=42"])
    



class FlagsHelpTest(unittest.TestCase):
    def setUp(self):
        self.flags = AthConfigFlags()
        self.parser = argparse.ArgumentParser(formatter_class = argparse.ArgumentDefaultsHelpFormatter)
        self.flags.addFlag("Flag0","",help="This is Flag0")
        self.flags.addFlag("CatA.Flag1","",help="This is Flag1")
        self.flags.addFlag("CatA.SubCatA.Flag2","",help="This is Flag2")
        self.flags.addFlag("CatB.Flag3","",help="This is Flag3")

    def do_test(self,args,expected):
        import io
        import contextlib
        with self.assertRaises(SystemExit):
            f = io.StringIO()
            with contextlib.redirect_stdout(f):
                self.flags.fillFromArgs(args.split(" "), parser=self.parser)
        # Ignore whitespace changes
        expected = ' '.join(expected.split())
        value = ' '.join(f.getvalue().split())
        if expected not in value:
            print(f.getvalue())
        self.assertTrue(expected in value)

    def test_basicHelp(self):
        # tests printing top-level help message
        self.do_test(args="--help",expected="""
flags and positional arguments:
  {CatA,CatB}           Flag subcategories:
    CatA                CatA flags
    CatB                CatB flags
  Flag0                 : This is Flag0 (default: '')
""")

    def test_catHelp(self):
        # tests getting help for a category of flags
        self.do_test(args="--help CatA",expected="""flags:
  {SubCatA}   Flag subcategories:
    SubCatA   CatA.SubCatA flags
  CatA.Flag1  : This is Flag1 (default: '')
""")

    def test_catHelpSub(self):
        # tests getting help for a subcategory of flags (tests navigation down through categories)
        self.do_test(args="--help CatA.SubCatA",expected="""flags:
  CatA.SubCatA.Flag2  : This is Flag2 (default: '')
""")

    def test_setFlagBeforeParse(self):
        # tests that we can change values of a flag before parsing and that will replace the "default"
        # in the help text
        self.flags.CatA.SubCatA.Flag2 = "test"
        self.do_test(args="--help CatA.SubCatA",expected="""flags:
  CatA.SubCatA.Flag2  : This is Flag2 (default: 'test')
""")

    def test_setFlagDuringParse(self):
        # tests that we can change values of a flag during parsing and that will replace the "default"
        # in the help text. This is useful to see the 'effect' of other arguments on the flags
        # this test also shows the use of the list terminator e.g. for fileInput list
        self.flags.addFlag("Input.Files",[],help="List of input files")
        self.do_test(args="CatA.SubCatA.Flag2=42 -- --help CatA.SubCatA",expected="""flags:
  CatA.SubCatA.Flag2  : This is Flag2 (default: 42)
""")


if __name__ == "__main__":
    unittest.main(verbosity=2)
