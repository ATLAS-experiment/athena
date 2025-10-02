#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

def ITkPixelCablingAlgCfg(flags, name = 'ITkPixelCablingAlg', **kwargs):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.ComponentFactory import CompFactory
    acc = ComponentAccumulator()
    acc.addEventAlgo(CompFactory.ITkPixelCablingAlg(name, **kwargs))
    return acc