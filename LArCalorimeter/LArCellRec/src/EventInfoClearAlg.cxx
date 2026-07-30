/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "EventInfoClearAlg.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "LArRecEvent/LArEventBitInfo.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODEventInfo/EventInfoLockHelper.h"

//__________________________________________________________________________
StatusCode EventInfoClearAlg::initialize() 
  {
    ATH_MSG_INFO("EventInfoClearAlg initialize()"  );
    ATH_CHECK( m_eventInfoKey.initialize() );
    //EventInfoLockHelper::unlockFunc_t fn=[](uint32_t runNbr, uint32_t lbNbr,uint64_t)->bool {return (runNbr==456729 && lbNbr>=732);};
    EventInfoLockHelper::addUnlockFunc(m_runIf);
    return StatusCode::SUCCESS; 

  }

//__________________________________________________________________________
StatusCode EventInfoClearAlg::finalize()
  {
    ATH_MSG_DEBUG( "EventInfoClearAlg finalize()"  );
    ATH_MSG_INFO( "Number of events processed " << m_nevt << ". Cleared EventInfo error state for " << m_nevtCleard << " events"  );
    return StatusCode::SUCCESS; 
  }
  
//__________________________________________________________________________
StatusCode EventInfoClearAlg::execute( const EventContext& ctx )
{
  m_nevt++;
  SG::ReadHandle<xAOD::EventInfo> eventInfo (m_eventInfoKey, ctx); 

  if (m_runIf(eventInfo->runNumber(),eventInfo->lumiBlock(),eventInfo->eventNumber())) {
    //m_runIf expands to unNbr==r456729 && lbNbr>=732
    if (eventInfo->errorState(xAOD::EventInfo::LAr)==xAOD::EventInfo::Error) {
      if (eventInfo->isEventFlagBitSet(xAOD::EventInfo::LAr, LArEventBitInfo::DATACORRUPTEDVETO)) {
        //ERROR bit set and LAr Data Corruption Event Veto bit set. Clear DATACORRUPTEDVETO bit
        xAOD::EventInfo* eventInfoNC=const_cast<xAOD::EventInfo*>(eventInfo.cptr()); 
        eventInfoNC->resetEventFlagBit(xAOD::EventInfo::LAr, LArEventBitInfo::DATACORRUPTEDVETO);
        if (eventInfoNC->eventFlags(xAOD::EventInfo::LAr)==0) {
          //No other LAr Flag set (like noise-burst), clear error state:
          eventInfoNC->setErrorState(xAOD::EventInfo::LAr, xAOD::EventInfo::NotSet);
          m_nevtCleard++;
          ATH_MSG_DEBUG("Cleared EventInfo Error state for run/LB/event " << eventInfo->runNumber() << "/" << eventInfo->lumiBlock() << "/" << eventInfo->eventNumber());
        }
        else {
          ATH_MSG_INFO("Not cleared EventInfo Error state for run/LB/event " << eventInfo->runNumber() << "/" << eventInfo->lumiBlock() << "/" << eventInfo->eventNumber() 
            << ",LArFlags still at " << std::hex <<eventInfoNC->eventFlags(xAOD::EventInfo::LAr));
        }
      }
    }
  }
  return StatusCode::SUCCESS;
}
