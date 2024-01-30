/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////
//                                                         //
//  Header file for class VP1TrackingGeometryChannel       //
//                                                         //
//  Author: Andreas.Salzburger@cern.ch (primary)           //
//          Thomas.Kittelmann@cern.ch                      //
//                                                         //
//  Initial version: June 2007                             //
//                                                         //
/////////////////////////////////////////////////////////////

#ifndef VP1TRACKINGGEOMETRYCHANNEL_H
#define VP1TRACKINGGEOMETRYCHANNEL_H

#include "VP1Base/IVP13DStandardChannelWidget.h"

class VP1TrackingGeometryChannel : public IVP13DStandardChannelWidget {

  Q_OBJECT

public:

  VP1TrackingGeometryChannel();
  void init();
  virtual ~VP1TrackingGeometryChannel(){}

};

#endif

