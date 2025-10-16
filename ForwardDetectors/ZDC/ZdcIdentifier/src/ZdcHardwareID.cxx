/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ZdcIdentifier/ZdcHardwareID.h"
#include "Identifier/IdentifierHash.h"
#include "GaudiKernel/MsgStream.h"


ZdcHardwareID::ZdcHardwareID()
  : AtlasDetectorID("ZdcHardwareID", "")
{
}

ZdcHardwareID::~ZdcHardwareID()
{
}

int ZdcHardwareID::ppm(const HWIdentifier& /*hwid*/) 
{
  return 0;
}

int ZdcHardwareID::channel(const HWIdentifier& /*hwid*/) 
{
  return 0;
}

HWIdentifier ZdcHardwareID::ppm_id(int /*ppm*/) 
{
  return HWIdentifier(0);
}

HWIdentifier ZdcHardwareID::channel_id(int /*ppm*/, int /*channel*/) 
{
  return HWIdentifier(0);
}
