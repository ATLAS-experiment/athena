#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# A test of copying for JetDefinition classes
# We want to ensure that copying (shallow or deep) reproduces the originals
# Also specially test that if the dependencies have been solved, we
# 

from JetRecConfig.JetDefinition import JetDefinition
from JetRecConfig.JetGrooming import GroomingDefinition
from JetRecConfig.StandardSmallRJets import AntiKt4EMPFlow, AntiKt4TruthDressedWZ, AntiKt4EMPFlowCSSKNoPtCut
from JetRecConfig.StandardLargeRJets import AntiKt10LCTopo_withmoms, AntiKt10LCTopoTrimmed, AntiKt10UFOCSSK, AntiKt10UFOCSSKSoftDrop, AntiKt10TruthDressedWZSoftDrop
from JetRecConfig.DependencyHelper import solveDependencies, solveGroomingDependencies
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.Enums import BeamType, LHCPeriod
import unittest
from copy import copy, deepcopy

# Start with a basic constituent and print some information

class TestJetDef(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.smallRdefs = [AntiKt4EMPFlow, AntiKt4TruthDressedWZ, AntiKt4EMPFlowCSSKNoPtCut]
        cls.largeRdefs = [AntiKt10LCTopo_withmoms, AntiKt10LCTopoTrimmed, AntiKt10UFOCSSK, AntiKt10UFOCSSKSoftDrop, AntiKt10TruthDressedWZSoftDrop]
        cls.flags = initConfigFlags()
        cls.flags.Input.Files=[]
        # Set flags in lieu of getting them from the file
        cls.flags.Beam.Type = BeamType.Collisions
        cls.flags.GeoModel.Run = LHCPeriod.Run3
        cls.flags.lock()

    def test_0_copy_smallRJets(self):
        for jetdef in self.smallRdefs:
            self.assertEqual(jetdef, copy(jetdef))

    def test_1_deepcopy_smallRJets(self):
        for jetdef in self.smallRdefs:
            self.assertEqual(jetdef, deepcopy(jetdef))

    def test_2_copy_largeRJets(self):
        for jetdef in self.largeRdefs:
            self.assertEqual(jetdef, copy(jetdef))

    def test_3_deepcopy_largeRJets(self):
        for jetdef in self.largeRdefs:
            self.assertEqual(jetdef, deepcopy(jetdef))

    def test_4_copy_smallRJets(self):
        for jetdef in self.smallRdefs:
            jetdef_solved = solveDependencies(jetdef,self.flags)
            jetdef_copy = copy(jetdef_solved)
            self.assertEqual(jetdef_solved, jetdef_copy)
            self.assertEqual(jetdef_solved._cflags, jetdef_copy._cflags)
            self.assertTrue(jetdef_copy._cflags.locked())

    def test_5_deepcopy_largeRJets(self):
        for jetdef in self.largeRdefs:
            if isinstance(jetdef, JetDefinition):
                jetdef_solved = solveDependencies(jetdef,self.flags)
                jetdef_copy = copy(jetdef_solved)
            elif isinstance(jetdef, GroomingDefinition):
                jetdef_solved = solveGroomingDependencies(jetdef,self.flags)
                jetdef_copy = copy(jetdef_solved)
            else:
                raise TypeError(f'Invalid definition type {type(jetdef)} for {jetdef}')
            self.assertEqual(jetdef_solved, jetdef_copy)
            self.assertEqual(jetdef_solved._cflags, jetdef_copy._cflags)
            self.assertTrue(jetdef_copy._cflags.locked())

if __name__ == '__main__':
    unittest.main()
