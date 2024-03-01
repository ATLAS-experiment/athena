# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def VertexTimeAlgCfg(flags, name='VertexTimeAlg', **kwargs):
    acc = ComponentAccumulator()

    acc.addEventAlgo(CompFactory.HGTD.VertexTimeAlg(name, **kwargs))
    return acc
