# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Shared helpers for the event reconstruction config blocks. These algorithms
# decorate EventInfo rather than writing their own output container, so they
# need a common convention for composing output branch prefixes.

from types import SimpleNamespace

# The topology each reconstruction algorithm operates on.
TOP_NAMESPACE = SimpleNamespace()
TOP_NAMESPACE.DILEPASSIGNER_TOPOLOGY = "TtbarDiLepton"
TOP_NAMESPACE.ELLIPSEMETHOD_TOPOLOGY = "TtbarDiLepton"
TOP_NAMESPACE.NEUTRINOWEIGHTER_TOPOLOGY = "TtbarDiLepton"
TOP_NAMESPACE.NUFLOWS_TOPOLOGY = "TtbarDiLepton"
TOP_NAMESPACE.SLHAD_CHI2_TOPOLOGY = "TtbarLJets"


def _resolve_reco_partons_prefix(topology, reco_partons_prefix):
    producer_label = reco_partons_prefix.strip()
    if not producer_label:
        raise ValueError(
            "outputName must not be empty when publishing reconstructed parton kinematics."
        )
    if producer_label.startswith(topology + "_"):
        return producer_label
    return topology + "_" + producer_label
