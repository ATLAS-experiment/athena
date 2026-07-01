# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def RootReadCfg(flags, tupleName, **kw):
    """Creates a ComponentAccumulator instance containing the
    athena services required for ROOT file reading.
    """

    cfg = ComponentAccumulator()

    # Add EventSelector
    evSel = CompFactory.Athena.RootNtupleEventSelector("EventSelector",
                                                       InputCollections = flags.Input.Files,
                                                       TupleName = tupleName,
                                                       SkipEvents = flags.Exec.SkipEvents,
                                                       **kw)
    cfg.addService(evSel)
    cfg.addService( CompFactory.Athena.NtupleCnvSvc() )

    cfg.setAppProperty("EvtSel", evSel.getFullJobOptName())

    return cfg
