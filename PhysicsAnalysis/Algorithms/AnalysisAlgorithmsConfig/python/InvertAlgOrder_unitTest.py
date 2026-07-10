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

from AnalysisAlgorithmsConfig.InvertAlgOrder import computeInvertedOrder


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


if __name__ == '__main__' :
    unittest.main()
