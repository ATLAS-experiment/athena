/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloEvent/CaloRecoStatus.h"

bool CaloRecoStatus::getStatus(std::vector<CaloRecoStatus::StatusIndicator>&
			       pStatusList) const
{
  const size_t oldSize = pStatusList.size();
  for (const auto& s : {TAGGEDEM, TAGGEDHAD, TAGGEDMUON, TAGGEDUNKNOWN, CALIBRATED, CALIBRATEDLHC,CALIBRATEDALT, UNKNOWNSTATUS}) {
    if ((s & m_status) == s) pStatusList.push_back(s);
  }
  return oldSize < pStatusList.size();
}
