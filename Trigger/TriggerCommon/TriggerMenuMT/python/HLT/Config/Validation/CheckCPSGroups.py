# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

"""
Validation of Coherent Prescale Groups
- Python check that all Coherent Prescale group assignments are reasonable
- Allows early failure in the case of invalid groups, such as:
  - Only one chain in CPS group
  - Multiple L1 inputs for same CPS group
  - CPS group not matching L1 name

Author: Teng Jian Khoo
"""

__doc__="Validation of coherent prescales set on the list of chains"

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

def checkCPSGroups(chainDicts):

  cps_to_chain = {}
  for hlt in chainDicts:
    # Extract all CPS items
    for group in hlt["groups"]:
      if 'CPS' in group:
        # Add if not existing
        if group not in cps_to_chain:
          cps_to_chain[group] = []
        cps_to_chain[group].append(hlt)
  
  CPS_OK = True
  for CPS_group, HLT_list in cps_to_chain.items():
    
    if not CPS_group.startswith('RATE:CPS_'):
      log.error("Malformed CPS group %s", CPS_group)
      CPS_OK = False
      continue
    CPS_item = CPS_group.split('CPS_',1)[1]

    # Checks for each CPS item
    if len(HLT_list) < 2:
      log.error("CPS group %s contains too few chains %s", CPS_group, [hlt['chainName'] for hlt in HLT_list])
      CPS_OK = False

    # Checks for every chain
    for hlt in HLT_list:
      for grp in hlt['groups']:
        if 'Primary' in grp:
          log.error("Primary trigger '%s' in CPS %s", hlt['chainName'], CPS_group)
          CPS_OK = False

      # Verify L1 item matches CPS group
      if ',' in hlt['L1item']:
        l1item = hlt['chainName'].rsplit('_L1')[1]
        if CPS_item != l1item:
          log.error("CPS group %s for HLT chain %s does not match mult-seed L1 item %s", CPS_group, hlt['chainName'], l1item)
          CPS_OK = False
          # Checks CPS for multi-seed items
      else:
        l1item = hlt['L1item'][3:]
        if CPS_item != l1item:
          log.error("CPS group %s for HLT chain %s does not match L1 item %s", CPS_group, hlt['chainName'], l1item)
          CPS_OK = False
          # Passing this check also implies that there is exactly 1 L1 for the CPS group

  if not CPS_OK:
    raise Exception("Invalid CPS group assignments found")
