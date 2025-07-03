# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
import collections

AthSequencer = CompFactory.AthSequencer  # cache lookup

def parAND(name, subs=[]):
    """parallel AND sequencer"""
    return AthSequencer( name,
                         ModeOR = False,
                         Sequential = False,
                         StopOverride = True,
                         Members = subs.copy() )

def parOR(name, subs=[]):
    """parallel OR sequencer
    This is the default sequencer and lets the DataFlow govern the execution entirely.
    """
    return AthSequencer( name,
                         ModeOR = True,
                         Sequential = False,
                         StopOverride = True,
                         Members = subs.copy() )

def seqAND(name, subs=[]):
    """sequential AND sequencer"""
    return AthSequencer( name,
                         ModeOR = False,
                         Sequential = True,
                         StopOverride = False,
                         Members = subs.copy() )

def seqOR(name, subs=[]):
    """sequential OR sequencer
    Used when a barrier needs to be set by all subs reached irrespective of the decision
    """
    return AthSequencer( name,
                         ModeOR = True,
                         Sequential = True,
                         StopOverride = True,
                         Members = subs.copy() )


def getSequenceChildren(comp):
    """Return sequence children (empty if comp is not a sequence)"""
    try:
        return comp.Members
    except AttributeError:
        return []


def checkSequenceConsistency( seq ):
    """ Enforce rules for sequence graph - identical items can not be added to itself (even indirectly) """

    def __noSubSequenceOfName( s, n, seen = set() ):
        seen = seen.copy()
        seen.add (s)
        for c in getSequenceChildren( s ):
            if c in seen:
                raise RuntimeError(f"Sequence {c.getName()} contains itself")
            if isSequence( c ):
                if c.getName() == n:
                    raise RuntimeError(f"Sequence {n} contains sub-sequence of the same name")
                __noSubSequenceOfName( c, c.getName(), seen ) # check each sequence for repetition as well
                __noSubSequenceOfName( c, n, seen )

    __noSubSequenceOfName( seq, seq.getName() )
    for c in getSequenceChildren( seq ):
        checkSequenceConsistency(c)


def isSequence( obj ):
    return isinstance(obj, AthSequencer)


def findSubSequence( start, nameToLookFor ):
    """ Traverse sequences tree to find a sequence of a given name. The first one is returned. """
    if start.getName() == nameToLookFor:
        return start
    for c in getSequenceChildren(start):
        if isSequence( c ):
            if  c.getName() == nameToLookFor:
                return c
            found = findSubSequence( c, nameToLookFor )
            if found:
                return found
    return None


def findOwningSequence( start, nameToLookFor ):
    """ find sequence that owns the sequence nameTooLookFor"""
    for c in getSequenceChildren(start):
        if c.getName() == nameToLookFor:
            return start
        if isSequence( c ):
            found = findOwningSequence( c, nameToLookFor )
            if found:
                return found
    return None


def findAlgorithmByPredicate( startSequence, predicate, depth = 1000000 ):
    """ Traverse sequences tree to find the first algorithm satisfying given predicate. The first encountered is returned.

    Depth of the search can be controlled by the depth parameter.
    Typical use is to limit search to the startSequence with depth parameter set to 1
    """
    for c in getSequenceChildren(startSequence):
        if not isSequence(c):
            if predicate(c):
                return c
        else:
            if depth > 1:
                found = findAlgorithmByPredicate( c, predicate, depth-1 )
                if found:
                    return found

    return None


def findAlgorithm( startSequence, nameToLookFor, depth = 1000000 ):
    """ Traverse sequences tree to find the algorithm of given name. The first encountered is returned.

    The name() method is used to obtain the algorithm name, that one has to match to the request.
    """
    return findAlgorithmByPredicate( startSequence, lambda alg: alg.getName() == nameToLookFor, depth )


def findAllAlgorithms(sequence, nameToLookFor=None):
    """
    Returns flat listof of all algorithm instances in this, and in sub-sequences
    """
    algorithms = []
    for child in getSequenceChildren(sequence):
        if isSequence(child):
            algorithms += findAllAlgorithms(child, nameToLookFor)
        else:
            if nameToLookFor is None or child.getName() == nameToLookFor:
                algorithms.append(child)
    return algorithms


def findAllAlgorithmsByName(sequence, namesToLookFor=None):
    """
    Finds all algorithms in sequence and groups them by name
    
    Resulting dict has a following structure
    {"Alg1Name":[(Alg1Instance, parentSequenceA, indexInSequenceA),(Alg1Instance, parentSequenceB, indexInSequenceB)],
     "Alg2Name":(Alg2Instance, parentSequence, indexInThisSequence),
     ....}
    """
    algorithms = collections.defaultdict(list)
    for idx, child in enumerate(getSequenceChildren(sequence)):
        if child.getName() == sequence.getName():
            raise RuntimeError(f"Recursively-nested sequence: {child.getName()} contains itself")
        if isSequence(child):
            childAlgs = findAllAlgorithmsByName(child, namesToLookFor)
            for algName in childAlgs:
                algorithms[algName] += childAlgs[algName]
        else:
            if namesToLookFor is None or child.getName() in namesToLookFor:
                algorithms[child.getName()].append( (child, sequence, idx) )
    return algorithms


def flatAlgorithmSequences( start ):
    """ Converts tree like structure of sequences into dictionary
    keyed by top/start sequence name containing lists of of algorithms & sequences."""

    def __inner( seq, collector ):
        for c in getSequenceChildren(seq):
            collector[seq.getName()].append( c )
            if isSequence( c ):
                __inner( c, collector )

    from collections import defaultdict,OrderedDict
    c = defaultdict(list)
    __inner(start, c)
    return OrderedDict(c)


def iterSequences( start ):
    """Iterator of sequences and their algorithms from (and including) the `start`
    sequence object. Do start from a sequence name use findSubSequence."""
    def __inner( seq ):
        for c in getSequenceChildren(seq):
            yield c
            if isSequence(c):
                yield from __inner(c)
    yield start
    yield from __inner(start)



# self test
import unittest
class TestCF( unittest.TestCase ):
    def setUp( self ):
        import AthenaPython.PyAthena as PyAthena

        top = parOR("top")
        top.Members += [parOR("nest1")]
        nest2 = seqAND("nest2")
        top.Members += [nest2]
        top.Members += [PyAthena.Alg("SomeAlg0")]
        nest2.Members += [parOR("deep_nest1")]
        nest2.Members += [parOR("deep_nest2")]

        nest2.Members += [PyAthena.Alg("SomeAlg1")]
        nest2.Members += [PyAthena.Alg("SomeAlg2")]
        nest2.Members += [PyAthena.Alg("SomeAlg3")]
        self.top = top

    def test_findTop( self ):
        f = findSubSequence( self.top, "top")
        self.assertIsNotNone( f, "Can not find sequence at start" )
        self.assertEqual( f.getName(), "top", "Wrong sequence" )
        # a one level deep search
        nest2 = findSubSequence( self.top, "nest2" )
        self.assertIsNotNone( nest2, "Can not find sub sequence" )
        self.assertEqual( nest2.getName(), "nest2", "Sub sequence incorrect" )

    def test_findDeep( self ):
        # deeper search
        d = findSubSequence( self.top, "deep_nest2")
        self.assertIsNotNone( d, "Deep searching for sub seqeunce fails" )
        self.assertEqual( d.getName(), "deep_nest2", "Wrong sub sequence in deep search" )

    def test_findMissing( self ):
        # algorithm is not a sequence
        d = findSubSequence( self.top, "SomeAlg1")
        self.assertIsNone( d, "Algorithm confused as a sequence" )

        # no on demand creation
        inexistent = findSubSequence( self.top, "not_there" )
        self.assertIsNone( inexistent, "ERROR, found sub sequence that does not relay exist" )

        # owner finding
        inexistent = findOwningSequence(self.top, "not_there")
        self.assertIsNone( inexistent, "ERROR, found owner of inexistent sequence " )

    def test_findRespectingScope( self ):
        owner = findOwningSequence( self.top, "deep_nest1")
        self.assertEqual( owner.getName(), "nest2", "Wrong owner %s" % owner.getName() )

        owner = findOwningSequence( self.top, "deep_nest2")
        self.assertEqual( owner.getName(), "nest2", "Wrong owner %s" % owner.getName() )

        owner = findOwningSequence( self.top, "SomeAlg1")
        self.assertEqual( owner.getName(), "nest2", "Wrong owner %s" % owner.getName() )

        owner = findOwningSequence( self.top, "SomeAlg0")
        self.assertEqual( owner.getName() , "top", "Wrong owner %s" % owner.getName() )

    def test_iterSequences( self ):
        # Traverse from top
        result = [seq.getName() for seq in iterSequences( self.top )]
        self.assertEqual( result, ['top', 'nest1', 'nest2', 'deep_nest1', 'deep_nest2',
                                   'SomeAlg1', 'SomeAlg2', 'SomeAlg3', 'SomeAlg0'] )

        # Traverse from nested sequence
        nest2 = findSubSequence( self.top, "nest2" )
        result = [seq.getName() for seq in iterSequences( nest2 )]
        self.assertEqual( result, ['nest2', 'deep_nest1', 'deep_nest2',
                                   'SomeAlg1', 'SomeAlg2', 'SomeAlg3'] )

        # Traverse empty sequence
        deep_nest2 = findSubSequence( self.top, "deep_nest2" )
        result = [seq.getName() for seq in iterSequences( deep_nest2 )]
        self.assertEqual( result, ['deep_nest2'] )

        # Traverse from algorithm
        alg1 = findAlgorithm( self.top, "SomeAlg1" )
        result = [seq.getName() for seq in iterSequences( alg1 )]
        self.assertEqual( result, ['SomeAlg1'] )

    def test_findAlgorithms( self ):
        a1 = findAlgorithm( self.top, "SomeAlg0" )
        self.assertIsNotNone( a1, "Can't find algorithm present in sequence" )

        a1 = findAlgorithm( self.top, "SomeAlg1" )
        self.assertIsNotNone( a1, "Can't find nested algorithm " )

        nest2 = findSubSequence( self.top, "nest2" )

        a1 = findAlgorithm( nest2, "SomeAlg0" )
        self.assertIsNone( a1, "Finding algorithm that is in the upper sequence" )

        a1 = findAlgorithm( nest2, "NonexistentAlg" )
        self.assertIsNone( a1, "Finding algorithm that is does not exist" )

        a1 = findAlgorithm( self.top, "SomeAlg0", 1)
        self.assertIsNotNone( a1, "Could not find algorithm within the required nesting depth == 1" )

        a1 = findAlgorithm( self.top, "SomeAlg1", 1)
        self.assertIsNone( a1, "Could find algorithm even if it is deep in sequences structure" )

        a1 = findAlgorithm( self.top, "SomeAlg1", 2)
        self.assertIsNotNone( a1, "Could not find algorithm within the required nesting depth == 2" )

        a1 = findAlgorithm( self.top, "SomeAlg3", 2)
        self.assertIsNotNone( a1 is None, "Could find algorithm even if it is deep in sequences structure" )


class TestNest( unittest.TestCase ):
    def test( self ):
        global isComponentAccumulatorCfg
        isComponentAccumulatorCfg = lambda : True  # noqa: E731 (lambda for mockup)

        top = parOR("top")
        nest1 = parOR("nest1")
        nest2 = seqAND("nest2")
        top.Members += [nest1, nest2]

        deep_nest1 = seqAND("deep_nest1")
        nest1.Members += [deep_nest1]

        nest2.Members += [nest1] # that one is ok
        checkSequenceConsistency( top )
        deep_nest1.Members += [nest1] # introducing an issue
        self.assertRaises( RuntimeError, checkSequenceConsistency, top )

