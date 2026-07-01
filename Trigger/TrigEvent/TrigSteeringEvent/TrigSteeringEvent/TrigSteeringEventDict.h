/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGSTEERINGEVENT_TRIGSTEERINGEVENTDICT_H
#define TRIGSTEERINGEVENT_TRIGSTEERINGEVENTDICT_H

#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"
#include "TrigSteeringEvent/HLTResult.h"
#include "TrigSteeringEvent/TrigSuperRoi.h"
#include "TrigSteeringEvent/TrigSuperRoiCollection.h"
#include "TrigSteeringEvent/HLTExtraData.h"
#include "TrigSteeringEvent/GenericResult.h"
#include "TrigSteeringEvent/Lvl1Result.h"
#include "TrigSteeringEvent/Lvl1Item.h"
#include "TrigSteeringEvent/Enums.h"
#include "TrigSteeringEvent/OnlineErrorCode.h"
#include "TrigSteeringEvent/TrigOperationalInfoCollection.h"
#include "TrigSteeringEvent/TrigPassBitsCollection.h"
#include "TrigSteeringEvent/TrigPassFlagsCollection.h"
#include "TrigSteeringEvent/StringSerializer.h"

#include "AthLinks/ElementLink.h"
#include <vector>

namespace TrigSteeringEventDict {

  TrigRoiDescriptorCollection troi;
  std::vector<TrigRoiDescriptor*> troi_vec;
  std::vector<const TrigRoiDescriptor*> troi_const_vec;
  ElementLink<TrigRoiDescriptorCollection> troi_el;

  TrigSuperRoiCollection tsroi;
  std::vector<TrigSuperRoi*> tsroi_vec;
  std::vector<const TrigSuperRoi*> tsroi_const_vec;

  TrigOperationalInfoCollection toi;

  std::vector<LVL1CTP::Lvl1Item*> l1_vec;
  std::vector<const LVL1CTP::Lvl1Item*> l1_const_vec;

  TrigPassBitsCollection tbits;
  std::vector<TrigPassBits*> tbits_vec;
  std::vector<const TrigPassBits*> tbits_const_vec;

  TrigPassFlagsCollection tflags;
  std::vector<TrigPassFlags*> tflags_vec;
  std::vector<const TrigPassFlags*> tflags_const_vec;

}

#endif
