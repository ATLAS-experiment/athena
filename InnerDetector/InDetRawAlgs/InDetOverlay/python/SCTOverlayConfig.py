"""Define methods to construct configured SCT overlay algorithms

Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def SCTOverlayAlgCfg(flags, name="SCTOverlay", **kwargs):
    """Return a ComponentAccumulator for SCTOverlay algorithm"""
    acc = ComponentAccumulator()

    kwargs.setdefault("BkgInputKey", f"{flags.Overlay.BkgPrefix}SCT_RDOs")
    kwargs.setdefault("SignalInputKey", f"{flags.Overlay.SigPrefix}SCT_RDOs")
    kwargs.setdefault("OutputKey", "SCT_RDOs")

    # Input setup
    if flags.Overlay.ByteStream:
        from SCT_RawDataByteStreamCnv.SCT_RawDataByteStreamCnvConfig import SCTRawDataProviderCfg, SCTEventFlagWriterCfg
        acc.merge(SCTRawDataProviderCfg(flags))
        acc.merge(SCTEventFlagWriterCfg(flags))
    else:
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        acc.merge(SGInputLoaderCfg(flags, [f'SCT_RDO_Container#{kwargs["BkgInputKey"]}']))

    # Do SCT overlay
    acc.addEventAlgo(CompFactory.SCTOverlay(name, **kwargs))

    # Setup output
    if flags.Output.doWriteRDO:
        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
        acc.merge(OutputStreamCfg(flags, "RDO", ItemList=[
            "SCT_RDO_Container#SCT_RDOs"
        ]))

        if not flags.Input.isMC:
            acc.merge(OutputStreamCfg(flags, "RDO", ItemList=[
                "IDCInDetBSErrContainer#SCT_ByteStreamErrs"
            ]))

    if flags.Output.doWriteRDO_SGNL:
        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
        acc.merge(OutputStreamCfg(flags, "RDO_SGNL", ItemList=[
            f"SCT_RDO_Container#{flags.Overlay.SigPrefix}SCT_RDOs"
        ]))

    # for track overlay, write out the signal RDOs because reco tracking will only run on them
    if flags.Overlay.doTrackOverlay:
        acc.merge(OutputStreamCfg(flags, "RDO", ItemList=[
            f"SCT_RDO_Container#{flags.Overlay.SigPrefix}SCT_RDOs"
        ]))

    return acc


def SCTTruthOverlayCfg(flags, name="SCTSDOOverlay", **kwargs):
    """Return a ComponentAccumulator for the SCT SDO overlay algorithm"""
    acc = ComponentAccumulator()

    # We do not need background SCT SDOs
    kwargs.setdefault("BkgInputKey", "")

    kwargs.setdefault("SignalInputKey", f"{flags.Overlay.SigPrefix}SCT_SDO_Map")
    kwargs.setdefault("OutputKey", "SCT_SDO_Map")

    # Do SCT truth overlay
    acc.addEventAlgo(CompFactory.InDetSDOOverlay(name, **kwargs))

    # Setup output
    if flags.Output.doWriteRDO:
        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
        acc.merge(OutputStreamCfg(flags, "RDO", ItemList=[
            "InDetSimDataCollection#SCT_SDO_Map"
        ]))

    if flags.Output.doWriteRDO_SGNL:
        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
        acc.merge(OutputStreamCfg(flags, "RDO_SGNL", ItemList=[
            f"InDetSimDataCollection#{flags.Overlay.SigPrefix}SCT_SDO_Map"
        ]))

    return acc


def SCTOverlayCfg(flags):
    """Configure and return a ComponentAccumulator for SCT overlay"""
    acc = ComponentAccumulator()

    # Add SCT overlay digitization algorithm
    from SCT_Digitization.SCT_DigitizationConfig import SCT_OverlayDigitizationBasicCfg
    acc.merge(SCT_OverlayDigitizationBasicCfg(flags))

    # Add SCT overlay algorithm
    acc.merge(SCTOverlayAlgCfg(flags))

    # Add SCT truth overlay
    if flags.Digitization.EnableTruth:
        acc.merge(SCTTruthOverlayCfg(flags))

    return acc
