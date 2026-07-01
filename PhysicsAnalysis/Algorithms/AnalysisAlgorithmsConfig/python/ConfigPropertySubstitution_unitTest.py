#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Unit tests for `ConfigPropertySubstitution`.
#
# The substitution module exposes two functions used by
# `ConfigAccumulator.renameFinalContainers`:
#   * `substituteValue` — recursively rewrites string references inside
#     a property value, handling private tools, tool-handle arrays, data
#     handles, and nested list/dict/set/tuple containers.
#   * `substituteComponentProperties` — walks a configurable component's
#     user-set properties and substitutes each value in place.
#
# The first half of this file exercises `substituteValue` directly on
# plain Python data, including the anchored-boundary edge cases. The
# second half builds real configurables via the dual-use interface (the
# same primitives `ConfigAccumulator` uses internally) and verifies the
# substitution lands in algorithm properties and inside a private tool.


import unittest

from AnalysisAlgorithmsConfig.ConfigPropertySubstitution import (
    substituteComponentProperties,
    substituteValue,
)


class TestSubstituteValue (unittest.TestCase) :
    """Pure tests of `substituteValue` — no configurables involved."""

    SUBS = [('Jets_STEP3', 'Jets')]

    def test_plain_string_match (self) :
        self.assertEqual (substituteValue ('Jets_STEP3', self.SUBS), 'Jets')

    def test_plain_string_no_match (self) :
        self.assertEqual (substituteValue ('Electrons_STEP1', self.SUBS),
                          'Electrons_STEP1')

    def test_string_with_suffix (self) :
        # `_%SYS%` follows the match — the leading boundary is still
        # the start of the string, so this is a valid rewrite.
        self.assertEqual (substituteValue ('Jets_STEP3_%SYS%', self.SUBS),
                          'Jets_%SYS%')

    def test_string_with_trailing_dot (self) :
        self.assertEqual (substituteValue ('Jets_STEP3.flag', self.SUBS),
                          'Jets.flag')

    def test_anchored_boundary_left (self) :
        # The `My` prefix means the char before the match is an
        # identifier char, so `_anchoredReplace` must NOT match.
        self.assertEqual (substituteValue ('MyJets_STEP3', self.SUBS),
                          'MyJets_STEP3')

    def test_anchored_inside_expression (self) :
        # Embedded in a larger expression; the spaces / operator chars
        # are non-identifier boundaries, so all matches rewrite.
        self.assertEqual (substituteValue ('Jets_STEP3.a && Jets_STEP3.b',
                                           self.SUBS),
                          'Jets.a && Jets.b')

    def test_anchored_inside_expression_with_trap (self) :
        # The `My`-prefixed term in the middle must survive.
        self.assertEqual (substituteValue ('Jets_STEP3 && MyJets_STEP3',
                                           self.SUBS),
                          'Jets && MyJets_STEP3')

    def test_multiple_substitutions_applied_in_order (self) :
        subs = [('Jets_STEP3', 'Jets'), ('Electrons_STEP2', 'Electrons')]
        self.assertEqual (substituteValue ('Jets_STEP3 && Electrons_STEP2',
                                           subs),
                          'Jets && Electrons')

    def test_list_recurses (self) :
        result = substituteValue (['Jets_STEP3', 'MyJets_STEP3', 'other'],
                                  self.SUBS)
        self.assertEqual (result, ['Jets', 'MyJets_STEP3', 'other'])
        self.assertIsInstance (result, list)

    def test_dict_recurses_on_keys_and_values (self) :
        result = substituteValue ({'Jets_STEP3': 'Jets_STEP3.flag',
                                   'MyJets_STEP3': 'unrelated'},
                                  self.SUBS)
        self.assertEqual (result, {'Jets': 'Jets.flag',
                                   'MyJets_STEP3': 'unrelated'})
        self.assertIsInstance (result, dict)

    def test_set_recurses (self) :
        result = substituteValue ({'Jets_STEP3', 'MyJets_STEP3'}, self.SUBS)
        self.assertEqual (result, {'Jets', 'MyJets_STEP3'})
        self.assertIsInstance (result, set)

    def test_tuple_recurses (self) :
        result = substituteValue (('Jets_STEP3', 'MyJets_STEP3'), self.SUBS)
        self.assertEqual (result, ('Jets', 'MyJets_STEP3'))
        self.assertIsInstance (result, tuple)

    def test_nested_dict_with_list (self) :
        value = {'a': ['Jets_STEP3', 'MyJets_STEP3'],
                 'Jets_STEP3.b': {'c': 'Jets_STEP3'}}
        self.assertEqual (substituteValue (value, self.SUBS),
                          {'a': ['Jets', 'MyJets_STEP3'],
                           'Jets.b': {'c': 'Jets'}})

    def test_non_string_scalars_unchanged (self) :
        self.assertEqual (substituteValue (42, self.SUBS), 42)
        self.assertEqual (substituteValue (3.14, self.SUBS), 3.14)
        self.assertEqual (substituteValue (True, self.SUBS), True)
        self.assertIsNone (substituteValue (None, self.SUBS))

    def test_empty_substitutions_is_identity (self) :
        self.assertEqual (substituteValue ('Jets_STEP3', []), 'Jets_STEP3')
        self.assertEqual (substituteValue (['Jets_STEP3'], []), ['Jets_STEP3'])


class TestSubstituteComponentProperties (unittest.TestCase) :
    """Build real configurables via the dual-use interface — the same
    primitives `ConfigAccumulator` uses internally — and verify the
    substitution walks into algorithm properties and a private tool."""

    SUBS = [('AnaJets_STEP3', 'AnaJets')]

    def _makeAlg (self) :
        import AnaAlgorithm.DualUseConfig as DualUseConfig
        alg = DualUseConfig.createAlgorithm (
            'CP::AsgSelectionAlg', 'TestSelectionAlg')
        DualUseConfig.addPrivateTool (
            alg, 'selectionTool', 'CP::AsgFlagSelectionTool')
        alg.particles = 'AnaJets_STEP3_%SYS%'
        alg.preselection = 'preselect_STEP3,as_char'
        alg.selectionDecoration = 'pass_AnaJets_STEP3,as_char'
        alg.selectionTool.selectionFlags = [
            'AnaJets_STEP3.flag',
            'MyAnaJets_STEP3.flag',
            'unrelated',
        ]
        return alg

    def test_substitution_rewrites_algorithm_and_private_tool (self) :
        alg = self._makeAlg()
        substituteComponentProperties (alg, self.SUBS)

        # algorithm-level properties
        self.assertEqual (alg.particles, 'AnaJets_%SYS%')
        self.assertEqual (alg.preselection, 'preselect_STEP3,as_char')
        self.assertEqual (alg.selectionDecoration, 'pass_AnaJets_STEP3,as_char')

        # private-tool vector<string> property
        self.assertEqual (list (alg.selectionTool.selectionFlags),
                          ['AnaJets.flag',
                           'MyAnaJets_STEP3.flag',
                           'unrelated'])

    def test_substitution_is_idempotent (self) :
        alg = self._makeAlg()
        substituteComponentProperties (alg, self.SUBS)
        before_particles = alg.particles
        before_flags = list (alg.selectionTool.selectionFlags)
        substituteComponentProperties (alg, self.SUBS)
        self.assertEqual (alg.particles, before_particles)
        self.assertEqual (list (alg.selectionTool.selectionFlags), before_flags)

    def test_no_matching_substitution_leaves_properties_unchanged (self) :
        alg = self._makeAlg()
        before_particles = alg.particles
        before_preselection = alg.preselection
        before_decoration = alg.selectionDecoration
        before_flags = list (alg.selectionTool.selectionFlags)
        substituteComponentProperties (alg, [('NonExistentContainer', 'Foo')])
        self.assertEqual (alg.particles, before_particles)
        self.assertEqual (alg.preselection, before_preselection)
        self.assertEqual (alg.selectionDecoration, before_decoration)
        self.assertEqual (list (alg.selectionTool.selectionFlags), before_flags)

    def test_empty_substitutions_leaves_properties_unchanged (self) :
        alg = self._makeAlg()
        before_particles = alg.particles
        before_flags = list (alg.selectionTool.selectionFlags)
        substituteComponentProperties (alg, [])
        self.assertEqual (alg.particles, before_particles)
        self.assertEqual (list (alg.selectionTool.selectionFlags), before_flags)


if __name__ == '__main__' :
    unittest.main()
