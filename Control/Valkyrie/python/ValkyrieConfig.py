# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def ValgrindServiceCfg(flags, **kwargs):
    """Configure ValgrindSvc profiler"""

    kwargs.setdefault("ProfiledAlgs", flags.PerfMon.Valgrind.ProfiledAlgs)
    from AthenaCommon.Constants import VERBOSE
    kwargs.setdefault("OutputLevel", VERBOSE)

    acc = ComponentAccumulator()
    acc.addService(CompFactory.ValgrindSvc(**kwargs), create=True)
    acc.addService(CompFactory.AuditorSvc(), create=True)
    acc.setAppProperty("AuditAlgorithms", True)
    acc.setAppProperty("AuditTools", True)
    acc.setAppProperty("AuditServices", True)
    return acc
