#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from ITkStripCabling.ITkStripCablingConfig import ITkStripCablingToolCfg
from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg

def ITkStripsRodDecoderCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    acc.merge(ITkStripReadoutGeometryCfg(flags))
    kwargs.setdefault("ITkStripCablingTool", acc.popToolsAndMerge(ITkStripCablingToolCfg(flags)))
    acc.setPrivateTools(CompFactory.ITkStripsRodDecoder(name="ITkStripsRodDecoder",**kwargs))
    return acc

def ITkStripRawDataProviderToolCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("Decoder", acc.popToolsAndMerge(ITkStripsRodDecoderCfg(flags)))
    acc.setPrivateTools(CompFactory.ITkStripRawDataProviderTool(name="ITkStripRawDataProviderTool",**kwargs))
    return acc

def ITkStripRawDataProviderCfg(flags, **kwargs):
    """ Configures the main algorithm for ITk raw data decoding """
    acc = ComponentAccumulator()    
    kwargs.setdefault("ProviderTool", acc.popToolsAndMerge(ITkStripRawDataProviderToolCfg(flags)))

    if flags.Overlay.ByteStream:
        kwargs.setdefault("RDOKey", f"{flags.Overlay.BkgPrefix}ITkStripRDOs")
        kwargs.setdefault("LVL1IDKey", f"{flags.Overlay.BkgPrefix}ITkStripLVL1ID")
        kwargs.setdefault("BCIDKey", f"{flags.Overlay.BkgPrefix}ITkStripBCID")

    acc.addEventAlgo(CompFactory.ITkStripRawDataProvider(name="ITkStripRawDataProvider",**kwargs))
    return acc

def ITkStripsEventFlagWriterCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    if flags.Overlay.ByteStream:
        kwargs.setdefault("xAODEventInfoKey", f"{flags.Overlay.BkgPrefix}EventInfo")
    acc.addEventAlgo(CompFactory.SCTEventFlagWriter(name="ITkStripsEventFlagWriter", **kwargs))

    return acc

