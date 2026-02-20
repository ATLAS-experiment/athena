/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#include "G4CosmicFilter/G4CosmicOrFilter.h"
#include "MCTruth/AtlasG4EventUserInfo.h"
#include "TrackRecord/TrackRecordCollection.h"
#include "G4RunManager.hh"
#include "G4Event.hh"

#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IMessageSvc.h"

namespace G4UA
{

  G4CosmicOrFilter::G4CosmicOrFilter(const Config& config)
    : AthMessaging(Gaudi::svcLocator()->service< IMessageSvc >("MessageSvc"), "G4CosmicOrFilter"),
      m_config(config), m_report()
  {
  }

  void G4CosmicOrFilter::EndOfEventAction(const G4Event* event)
  {
    int counterOne(0), counterTwo(0), counterThree(0);
    //need way to get "and" or "or" in
    m_report.ntot++;

    auto find_coll = [&] (const std::string& name) -> TrackRecordCollection* {
      auto* eventInfo = static_cast<AtlasG4EventUserInfo*>( event->GetUserInformation());
      return eventInfo ?
        eventInfo->GetHitCollectionMap()->Find<TrackRecordCollection>(name) :
        nullptr;
    };

    auto* coll = find_coll(m_config.collectionName);

    if (!coll) {
      ATH_MSG_WARNING( "Cannot retrieve TrackRecordCollection " );
    }
    else {
      counterOne = coll->size();
    }

    auto* coll2 = find_coll(m_config.collectionName2);

    if (!coll2) {
      ATH_MSG_WARNING( "Cannot retrieve TrackRecordCollection " );
    }
    else {
      counterTwo = coll2->size();
    }

    auto* coll3 = find_coll(m_config.collectionName3);

    if (!coll3) {
      ATH_MSG_WARNING( "Cannot retrieve TrackRecordCollection" );
    }
    else {
      counterThree = coll3->size();
    }

    if (counterOne==0 && counterTwo==0 && counterThree==0) {
      ATH_MSG_INFO("aborting event due to failing OR filter");
      G4RunManager::GetRunManager()->AbortEvent();
    }
    else {
      m_report.npass++;
    }
  }

} // namespace G4UA
