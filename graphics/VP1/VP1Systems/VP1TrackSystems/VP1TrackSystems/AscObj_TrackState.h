/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


////////////////////////////////////////////////////////////////
//                                                            //
//  Header file for class AscObj_TrackState                   //
//                                                            //
//  Description: Handle for ACTS TrackStates                  //
//                                                            //
//                                                            //
////////////////////////////////////////////////////////////////

#ifndef ASCOBJ_TRACKSTATE_H
#define ASCOBJ_TRACKSTATE_H

#include "VP1TrackSystems/AssociatedObjectHandleBase.h"
#include "VP1TrackSystems/TrkObjToString.h"

#include "GeoPrimitives/GeoPrimitives.h"

#include <QTreeWidgetItem>
#include "ActsEvent/TrackContainer.h"


class SoTranslation;

class AscObj_TrackState : public AssociatedObjectHandleBase {
public:

  AscObj_TrackState(TrackHandleBase *, unsigned indexOfPointOnTrack, const typename ActsTrk::TrackStateBackend::ConstTrackStateProxy &state);
  void setDistToNextPar(const double&);

  void buildShapes(SoSeparator*&shape_simple, SoSeparator*&shape_detailed);
  QStringList clicked();
  void zoomView(); //!< Depending on the controller settings, will attempt to zoom the view to this TSOS, for example in click()
  bool isShortMeasurement(); //!< Return true if TRT/MDT & shortened mode is on, false otherwise  
  
  virtual void setVisible(bool);
  
  virtual TrackCommonFlags::TSOSPartsFlags parts() const { return m_parts; }

  const Acts::Surface& surface() const { return m_trackstate.referenceSurface(); }
  // const Acts::ConstParameters& parameters() const { return m_trackstate.parameters(); }
  const ActsTrk::TrackStateBackend::ConstTrackStateProxy trackState() const { return m_trackstate; }
  Amg::Vector3D approxCenter() const;

  virtual bool initiatesOwnZooms() const { return true; }
  
  virtual QTreeWidgetItem* browserTreeItem() const {return m_objBrowseTree;}
  virtual void setBrowserTreeItem(QTreeWidgetItem* obt)  {m_objBrowseTree=obt;}

protected:
  int regionIndex() const;
  double lodCrossOverValue() const {return 1000; }

private:
  virtual ~AscObj_TrackState() {}//Private so it can only be deleted by TrackHandleBase
  TrackCommonFlags::TSOSPartsFlags m_parts;
  unsigned m_indexOfPointOnTrack;
  double m_distToNextPar;
  QTreeWidgetItem* m_objBrowseTree;
  const typename ActsTrk::TrackStateBackend::ConstTrackStateProxy m_trackstate;

  void addTrackParamInfoToShapes( SoSeparator*&shape_simple, SoSeparator*&shape_detailed,
          bool showPars, bool showParsErrors, bool showSurfaces );
  void addSurfaceToShapes( SoSeparator*&shape_simple, SoSeparator*&shape_detailed);
  void addMaterialToSurfaceShapes(SoNode*& shape_simple,  SoNode*& shape_detailed);
  void addPlaneSurfaceToShapes(SoSeparator*& shape_simple,
                                           SoSeparator*& shape_detailed, const Acts::Surface & surface);
  void addCylindricalSurfaceToShapes(SoSeparator*& shape_simple,
                                           SoSeparator*& shape_detailed, const Acts::Surface & surface);
  void addMaterialEffectsToShapes( SoSeparator*&shape_simple, SoSeparator*&shape_detailed);
  void addMeasurementToShapes(SoSeparator*&shape_simple, SoSeparator*&shape_detailed);
  static void ensureInitSeps( SoSeparator*&shape_simple, SoSeparator*&shape_detailed);
};

#endif
