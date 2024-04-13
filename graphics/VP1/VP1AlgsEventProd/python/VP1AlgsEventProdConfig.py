# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def VP1AlgsEventProdCfg(flags, StreamESD, OnlineEventDisplaysSvc = None):

    acc = ComponentAccumulator()
    
    vp1Alg = CompFactory.VP1EventProd(name="VP1AlgsEventProd",
                                      InputPoolFile = StreamESD.OutputFile,
                                      IsOnline = True,
                                      MaxNumberOfFiles = -1,
                                      OnlineEventDisplaysSvc = OnlineEventDisplaysSvc)
    acc.addEventAlgo(vp1Alg, primary=True)

    return acc
