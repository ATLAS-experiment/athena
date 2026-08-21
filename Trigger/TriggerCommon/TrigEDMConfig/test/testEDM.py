#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
from CLIDComps.clidGenerator import clidGenerator
log = logging.getLogger('testEDM')

from TrigEDMConfig.TriggerEDM import (TriggerL2List, TriggerEFList, TriggerResultsList, TriggerResultsRun1List,
                        TriggerLvl1List, TriggerHLTList, _addExtraCollectionsToEDMList, isCLIDDefined)
from TrigEDMConfig.TriggerEDMRun2 import EDMDetails
from TrigEDMConfig.TriggerEDMDefs import InViews, allowTruncation
from TrigEDMConfig import TriggerEDMRun1

def main():
  import re
  serializable_names = []
  serializable_names_no_label = []
  cgen = clidGenerator("", False)
  TriggerList = TriggerL2List + TriggerEFList + TriggerResultsList + TriggerResultsRun1List + TriggerLvl1List + TriggerHLTList
  TriggerList += TriggerEDMRun1.TriggerL2List + TriggerEDMRun1.TriggerEFList + TriggerEDMRun1.TriggerResultsRun1List
  for TriggerSerializable in TriggerList:
    #print TriggerSerializable 
    serializable_name = TriggerSerializable[0]
    serializable_name_no_label = re.sub(r"\#.*", "", serializable_name)
    if '#' not in serializable_name:
      log.error("ERROR, no label for " + serializable_name)
      return 1
    #Check container can be assigned a CLID
    if not isCLIDDefined(cgen, serializable_name_no_label):
      log.error("no CLID for " + serializable_name)
      return 1
    if serializable_name_no_label not in EDMDetails:
      log.error(serializable_name_no_label + " does not correspond to any name in EDMDetails")

    
    #check for Aux "."
    if "Aux" in serializable_name and "Aux." not in serializable_name:
      log.error("no final Aux. in label for " + serializable_name)
    
    file_types = TriggerSerializable[1].split(" ")

    allowed_file_types = ("", "BS", "DS", "ESD", "AODFULL", "AODSLIM", "AODVERYSLIM", "AODBLSSLIM", "AODCONV")

    for file_type in file_types:
      if file_type not in allowed_file_types:
        log.error("unknown file type " + file_type + " for " + serializable_name)
        return 1

    serializable_names.append(serializable_name)
    serializable_names_no_label.append(serializable_name_no_label)

  #check EDMDetails
  for EDMDetail in EDMDetails:
    if EDMDetail not in serializable_names_no_label:
      log.warning("EDMDetail for " + EDMDetail + " does not correspond to any name in TriggerList")

  # Check EDM flag manipulation
  dummyEDM = [
    ('xAOD::A#HLT_A',                        'BS ESD', 'Steer', [InViews('SomeView1')]),
    ('xAOD::AAuxContainer#HLT_AAux.',        'BS ESD', 'Steer'),
    ('xAOD::B#HLT_B',                        'BS ESD', 'Steer'),
    ('xAOD::BAuxContainer#HLT_BAux.DecOne.', 'BS ESD', 'Steer'),
    ('xAOD::B#HLT_C',                        'BS ESD', 'Steer'),
    ('xAOD::BAuxContainer#HLT_CAux.',        'BS ESD', 'Steer'),
    ('xAOD::E#HLT_E',                        'BS ESD', 'Steer', [allowTruncation]),
    ('xAOD::EAuxContainer#HLT_EAux.',        'BS ESD', 'Steer', [allowTruncation]),
  ]

  updateList = [
    # Add new
    ('xAOD::D#HLT_D',                                'BS ESD AODFULL', 'Steer', [InViews('SomeView2')]),
    ('xAOD::DAuxContainer#HLT_DAux.',                'BS ESD AODFULL', 'Steer'),
    # Add decorations & target
    ('xAOD::AAuxContainer#HLT_AAux.DecTwo.DecThree', 'BS ESD', 'Steer'),
    ('xAOD::B#HLT_B',                                'BS ESD AODFULL', 'Steer'),
    ('xAOD::BAuxContainer#HLT_BAux.DecFour',         'BS ESD AODFULL', 'Steer'),
    # Duplicate - leave untouched
    ('xAOD::C#HLT_C',                                'BS ESD', 'Steer'),
    ('xAOD::CAuxContainer#HLT_CAux.',                'BS ESD', 'Steer'),
  ]

  _addExtraCollectionsToEDMList(dummyEDM, updateList) # Note: Function updates dummyEDM in-place

  expectedEDM = [
    ('xAOD::A#HLT_A',                                'BS ESD',         'Steer', [InViews('SomeView1')]),
    ('xAOD::AAuxContainer#HLT_AAux.DecTwo.DecThree', 'BS ESD',         'Steer'),
    ('xAOD::B#HLT_B',                                'BS ESD AODFULL', 'Steer'),
    ('xAOD::BAuxContainer#HLT_BAux.DecOne.DecFour',  'BS ESD AODFULL', 'Steer'),
    ('xAOD::C#HLT_C',                                'BS ESD', 'Steer'),
    ('xAOD::CAuxContainer#HLT_CAux.',                'BS ESD', 'Steer'),
    ('xAOD::D#HLT_D',                                'BS ESD AODFULL', 'Steer', [InViews('SomeView2')]),
    ('xAOD::DAuxContainer#HLT_DAux.',                'BS ESD AODFULL', 'Steer'),
    ('xAOD::E#HLT_E',                        'BS ESD', 'Steer', [allowTruncation]),
    ('xAOD::EAuxContainer#HLT_EAux.',        'BS ESD', 'Steer', [allowTruncation]),
  ]

  if (dummyEDM != expectedEDM):
    log.error("There is a problem in _addExtraCollectionsToEDMList, expecting:")
    log.error(expectedEDM)
    log.error("Got:")
    log.error(dummyEDM)

if __name__ == "__main__":
  import sys
  sys.exit(main())
