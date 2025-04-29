#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ITkPixelTranslatorAlgCfg(flags, name = "ITkPixelTranslatorAlg", **kwargs):
    acc = ComponentAccumulator()

    # Required for PixelID
    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc.merge(ITkPixelReadoutGeometryCfg(flags))


    acc.addEventAlgo(CompFactory.ITkPixelTranslatorAlg(name, **kwargs))

    return acc
