/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


////////////////////////////////////////////////////////////////
//                                                            //
//  Header file for class TrackHandle_TrackContainer              //
//                                                            //
//  Description: TrackHandle for Acts::TrackProxy             //
//                                                            //
////////////////////////////////////////////////////////////////

#ifndef TRACKHANDLE_TRACKCONTAINER_H
#define TRACKHANDLE_TRACKCONTAINER_H

#include "VP1TrackSystems/TrackHandleBase.h"

#include "GeoPrimitives/GeoPrimitives.h"
#include "ActsEvent/TrackContainer.h"
#include "VP1TrackSystems/TrkObjToString.h"

#include <memory>

namespace Trk {
  class Track;
  class MeasurementBase;
}

class TrackHandle_TrackContainer : public TrackHandleBase {
public:

  TrackHandle_TrackContainer(TrackCollHandleBase*, ActsTrk::TrackContainer::ConstTrackProxy, const ActsTrk::TrackContainer&);
  virtual ~TrackHandle_TrackContainer();

  virtual QStringList clicked() const;
  virtual bool isRun4EDM() const final { return true; }

  virtual Amg::Vector3D momentum() const;
  virtual const std::vector< Amg::Vector3D > * provide_pathInfoPoints();

  ActsTrk::TrackContainer::ConstTrackProxy track() const { return m_track; }
  const std::vector<ActsTrk::TrackContainer::ConstTrackStateProxy>& trackStates() const { return m_trackStates; }

  virtual bool containsDetElement(const QString&) const;

  virtual void fillObjectBrowser(QList<QTreeWidgetItem *>& list); 
  virtual void updateObjectBrowser();
  
  SoNode* zoomToTSOS(unsigned int index); //!< Depending on controller configuration attempt to zoom to the corresponding TSOS & returned detailed node matching it
  
  virtual QString shortInfo() const; //!< returns mom and hit information about track
  virtual QString type() const { return QString("Track"); } //!< return very short word with type (maybe link with collection type?)
  
private:
  void visibleStateChanged();
  void currentMaterialChanged();
  virtual void ensureInitTSOSs(std::unique_ptr<std::vector<AssociatedObjectHandleBase*>>& ascobjs); //!< Ensure that the TSOSs are initialized. This is called by update3DObjects() and should be called by any method that needs to access the TSOSs.
  void ensureInitTrackStateCache(); //!< Ensure that the track state cache is initialized. 
  void addTrackState(const typename ActsTrk::TrackContainer::ConstTrackStateProxy &state, std::vector<AssociatedObjectHandleBase*>* ascobjs, unsigned int index);
  TrkObjToString::MeasurementType measurementType(const ActsTrk::TrackStateBackend::ConstTrackStateProxy &state) const;
  QString measurementText(const ActsTrk::TrackContainer::ConstTrackStateProxy& state) const;
protected:
  ActsTrk::TrackContainer::ConstTrackProxy m_track;
  const ActsTrk::TrackContainer& m_container;
  std::vector<ActsTrk::TrackContainer::ConstTrackStateProxy> m_trackStates;

};

#endif
