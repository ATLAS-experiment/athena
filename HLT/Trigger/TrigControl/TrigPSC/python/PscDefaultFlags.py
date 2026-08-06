#
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
'''Functions setting default flags for generating online HLT python configuration'''

_flags = None

def defaultOnlineFlags():
    """On first call will create ConfigFlags and return instance. This is only to be used within
    TrigPSC/TrigServices/athenaHLT as we cannot explicitly pass flags everywhere."""
    global _flags
    if _flags is None:
        from AthenaConfiguration.AllConfigFlags import initConfigFlags
        from TrigServices.TriggerUnixStandardSetup import setDefaultOnlineFlags
        _flags = initConfigFlags()
        setDefaultOnlineFlags(_flags)
    return _flags
