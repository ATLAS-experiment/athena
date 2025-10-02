"""ComponentAccumulator Geant4 tools config for ISF

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod, ProductionStep
from ISF_Services.ISF_ServicesCoreConfig import GeoIDSvcCfg, ATLFAST_GeoIDSvcCfg
from ISF_Tools.ISF_ToolsConfig import EntryLayerFilterCfg


def EntryLayerToolCfg(flags, name="ISF_EntryLayerTool", **kwargs):
    result = ComponentAccumulator()
    kwargs["GeoIDSvc"] = result.getPrimaryAndMerge(GeoIDSvcCfg(flags))

    if not flags.Sim.RecordStepInfo:
        # No filtering for FCS Parameterization input samples
        kwargs.setdefault("ParticleFilters", [result.addPublicTool(result.popToolsAndMerge(EntryLayerFilterCfg(flags)))])

    if flags.GeoModel.Run < LHCPeriod.Run4:
        kwargs.setdefault("CaloEntryVolumeString", "IDET::IDET")
    else:
        kwargs.setdefault("CaloEntryVolumeString", "ITK::ITK")

    if flags.Common.ProductionStep == ProductionStep.FastChain:
        if flags.Digitization.PileUp:
            OEsvc = CompFactory.StoreGateSvc("OriginalEvent_SG")
            result.addService(OEsvc)
            kwargs.setdefault("EvtStore", OEsvc.name)

    result.setPrivateTools(CompFactory.ISF.EntryLayerTool(name, **kwargs))
    return result


def EntryLayerToolMTCfg(flags, name="ISF_EntryLayerToolMT", **kwargs):
    result = ComponentAccumulator()
    kwargs["GeoIDSvc"] = result.getPrimaryAndMerge(GeoIDSvcCfg(flags))

    if not flags.Sim.RecordStepInfo:
        # No filtering for FCS Parameterization input samples
        filt = result.popToolsAndMerge(EntryLayerFilterCfg(flags))
        kwargs.setdefault("ParticleFilters", [filt])

    if flags.GeoModel.Run < LHCPeriod.Run4:
        kwargs.setdefault("CaloEntryVolumeString", "IDET::IDET")
    else:
        kwargs.setdefault("CaloEntryVolumeString", "ITK::ITK")

    result.setPrivateTools(CompFactory.ISF.EntryLayerToolMT(name, **kwargs))
    return result


def ATLFAST_EntryLayerToolCfg(flags, name="ISF_ATLFAST_EntryLayerTool", **kwargs):
    result = ComponentAccumulator()
    kwargs["GeoIDSvc"] = result.getPrimaryAndMerge(ATLFAST_GeoIDSvcCfg(flags))
    kwargs.setdefault("ParticleFilters", [result.addPublicTool(result.popToolsAndMerge(EntryLayerFilterCfg(flags)))])

    if flags.GeoModel.Run < LHCPeriod.Run4:
        kwargs.setdefault("CaloEntryVolumeString", "IDET::IDET")
    else:
        kwargs.setdefault("CaloEntryVolumeString", "ITK::ITK")

    if flags.Common.ProductionStep == ProductionStep.FastChain:
        if flags.Digitization.PileUp:
            OEsvc = CompFactory.StoreGateSvc("OriginalEvent_SG")
            result.addService(OEsvc)
            kwargs.setdefault("EvtStore", OEsvc.name)

    result.setPrivateTools(CompFactory.ISF.EntryLayerTool(name, **kwargs))
    return result


def ATLFAST_EntryLayerToolMTCfg(flags, name="ISF_ATLFAST_EntryLayerToolMT", **kwargs):
    result = ComponentAccumulator()
    kwargs["GeoIDSvc"] = result.getPrimaryAndMerge(ATLFAST_GeoIDSvcCfg(flags))

    filt = result.popToolsAndMerge(EntryLayerFilterCfg(flags))
    kwargs.setdefault("ParticleFilters", [filt])

    if flags.GeoModel.Run < LHCPeriod.Run4:
        kwargs.setdefault("CaloEntryVolumeString", "IDET::IDET")
    else:
        kwargs.setdefault("CaloEntryVolumeString", "ITK::ITK")

    result.setPrivateTools(CompFactory.ISF.EntryLayerToolMT(name, **kwargs))
    return result
