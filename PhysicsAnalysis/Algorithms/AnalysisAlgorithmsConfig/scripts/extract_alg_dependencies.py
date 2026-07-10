#!/usr/bin/env python3

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

"""Extract algorithm-to-algorithm dependencies from a CPRun.py dependency log.

A dependency log is produced e.g. by::

    CPRun.py ... --test-mt-dependencies 1 | tee dep_log.txt

It contains a block printed by ``AvalancheSchedulerSvc`` that starts with
``Data Dependencies for Algorithms:`` and lists, per algorithm, its INPUT and
OUTPUT data dependencies.  This script parses that block and reconstructs the
direct algorithm dependency graph: algorithm A depends on algorithm B when B
produces (OUTPUT) a data object that A consumes (INPUT).

INPUT dependencies with no matching OUTPUT are assumed to come from the input
file and are ignored.  Only direct producer->consumer edges are recorded; no
transitive closure is computed.
"""

import argparse
import json
import re
from collections import OrderedDict

# Matches e.g.:  o INPUT   ( 'xAOD::JetContainer' , 'StoreGateSvc+AnaJets_STEP1_NOSYS' )
# A trailing "[tool-or-decorator]" annotation, if present, is simply ignored.
DEP_RE = re.compile(r"o\s+(INPUT|OUTPUT)\s+\(\s*'([^']*)'\s*,\s*'([^']*)'\s*\)")

# Algorithm header: exactly two leading spaces, then a bare name (no "o INPUT/..").
ALG_HEADER_RE = re.compile(r"^  (\S.*?)\s*$")

BLOCK_START = "Data Dependencies for Algorithms:"


def parse_log(path):
    """Parse the dependency block of *path*.

    Returns an OrderedDict mapping algorithm name to
    ``{"inputs": set of (type, key), "outputs": set of (type, key)}``,
    preserving the order in which algorithms first appear.
    """
    algs = OrderedDict()
    in_block = False
    current = None

    with open(path) as f:
        for line in f:
            if not in_block:
                if BLOCK_START in line:
                    in_block = True
                continue

            # The block is emitted with no log-line prefix; the next prefixed
            # line (e.g. "AvalancheSchedulerSvc ... INFO ...") ends it.
            if line.startswith("AvalancheSchedulerSvc"):
                break

            dep = DEP_RE.search(line)
            if dep:
                direction, typ, key = dep.groups()
                if current is not None:
                    bucket = "inputs" if direction == "INPUT" else "outputs"
                    algs[current][bucket].add((typ, key))
                continue

            if line.strip() == "none":
                continue

            header = ALG_HEADER_RE.match(line)
            if header:
                current = header.group(1)
                algs.setdefault(current, {"inputs": set(), "outputs": set()})

    return algs


def build_producers(algs):
    """Map each produced ``(type, key)`` to the list of algorithms outputting it."""
    producers = {}
    for name, data in algs.items():
        for obj in data["outputs"]:
            producers.setdefault(obj, []).append(name)
    return producers


def build_dependencies(algs, producers):
    """Build the direct dependency graph.

    Returns ``(dependencies, unmatched_inputs)`` where *dependencies* is an
    OrderedDict mapping each algorithm to a sorted list of
    ``{"depends_on": alg, "via": [[type, key], ...]}`` entries, and
    *unmatched_inputs* counts INPUTs with no producer (assumed from the file).
    """
    dependencies = OrderedDict()
    unmatched_inputs = 0

    for name, data in algs.items():
        # producer alg -> set of (type, key) objects driving the edge
        via = {}
        for obj in data["inputs"]:
            prods = producers.get(obj)
            if not prods:
                unmatched_inputs += 1
                continue
            for producer in prods:
                if producer == name:
                    continue  # skip self-dependency (in-place decoration)
                via.setdefault(producer, set()).add(obj)

        dependencies[name] = [
            {
                "depends_on": producer,
                "via": sorted([list(obj) for obj in objs]),
            }
            for producer, objs in sorted(via.items())
        ]

    return dependencies, unmatched_inputs


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("logfile", help="CPRun.py dependency log to parse")
    parser.add_argument(
        "-o",
        "--output",
        default="alg_dependencies.json",
        help="output JSON file (default: alg_dependencies.json)",
    )
    args = parser.parse_args()

    algs = parse_log(args.logfile)
    if not algs:
        parser.error(
            f"no '{BLOCK_START}' block found in {args.logfile}"
        )

    producers = build_producers(algs)
    dependencies, unmatched_inputs = build_dependencies(algs, producers)

    with open(args.output, "w") as f:
        json.dump(dependencies, f, indent=2)
        f.write("\n")

    edge_count = sum(len(deps) for deps in dependencies.values())
    print(
        "Parsed %d algorithms, %d direct dependency edges, "
        "%d input(s) assumed from the input file. Wrote %s"
        % (len(algs), edge_count, unmatched_inputs, args.output)
    )


if __name__ == "__main__":
    main()
