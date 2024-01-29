# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# ********************* Common Definitons **********************
# For import into the Run3 and Run4 menu files
# **************************************************************

from AthenaCommon.Logging import logging
log = logging.getLogger('TriggerEDMDefs')

from collections import UserString

class Alias(UserString):
    """EDM property to alias collection name"""
    pass

class InViews(UserString):
    """EDM property to configure collections available in a given view"""
    def __init__(self, value):
        if ',' in value:
            raise ValueError(f'Invalid view name: {value}')
        super().__init__(value)

# EDM property to allow truncation for a collection
allowTruncation = object()