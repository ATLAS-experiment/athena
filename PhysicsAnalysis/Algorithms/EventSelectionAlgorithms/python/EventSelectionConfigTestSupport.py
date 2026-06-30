# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Shared harness for the EventSelectionConfig unit tests.

Each *_unitTest.py script builds a real ConfigSequence containing a single
EventSelectionConfig block, configures it through a real ConfigAccumulator,
and inspects the resulting algorithm sequence. No mocking: the accumulator
instantiates the real CP algorithms, so a Python/C++ property-name mismatch
is caught here rather than at runtime.

In a full job the object-config blocks register the input containers and their
selection working points upstream. In isolation we register them here:
  * input containers      -> config.setSourceName(container, container)
  * object selection WPs  -> config.addSelection(container, wp, decoration)
"""

from AnaAlgorithm.AlgSequence import AlgSequence
from AnaAlgorithm.DualUseConfig import isAthena
from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigAccumulator import ConfigAccumulator, DataType
from EventSelectionAlgorithms.EventSelectionConfig import (
    EventSelectionConfig, EventSelectionMergerConfig,
)

_UNSET = object()

# Dummy object-selection working points referenced by the tests' extra-selection
# forms. Registered on every input container so getFullSelection resolves them.
_DUMMY_WPS = ["baseline", "mysel", "goodjet", "centraljet", "topjet",
              "elsel", "musel", "esel", "msel", "tsel"]


def _container(spec):
    """Strip a trailing '.selection' to get the bare container name."""
    return spec.split(".")[0]


def _new_accumulator(dataType, containers):
    algSeq = AlgSequence()
    config = ConfigAccumulator(dataType=dataType, algSeq=algSeq)
    for cont in {_container(c) for c in containers}:
        config.setSourceName(cont, cont)
        for wp in _DUMMY_WPS:
            # decoration string deliberately embeds the WP name so the
            # extra-selection tests can assert on it via substring.
            config.addSelection(cont, wp, f"{wp}_%SYS%")
    return algSeq, config


def run(cuts, *, dataType=DataType.FullSim, containers=None,
        selectionName="SR", **options):
    """Configure one EventSelectionConfig block and return its algorithms.

    `containers` maps EventSelectionConfig option names to container specs,
    e.g. {'electrons': 'AnaElectrons'} or {'jets': 'AnaJets.baseline'}.
    """
    containers = containers or {}
    algSeq, config = _new_accumulator(dataType, containers.values())
    seq = ConfigSequence()
    block = EventSelectionConfig()
    block.setOptionValue("selectionName", selectionName)
    block.setOptionValue("selectionCuts", cuts)
    for opt, cname in containers.items():
        block.setOptionValue(opt, cname)
    for opt, val in options.items():
        block.setOptionValue(opt, val)
    seq.append(block)
    seq.fullConfigure(config)
    if isAthena:
        return config.CA.getEventAlgos()
    return list(algSeq)


def run_merger(region_names, *, dataType=DataType.FullSim, noFilter=False):
    """Build real regions then the merger, and return the algorithm sequence.

    Each region is a trivial EventSelectionConfig (a single RUN_NUMBER cut) so
    that the merger's `EventSelection` dependency is satisfied and
    `eventSelectionNames` is populated. The event filter is emitted implicitly
    at the end of each block (no explicit SAVE needed).
    """
    algSeq, config = _new_accumulator(dataType, [])
    seq = ConfigSequence()
    for name in region_names:
        block = EventSelectionConfig()
        block.setOptionValue("selectionName", name)
        block.setOptionValue("selectionCuts", "RUN_NUMBER >= 0")
        seq.append(block)
    merger = EventSelectionMergerConfig()
    merger._instance_number = 1            # defensive: ensure the guard runs
    merger.setOptionValue("noFilter", noFilter)
    seq.append(merger)
    seq.fullConfigure(config)
    if isAthena:
        return config.CA.getEventAlgos()
    return list(algSeq)


def selectors(algs):
    """Selector algorithms only (exclude the SAVE filter)."""
    return [a for a in algs if a.getType() != "CP::SaveFilterAlg"]


def named(algs, substr):
    return [a for a in algs if substr in a.getName()]


def prop(alg, name, default=_UNSET):
    """Read a property set on the algorithm; return `default` if unset."""
    val = getattr(alg, name, _UNSET)
    if val is _UNSET:
        if default is _UNSET:
            raise AttributeError(f"{alg.getName()} has no property {name!r}")
        return default
    return val
