/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/


////////////////////////////////////////////////////////////////
//                                                            //
//  Implementation of class TrackCollHandle_TrackContainer     //
//                                                            //
//  Author: Thomas H. Kittelmann (Thomas.Kittelmann@cern.ch)  //
//  Initial version: May 2008                                 //
//                                                            //
////////////////////////////////////////////////////////////////

#include "VP1TrackSystems/TrackCollHandle_TrackContainer.h"
#include "VP1TrackSystems/TrackHandle_TrackContainer.h"

//#include "TrackProxy/TrackParticleContainer.h"
#include "xAODTracking/TrackParticleContainer.h"

#include "VP1Base/IVP1System.h"
#include "VP1Utils/VP1SGAccessHelper.h"
#include "VP1Utils/VP1SGContentsHelper.h"
#include <QStringList>

//____________________________________________________________________
QStringList TrackCollHandle_TrackContainer::availableCollections( IVP1System*sys )
{
  return VP1SGContentsHelper(sys).getKeys<ActsTrk::TrackContainer>();
}

//____________________________________________________________________
class TrackCollHandle_TrackContainer::Imp {
public:
  TrackCollHandle_TrackContainer * theclass;
  int updateGUICounter;
  void possiblyUpdateGUI() {//Fixme: to TrackCollHandleBase
    if (!((updateGUICounter++)%50)) {
      theclass->systemBase()->updateGUI();
    }
  }
};


//____________________________________________________________________
TrackCollHandle_TrackContainer::TrackCollHandle_TrackContainer(TrackSysCommonData * cd,
							     const QString& name)
  : TrackCollHandleBase(cd,name,TrackType::ACTS), m_d(new Imp)
{
  m_d->theclass = this;
  m_d->updateGUICounter = 0;
}

//____________________________________________________________________
TrackCollHandle_TrackContainer::~TrackCollHandle_TrackContainer()
{
  delete m_d;
}

//____________________________________________________________________
bool TrackCollHandle_TrackContainer::load()
{
  //Get collection:
  const ActsTrk::TrackContainer * coll(nullptr);
  if (!VP1SGAccessHelper(systemBase()).retrieve(coll, name())) {
    message("Error: Could not retrieve track particle collection with key="+name());
    return false;
  }

  //Make appropriate trk::track handles:
  hintNumberOfTracksInEvent(coll->size());
  for (auto it : *coll) {
    m_d->possiblyUpdateGUI();
    // if (!*it) {
    //   messageDebug("WARNING: Ignoring null TrackProxy pointer.");
    //   continue;
    // }
    // if ((*it)->charge()==0.0) {
    //   messageDebug("WARNING: Ignoring TrackProxy which claims to be neutral (charge()==0.0).");
    //   continue;
    // }
    addTrackHandle(new TrackHandle_TrackContainer(this,it, *coll));
  }

  return true;
}

//____________________________________________________________________
bool TrackCollHandle_TrackContainer::cut(TrackHandleBase* handle)
{
  if (!TrackCollHandleBase::cut(handle))
    return false;

  //Fixme: more?

  return true;
}
