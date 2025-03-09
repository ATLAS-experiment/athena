/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MMLOADVARIABLES_H
#define MMLOADVARIABLES_H

#include "AthenaBaseComps/AthMessaging.h"
#include "AthenaKernel/getMessageSvc.h"
#include "AtlasHepMC/GenEvent.h"
#include "GeneratorObjects/McEventCollection.h"
#include "MuonDigitContainer/MmDigitContainer.h"
#include "MuonDigitContainer/MmDigit.h"
#include "MuonIdHelpers/MmIdHelper.h"
#include "StoreGate/StoreGateSvc.h"
#include "TrackRecord/TrackRecordCollection.h"
#include "TrigT1NSWSimTools/MMT_struct.h" //for digitWrapper, hitData_key, hitData_entry, evInf_entry
#include "Math/Vector4D.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include <map>
#include <vector>
#include <string>
#include <cmath>
#include <stdexcept>

namespace MuonGM {
  class MuonDetectorManager;
}

 class MMLoadVariables : public AthMessaging {

  public:
   MMLoadVariables(const MuonGM::MuonDetectorManager* detManager, const MmIdHelper* idhelper);

    StatusCode getMMDigitsInfo(const EventContext& ctx,
                               const McEventCollection *truthContainer,
                               const TrackRecordCollection* trackRecordCollection,
                               const MmDigitContainer *nsw_MmDigitContainer,
                               std::map<std::pair<int,unsigned int>,std::vector<digitWrapper> >& entries,
                               std::map<std::pair<int,unsigned int>,std::vector<hitData_entry> >& Hits_Data_Set_Time,
                               std::map<std::pair<int,unsigned int>,evInf_entry>& Event_Info) const;

  private:
    const MuonGM::MuonDetectorManager* m_detManager;        //!< MuonDetectorManager
    const MmIdHelper* m_MmIdHelper;        //!< MM offline Id helper
  };
#endif
