/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#include "G4CosmicFilter/G4CosmicFilter.h"

#include "MCTruth/AtlasG4EventUserInfo.h"
#include "TrackRecord/TrackRecordCollection.h"

#include "G4RunManager.hh"
#include "G4Event.hh"

#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IMessageSvc.h"

namespace G4UA
{

  G4CosmicFilter::G4CosmicFilter(const Config& config)
    : AthMessaging(Gaudi::svcLocator()->service< IMessageSvc >( "MessageSvc" ), "G4CosmicFilter"),
      m_config(config), m_report()
  {
  }

  void G4CosmicFilter::EndOfEventAction(const G4Event* event)
  {
    m_report.ntot++;
    auto* coll = [&] () -> TrackRecordCollection* {
      auto* eventInfo = static_cast<AtlasG4EventUserInfo*>( event->GetUserInformation());
      return eventInfo ?
        eventInfo->GetHitCollectionMap()->Find<TrackRecordCollection>(m_config.collectionName) :
        nullptr;
    }();
    if (!coll)
      {
        ATH_MSG_WARNING( "Cannot retrieve TrackRecordCollection " << m_config.collectionName );
        G4RunManager::GetRunManager()->AbortEvent();
        return;
      }

    int counter = coll->size();

    if (m_config.PDGId!=0 || m_config.ptMin>0 || m_config.ptMax>0)
      {
        counter=0;
        for (const auto& a_tr : *coll)
          {
            if (m_config.PDGId!=0 && m_config.PDGId != std::abs(a_tr.GetPDGCode())) continue;
            if (m_config.ptMin>0 && m_config.ptMin > a_tr.GetMomentum().perp() ) continue;
            if (m_config.ptMax>0 && m_config.ptMax < a_tr.GetMomentum().perp() ) continue;
            counter++;
          }
      }

    //std::cout << "EndOfEventAction counter is "<<counter<<std::endl;
    if (counter==0)
      {
        ATH_MSG_INFO("aborting event due to failing filter");
        G4RunManager::GetRunManager()->AbortEvent();
        return;
      }

    m_report.npass++;
    return;

  }

} // namespace G4UA
