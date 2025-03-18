# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from TrigmuRoI.TrigmuRoIMonitoring import TrigmuRoIMonitoring

def TrigmuRoIConfig(flags, name="TrigmuRoI", outputRoIs="RoIsOut"):
    acc = ComponentAccumulator()
    alg = CompFactory.TrigmuRoI(name,
                                MonTool = TrigmuRoIMonitoring(flags),
                                RoisWriteHandleKey=outputRoIs)
    acc.addEventAlgo(alg)
    return acc
