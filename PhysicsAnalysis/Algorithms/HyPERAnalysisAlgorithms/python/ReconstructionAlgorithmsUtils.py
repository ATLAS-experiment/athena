# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Vendored from TopCPToolkit/python/ReconstructionAlgorithmsUtils.py so that this
# package stays standalone. Only the helper needed by HyPERConfig is kept here;
# once TopCPToolkit depends on this package, the shared version should be used.


def _resolve_reco_partons_prefix(topology, reco_partons_prefix):
    producer_label = reco_partons_prefix.strip()
    if not producer_label:
        raise ValueError(
            "outputName must not be empty when publishing reconstructed parton kinematics."
        )
    if producer_label.startswith(topology + "_"):
        return producer_label
    return topology + "_" + producer_label
