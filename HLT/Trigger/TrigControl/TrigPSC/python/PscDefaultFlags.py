#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
'''Functions setting default flags for generating online HLT python configuration'''

_flags = None


def setDefaultOnlineFlags(flags):
    from AthenaConfiguration.Enums import Format
    flags.Common.isOnline = True
    flags.Input.Files = []
    flags.Input.isMC = False
    flags.Input.Format = Format.BS
    flags.Trigger.doHLT = True  # This distinguishes the HLT setup from online reco (GM, EventDisplay)
    flags.Trigger.Online.isPartition = True  # athenaHLT and partition at P1
    flags.Trigger.EDMVersion = 3
    flags.Trigger.writeBS = True
    flags.Scheduler.CheckDependencies = True
    flags.Scheduler.ShowDataDeps = True
    flags.Scheduler.ShowControlFlow = True
    flags.Scheduler.ShowDataFlow = True
    flags.Scheduler.EnableVerboseViews = True
    flags.Scheduler.AutoLoadUnmetDependencies = False
    flags.Input.FailOnUnknownCollections = True


def defaultOnlineFlags():
    """On first call will create ConfigFlags and return instance. This is only to be used within
    TrigPSC/TrigServices/athenaHLT as we cannot explicitly pass flags everywhere."""
    global _flags
    if _flags is None:
        from AthenaConfiguration.AllConfigFlags import initConfigFlags
        _flags = initConfigFlags()
        setDefaultOnlineFlags(_flags)
    return _flags
