#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Get logger
from AthenaCommon.Logging import logging
evgenLog = logging.getLogger('EvgenConfig')

class EvgenConfig():
    """The CA-based EvgenConfig class that holds the configuration for a sample to be generated"""

    __slots__ = ()
 
    def __init__(self, flags):
        self.keywords = []
        self.contact = []
        self.nEventsPerJob = None

    def setupFlags(self, flags):
        raise RuntimeError("setupFlags method needs to be implemented in Sample(EvgenConfig)")
        
    def setupProcess(self, flags):
        raise RuntimeError("setupProcess method needs to be implemented in Sample(EvgenConfig)")
