"""Define methods to construct configured EventInfo conversion algorithms

Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod, ProductionStep


def EventInfoCnvAlgCfg(flags, name="EventInfoCnvAlg",
                       inputKey="McEventInfo",
                       outputKey="EventInfo",
                       disableBeamSpot=False,
                       **kwargs):
    """Return a ComponentAccumulator for EventInfoCnvAlg algorithm"""

    acc = ComponentAccumulator()

    kwargs.setdefault("AODKey", inputKey)
    kwargs.setdefault("xAODKey", outputKey)

    if f"PileUpEventInfo#{inputKey}" in flags.Input.TypedCollections:
        kwargs.setdefault("PileupKey", f"Pileup{outputKey}")
    else:
        kwargs.setdefault("PileupKey", "")

    # TODO: luminosity

    if not disableBeamSpot:
        from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
        acc.merge(BeamSpotCondAlgCfg(flags))

    acc.addEventAlgo(CompFactory.xAODMaker.EventInfoCnvAlg(name, **kwargs))

    return acc


def EventInfoOverlayAlgCfg(flags, name="EventInfoOverlay", **kwargs):
    """Return a ComponentAccumulator for EventInfoOverlay algorithm"""
    acc = ComponentAccumulator()

    # Add beam spot conditions
    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    acc.merge(BeamSpotCondAlgCfg(flags))

    kwargs.setdefault("BkgInputKey", f"{flags.Overlay.BkgPrefix}EventInfo")
    kwargs.setdefault("SignalInputKey", f"{flags.Overlay.SigPrefix}EventInfo")
    kwargs.setdefault("OutputKey", "EventInfo")

    kwargs.setdefault("DataOverlay", flags.Overlay.DataOverlay)
    kwargs.setdefault("ValidateBeamSpot", not (flags.Common.ProductionStep == ProductionStep.FastChain and flags.Common.isOverlay) and not flags.Overlay.DataOverlay and flags.GeoModel.Run is LHCPeriod.Run3)

    if flags.Input.MCChannelNumber > 0:
        kwargs.setdefault("MCChannelNumber", flags.Input.MCChannelNumber)

    # Do the xAOD::EventInfo overlay
    acc.addEventAlgo(CompFactory.xAODMaker.EventInfoOverlay(name, **kwargs))

    # Add output
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    if flags.Output.doWriteRDO:
        acc.merge(OutputStreamCfg(flags, "RDO"))

    # Add signal output
    if flags.Output.doWriteRDO_SGNL:
        acc.merge(OutputStreamCfg(flags, "RDO_SGNL", ItemList=[
            f"xAOD::EventInfo#{flags.Overlay.SigPrefix}EventInfo",
            f"xAOD::EventAuxInfo#{flags.Overlay.SigPrefix}EventInfoAux."
        ]))

    return acc


def EventInfoOverlayCfg(flags, **kwargs):
    """Return a ComponentAccumulator for the full EventInfoOverlay algorithm accumulator"""

    acc = ComponentAccumulator()
    inputs = [f"xAOD::EventInfo#{flags.Overlay.BkgPrefix}EventInfo"]
    # Check if running on legacy HITS
    if "EventInfo" not in flags.Input.Collections and "EventInfo" not in flags.Input.SecondaryCollections:
        acc.merge(EventInfoCnvAlgCfg(flags,
                                     inputKey=f"{flags.Overlay.SigPrefix}McEventInfo",
                                     outputKey=f"{flags.Overlay.SigPrefix}EventInfo",
                                     **kwargs))
        # Re-map signal address
        from SGComps.AddressRemappingConfig import AddressRemappingCfg
        acc.merge(AddressRemappingCfg([
            f"EventInfo#McEventInfo->{flags.Overlay.SigPrefix}McEventInfo",
        ]))

        inputs.append(f"EventInfo#{flags.Overlay.SigPrefix}McEventInfo")
    else:
        # Re-map signal address
        from SGComps.AddressRemappingConfig import AddressRemappingCfg
        acc.merge(AddressRemappingCfg([
            f"xAOD::EventInfo#EventInfo->{flags.Overlay.SigPrefix}EventInfo",
            f"xAOD::EventAuxInfo#EventInfoAux.->{flags.Overlay.SigPrefix}EventInfoAux.",
        ]))

        inputs.append(f"xAOD::EventInfo#{flags.Overlay.SigPrefix}EventInfo")

    from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
    acc.merge(SGInputLoaderCfg(flags, inputs))

    acc.merge(EventInfoOverlayAlgCfg(flags, **kwargs))
    return acc


def EventInfoUpdateFromContextAlgCfg(flags, name="EventInfoUpdateFromContextAlg", disableBeamSpot=False, **kwargs):
    """Return a ComponentAccumulator for EventInfoUpdateFromContext algorithm"""
    acc = ComponentAccumulator()

    # Add beam spot conditions
    if not disableBeamSpot:
        from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
        acc.merge(BeamSpotCondAlgCfg(flags))

    kwargs.setdefault("SignalInputKey", "Input_EventInfo")
    kwargs.setdefault("OutputKey", f"{flags.Overlay.SigPrefix}EventInfo" if flags.Common.ProductionStep == ProductionStep.FastChain and flags.Common.isOverlay else "EventInfo")

    if flags.Input.MCChannelNumber > 0:
        kwargs.setdefault("MCChannelNumber", flags.Input.MCChannelNumber)

    if disableBeamSpot:
        kwargs.setdefault("BeamSpotKey", "")

    # Do the xAOD::EventInfo overlay
    acc.addEventAlgo(CompFactory.xAODMaker.EventInfoUpdateFromContextAlg(name, **kwargs))

    # Re-map signal address
    from SGComps.AddressRemappingConfig import AddressRemappingCfg
    acc.merge(AddressRemappingCfg([
        "xAOD::EventInfo#EventInfo->" + "Input_EventInfo",
        "xAOD::EventAuxInfo#EventInfoAux.->" + "Input_EventInfoAux.",
    ]))

    return acc


def EventInfoBeamSpotDecoratorAlgCfg(flags, name="EventInfoBeamSpotDecoratorAlg", eventInfoKey="EventInfo", **kwargs):
    result = ComponentAccumulator()

    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    result.merge(BeamSpotCondAlgCfg(flags))

    kwargs.setdefault("EventInfoKey", eventInfoKey)

    result.addEventAlgo(CompFactory.xAODMaker.EventInfoBeamSpotDecoratorAlg(name, **kwargs))

    return result
