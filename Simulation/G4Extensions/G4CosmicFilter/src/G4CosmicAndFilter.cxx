/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#include "G4CosmicFilter/G4CosmicAndFilter.h"
#include "MCTruth/AtlasG4EventUserInfo.h"
#include "TrackRecord/TrackRecordCollection.h"
#include "G4RunManager.hh"
#include "G4Event.hh"

#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IMessageSvc.h"

namespace G4UA
{

  G4CosmicAndFilter::G4CosmicAndFilter(const Config& config)
    : AthMessaging(Gaudi::svcLocator()->service< IMessageSvc >( "MessageSvc" ), "G4CosmicAndFilter"),
      m_config(config), m_report()
  {
  }

  void G4CosmicAndFilter::EndOfEventAction(const G4Event* event)
  {

    m_report.ntot++;
    int counter(0);
    auto find_coll = [&] (const std::string& name) -> TrackRecordCollection* {
      auto* eventInfo = static_cast<AtlasG4EventUserInfo*>( event->GetUserInformation());
      return eventInfo ?
        eventInfo->GetHitCollectionMap()->Find<TrackRecordCollection>(name) :
        nullptr;
    };

    auto* coll = find_coll(m_config.collectionName);
    if (!coll)
      {
        ATH_MSG_WARNING( "Cannot retrieve TrackRecordCollection " << m_config.collectionName);
      }
    else
      {
        counter = coll->size();
      }

    if (counter==0)
      {
        ATH_MSG_INFO("aborting event due to failing AND filter");
        G4RunManager::GetRunManager()->AbortEvent();
        return;
      }

    auto* coll2 = find_coll(m_config.collectionName2);

    if (!coll2)
      {
        ATH_MSG_INFO( "Cannot retrieve TrackRecordCollection " << m_config.collectionName2 );
      }
    else
      {
        counter = coll2->size();
      }

    if (counter==0)
      {
        ATH_MSG_INFO("aborting event due to failing AND filter");
        G4RunManager::GetRunManager()->AbortEvent();
        return;
      }

    m_report.npass++;
    return;

  }

} // namespace G4UA
