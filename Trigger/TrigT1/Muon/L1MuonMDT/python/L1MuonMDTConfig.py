#Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration


from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Logging import logging

_log = logging.getLogger(__name__)


def L1MuonMDTSimCfg(flags, name="L1MuonMDTSim", timeWindowLower=30., timeWindowUpper=100., **kwargs):
    """
    Config for L1Muon::MDTSimulation.

    - Books THistSvc (stream 'EXPERT').
    - MinWindow95 histograms are created/registered directly in C++ finalize()
      via THistSvc.
    - Hit extraction time window (user mode of the MDT calibration tool), on the drift time
      t = tdc * 0.78125 ns - t0 - TOF: tLower(rt) - timeWindowLower <= t <= tUpper(rt) + timeWindowUpper [ns]
    """
    acc = ComponentAccumulator()

    # --- Hit extraction time window: drift time = tdc - t0 - TOF, from the MDT calibration tool ---
    # No signal propagation correction: the position along the tube is unknown to the trigger
    if "CalibrationTool" not in kwargs:
        from MuonConfig.MuonCalibrationConfig import MdtCalibrationToolCfg
        from MuonConfig.MuonRIO_OnTrackCreatorToolConfig import MdtCalibWindowNumber
        kwargs["CalibrationTool"] = acc.popToolsAndMerge(MdtCalibrationToolCfg(flags,
                                                                               name="L1MuonMdtCalibrationTool",
                                                                               DoPropagationCorrection=False,
                                                                               TimeWindowSetting=MdtCalibWindowNumber('User'),
                                                                               TimeWindowLowerBound=timeWindowLower,
                                                                               TimeWindowUpperBound=timeWindowUpper))

    # --- Algorithm ---
    alg = CompFactory.L1Muon.MDTSimulation(name=name, **kwargs)



    acc.addEventAlgo(alg, primary=True)
    return acc
