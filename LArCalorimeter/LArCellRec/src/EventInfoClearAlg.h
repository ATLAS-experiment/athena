/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARCELLREC_EVENTINFOCELARALG_H
#define LARCELLREC_EVENTINFOCELARALG_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "xAODEventInfo/EventInfo.h"
#include <atomic>
#include "CxxUtils/checker_macros.h"
#include "xAODEventInfo/EventInfoLockHelper.h"

//Ad-hoc xAODFix algorithm, intended to cear the EventInfo::ErrorState for run 456721, LB 731 - End of run 
//One Front End boad failed, producing data-corruption errors. One failing FEB is considered a tolerable defect
//Now, this one FEB is properly marked as "deadReadout" but the veto-periods have not been cleared from 
//the EventVeto DB folder. Therefore the Event-Info ERROR-bit is (incorrectly) set  
//This algo is not thread-safe, it relies on const_cast to modify the xAOD::EventInfo object
//Therefore, it needs to be scheduled in the AthBeginSeq 


class AthenaAttributeList;

class  ATLAS_NOT_THREAD_SAFE EventInfoClearAlg : public AthAlgorithm {
  public:
  //Delegate constructor:
  using AthAlgorithm::AthAlgorithm; 
    
    virtual StatusCode          initialize() override;
    virtual StatusCode          execute (const EventContext& ctx) override;
    virtual StatusCode          finalize() override;

  private:
    mutable std::atomic<unsigned> m_nevt{0};
    mutable std::atomic<unsigned> m_nevtCleard{0};

    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this,"EventInfoKey","EventInfo"};

    EventInfoLockHelper::unlockFunc_t m_runIf=[](uint32_t runNbr, uint32_t lbNbr,uint64_t)->bool {return (runNbr==456729 && lbNbr>=732);};
};
#endif
