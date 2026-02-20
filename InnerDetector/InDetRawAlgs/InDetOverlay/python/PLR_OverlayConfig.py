"""Define methods to construct configured PLR overlay algorithms

Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def PLR_OverlayAlgCfg(flags, name="PLR_Overlay", **kwargs):
    """Return a ComponentAccumulator for PLR Overlay algorithm"""
    acc = ComponentAccumulator()

    kwargs.setdefault("BkgInputKey", f"{flags.Overlay.BkgPrefix}PLR_RDOs")
    kwargs.setdefault("SignalInputKey", f"{flags.Overlay.SigPrefix}PLR_RDOs")
    kwargs.setdefault("OutputKey", "PLR_RDOs")

    # Input setup
    from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
    acc.merge(SGInputLoaderCfg(flags, [f'PixelRDO_Container#{kwargs["BkgInputKey"]}']))

    # Do PLR overlay
    acc.addEventAlgo(CompFactory.PixelOverlay(name, **kwargs))

    # Setup output
    if flags.Output.doWriteRDO:
        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
        acc.merge(OutputStreamCfg(flags, "RDO", ItemList=[
            "PixelRDO_Container#PLR_RDOs"
        ]))

        if not flags.Input.isMC:
            acc.merge(OutputStreamCfg(flags, "RDO", ItemList=[
                "IDCInDetBSErrContainer#PLR_ByteStreamErrs"
            ]))

    if flags.Output.doWriteRDO_SGNL:
        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
        acc.merge(OutputStreamCfg(flags, "RDO_SGNL", ItemList=[
            f"PixelRDO_Container#{flags.Overlay.SigPrefix}PLR_RDOs"
        ]))

    return acc


def PLR_TruthOverlayCfg(flags, name="PLR_SDOOverlay", **kwargs):
    """Return a ComponentAccumulator for the PLR SDO overlay algorithm"""
    acc = ComponentAccumulator()

    # We do not need background PLR SDOs
    kwargs.setdefault("BkgInputKey", "")

    kwargs.setdefault("SignalInputKey", f"{flags.Overlay.SigPrefix}PLR_SDO_Map")
    kwargs.setdefault("OutputKey", "PLR_SDO_Map")

    # Do PLR truth overlay
    acc.addEventAlgo(CompFactory.InDetSDOOverlay(name, **kwargs))

    # Setup output
    if flags.Output.doWriteRDO:
        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
        acc.merge(OutputStreamCfg(flags, "RDO", ItemList=[
            "InDetSimDataCollection#PLR_SDO_Map"
        ]))

    if flags.Output.doWriteRDO_SGNL:
        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
        acc.merge(OutputStreamCfg(flags, "RDO_SGNL", ItemList=[
            f"InDetSimDataCollection#{flags.Overlay.SigPrefix}PLR_SDO_Map"
        ]))

    return acc


def PLR_OverlayCfg(flags):
    """Configure and return a ComponentAccumulator for PLR overlay"""
    acc = ComponentAccumulator()

    # Add PLR overlay digitization algorithm
    from PixelDigitization.PLR_DigitizationConfig import PLR_OverlayDigitizationBasicCfg
    acc.merge(PLR_OverlayDigitizationBasicCfg(flags))

    # Add PLR overlay algorithm
    acc.merge(PLR_OverlayAlgCfg(flags))

    # Add PLR truth overlay
    if flags.Digitization.EnableTruth:
        acc.merge(PLR_TruthOverlayCfg(flags))

    return acc
