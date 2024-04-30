#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def ITkPixelEncodingAlgCfg(flags, name = "ITkPixelEncodingAlg", **kwargs):
    acc = ComponentAccumulator()

    # from InDetEventCnv.ITkPixelRDOToolConfig import ITkPixelRDOToolCfg
    # kwargs.setdefault("clusteringTool", acc.popToolsAndMerge(ITkMergedPixelsToolCfg(flags)))
    # from PixelReadoutGeometry.PixelReadoutGeometryConfig import PixelReadoutManagerCfg
    # acc.merge(PixelReadoutManagerCfg(flags))



    # Seems this is needed for PixelID
    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc.merge(ITkPixelReadoutGeometryCfg(flags))

    # kwargs.setdefault("PixManagerLocation", 'Pixel')
    # from AtlasGeoModel.GeoModelConfig import GeoModelCfg
    # acc.merge (GeoModelCfg (flags))


    
    # result.merge(PixelReadoutManagerCfg(flags))

    
    acc.addEventAlgo(CompFactory.ITkPixelEncodingAlg(name, **kwargs))

    return acc
