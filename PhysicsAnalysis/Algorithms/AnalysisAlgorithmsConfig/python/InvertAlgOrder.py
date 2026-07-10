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


def invertAlgOrder(ca, dependencies, sequenceName=None, logger=None):
    """Invert the algorithm order of a sequence in a ComponentAccumulator.

    *ca* is the (fully assembled) ComponentAccumulator to modify in place.
    *dependencies* is either the dependency mapping or a path to the JSON file
    containing it.  *sequenceName* selects the sequence whose members are
    reordered (``None`` uses the primary sequence).
    """
    if logger is None:
        logger = logging.getLogger("invertAlgOrder")
    if isinstance(dependencies, str):
        with open(dependencies) as depFile:
            dependencies = json.load(depFile)

    seq = ca.getSequence(sequenceName)
    members = list(seq.Members)
    byName = {member.getName(): member for member in members}
    names = [member.getName() for member in members]

    missing = [name for name in names if name not in dependencies]
    if missing:
        logger.warning("%d algorithm(s) in sequence '%s' are not in the "
                       "dependency file (assuming no dependencies): %s",
                       len(missing), seq.getName(), ", ".join(missing))

    newOrder = computeInvertedOrder(names, dependencies)
    seq.Members = [byName[name] for name in newOrder]
    logger.info("Inverted algorithm order in sequence '%s' (%d algorithms)",
                seq.getName(), len(newOrder))
    return ca
