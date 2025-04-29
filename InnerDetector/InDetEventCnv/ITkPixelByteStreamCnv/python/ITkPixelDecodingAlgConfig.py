#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ITkPixelDecodingAlgCfg(flags, name = "ITkPixelDecodingAlg", **kwargs):
    acc = ComponentAccumulator()

    # Required for PixelID
    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc.merge(ITkPixelReadoutGeometryCfg(flags))

    acc.addEventAlgo(CompFactory.ITkPixelDecodingAlg(name, **kwargs))

    return acc
