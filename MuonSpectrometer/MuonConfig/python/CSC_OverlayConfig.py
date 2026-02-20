
"""Define methods to construct configured CSC overlay algorithms

Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def CSC_OverlayAlgCfg(flags, name="CscOverlay", **kwargs):
    """Return a ComponentAccumulator for CSCOverlay algorithm"""
    acc = ComponentAccumulator()

    kwargs.setdefault("BkgInputKey", f"{flags.Overlay.BkgPrefix}CSCRDO")
    kwargs.setdefault("SignalInputKey", f"{flags.Overlay.SigPrefix}CSCRDO")
    kwargs.setdefault("OutputKey", "CSCRDO")

    kwargs.setdefault("isDataOverlay", not flags.Input.isMC)

    from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
    acc.merge(SGInputLoaderCfg(flags, [f'CscRawDataContainer#{kwargs["BkgInputKey"]}']))

    from MuonConfig.MuonCalibrationConfig import CscCalibToolCfg
    kwargs.setdefault("CalibTool", acc.popToolsAndMerge(CscCalibToolCfg(flags)))

    from MuonConfig.MuonCSC_CnvToolsConfig import MuonCscRDODecoderCfg
    kwargs.setdefault("CscRdoDecoderTool", acc.popToolsAndMerge(MuonCscRDODecoderCfg(flags)))

    # Do CSC overlay
    acc.addEventAlgo(CompFactory.CscOverlay(name, **kwargs))

    # Setup output
    if flags.Output.doWriteRDO:
        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
        acc.merge(OutputStreamCfg(flags, "RDO", ItemList=[
            "CscRawDataContainer#CSCRDO"
        ]))

    if flags.Output.doWriteRDO_SGNL:
        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
        acc.merge(OutputStreamCfg(flags, "RDO_SGNL", ItemList=[
            f"CscRawDataContainer#{flags.Overlay.SigPrefix}CSCRDO"
        ]))

    return acc


def CSC_TruthOverlayCfg(flags, name="CscTruthOverlay", **kwargs):
    """Return a ComponentAccumulator for the CSC SDO overlay algorithm"""
    acc = ComponentAccumulator()

    # We do not need background CSC SDOs
    if not flags.Input.isMC:
        kwargs.setdefault("BkgInputKey", "")
    else:
        kwargs.setdefault("BkgInputKey", f"{flags.Overlay.BkgPrefix}CSC_SDO")

    if kwargs["BkgInputKey"]:
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        acc.merge(SGInputLoaderCfg(flags, [f'CscSimDataCollection#{kwargs["BkgInputKey"]}']))

    kwargs.setdefault("SignalInputKey", f"{flags.Overlay.SigPrefix}CSC_SDO")
    kwargs.setdefault("OutputKey", "CSC_SDO")

    # Do CSC truth overlay
    acc.addEventAlgo(CompFactory.CscSimDataOverlay(name, **kwargs))

    # Setup output
    if flags.Output.doWriteRDO:
        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
        acc.merge(OutputStreamCfg(flags, "RDO", ItemList=[
            "CscSimDataCollection#CSC_SDO"
        ]))

    if flags.Output.doWriteRDO_SGNL:
        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
        acc.merge(OutputStreamCfg(flags, "RDO_SGNL", ItemList=[
            f"CscSimDataCollection#{flags.Overlay.SigPrefix}CSC_SDO"
        ]))

    return acc


def CSC_OverlayCfg(flags):
    """Configure and return a ComponentAccumulator for CSC overlay"""
    acc = ComponentAccumulator()

    # Add CSC overlay digitization algorithm
    from MuonConfig.CSC_DigitizationConfig import CSC_OverlayDigitizationBasicCfg
    acc.merge(CSC_OverlayDigitizationBasicCfg(flags))

    # Add CSC digit to RDO config
    from MuonConfig.MuonByteStreamCnvTestConfig import CscDigitToCscRDOCfg
    acc.merge(CscDigitToCscRDOCfg(flags))

    # Add CSC overlay algorithm
    acc.merge(CSC_OverlayAlgCfg(flags))

    # Add CSC truth overlay
    if flags.Digitization.EnableTruth:
        acc.merge(CSC_TruthOverlayCfg(flags))

    return acc
