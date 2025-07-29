# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

"""
Validation of Monitoring groups
- Check thatall chains streamed in express have a signature or detector monGroup

Author: John Patrick Mc Gowan 
"""

__doc__="Validation of monGroups"

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

def checkMonGroups(chainDicts):

  
  MON_OK = True
  for hlt in chainDicts:
    if 'express' in hlt['stream']:
      if 'monGroups' not in hlt or len(hlt['monGroups']) < 1:
        log.error("Chain %s is streamed to express but does not have a signature or detector monGroup assigned", hlt['chainName'])
        MON_OK = False 

  if not MON_OK:
    raise Exception("Express chains found without monGroups")
