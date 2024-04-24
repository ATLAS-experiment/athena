#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# A test of copying for JetDefinition classes
# We want to ensure that copying (shallow or deep) reproduces the originals
# Also specially test that if the dependencies have been solved, we
# 

from JetRecConfig.JetDefinition import JetDefinition
from JetRecConfig.JetGrooming import GroomingDefinition
from JetRecConfig.StandardSmallRJets import AntiKt4EMPFlow, AntiKt4TruthDressedWZ, AntiKt4EMPFlowCSSKNoPtCut, AntiKt4PV0Track, AntiKtVR30Rmax4Rmin02PV0Track
from JetRecConfig.StandardLargeRJets import AntiKt10LCTopo_withmoms, AntiKt10LCTopoTrimmed, AntiKt10UFOCSSK, AntiKt10UFOCSSKSoftDrop, AntiKt10TruthDressedWZSoftDrop
from JetRecConfig.DependencyHelper import solveDependencies, solveGroomingDependencies
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.Enums import BeamType, LHCPeriod
import unittest
from copy import copy, deepcopy

def compareProperties(a,b):
    print(f"Comparing {a} with {b}")
    for k,v in a.__dict__.items():
        if v != b.__dict__[k]:
            print(f"Mismatch for {k}: {v} --> {b.__dict__[k]}")
            print(f"Hash for {k}: {hash(v)} --> {hash(b.__dict__[k])}")

class TestJetDef(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.smallRdefs = [AntiKt4EMPFlow, AntiKt4TruthDressedWZ, AntiKt4EMPFlowCSSKNoPtCut, AntiKt4PV0Track, AntiKtVR30Rmax4Rmin02PV0Track]
        cls.largeRdefs = [AntiKt10LCTopo_withmoms, AntiKt10LCTopoTrimmed, AntiKt10UFOCSSK, AntiKt10UFOCSSKSoftDrop, AntiKt10TruthDressedWZSoftDrop]
        cls.flags = initConfigFlags()
        cls.flags.Input.Files=[]
        # Set flags in lieu of getting them from the file
        cls.flags.Beam.Type = BeamType.Collisions
        cls.flags.GeoModel.Run = LHCPeriod.Run3
        cls.flags.lock()

    def test_0_copy_smallRJets(self):
        for jetdef in self.smallRdefs:
            jetdef_copy = copy(jetdef)
            try:
                self.assertEqual(jetdef, jetdef_copy)
            except AssertionError as e:
                compareProperties(jetdef,jetdef_copy)
                raise e

    def test_1_deepcopy_smallRJets(self):
        for jetdef in self.smallRdefs:
            jetdef_copy = deepcopy(jetdef)
            try:
                self.assertEqual(jetdef, jetdef_copy)
            except AssertionError as e:
                compareProperties(jetdef,jetdef_copy)
                raise e

    def test_2_copy_largeRJets(self):
        for jetdef in self.largeRdefs:
            jetdef_copy = copy(jetdef)
            try:
                self.assertEqual(jetdef, jetdef_copy)
            except AssertionError as e:
                compareProperties(jetdef,jetdef_copy)
                raise e

    def test_3_deepcopy_largeRJets(self):
        for jetdef in self.largeRdefs:
            jetdef_copy = deepcopy(jetdef)
            try:
                self.assertEqual(jetdef, jetdef_copy)
            except AssertionError as e:
                compareProperties(jetdef,jetdef_copy)
                raise e

    def test_4_copy_smallRJets_solved(self):
        for jetdef in self.smallRdefs:
            jetdef_solved = solveDependencies(jetdef,self.flags)
            jetdef_copy = copy(jetdef_solved)
            try:
                self.assertEqual(jetdef, jetdef_copy)
            except AssertionError as e:
                compareProperties(jetdef,jetdef_copy)
                raise e

    def test_5_deepcopy_smallRJets_solved(self):
        for jetdef in self.smallRdefs:
            jetdef_solved = solveDependencies(jetdef,self.flags)
            jetdef_copy = copy(jetdef_solved)

            try:
                self.assertEqual(jetdef, jetdef_copy)
            except AssertionError as e:
                compareProperties(jetdef,jetdef_copy)
                raise e

            self.assertEqual(jetdef_solved._cflags, jetdef_copy._cflags)
            self.assertTrue(jetdef_copy._cflags.locked())

    def test_4_copy_largeRJets_solved(self):
        for jetdef in self.largeRdefs:
            if isinstance(jetdef, JetDefinition):
                jetdef_solved = solveDependencies(jetdef,self.flags)
                jetdef_copy = copy(jetdef_solved)
            elif isinstance(jetdef, GroomingDefinition):
                jetdef_solved = solveGroomingDependencies(jetdef,self.flags)
                jetdef_copy = copy(jetdef_solved)
            else:
                raise TypeError(f'Invalid definition type {type(jetdef)} for {jetdef}')

            try:
                self.assertEqual(jetdef_solved, jetdef_copy)
            except AssertionError as e:
                compareProperties(jetdef_solved,jetdef_copy)
                raise e

    def test_5_deepcopy_largeRJets_solved(self):
        for jetdef in self.largeRdefs:
            if isinstance(jetdef, JetDefinition):
                jetdef_solved = solveDependencies(jetdef,self.flags)
                jetdef_copy = deepcopy(jetdef_solved)
            elif isinstance(jetdef, GroomingDefinition):
                jetdef_solved = solveGroomingDependencies(jetdef,self.flags)
                jetdef_copy = deepcopy(jetdef_solved)
            else:
                raise TypeError(f'Invalid definition type {type(jetdef)} for {jetdef}')

            try:
                self.assertEqual(jetdef_solved, jetdef_copy)
            except AssertionError as e:
                compareProperties(jetdef_solved,jetdef_copy)
                raise e

            self.assertEqual(jetdef_solved._cflags, jetdef_copy._cflags)
            self.assertTrue(jetdef_copy._cflags.locked())

if __name__ == '__main__':
    unittest.main()
