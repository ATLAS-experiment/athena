#! /usr/bin/env python3

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import os
import unittest

import ROOT

try:
    RNTupleModel = ROOT.RNTupleModel
    RNTupleReader = ROOT.RNTupleReader
    RNTupleWriter = ROOT.RNTupleWriter
    RNTupleWriteOptions = ROOT.RNTupleWriteOptions
except AttributeError:
    RNTupleModel = ROOT.Experimental.RNTupleModel
    RNTupleReader = ROOT.Experimental.RNTupleReader
    RNTupleWriter = ROOT.Experimental.RNTupleWriter
    RNTupleWriteOptions = ROOT.Experimental.RNTupleWriteOptions

from PyJobTransforms.trfLogger import logging, msg

msg.setLevel(logging.DEBUG)

class trfValidateRootFiletest(unittest.TestCase):

    fname = "RootFile.root"
    nevents = 250
    clustersize = 1024

    def setUp(self):
        if ROOT.gROOT.GetVersionInt() <= 63499:
            self.skipTest("Impossible to write an RNTuple in Python with ROOT version 6.34 and earlier")

        model = RNTupleModel.CreateBare()
        model.MakeField["int"]("i")
        model.MakeField["float"]("f")
        model.MakeField["std::string"]("str")

        options = RNTupleWriteOptions()
        options.SetApproxZippedClusterSize(self.clustersize)

        with RNTupleWriter.Recreate(model, "ntuple", self.fname, options) as writer:
            entry = writer.CreateEntry()
            for i in range(self.nevents):
                entry["i"] = i
                entry["f"] = 1/(i + 2)
                entry["str"] = str(i*i)
                writer.Fill(entry)

    def tearDown(self):
        if os.path.exists(self.fname):
            os.unlink(self.fname)

    def test_checkNTupleFieldWise(self):
        from PyJobTransforms.trfValidateRootFile import checkFile

        rc = checkFile(self.fname, 'basket', requireTree=False)
        self.assertEqual(rc, 0)

if __name__ == '__main__':
    unittest.main()
