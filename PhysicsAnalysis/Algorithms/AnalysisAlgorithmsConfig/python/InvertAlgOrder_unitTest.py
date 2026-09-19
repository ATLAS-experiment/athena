#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Unit tests for `InvertAlgOrder.computeInvertedOrder`.
#
# `computeInvertedOrder` takes the original (valid) ordering of algorithm
# names plus the declared dependencies (in the shape written by
# `extract_alg_dependencies.py`, i.e. `{name: [{"depends_on": other}, ...]}`)
# and returns a maximally-inverted ordering that still respects those
# dependencies: each algorithm is moved to the front unless it depends on
# already-placed algorithms, in which case it lands right after the last of
# them.


import unittest

from AnalysisAlgorithmsConfig.InvertAlgOrder import (
    aggregateMemberDependencies,
    computeInvertedOrder,
    invertMembers,
)


def _deps (mapping) :
    """Build a dependency dict from ``{name: [other, ...]}`` shorthand."""
    return {name: [{"depends_on": other} for other in others]
            for name, others in mapping.items()}


class TestComputeInvertedOrder (unittest.TestCase) :

    def _assertValid (self, names, dependencies, result) :
        """The result must be a permutation of *names* in which every
        dependency still precedes the algorithm that declares it."""
        self.assertEqual (sorted (result), sorted (names))
        position = {name: idx for idx, name in enumerate (result)}
        for name, deps in dependencies.items() :
            for dep in deps :
                other = dep["depends_on"]
                if other in position and name in position :
                    self.assertLess (position[other], position[name],
                                     f"{other} must precede {name}")

    def test_all_independent_are_reversed (self) :
        names = ['a', 'b', 'c', 'd']
        result = computeInvertedOrder (names, {})
        self.assertEqual (result, ['d', 'c', 'b', 'a'])

    def test_linear_chain_is_preserved (self) :
        # c depends on b, b depends on a -> the chain order cannot change.
        names = ['a', 'b', 'c']
        dependencies = _deps ({'b': ['a'], 'c': ['b']})
        result = computeInvertedOrder (names, dependencies)
        self.assertEqual (result, ['a', 'b', 'c'])
        self._assertValid (names, dependencies, result)

    def test_dependent_lands_after_last_dependency (self) :
        # d depends on both a and c; independents get pushed to the front,
        # but d must stay right after the last of its dependencies.
        names = ['a', 'b', 'c', 'd']
        dependencies = _deps ({'d': ['a', 'c']})
        result = computeInvertedOrder (names, dependencies)
        # a, b, c are independent and get reversed to the front -> [c, b, a];
        # d depends on a and c, so it lands after the last of them (a, index 2).
        self.assertEqual (result, ['c', 'b', 'a', 'd'])
        self._assertValid (names, dependencies, result)

    def test_chain_with_independent_tail (self) :
        names = ['a', 'b', 'c', 'x']
        dependencies = _deps ({'b': ['a'], 'c': ['b']})
        result = computeInvertedOrder (names, dependencies)
        # x is independent and appears last -> jumps to the very front;
        # the a->b->c chain keeps its relative order.
        self.assertEqual (result, ['x', 'a', 'b', 'c'])
        self._assertValid (names, dependencies, result)

    def test_dependencies_outside_the_set_are_ignored (self) :
        # A dependency on a name that is not among the reordered algorithms
        # (e.g. an input-file / framework producer) must not affect placement.
        names = ['a', 'b']
        dependencies = _deps ({'a': ['external'], 'b': []})
        result = computeInvertedOrder (names, dependencies)
        self.assertEqual (result, ['b', 'a'])

    def test_missing_entries_treated_as_no_dependencies (self) :
        # Names absent from the dependency map are treated as independent.
        names = ['a', 'b', 'c']
        dependencies = _deps ({'c': ['a']})
        result = computeInvertedOrder (names, dependencies)
        self._assertValid (names, dependencies, result)
        # a -> front; b (missing from map, independent) -> front, giving [b, a];
        # c depends on a (index 1), so it lands right after it.
        self.assertEqual (result, ['b', 'a', 'c'])


class TestAggregateMemberDependencies (unittest.TestCase) :
    """Lifting algorithm-level dependencies to sequence-member level."""

    def test_dependency_into_a_group_resolves_to_the_group (self) :
        # 'q' depends on 'g1', which lives inside the subsequence 'G' -> at the
        # member level, 'q' must depend on 'G'.
        names = ['q', 'G']
        contained = {'q': ['q'], 'G': ['g1', 'g2']}
        deps = _deps ({'q': ['g1']})
        member = aggregateMemberDependencies (names, contained, deps)
        self.assertEqual (member['q'], [{'depends_on': 'G'}])
        self.assertEqual (member['G'], [])

    def test_internal_group_dependency_is_not_an_external_edge (self) :
        # 'g2' depends on 'g1' but both are inside 'G' -> no member-level edge.
        names = ['G']
        contained = {'G': ['g1', 'g2']}
        deps = _deps ({'g2': ['g1']})
        member = aggregateMemberDependencies (names, contained, deps)
        self.assertEqual (member['G'], [])

    def test_shared_algorithm_owned_by_first_member (self) :
        # 'shared' appears both as a top-level alg and inside 'G'; the first
        # (top-level) owner wins, so a dependency on it resolves to 'shared'.
        names = ['shared', 'G', 'q']
        contained = {'shared': ['shared'], 'G': ['shared', 'g2'], 'q': ['q']}
        deps = _deps ({'q': ['shared']})
        member = aggregateMemberDependencies (names, contained, deps)
        self.assertEqual (member['q'], [{'depends_on': 'shared'}])


class TestInvertMembers (unittest.TestCase) :
    """Maximally-inverting a sequence's members, groups treated as units."""

    NAMES = ['A', 'G', 'B', 'X']
    CONTAINED = {'A': ['A'], 'G': ['g1', 'g2'], 'B': ['B'], 'X': ['X']}
    # g1 (in G) depends on A; B depends on g2 (in G): chain A -> G -> B, X free.
    DEPS = _deps ({'g1': ['A'], 'B': ['g2']})

    def test_group_positioned_as_a_unit_and_independent_moved_to_front (self) :
        result = invertMembers (self.NAMES, self.CONTAINED, self.DEPS)
        self.assertEqual (result, ['X', 'A', 'G', 'B'])

    def test_pinned_member_stays_at_front (self) :
        result = invertMembers (self.NAMES, self.CONTAINED, self.DEPS,
                                pinned=['A'])
        self.assertEqual (result, ['A', 'X', 'G', 'B'])


class TestInvertSequence (unittest.TestCase) :
    """End-to-end reorder of a real (nested) AthSequencer tree."""

    def setUp (self) :
        from AthenaConfiguration.ComponentFactory import CompFactory
        import AnaAlgorithm.DualUseConfig as DualUseConfig

        def alg (name) :
            return DualUseConfig.createAlgorithm ('CP::AsgSelectionAlg', name)

        self.sub = CompFactory.AthSequencer ('sub', Sequential=False)
        self.sub.Members = [alg ('s1'), alg ('s2')]
        self.filt = CompFactory.AthSequencer ('filt', Sequential=True)
        self.filt.Members = [alg ('f1'), alg ('f2')]
        self.top = CompFactory.AthSequencer ('top', Sequential=False)
        self.top.Members = [alg ('p'), self.sub, self.filt, alg ('ind')]

        # s1 -> p (external), s2 -> s1 (internal to sub), f2 -> f1 (internal).
        self.deps = _deps ({'s1': ['p'], 's2': ['s1'], 'f2': ['f1']})

    def _names (self, seq) :
        return [member.getName() for member in seq.Members]

    def _invert (self, pinned=()) :
        from AnaAlgorithm.Logging import logging
        from AnalysisAlgorithmsConfig.InvertAlgOrder import _invertSequence
        _invertSequence (self.top, self.deps, pinned,
                         logging.getLogger ('test'))

    def test_top_level_inverted_with_groups_as_units (self) :
        self._invert()
        # p produces for sub, so sub stays after p; the independent 'ind' and
        # the (dependency-free) 'filt' group jump to the front.
        self.assertEqual (self._names (self.top), ['ind', 'filt', 'p', 'sub'])

    def test_concurrent_subsequence_is_recursed_into (self) :
        self._invert()
        # sub is concurrent: its members are inverted, but the s1 -> s2 chain
        # keeps their relative order.
        self.assertEqual (self._names (self.sub), ['s1', 's2'])

    def test_sequential_subsequence_internals_untouched (self) :
        self._invert()
        self.assertEqual (self._names (self.filt), ['f1', 'f2'])

    def test_pinned_member_kept_first_at_top_level (self) :
        self._invert (pinned=['p'])
        self.assertEqual (self._names (self.top), ['p', 'ind', 'filt', 'sub'])


if __name__ == '__main__' :
    unittest.main()
