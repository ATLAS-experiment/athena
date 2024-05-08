#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ITkPixelEncodingAlgCfg(flags, name = "ITkPixelEncodingAlg", **kwargs):
    acc = ComponentAccumulator()

    # Required for PixelID
    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc.merge(ITkPixelReadoutGeometryCfg(flags))
    
    acc.addEventAlgo(CompFactory.ITkPixelEncodingAlg(name, **kwargs))

    return acc
