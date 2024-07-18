/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

////////////////////////////////////////////////////////////////

//                                                            //
//  Header file for class TrackCollHandle_TrackParticle       //
//                                                            //
//  Description: Collection handle for track particles        //
//                                                            //
//                                                            //
////////////////////////////////////////////////////////////////

#ifndef TRACKCOLLHANDLE_TRACKCONTAINER_H
#define TRACKCOLLHANDLE_TRACKCONTAINER_H

#include "VP1TrackSystems/TrackCollHandleBase.h"

class TrackCollHandle_TrackContainer : public TrackCollHandleBase {

  Q_OBJECT

public:

  static QStringList availableCollections(IVP1System*);//For the collection widget.

  TrackCollHandle_TrackContainer( TrackSysCommonData *,
         const QString& name );
  virtual ~TrackCollHandle_TrackContainer();

protected:
  virtual bool load();
  virtual bool cut(TrackHandleBase*);
  virtual QColor defaultColor() const { return QColor::fromRgbF(1.0f, 1.0f, 0.5f); }
  bool mightHaveSubSystemHitInfo() const { return true; }

private:

  class Imp;
  Imp * m_d;

};

#endif
