
"""Define methods to construct configured MDT overlay algorithms

Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def MDT_OverlayAlgCfg(flags, name="MdtOverlay", **kwargs):
    """Return a ComponentAccumulator for MDTOverlay algorithm"""
    acc = ComponentAccumulator()

    kwargs.setdefault("BkgInputKey", f"{flags.Overlay.BkgPrefix}MDT_DIGITS")
    kwargs.setdefault("SignalInputKey", f"{flags.Overlay.SigPrefix}MDT_DIGITS")
    kwargs.setdefault("OutputKey", "MDT_DIGITS")

    kwargs.setdefault("IntegrationWindow", 20) # in ns

    # Do MDT overlay
    acc.addEventAlgo(CompFactory.MdtOverlay(name, **kwargs))

    # Setup output
    if flags.Output.doWriteRDO:
        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
        acc.merge(OutputStreamCfg(flags, "RDO", ItemList=[
            "MdtCsmContainer#MDTCSM"
        ]))

    if flags.Output.doWriteRDO_SGNL:
        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
        acc.merge(OutputStreamCfg(flags, "RDO_SGNL", ItemList=[
            f"MdtCsmContainer#{flags.Overlay.SigPrefix}MDTCSM"
        ]))

    return acc


def MDT_TruthOverlayCfg(flags, name="MdtTruthOverlay", **kwargs):
    """Return a ComponentAccumulator for the MDT SDO overlay algorithm"""
    acc = ComponentAccumulator()

    # We do not need background MDT SDOs
    if not flags.Input.isMC:
        kwargs.setdefault("BkgInputKey", "")
    else:
        kwargs.setdefault("BkgInputKey", f"{flags.Overlay.BkgPrefix}MDT_SDO")

    if kwargs["BkgInputKey"]:
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        acc.merge(SGInputLoaderCfg(flags, [f'MuonSimDataCollection#{kwargs["BkgInputKey"]}']))

    kwargs.setdefault("SignalInputKey", f"{flags.Overlay.SigPrefix}MDT_SDO")
    kwargs.setdefault("OutputKey", "MDT_SDO")

    # Do MDT truth overlay
    acc.addEventAlgo(CompFactory.MuonSimDataOverlay(name, **kwargs))

    # Setup output
    if flags.Output.doWriteRDO:
        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
        acc.merge(OutputStreamCfg(flags, "RDO", ItemList=[
            "MuonSimDataCollection#MDT_SDO"
        ]))

    if flags.Output.doWriteRDO_SGNL:
        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
        acc.merge(OutputStreamCfg(flags, "RDO_SGNL", ItemList=[
            f"MuonSimDataCollection#{flags.Overlay.SigPrefix}MDT_SDO"
        ]))

    return acc

def MDT_OverlayCfg(flags):
    """Configure and return a ComponentAccumulator for MDT overlay"""
    acc = ComponentAccumulator()

    # Add MDT RDO to digit config
    from MuonConfig.MuonByteStreamCnvTestConfig import MdtRdoToMdtDigitCfg
    acc.merge(MdtRdoToMdtDigitCfg(flags))

    # Add MDT overlay digitization algorithm
    from MuonConfig.MDT_DigitizationConfig import MDT_OverlayDigitizationBasicCfg
    acc.merge(MDT_OverlayDigitizationBasicCfg(flags))

    # Add MDT overlay algorithm
    acc.merge(MDT_OverlayAlgCfg(flags))

    # Add MDT truth overlay
    if flags.Digitization.EnableTruth:
        if flags.Muon.usePhaseIIGeoSetup:
            from MuonTruthOverlayR4.TruthOverlayConfig import TruthOverlayCfg
            acc.merge(TruthOverlayCfg(flags, name="MdtTruthOverlay", WriteKey="MDT_SDO", deadTime = 700.))
        else:
            acc.merge(MDT_TruthOverlayCfg(flags))

    # Add MDT digit to RDO config
    from MuonConfig.MuonByteStreamCnvTestConfig import MdtDigitToMdtRDOCfg
    acc.merge(MdtDigitToMdtRDOCfg(flags))

    return acc
