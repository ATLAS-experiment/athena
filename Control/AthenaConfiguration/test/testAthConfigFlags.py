#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags, isGaudiEnv
from AthenaConfiguration.Enums import Format

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
        self.assertFalse( self.flags.hasCategory("Z") )

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
        self.flags.addFlag("Format", Format.BS, enum=Format)
        self.flags.addFlag("FormatFun", lambda flags : Format.POOL if flags.Atest else Format.BS, enum=Format)
        self.flags.addFlag("FormatPOOL", Format.BS, enum=Format)
        self.flags.FormatPOOL = Format.POOL
        self.flags.lock()

    def test_enums_incorrect_assign(self):
        """Test that enums are properly validated (incorrect flags)"""
        self.flags.addFlag("FormatWrong", Format.BS, enum=Format)
        with self.assertRaises(Exception) as _:
            self.flags.FormatWrong == "BS"

        with self.assertRaises(Exception) as _:
            self.flags.FormatWrong = "POOL"

    def test_enums_incorrect_lambda(self):
        """Test that enums are properly validated (incorrect flags)"""
        self.flags.addFlag("FormatWrong", lambda flags : "ABC", enum=Format)
        with self.assertRaises(RuntimeError) as _:
            x = self.flags.FormatWrong  # noqa: F841

    def test_copy(self):
        """Test that flags can be copied"""
        copy.copy(self.flags)
        copy.deepcopy(self.flags)

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
        self.assertEqual(no_C.athHash(), self.flags.athHash())

    def test_asdict(self):
        adict = self.flags.A.asdict()
        self.assertEqual(self.flags.A.B.C, adict['B']['C'])
        full_dict = self.flags.asdict()
        self.assertEqual(self.flags.A.B.C, full_dict['A']['B']['C'])
        bdict = self.flags.A.B.asdict()
        self.assertEqual(self.flags.A.B.C, bdict['C'])

    def test_iterator(self):
        self.assertTrue('A' in self.flags)
        self.assertFalse('Z' in self.flags)
        self.assertTrue('B' in self.flags.A)
        self.assertFalse('Z' in self.flags.A)

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
#        self.assertEqual( copyf.T.Abool, False, "The flags clone does not have dynamic flags")
        copyf.dump()

        self.flags.lock()
        copyf = self.flags.cloneAndReplace( "X", "Z.Xclone2")
        self.assertEqual( copyf.X.a, 40, "dynamically loaded flags have wrong value")
        self.assertEqual( copyf.T.Abool, False, "The flags clone does not have dynamic flags")

    def test_exists(self):
        """Test `has` methods"""
        self.assertTrue( self.flags.hasCategory("Z") )
        self.assertFalse( self.flags.hasCategory("Z.C") )  # sub-category not auto-resolved
        # now load category:
        self.flags.needFlagsCategory("Z")
        self.assertTrue( self.flags.hasFlag("Z.A") )
        self.assertTrue( self.flags.hasCategory("Z.C") )


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
        self.flags = AthConfigFlags()
        self.flags.addFlag('Exec.OutputLevel',3) #Global Output Level
        self.flags.addFlag('Exec.MaxEvents',-1)
        self.flags.addFlag("Exec.SkipEvents",0)
        self.flags.addFlag("Exec.DebugStage","")
        self.flags.addFlag('Input.Files',[])
        self.flags.addFlag('detA.flagB',0)
        self.flags.addFlag("detA.flagC","")
        self.flags.addFlag("detA.flagD",[])
        self.flags.addFlag("Format", Format.BS, enum=Format)

    def test(self):
        argline="-l VERBOSE --evtMax=10 --skipEvents=3 --filesInput=bla1.data,bla2.data detA.flagB=7 Format=Format.BS detA.flagC=a.2 detA.flagD+=['val']"
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
        self.assertEqual(self.flags.Format, Format.BS,"Failed to set FlagEnum")


class FlagsHelpTest(unittest.TestCase):
    def setUp(self):
        self.flags = AthConfigFlags()
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
                self.flags.fillFromArgs(args.split(" "))
        if expected not in f.getvalue(): print(f.getvalue())
        self.assertTrue(expected in f.getvalue())

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
        self.do_test(args="--filesInput file1 file2 -- --help Input",expected="""flags:
  Input.Files  : List of input files (default: ['file1', 'file2'])
""")


if __name__ == "__main__":
    unittest.main(verbosity=2)
