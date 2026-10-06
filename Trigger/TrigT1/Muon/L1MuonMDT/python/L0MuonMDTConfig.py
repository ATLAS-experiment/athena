#Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration


from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Logging import logging

_log = logging.getLogger(__name__)


def L0MuonMDTSimCfg(flags, name="L0MuonMDTSim", **kwargs):
    """
    Config for L1Muon::MDTSimulation.

    - Books THistSvc (stream 'EXPERT').
    - MinWindow95 histograms are created/registered directly in C++ finalize()
      via THistSvc.
    """
    acc = ComponentAccumulator()


    # --- Algorithm ---
    alg = CompFactory.L1Muon.MDTSimulation(name=name, **kwargs)



    acc.addEventAlgo(alg, primary=True)
    return acc
