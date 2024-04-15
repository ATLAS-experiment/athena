# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def VP1AlgsEventProdCfg(flags, StreamESD, **kwargs):

    acc = ComponentAccumulator()

    if "OnlineEventDisplaysSvc" not in kwargs:
        from EventDisplaysOnline.EventDisplaysOnlineConfig import OnlineEventDisplaysSvcCfg
        acc.merge(OnlineEventDisplaysSvcCfg(flags))
        kwargs.setdefault("OnlineEventDisplaysSvc", acc.getService("OnlineEventDisplaysSvc"))

    kwargs.setdefault("InputPoolFile", StreamESD.OutputFile)
    kwargs.setdefault("IsOnline", True)
    kwargs.setdefault("MaxNumberOfFiles", -1)
    vp1Alg = CompFactory.VP1EventProd(name="VP1AlgsEventProd", **kwargs)
    acc.addEventAlgo(vp1Alg, primary=True)

    return acc
