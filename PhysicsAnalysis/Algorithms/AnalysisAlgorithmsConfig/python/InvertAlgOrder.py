# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

"""Reorder a ComponentAccumulator's algorithms into a maximally-inverted order.

Given a set of declared algorithm-to-algorithm dependencies (as produced by
``extract_alg_dependencies.py``), this reorders an algorithm sequence so that it
is as reversed as the dependencies allow, while still respecting them.  It is
meant as a check that the declared dependencies are by themselves sufficient to
enforce a correct ordering, rather than the job merely following a roughly
correct predefined order.
"""

import json

from AnaAlgorithm.Logging import logging
from AthenaCommon.CFElements import isSequence


def computeInvertedOrder(names, dependencies):
    """Return a maximally-inverted ordering of *names*.

    *names* is the original (already valid) ordered list of algorithm names.
    *dependencies* is a mapping ``{name: [{"depends_on": other, ...}, ...]}``
    as written by ``extract_alg_dependencies.py``.

    Walking the algorithms in their original order, each algorithm is inserted
    at the front of the new order unless it depends on algorithms already
    placed there, in which case it is inserted right after the last such
    dependency.  Since the original order is a valid topological order, every
    dependency of an algorithm is already placed by the time it is processed,
    so the result is a valid ordering that is otherwise as reversed as the
    dependencies permit.
    """
    nameSet = set(names)
    newOrder = []
    for name in names:
        deps = {dep["depends_on"] for dep in dependencies.get(name, [])}
        deps &= nameSet
        lastIdx = -1
        for idx, placed in enumerate(newOrder):
            if placed in deps:
                lastIdx = idx
        newOrder.insert(lastIdx + 1, name)
    return newOrder


def aggregateMemberDependencies(names, containedByName, dependencies):
    """Lift algorithm-level dependencies to the level of sequence members.

    *names* is the ordered list of member names of a sequence.  Each member may
    be a plain algorithm or a subsequence; *containedByName* maps each member
    name to the list of algorithm names it (recursively) contains.  Returns a
    dependency mapping (same shape as *dependencies*) in which member A depends
    on member B when some algorithm contained in A depends on some algorithm
    contained in B (A != B).
    """
    # Each algorithm is owned by the first member that contains it; this
    # resolves algorithms shared between a subsequence and the top level.
    algToMember = {}
    for name in names:
        for alg in containedByName[name]:
            algToMember.setdefault(alg, name)

    memberDeps = {}
    for name in names:
        deps = set()
        for alg in containedByName[name]:
            for dep in dependencies.get(alg, []):
                owner = algToMember.get(dep["depends_on"])
                if owner is not None and owner != name:
                    deps.add(owner)
        memberDeps[name] = [{"depends_on": owner} for owner in deps]
    return memberDeps


def invertMembers(names, containedByName, dependencies, pinned=()):
    """Return the maximally-inverted order of a sequence's members.

    *pinned* members are kept at the front in their original order; the rest are
    inverted via :func:`computeInvertedOrder` using member-level dependencies
    aggregated by :func:`aggregateMemberDependencies`.
    """
    memberDeps = aggregateMemberDependencies(names, containedByName, dependencies)
    pinnedSet = set(pinned)
    pinnedNames = [name for name in names if name in pinnedSet]
    freeNames = [name for name in names if name not in pinnedSet]
    return pinnedNames + computeInvertedOrder(freeNames, memberDeps)


def _containedAlgNames(member):
    """The algorithm names (recursively) contained in a sequence member.

    A plain algorithm contains just itself; a subsequence contains all the
    algorithms of its members.
    """
    if isSequence(member):
        names = []
        for child in member.Members:
            names.extend(_containedAlgNames(child))
        return names
    return [member.getName()]


def _isSequential(seq):
    """Whether an AthSequencer runs its members in strict order (vs concurrent).

    The order of a sequential sequence carries meaning (e.g. a filter that stops
    later members), so its internal order is left untouched.
    """
    return bool(getattr(seq, "Sequential", False))


def _invertSequence(seq, dependencies, pinned, logger):
    """Recursively invert *seq* in place (see :func:`invertAlgOrder`)."""
    members = list(seq.Members)

    # Recurse into concurrent subsequences to invert their internals; leave
    # sequential subsequences internally untouched.
    for member in members:
        if isSequence(member) and not _isSequential(member):
            _invertSequence(member, dependencies, (), logger)

    # Only reorder this sequence's own members if it is concurrent.
    if _isSequential(seq):
        return

    names = [member.getName() for member in members]
    byName = {member.getName(): member for member in members}
    containedByName = {name: _containedAlgNames(member)
                       for name, member in zip(names, members)}
    newOrder = invertMembers(names, containedByName, dependencies, pinned)
    seq.Members = [byName[name] for name in newOrder]
    logger.info("Inverted order in sequence '%s' (%d members)",
                seq.getName(), len(newOrder))


def invertAlgOrder(ca, dependencies, sequenceName=None, pinned=(), logger=None):
    """Invert the algorithm order of a sequence in a ComponentAccumulator.

    *ca* is the (fully assembled) ComponentAccumulator to modify in place.
    *dependencies* is either the dependency mapping or a path to the JSON file
    containing it.  *sequenceName* selects the sequence to reorder (``None`` uses
    the primary sequence).  *pinned* is a collection of algorithm/member names to
    keep at the front of the top-level sequence in their original order (e.g.
    ``SGInputLoader``, which produces the input-file data but declares no
    dependencies).

    The reorder is recursive: each concurrent sequence has its members inverted
    (subsequences positioned as units by the aggregate dependencies of the
    algorithms they contain), and concurrent subsequences are additionally
    inverted internally.  Sequential subsequences keep their internal order.
    """
    if logger is None:
        logger = logging.getLogger("invertAlgOrder")
    if isinstance(dependencies, str):
        with open(dependencies) as depFile:
            dependencies = json.load(depFile)

    seq = ca.getSequence(sequenceName)

    missing = sorted({alg for alg in _containedAlgNames(seq)
                      if alg not in dependencies})
    if missing:
        logger.warning("%d algorithm(s) under sequence '%s' are not in the "
                       "dependency file (assuming no dependencies): %s",
                       len(missing), seq.getName(), ", ".join(missing))

    _invertSequence(seq, dependencies, pinned, logger)
    return ca
