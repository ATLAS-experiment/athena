# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def McEventSelectorCfg(flags, **kwargs):
    cfg = ComponentAccumulator()

    service = CompFactory.McCnvSvc()
    cfg.addService(service)
    cfg.addService(CompFactory.EvtPersistencySvc("EventPersistencySvc",
                                                 CnvServices=[service.getFullJobOptName()]))

    if flags.Input.RunNumbers:
        kwargs.setdefault("RunNumber", flags.Input.RunNumbers[0])
    if flags.Input.TimeStamps:
        kwargs.setdefault("InitialTimeStamp", flags.Input.TimeStamps[0])

    if flags.Exec.FirstEvent <= 0:
        raise ValueError("FirstEvent must be a positive, non-zero integer")

    kwargs.setdefault("FirstEvent", flags.Exec.FirstEvent)

    # we use 64-bit numbers by default now
    kwargs.setdefault("EventsPerRun", int(2**63 - 1))  # sys.maxint on a 64-bit machine

    evSel = CompFactory.McEventSelector("EventSelector", **kwargs)
    cfg.addService(evSel)
    cfg.setAppProperty("EvtSel", evSel.getFullJobOptName())

    return cfg
