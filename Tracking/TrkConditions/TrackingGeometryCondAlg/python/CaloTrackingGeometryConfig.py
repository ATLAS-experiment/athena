# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# Configuration of LArTrackingGeometry + TileTrackingGeometry + CaloTrackingGeometry packages

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def LArVolumeBuilderCfg(flags, name='LArVolumeBuilder', **kwargs):
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    result = LArGMCfg(flags)
    kwargs.setdefault("BarrelEnvelopeCover", 5.0)
    kwargs.setdefault("EndcapEnvelopeCover", 5.0)

    result.setPrivateTools(CompFactory.LAr.LArVolumeBuilder(name, **kwargs))
    return result

def TileVolumeBuilderCfg(flags, name='TileVolumeBuilder', **kwargs):
    from TileGeoModel.TileGMConfig import TileGMCfg
    result = TileGMCfg(flags)

    result.setPrivateTools(CompFactory.Tile.TileVolumeBuilder(name, **kwargs))
    return result


def CaloTrackingGeometryBuilderCfg(flags, name='CaloTrackingGeometryBuilder',
                                   useCond = True,
                                   **kwargs):
    result = ComponentAccumulator()

    # Subtools are renamed to avoid conflict in CA wrapper with legacy config
    # Renaming can be removed when support of legacy config is dropped

    nameSuffix = 'Cond' if useCond else ''

    lArVolumeBuilder = result.popToolsAndMerge(
        LArVolumeBuilderCfg(flags, name = 'LArVolumeBuilder' + nameSuffix))
    result.addPublicTool(lArVolumeBuilder)
    
    tileVolumeBuilder = result.popToolsAndMerge(
        TileVolumeBuilderCfg(flags, name = 'TileVolumeBuilder' + nameSuffix))
    result.addPublicTool(tileVolumeBuilder)
    
    from SubDetectorEnvelopes.SubDetectorEnvelopesConfig import (
        EnvelopeDefSvcCfg)
    envelopeDefinitionSvc = result.getPrimaryAndMerge(EnvelopeDefSvcCfg(flags))
    
    kwargs.setdefault("LArVolumeBuilder", lArVolumeBuilder)
    kwargs.setdefault("TileVolumeBuilder", tileVolumeBuilder)
    kwargs.setdefault("EnvelopeDefinitionSvc", envelopeDefinitionSvc)
    kwargs.setdefault("EntryVolumeName", "InDet::Containers::EntryVolume")
    kwargs.setdefault("ExitVolumeName", "Calo::Container")
    kwargs.setdefault("GapLayerEnvelope", 5.0)
    
    name = name + nameSuffix
    geometryBuilder = CompFactory.Calo.CaloTrackingGeometryBuilderCond(name, **kwargs) if useCond else \
                      CompFactory.Calo.CaloTrackingGeometryBuilder(name, **kwargs)
    result.setPrivateTools(geometryBuilder)
    return result
